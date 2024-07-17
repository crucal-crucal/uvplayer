#include "uvffplayer.hpp"

#include <QDebug>

#include "conf/uvconf.hpp"
#include "global/uvscope.hpp"

#define DEFAULT_BLOCK_TIMEOUT   10  // s

std::atomic_flag CUVFFPlayer::s_ffmpeg_init = ATOMIC_FLAG_INIT;

static void list_devices() {
	AVFormatContext* fmt_ctx = avformat_alloc_context();
	AVDictionary* options = nullptr;
	av_dict_set(&options, "list_devices", "true", 0);
	// 根据操作系统设置输入格式的驱动名称
#ifdef _WIN32
	constexpr char drive[] = "dshow"; // Windows 平台使用 DirectShow
#elif defined(__linux__)
    constexpr char drive[] = "v4l2"; // Linux 平台使用 Video4Linux2
#else
    constexpr char drive[] = "avfoundation"; // macOS 平台使用 AVFoundation
#endif

	if (const auto ifmt = av_find_input_format(drive)) {
		avformat_open_input(&fmt_ctx, "video=dummy", ifmt, &options);
	}
	avformat_close_input(&fmt_ctx);
	avformat_free_context(fmt_ctx);
	av_dict_free(&options);
}

// NOTE: avformat_open_input,av_read_frame block
static int interrupt_callback(void* opaque) {
	if (opaque == nullptr) return 0;
	if (const auto player = static_cast<CUVFFPlayer*>(opaque); player->quit || time(nullptr) - player->block_starttime > player->block_timeout) {
		qDebug() << "interrupt quit = " << player->quit << "media.src = " << player->media.src.c_str();
		return 1;
	}
	return 0;
}

CUVFFPlayer::CUVFFPlayer() : CUVVideoPlayer(), CUVThread() {
	fmt_opts = nullptr;
	codec_opts = nullptr;
	fmt_ctx = nullptr;
	video_codec_ctx = nullptr;
	video_packet = nullptr;
	video_frame = nullptr;
	sws_ctx = nullptr;

	block_starttime = time(nullptr);
	block_timeout = DEFAULT_BLOCK_TIMEOUT;
	quit = 0;

	if (!s_ffmpeg_init.test_and_set()) {
		avformat_network_init();
		avdevice_register_all();
		list_devices();
	} else {
		qCritical() << "CUVFFPlayer::CUVFFPlayer() ffmpeg already initialized";
	}
}

CUVFFPlayer::~CUVFFPlayer() {
	close();
}

int CUVFFPlayer::seek(const int64_t ms) {
	if (fmt_ctx) {
		clear_frame_cache();
		qDebug() << "seek => " << ms << " ms";
		return av_seek_frame(fmt_ctx, video_stream_index, (start_time + ms) / 1000 / (double) video_time_base_num * video_time_base_den, AVSEEK_FLAG_BACKWARD); // NOLINT
	}
	return 0;
}

bool CUVFFPlayer::doPrepare() {
	if (const int ret = open(); ret != 0) {
		if (!quit) {
			error = ret;
			event_callback(UVPLAYER_OPEN_FAILED);
		}
		return false;
	}
	event_callback(UVPLAYER_OPENED);
	return true;
}

void CUVFFPlayer::doTask() {
	// loop until get a video frame
	while (!quit) {
		av_init_packet(video_packet);

		fmt_ctx->interrupt_callback.callback = interrupt_callback; // 设置中断回调
		fmt_ctx->interrupt_callback.opaque = this;
		block_starttime = time(nullptr);

		int ret = av_read_frame(fmt_ctx, video_packet);
		fmt_ctx->interrupt_callback.callback = nullptr;
		if (ret != 0) {
			qDebug() << "video No frame: " << ret;
			if (!quit) {
				if (ret == AVERROR_EOF || avio_feof(fmt_ctx->pb)) {
					eof = 1;
					event_callback(UVPLAYER_EOF);
				} else {
					error = ret;
					event_callback(UVPLAYER_ERROR);
				}
				return;
			}
		}
		// NOTE: if not call av_packet_unref, memory leak.
		defer(
			av_packet_unref(video_packet);
		)

		if (video_packet->stream_index == video_stream_index) {
			ret = avcodec_send_packet(video_codec_ctx, video_packet);
			if (ret != 0) {
				qCritical() << "video avcodec_send_packet error: " << ret;
				return;
			}

			ret = avcodec_receive_frame(video_codec_ctx, video_frame);
			if (ret != 0) {
				if (ret != -EAGAIN) {
					qCritical() << "video avcodec_receive_frame error: " << ret;
					return;
				}
			} else {
				break;
			}
		}
	}

	if (sws_ctx) {
		if (const int h = sws_scale(sws_ctx, video_frame->data, video_frame->linesize, 0, video_frame->height, data, linesize); h <= 0 || h != video_frame->height) {
			return;
		}
	}

	if (video_time_base_num && video_time_base_den) {
		m_frame.ts = video_frame->pts / (double) video_time_base_den * video_time_base_num * 1000; // NOLINT
	}

	push_frame(&m_frame);
}

bool CUVFFPlayer::doFinish() {
	const int ret = close();
	event_callback(UVPLAYER_CLOSED);
	return !ret;
}

int CUVFFPlayer::open() {
	std::string ifile;

	AVInputFormat* ifmt{ nullptr };
	switch (media.type) {
		case MEDIA_TYPE_CAPTURE: {
			ifile = "video=";
			ifile += media.src;
#ifdef _WIN32
			constexpr char drive[] = "dshow";
#elif defined(__linux__)
            constexpr char drive[] = "v4l2";
#else
            constexpr char drive[] = "avfoundation";
#endif
#ifdef FFMPEG_VERSION_GTE_5_0_0
            ifmt = const_cast<AVInputFormat*>(av_find_input_format(drive));
#else
			ifmt = av_find_input_format(drive);
#endif
			if (!ifmt) {
				std::cerr << "Can not find dshow" << std::endl;
				return -5;
			}
		}
		break;
		case MEDIA_TYPE_FILE:
		case MEDIA_TYPE_NETWORK:
			ifile = media.src;
			break;
		default:
			return -10;
	}
	qDebug() << "source: " << QString::fromStdString(ifile);

	int ret = 0;
	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx) {
		qCritical() << "avformat_alloc_context error";
		ret = -10;
		return ret;
	}

	defer(
		if (ret != 0 && fmt_ctx) {
		avformat_free_context(fmt_ctx);
		fmt_ctx = nullptr;
		}
	)

	if (media.type == MEDIA_TYPE_NETWORK) {
		if (strncmp(media.src.c_str(), "rtsp:", 5) == 0) {
			const std::string str = g_confile->getValue("rtsp_transport", "video");
			if (strcmp(str.c_str(), "tcp") == 0 || strcmp(str.c_str(), "udp") == 0) {
				av_dict_set(&fmt_opts, "rtsp_transport", str.c_str(), 0);
			}
		}
		av_dict_set(&fmt_opts, "stimeout", "5000000", 0); // us
	}
	av_dict_set(&fmt_opts, "buffer_size", "2048000", 0);
	fmt_ctx->interrupt_callback.callback = interrupt_callback;
	fmt_ctx->interrupt_callback.opaque = this;
	block_starttime = time(nullptr);
	ret = avformat_open_input(&fmt_ctx, ifile.c_str(), ifmt, &fmt_opts);
	if (ret != 0) {
		std::cerr << "Open input file: " << ifile << " failed: " << ret << std::endl;
		return ret;
	}
	fmt_ctx->interrupt_callback.callback = nullptr;
	defer(
		if (ret != 0 && fmt_ctx) {
		avformat_close_input(&fmt_ctx);
		}
	)

	ret = avformat_find_stream_info(fmt_ctx, nullptr);
	if (ret != 0) {
		qCritical() << "Can not find stream: " << ret;
		return ret;
	}
	qDebug() << "stream_num: " << fmt_ctx->nb_streams;

	video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	subtitle_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_SUBTITLE, -1, -1, nullptr, 0);
	qDebug() << "video_stream_index = " << video_stream_index;
	qDebug() << "audio_stream_index = " << audio_stream_index;
	qDebug() << "subtitle_stream_index = " << subtitle_stream_index;

	// 初始化视频解码器
	if (video_stream_index >= 0) {
		AVStream* video_stream = fmt_ctx->streams[video_stream_index];
		video_time_base_num = video_stream->time_base.num;
		video_time_base_den = video_stream->time_base.den;
		qDebug() << "video_stream time_base = " << video_stream->time_base.num << " / " << video_stream->time_base.den;

		AVCodecParameters* codec_param = video_stream->codecpar;
		qDebug() << "codec_id = " << codec_param->codec_id << " => " << avcodec_get_name(codec_param->codec_id);

		AVCodec* codec{ nullptr };
		if (decode_mode != SOFTWARE_DECODE) {
try_hardware_decode:
			std::string decoder(avcodec_get_name(codec_param->codec_id));
			if (decode_mode == UVARDWARE_DECODE_CUVID) {
				decoder += "_cuvid";
				real_decode_mode = UVARDWARE_DECODE_CUVID;
			} else if (decode_mode == UVARDWARE_DECODE_QSV) {
				decoder += "_qsv";
				real_decode_mode = UVARDWARE_DECODE_QSV;
			}
#ifdef FFMPEG_VERSION_GTE_5_0_0
            codec = const_cast<AVCodec *>(avcodec_find_decoder_by_name(decoder.c_str()));
#else
			codec = avcodec_find_decoder_by_name(decoder.c_str());
#endif
			if (!codec) {
				qCritical() << "Can not find decoder " << QString::fromStdString(decoder);
			}
			qDebug() << "codec = " << QString::fromStdString(decoder);
		}

		if (!codec) {
try_software_decode:
#ifdef FFMPEG_VERSION_GTE_5_0_0
            codec = const_cast<AVCodec *>(avcodec_find_decoder(codec_param->codec_id));
#else
			codec = avcodec_find_decoder(codec_param->codec_id);
#endif
			if (!codec) {
				qCritical() << "Can not find decoder " << avcodec_get_name(codec_param->codec_id);
				ret = -30;
				return ret;
			} else {
				qInfo() << "Use software decoder " << avcodec_get_name(codec_param->codec_id);
			}
			real_decode_mode = SOFTWARE_DECODE;
		}

		qDebug() << "codec = " << codec->name << " => " << avcodec_get_name(codec_param->codec_id);

		video_codec_ctx = avcodec_alloc_context3(codec);
		if (!video_codec_ctx) {
			qCritical() << "avcodec_alloc_context3 error";
			ret = -40;
			return ret;
		}
		defer(
			if (ret != 0 && video_codec_ctx) {
			avcodec_free_context(&video_codec_ctx);
			video_codec_ctx = nullptr;
			}
		)

		ret = avcodec_parameters_to_context(video_codec_ctx, codec_param);
		if (ret != 0) {
			qCritical() << "avcodec_parameters_to_context error: " << ret;
			return ret;
		}

		if (video_codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || video_codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			av_dict_set(&codec_opts, "refcounted_frames", "1", 0);
		}

		ret = avcodec_open2(video_codec_ctx, codec, &codec_opts);
		if (ret != 0) {
			if (real_decode_mode != SOFTWARE_DECODE) {
				qWarning() << "Can not open hardware codec error: " << ret << ", try software codec.";
				goto try_software_decode;
			}
			qCritical() << "Can not open software codec error: " << ret;
			return ret;
		}
		video_stream->discard = AVDISCARD_DEFAULT;

		const int sw = video_codec_ctx->width;
		const int sh = video_codec_ctx->height;
		if (g_confile->get<bool>("use_source_aspect_ratio", "video")) {
			aspect_ratio_t aspect_ratio;
			aspect_ratio.type = ASPECT_ORIGINAL_RATIO;
			aspect_ratio.w = sw;
			aspect_ratio.h = sh;
			emit videoAspectRatio(aspect_ratio);
		}

		src_pix_fmt = video_codec_ctx->pix_fmt;

		qDebug() << "sw = " << sw << " sh = " << sh << " src_pix_fmt = " << src_pix_fmt << " : " << av_get_pix_fmt_name(src_pix_fmt);
		if (sw <= 0 || sh <= 0 || src_pix_fmt == AV_PIX_FMT_NONE) {
			qCritical() << "Codec parameters wrong!";
			ret = -45;
			return ret;
		}

		const int dw = sw >> 2 << 2; // align = 4
		const int dh = sh;
		dst_pix_fmt = AV_PIX_FMT_YUV420P;
		const std::string str = g_confile->getValue("dst_pix_fmt", "video");
		if (!str.empty()) {
			if (strcmp(str.c_str(), "YUV") == 0) {
				dst_pix_fmt = AV_PIX_FMT_YUV420P;
			} else if (strcmp(str.c_str(), "RGB") == 0) {
				dst_pix_fmt = AV_PIX_FMT_BGR24;
			}
		}
		qDebug() << "dw = " << dw << " dh = " << dh << " dst_pix_fmt = " << dst_pix_fmt << " : " << av_get_pix_fmt_name(dst_pix_fmt);

		sws_ctx = sws_getContext(sw, sh, src_pix_fmt, dw, dh, dst_pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
		if (!sws_ctx) {
			qCritical() << "sws_getContext failed";
			ret = -50;
			return ret;
		}

		video_packet = av_packet_alloc();
		video_frame = av_frame_alloc();

		m_frame.w = dw;
		m_frame.h = dh;
		// ARGB
		m_frame.buf.resize(dw * dh * 4);

		if (dst_pix_fmt == AV_PIX_FMT_YUV420P) {
			m_frame.type = PIX_FMT_IYUV;
			m_frame.bpp = 12;
			const int y_size = dw * dh;
			m_frame.buf.len = y_size * 3 / 2;
			data[0] = reinterpret_cast<uint8_t*>(m_frame.buf.base);
			data[1] = data[0] + y_size;
			data[2] = data[1] + y_size / 4;
			linesize[0] = dw;
			linesize[1] = linesize[2] = dw / 2;
		} else {
			dst_pix_fmt = AV_PIX_FMT_BGR24;
			m_frame.type = PIX_FMT_BGR;
			m_frame.bpp = 24;
			m_frame.buf.len = dw * dh * 3;
			data[0] = reinterpret_cast<uint8_t*>(m_frame.buf.base);
			linesize[0] = dw * 3;
		}

		// HVideoPlayer member vars
		if (video_stream->avg_frame_rate.num && video_stream->avg_frame_rate.den) {
			fps = video_stream->avg_frame_rate.num / video_stream->avg_frame_rate.den;
		}
		width = sw;
		height = sh;
		duration = 0;
		start_time = 0;
		eof = 0;
		error = 0;
		if (video_time_base_num && video_time_base_den) {
			if (video_stream->duration > 0) {
				duration = video_stream->duration / static_cast<double>(video_time_base_den) * video_time_base_num * 1000; // NOLINT
			} else if (fmt_ctx->duration > 0) {
				duration = fmt_ctx->duration / static_cast<int64_t>(AV_TIME_BASE) * 1000;
			} else {
				duration = 0;
			}

			if (video_stream->start_time > 0) {
				start_time = video_stream->start_time / static_cast<double>(video_time_base_den) * video_time_base_num * 1000; // NOLINT
			} else {
				start_time = 0;
			}
		}
		qDebug() << "fps = " << fps << " duration = " << duration << " start_time = " << start_time;
		qDebug() << "Video stream initialized.";
		CUVThread::setSleepPolicy(CUVThread::SLEEP_UNTIL, 1000 / fps);
	} else {
		qCritical() << "Can not find video stream.";
		ret = -20;
		return ret;
	}

#if 0
    // 初始化音频解码器
    if (audio_stream_index >= 0) {
        AVStream* audio_stream = fmt_ctx->streams[audio_stream_index];
        AVCodecParameters* audio_codec_param = audio_stream->codecpar;
        AVCodec* audio_codec = avcodec_find_decoder(audio_codec_param->codec_id);
        if (!audio_codec) {
            std::cerr << "Can not find audio codec." << std::endl;
            return -30;
        }

        audio_codec_ctx = avcodec_alloc_context3(audio_codec);
        if (!audio_codec_ctx) {
            std::cerr << "avcodec_alloc_context3 failed." << std::endl;
            ret = -40;
            return ret;
        }

        defer(
            if (ret != 0 && audio_codec_ctx) {
            avcodec_free_context(&audio_codec_ctx);
            audio_codec_ctx = nullptr;
            }
        )

        ret = avcodec_parameters_to_context(audio_codec_ctx, audio_codec_param);
        if (ret != 0) {
            std::cerr << "avcodec_parameters_to_context failed." << std::endl;
            return ret;
        }

        ret = avcodec_open2(audio_codec_ctx, audio_codec, nullptr);
        if (ret != 0) {
            std::cerr << "avcodec_open2 failed." << std::endl;
            return ret;
        }

        audio_stream->discard = AVDISCARD_DEFAULT;
        std::cout << "Audio stream initialized." << std::endl;
    } else {
        std::cerr << "Can not find audio stream." << std::endl;
    }

    // 初始化字幕解码器
    if (subtitle_stream_index >= 0) {
        AVStream* subtitle_stream = fmt_ctx->streams[subtitle_stream_index];
        AVCodecParameters* subtitle_codec_param = subtitle_stream->codecpar;
        AVCodec* subtitle_codec = avcodec_find_decoder(subtitle_codec_param->codec_id);
        if (!subtitle_codec) {
            std::cerr << "Can not find subtitle codec." << std::endl;
            return -30;
        }

        subtitle_codec_ctx = avcodec_alloc_context3(subtitle_codec);
        if (!subtitle_codec_ctx) {
            std::cerr << "avcodec_alloc_context3 failed." << std::endl;
            ret = -40;
            return ret;
        }

        defer(
            if (ret != 0 && subtitle_codec_ctx) {
            avcodec_free_context(&subtitle_codec_ctx);
            subtitle_codec_ctx = nullptr;
            }
        )

        ret = avcodec_parameters_to_context(subtitle_codec_ctx, subtitle_codec_param);
        if (ret != 0) {
            std::cerr << "avcodec_parameters_to_context failed." << std::endl;
            return ret;
        }

        ret = avcodec_open2(subtitle_codec_ctx, subtitle_codec, nullptr);
        if (ret != 0) {
            std::cerr << "Can not open subtitle codec error: " << ret << std::endl;
            return ret;
        }

        subtitle_stream->discard = AVDISCARD_DEFAULT;
        std::cout << "Subtitle stream initialized." << std::endl;
    } else {
        std::cerr << "Can not find subtitle stream." << std::endl;
    }
#endif


	return ret;
}

int CUVFFPlayer::close() {
	int nRet = 0;
	if (fmt_opts) {
		av_dict_free(&fmt_opts);
		fmt_opts = nullptr;
	}

	if (codec_opts) {
		av_dict_free(&codec_opts);
		codec_opts = nullptr;
	}

	if (fmt_ctx) {
		avformat_close_input(&fmt_ctx);
		avformat_free_context(fmt_ctx);
		fmt_ctx = nullptr;
	}

	if (video_codec_ctx) {
		nRet = avcodec_close(video_codec_ctx);
		avcodec_free_context(&video_codec_ctx);
		video_codec_ctx = nullptr;
	}

	if (video_frame) {
		av_frame_unref(video_frame);
		av_frame_free(&video_frame);
		video_frame = nullptr;
	}

	if (video_packet) {
		av_packet_unref(video_packet);
		av_packet_free(&video_packet);
		video_packet = nullptr;
	}

#if 0
    if (audio_codec_ctx) {
        nRet = avcodec_close(audio_codec_ctx);
        avcodec_free_context(&audio_codec_ctx);
        audio_codec_ctx = nullptr;
    }

    if (audio_frame) {
        av_frame_unref(audio_frame);
        av_frame_free(&audio_frame);
        audio_frame = nullptr;
    }

    if (audio_packet) {
        av_packet_unref(audio_packet);
        av_packet_free(&audio_packet);
        audio_packet = nullptr;
    }

    if (subtitle_codec_ctx) {
        nRet = avcodec_close(subtitle_codec_ctx);
        avcodec_free_context(&subtitle_codec_ctx);
        subtitle_codec_ctx = nullptr;
    }

    if (subtitle_packet) {
        av_packet_unref(subtitle_packet);
        av_packet_free(&subtitle_packet);
        subtitle_packet = nullptr;
    }

    if (subtitle) {
        avsubtitle_free(subtitle);
        subtitle = nullptr;
    }
#endif

	if (sws_ctx) {
		sws_freeContext(sws_ctx);
		sws_ctx = nullptr;
	}

	m_frame.buf.cleanup();
	return nRet;
}
