#include "uvcodec.hpp"

#include <QDateTime>

//CCalcPtsDur
inline CCalcPtsDur::CCalcPtsDur() {
	m_dTimeBase = 0.0;
	m_dFpsBen = 0.0;
	m_dFpsNum = 0.0;
	m_dFrameDur = 0.0;

	m_llAbsBaseTime = 0;
}

inline CCalcPtsDur::~CCalcPtsDur() = default;

inline void CCalcPtsDur::SetAbsBaseTime(const int64_t& llAbsBaseTime) {
	m_llAbsBaseTime = llAbsBaseTime;
}

inline void CCalcPtsDur::SetTimeBase(int iTimeBase, int iFpsNum, int iFpsBen) {
	m_dTimeBase = static_cast<double>(iTimeBase);
	m_dFpsBen = static_cast<double>(iFpsBen);
	m_dFpsNum = static_cast<double>(iFpsNum);

	//	m_dFrameDur = 1000.0 / (m_dFpsNum / m_dFpsBen) * (m_dTimeBase / 1000.0);
	m_dFrameDur = m_dTimeBase / (m_dFpsNum / m_dFpsBen);
}

inline int64_t CCalcPtsDur::GetVideoPts(int64_t lFrameIndex) const {
	int64_t lPts = m_dFrameDur * lFrameIndex; // NOLINT
	lPts += m_llAbsBaseTime;
	return lPts;
}

inline int CCalcPtsDur::GetVideoDur(int64_t lFrameIndex) const {
	int64_t lPts0 = m_dFrameDur * lFrameIndex;       // NOLINT
	int64_t lPts1 = m_dFrameDur * (lFrameIndex + 1); // NOLINT
	return lPts1 - lPts0;                            // NOLINT
}

inline int64_t CCalcPtsDur::GetAudioPts(int64_t lPaketIndex, int iAudioSample) const {
	double dAACSample = 1024.0;

	int64_t llPts = lPaketIndex * m_dTimeBase * dAACSample / static_cast<double>(iAudioSample); // NOLINT
	llPts += m_llAbsBaseTime;
	return llPts;
}

int CCalcPtsDur::GetAudioDur(int64_t lPaketIndex, int iAudioSample) const {
	int64_t lPts0 = GetAudioPts(lPaketIndex, iAudioSample);
	int64_t lPts1 = GetAudioPts(lPaketIndex + 1, iAudioSample);
	return lPts1 - lPts0; // NOLINT
}

// 根据给定的采样率和通道数，为 ADTS 格式的音频帧补充 ADTS 头部信息
void GetADTS(uint8_t* pBuf, int64_t nSampleRate, int64_t nChannels) {
	// 根据采样率确定 ADTS 头部中的 smap 值
	int iSmapIndex = 0;
	if (nSampleRate == 44100)
		iSmapIndex = 4;
	else if (nSampleRate == 48000)
		iSmapIndex = 3;
	else if (nSampleRate == 32000)
		iSmapIndex = 5;
	else if (nSampleRate == 64000)
		iSmapIndex = 2;

	// 构建 ADTS 头部中的 audio_specific_config 字段
	uint16_t audio_specific_config = 0;
	audio_specific_config |= ((2 << 11) & 0xF800);
	audio_specific_config |= ((iSmapIndex << 7) & 0x0780);
	audio_specific_config |= ((nChannels << 3) & 0x78);
	audio_specific_config |= 0 & 0x07;

	// 将 audio_specific_config 写入 ADTS 头部的前两个字节
	pBuf[0] = (audio_specific_config >> 8) & 0xFF;
	pBuf[1] = audio_specific_config & 0xFF;
	// 后续三个字节设置为常量值
	pBuf[2] = 0x56;
	pBuf[3] = 0xe5;
	pBuf[4] = 0x00;
}

//CLXCodecThread
FILE* CLXCodecThread::m_pLogFile{ nullptr };

CLXCodecThread::CLXCodecThread(QString strFile, LXPushStreamInfo stStreamInfo, QSize szPlay,
                               QPair<AVCodecParameters*, AVCodecParameters*> pairRecvCodecPara, OpenMode mode, bool bLoop, bool bPicture,
                               QObject* parent)
: QThread(parent), m_pairRecvCodecPara(pairRecvCodecPara), m_strFile(std::move(strFile)), m_szPlay(szPlay),
  m_stPushStreamInfo(std::move(stStreamInfo)), m_eMode(mode), m_bLoop(bLoop), m_bPicture(bPicture) {
	// 设置日志级别为最详细的日志信息
	av_log_set_level(AV_LOG_TRACE);
	// 设置回调函数，写入日志信息
	av_log_set_callback(LogCallBack);
	// 线程结束后自动删除
	setAutoDelete(true);
}

CLXCodecThread::CLXCodecThread(QObject* parent)
: QThread(parent) {
	av_log_set_level(AV_LOG_TRACE);
	av_log_set_callback(LogCallBack);
	setAutoDelete(true);
}

CLXCodecThread::~CLXCodecThread() {
	stop();
	clearMemory();
}

void CLXCodecThread::open(QString strFile, QSize szPlay, LXPushStreamInfo stStreamInfo, OpenMode mode, bool bLoop, bool bPicture) {
	stop();
	m_strFile = std::move(strFile);
	m_stPushStreamInfo = std::move(stStreamInfo);
	m_eMode = mode;
	m_bLoop = bLoop;
	m_szPlay = szPlay;
	m_bPicture = bPicture;
	resume();
	start();
}

void CLXCodecThread::seek(const quint64& nDuration) {
	if (m_pFormatCtx) {
		// 使用 QMutexLocker 确保线程安全地操作音频和视频相关资源
		QMutexLocker videoLocker(&m_videoMutex);
		QMutexLocker audioLocker(&m_audioMutex);
		// 处理视频定位
		if (m_pVideoThread) {
			// 定位视频流
			int nRet = avformat_seek_file(m_pFormatCtx, m_nVideoIndex, m_pFormatCtx->streams[m_nVideoIndex]->start_time,
			                              nDuration / av_q2d(m_pFormatCtx->streams[m_nVideoIndex]->time_base), // NOLINT
			                              m_pFormatCtx->streams[m_nVideoIndex]->duration, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			if (nRet >= 0) {
				// 设置视频线程当前的 PTS
				m_pVideoThread->setCurrentPts(nDuration / av_q2d(m_pFormatCtx->streams[m_nVideoIndex]->time_base)); // NOLINT
				// 清空视频队列中的数据
				while (!m_videoPacketQueue.isEmpty()) {
					AVPacket* pkt = m_videoPacketQueue.pop();
					av_packet_unref(pkt);
					av_packet_free(&pkt);
				}
				m_videoPacketQueue.clear();
				while (!m_videoPushPacketQueue.isEmpty()) {
					AVPacket* pkt = m_videoPushPacketQueue.pop();
					av_packet_unref(pkt);
					av_packet_free(&pkt);
				}
				m_videoPushPacketQueue.clear();
			} else {
				av_log(nullptr, AV_LOG_WARNING, "seek video fail, avformat_seek_file return %d\n", nRet);
			}
		}
		// 处理音频定位
		if (m_pAudioThread) {
			if (m_nAudioIndex >= 0) {
				// 定位音频流
				int nRet = avformat_seek_file(m_pFormatCtx, m_nAudioIndex, m_pFormatCtx->streams[m_nAudioIndex]->start_time,
				                              nDuration / av_q2d(m_pFormatCtx->streams[m_nAudioIndex]->time_base), // NOLINT
				                              m_pFormatCtx->streams[m_nAudioIndex]->duration, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
				if (nRet >= 0) {
					m_pAudioThread->setCurrentPts(nDuration / av_q2d(m_pFormatCtx->streams[m_nAudioIndex]->time_base)); // NOLINT
					while (!m_audioPacketQueue.isEmpty()) {
						AVPacket* pkt = m_audioPacketQueue.pop();
						av_packet_unref(pkt);
						av_packet_free(&pkt);
					}
					m_audioPacketQueue.clear();
					while (!m_audioPushPacketQueue.isEmpty()) {
						AVPacket* pkt = m_audioPushPacketQueue.pop();
						av_packet_unref(pkt);
						av_packet_free(&pkt);
					}
					m_audioPushPacketQueue.clear();
				} else {
					av_log(nullptr, AV_LOG_WARNING, "seek audio fail, avformat_seek_file return %d\n", nRet);
				}
			} else if (m_pEncodeMuteThread) {
				//while (!m_audioPacketQueue.isEmpty())
				//{
				//    AVPacket* pkt = m_audioPacketQueue.pop();
				//    av_packet_unref(pkt);
				//    av_packet_free(&pkt);
				//}
				//m_audioPacketQueue.clear();
				//while (!m_audioPushPacketQueue.isEmpty())
				//{
				//    AVPacket* pkt = m_audioPushPacketQueue.pop();
				//    av_packet_unref(pkt);
				//    av_packet_free(&pkt);
				//}
				//m_audioPushPacketQueue.clear();
			}
		}
	}
}

void CLXCodecThread::pause() {
	m_bPause = true;
	if (m_pAudioThread) {
		m_pAudioThread->pause();
	}
	if (m_pVideoThread) {
		m_pVideoThread->pause();
	}
	if (m_pEncodeMuteThread) {
		m_pEncodeMuteThread->pause();
	}
}

void CLXCodecThread::resume() {
	// 线程继续执行
	m_bRunning = true;
	// 线程不再暂停
	m_bPause = false;
	// 唤醒音频线程和视频线程，使其从暂停状态中恢复
	m_audioWaitCondition.wakeOne();
	m_videoWaitCondition.wakeOne();
	// 如果线程存在，恢复线程的执行
	if (m_pAudioThread) {
		m_pAudioThread->resume();
	}
	if (m_pVideoThread) {
		m_pVideoThread->resume();
	}
	if (m_pEncodeMuteThread) {
		m_pEncodeMuteThread->resume();
	}
}

void CLXCodecThread::stop() {
	m_bLoop = false;
	m_bRunning = false;
	m_videoWaitCondition.wakeAll();
	m_audioWaitCondition.wakeAll();
	m_AVSyncWaitCondition.wakeAll();
	wait();
}

void CLXCodecThread::run() {
	int nRet = -1;
	char errBuf[ERRBUF_SIZE]{};
	//查找视频流和音频流
	nRet = avformat_open_input(&m_pFormatCtx, m_strFile.toStdString().c_str(), nullptr, nullptr);
	if (nRet < 0) {
		av_strerror(nRet, errBuf, ERRBUF_SIZE);
		av_log(nullptr, AV_LOG_ERROR, "Can't open input, %s\n", errBuf);
		return;
	}
	nRet = avformat_find_stream_info(m_pFormatCtx, nullptr);
	if (nRet < 0) {
		av_strerror(nRet, errBuf, ERRBUF_SIZE);
		av_log(nullptr, AV_LOG_ERROR, "Can't find stream info, %s\n", errBuf);
		avformat_close_input(&m_pFormatCtx);
		return;
	}
	av_dump_format(m_pFormatCtx, 0, m_strFile.toStdString().c_str(), 0);
	m_nVideoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	m_nAudioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (-1 == m_nVideoIndex && -1 == m_nAudioIndex) {
		av_log(nullptr, AV_LOG_ERROR, "Can't find video and audio stream, %s\n");
		avformat_close_input(&m_pFormatCtx);
		return;
	}
	//push
	int nVideoEncodeIndex = -1;
	int nAudioEncodeIndex = -1;
	QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*, AVCodecContext*>> pairPushFormat{};
	if (CLXCodecThread::OpenMode::OpenMode_Push & m_eMode) {
		// 解析推流地址的 scheme
		QUrl url(m_stPushStreamInfo.strAddress);
		AVFormatContext* pOutputFormatCtx = nullptr;
		CCalcPtsDur calPts{};
		QString strFileFormat = url.scheme();
		// 根据 scheme 设置输出格式
		int nTimeBase = 90000;
		if (0 == url.scheme().compare("udp", Qt::CaseInsensitive)) {
			strFileFormat = "mpegts";
		} else if (0 == url.scheme().compare("srt", Qt::CaseInsensitive)) {
			strFileFormat = "mpegts";
			if (url.query().contains("transtype=file", Qt::CaseInsensitive)) {
				nTimeBase = 1000;
			}
		} else if (0 == url.scheme().compare("rtmp", Qt::CaseInsensitive)) {
			strFileFormat = "flv";
			nTimeBase = 1000;
		} else if (0 == url.scheme().compare("rtsp", Qt::CaseInsensitive)) {
			strFileFormat = "rtsp";
			nTimeBase = 1000;
		} else {
			av_log(nullptr, AV_LOG_ERROR, "Unsupported scheme: %s\n", url.scheme().toStdString().c_str());
			return;
		}

		// 设置时间戳计算对象的时间基准和帧率
		calPts.SetTimeBase(nTimeBase, m_stPushStreamInfo.nFrameRateDen > 0 ? m_stPushStreamInfo.nFrameRateDen : 25,
		                   m_stPushStreamInfo.nFrameRateNum > 0 ? m_stPushStreamInfo.nFrameRateNum : 1);

		// 分配输出上下文并设置相关参数
		nRet = avformat_alloc_output_context2(&pOutputFormatCtx, nullptr, strFileFormat.toStdString().c_str(),
		                                      m_stPushStreamInfo.strAddress.toStdString().c_str());
		if (nRet < 0) {
			av_strerror(nRet, errBuf, ERRBUF_SIZE);
			av_log(nullptr, AV_LOG_ERROR, "Can't alloc output ctx, %s\n", errBuf);
			avformat_close_input(&m_pFormatCtx);
			return;
		}

		//video
		AVCodecContext* pOutputVideoCodecCtx = nullptr;
		if (LXPushStreamInfo::Video & m_stPushStreamInfo.eStream) {
			// 查找 H.264 视频编码器
			const AVCodec* out_VideoCodec = avcodec_find_encoder_by_name("h264_nvenc");
			if (!out_VideoCodec) {
				out_VideoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
				if (!out_VideoCodec) {
					av_log(nullptr, AV_LOG_DEBUG, "Encoder not found\n");
					return;
				}
			}
			// 分配和初始化输出视频编码器上下文
			pOutputVideoCodecCtx = avcodec_alloc_context3(nullptr);
			if (m_pairRecvCodecPara.first && m_pairRecvCodecPara.first->width > 0 && m_pairRecvCodecPara.first->height > 0) {
				// 存在输入流参数，将其拷贝到输出视频编码器的上下文中
				avcodec_parameters_to_context(pOutputVideoCodecCtx, m_pairRecvCodecPara.first);
				avcodec_parameters_free(&m_pairRecvCodecPara.first);
				//av_opt_set_dict(pOutputVideoCodecCtx, &m_pRectFormatCtx->streams[nVideoIndex]->metadata);
			} else {
				pOutputVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
				//编码视频宽度
				pOutputVideoCodecCtx->width = m_stPushStreamInfo.nWidth > 0 ? m_stPushStreamInfo.nWidth :
					                              m_pFormatCtx->streams[m_nVideoIndex]->codecpar->width;
				//编码视频高度
				pOutputVideoCodecCtx->height = m_stPushStreamInfo.nHeight > 0 ? m_stPushStreamInfo.nHeight :
					                               m_pFormatCtx->streams[m_nVideoIndex]->codecpar->height;
				//平均码率
				//变小码率画质不清晰
				pOutputVideoCodecCtx->bit_rate = m_stPushStreamInfo.fVideoBitRate > 0 ? m_stPushStreamInfo.fVideoBitRate : 2000000; // NOLINT
				//指定图像中每个像素的颜色数据的格式
				//I帧间隔  50帧一个I帧
				pOutputVideoCodecCtx->gop_size = 50;

				pOutputVideoCodecCtx->thread_count = 4;
				//两个非b帧之间b帧最大数
				pOutputVideoCodecCtx->max_b_frames = 0;
			}

			if (pOutputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
				pOutputVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			}
			//最小量化因子 建议值 10~30
			pOutputVideoCodecCtx->qmin = 10; //测试结果：当QMIN升高或qmax降低，对视音频质量和大小产生明显影响
			//最大量化因子
			pOutputVideoCodecCtx->qmax = 51; //测试结果：单纯减少qmax值增大体积，但并不能提高画质，还与其他因素有关
			//运动估计的最大搜索范围。跟运动补偿有关，值越大，补偿参考范围越广，越精确，编码效率下降。
			pOutputVideoCodecCtx->me_range = 16;
			//帧间最大量化因子
			pOutputVideoCodecCtx->max_qdiff = 4;
			//压缩变化的难易程度。值越大，越难压缩变换，压缩率越高，质量损失较大
			pOutputVideoCodecCtx->qcompress = 0.4;
			//编码视频帧率  25
			pOutputVideoCodecCtx->time_base.num = m_stPushStreamInfo.nFrameRateNum > 0 ?
				                                      m_stPushStreamInfo.nFrameRateNum : m_pFormatCtx->streams[m_nVideoIndex]->r_frame_rate.num;
			pOutputVideoCodecCtx->time_base.den = m_stPushStreamInfo.nFrameRateDen > 0 ?
				                                      m_stPushStreamInfo.nFrameRateDen : m_pFormatCtx->streams[m_nVideoIndex]->r_frame_rate.den;

			//            pOutputVideoCodecCtx->level = 31;
			/**
			 * preset有ultrafast，superfast，veryfast，faster，fast，medium，slow，slower，veryslow，placebo这10个级别，每个级别的preset对应一组编码参数，
			 * 不同级别的preset对应的编码参数集不一致。preset级别越高，编码速度越慢，解码后的质量也越高；级别越低，速度也越快，解码后的图像质量也就越差，从左到右，编码速度越来越慢，
			 * 编码质量越来越好
			 */
			AVDictionary* param = nullptr;
			// av_dict_set(&param, "preset", "superfast", 0);
			av_dict_set(&param, "profile", "main", 0);
			av_dict_set(&param, "rc", "cbr", 0);
			av_dict_set(&param, "cbr", "1", 0);
			av_dict_set(&param, "gpu", "0", 0);
			//av_dict_set(&param, "profile", "baseline", 0);
			// av_dict_set(&param, "rtsp_transport", "tcp", 0);
			nRet = avcodec_open2(pOutputVideoCodecCtx, out_VideoCodec, &param);
			if (nRet < 0) {
				av_strerror(nRet, errBuf, ERRBUF_SIZE);
				av_log(nullptr, AV_LOG_ERROR, "Can't open video encoder\n");
				avcodec_free_context(&pOutputVideoCodecCtx);
				avformat_free_context(pOutputFormatCtx);
				avformat_close_input(&m_pFormatCtx);
				return;
			}
			//unsigned char sps_pps[23] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x0a, 0xf8, 0x0f, 0x00, 0x44, 0xbe, 0x8,
			//    0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x38, 0x80 };
			//pOutputVideoCodecCtx->extradata_size = 23;
			//pOutputVideoCodecCtx->extradata = (uint8_t*)av_malloc(23 + AV_INPUT_BUFFER_PADDING_SIZE);
			//if (pOutputVideoCodecCtx->extradata == nullptr) {
			//    printf("could not av_malloc the video params extradata!\n");
			//    return;
			//}
			//memcpy(pOutputVideoCodecCtx->extradata, sps_pps, 23);

			// 创建输出流视频
			AVStream* out_VideoStream = avformat_new_stream(pOutputFormatCtx, nullptr);
			if (!out_VideoStream) {
				av_log(nullptr, AV_LOG_ERROR, "Can't create video streeam\n");
				avcodec_free_context(&pOutputVideoCodecCtx);
				avformat_free_context(pOutputFormatCtx);
				avformat_close_input(&m_pFormatCtx);
				return;
			}
			// 存储视频流的索引
			nVideoEncodeIndex = out_VideoStream->index;
			// 将视频编码器上下文的参数拷贝到输出视频流的编码参数中
			avcodec_parameters_from_context(out_VideoStream->codecpar, pOutputVideoCodecCtx);
			out_VideoStream->codecpar->codec_tag = 0;

			//            out_VideoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
			//            out_VideoStream->codecpar->codec_id = AV_CODEC_ID_H264;
			//            out_VideoStream->codecpar->bit_rate = m_stPushStreamInfo.nAudioBitRate > 0 ? m_stPushStreamInfo.nAudioBitRate * 1024 : 1672320;
			//            out_VideoStream->codecpar->bits_per_coded_sample = 24;
			//            out_VideoStream->codecpar->bits_per_raw_sample = m_stPushStreamInfo.nColorDepth > 0 ? m_stPushStreamInfo.nColorDepth : 8;
			//            out_VideoStream->codecpar->profile = 100;
			//            out_VideoStream->codecpar->level = 41;
			//            out_VideoStream->codecpar->width = m_stPushStreamInfo.nWidth > 0 ? m_stPushStreamInfo.nWidth : m_pFormatCtx->streams[m_nVideoIndex]->codecpar->width;
			//            out_VideoStream->codecpar->height = m_stPushStreamInfo.nHeight > 0 ? m_stPushStreamInfo.nHeight : m_pFormatCtx->streams[m_nVideoIndex]->codecpar->height;
			//            out_VideoStream->codecpar->field_order = AV_FIELD_PROGRESSIVE;
			//            out_VideoStream->codecpar->chroma_location = AVCHROMA_LOC_LEFT;
			//            out_VideoStream->codecpar->video_delay = 2;

			//           // avcodec_parameters_copy(out_VideoStream->codecpar, m_pFormatCtx->streams[m_nVideoIndex]->codecpar);
		}

		//audio
		AVCodecContext* pOutputAudioCodecCtx = nullptr;
		if (LXPushStreamInfo::Audio & m_stPushStreamInfo.eStream) {
			// 查找 AAC 音频编码器
			const AVCodec* out_AudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
			pOutputAudioCodecCtx = avcodec_alloc_context3(out_AudioCodec);
			if (m_pairRecvCodecPara.second && m_pairRecvCodecPara.second->sample_rate > 0 && m_pairRecvCodecPara.second->channels > 0) {
				avcodec_parameters_to_context(pOutputAudioCodecCtx, m_pairRecvCodecPara.second);
				avcodec_parameters_free(&m_pairRecvCodecPara.second);
				//av_opt_set_dict(pOutputVideoCodecCtx, &m_pRectFormatCtx->streams[nVideoIndex]->metadata);
			} else {
				//avcodec_parameters_to_context(pOutputAudioCodecCtx, m_pFormatCtx->streams[m_nAudioIndex]->codecpar);
				//av_opt_set_dict(pOutputAudioCodecCtx, &m_pFormatCtx->streams[m_nAudioIndex]->metadata);
				pOutputAudioCodecCtx->profile = (strFileFormat == "mpegts") ? FF_PROFILE_MPEG2_AAC_HE : FF_PROFILE_AAC_MAIN;               // 编码协议
				pOutputAudioCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;                                                                     // 音频编码
				pOutputAudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;                                                                     // 指定音频采样格式为浮点数
				pOutputAudioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;                                                                // 设置音频通道布局为立体声
				pOutputAudioCodecCtx->channels = 2;                                                                                        // 双通道
				pOutputAudioCodecCtx->sample_rate = m_stPushStreamInfo.nAudioSampleRate > 0 ? m_stPushStreamInfo.nAudioSampleRate : 44100; // 设置采样频率
				pOutputAudioCodecCtx->bit_rate = m_stPushStreamInfo.nAudioBitRate > 0 ? m_stPushStreamInfo.nAudioBitRate * 1024 : 327680;  // 设置采样比特率
				//                             if (pOutputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
				//                             {
				//                                 pOutputAudioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
				//                             }

				pOutputAudioCodecCtx->extradata_size = 5;                               // 设置音频编码器的额外数据大小为5个字节
				pOutputAudioCodecCtx->extradata = static_cast<uint8_t*>(av_mallocz(5)); // 分配5个字节额外空间
				// 生成 AAC 的 ADTS 头信息
				GetADTS(pOutputAudioCodecCtx->extradata, pOutputAudioCodecCtx->sample_rate, pOutputAudioCodecCtx->channels);
			}
			// 创建字典，存储音频编码器的配置参数
			AVDictionary* param = nullptr;
			// 设置音频编码器的预设值，预设值影响编码速度和质量之前的权衡
			av_dict_set(&param, "preset", "superfast", 0);
			// 设置音频编码器的调整参数，优化编码器以减小延迟
			av_dict_set(&param, "tune", "zerolatency", 0);

			av_dict_set(&param, "rtsp_transport", "tcp", 0);
			// 打开音频编码器
			nRet = avcodec_open2(pOutputAudioCodecCtx, out_AudioCodec, &param);
			if (nRet < 0) {
				av_strerror(nRet, errBuf, ERRBUF_SIZE);
				av_log(nullptr, AV_LOG_ERROR, "Can't open audio encoder\n");
				avcodec_free_context(&pOutputAudioCodecCtx);
				avformat_free_context(pOutputFormatCtx);
				avformat_close_input(&m_pFormatCtx);
				return;
			}
			// 创建音频流，并将其添加到输出格式上下文中
			AVStream* out_AudioStream = avformat_new_stream(pOutputFormatCtx, nullptr);
			if (!out_AudioStream) {
				av_log(nullptr, AV_LOG_ERROR, "Can't create audio stream\n");
				avcodec_free_context(&pOutputAudioCodecCtx);
				avformat_free_context(pOutputFormatCtx);
				avformat_close_input(&m_pFormatCtx);
				return;
			}
			// 存储音频流的索引
			nAudioEncodeIndex = out_AudioStream->index;
			// 将音频编码器的参数复制到音频流的参数中
			nRet = avcodec_parameters_from_context(out_AudioStream->codecpar, pOutputAudioCodecCtx);
			// 不使用标签
			out_AudioStream->codecpar->codec_tag = 0;

			//            out_AudioStream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
			//            out_AudioStream->codecpar->codec_id = AV_CODEC_ID_AAC;
			//            out_AudioStream->codecpar->bit_rate = m_stPushStreamInfo.nAudioBitRate > 0 ? m_stPushStreamInfo.nAudioBitRate * 1024 : 327680;
			//            out_AudioStream->codecpar->bits_per_coded_sample = 16;
			//            out_AudioStream->codecpar->bits_per_raw_sample = 24;
			//            out_AudioStream->codecpar->profile = 1;
			//            out_AudioStream->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
			//            out_AudioStream->codecpar->channels = 2;
			//            out_AudioStream->codecpar->sample_rate = m_stPushStreamInfo.nAudioSampleRate > 0 ? m_stPushStreamInfo.nAudioSampleRate : 44100;
			//            avcodec_parameters_copy(out_AudioStream->codecpar, m_pFormatCtx->streams[m_nAudioIndex]->codecpar);
		}

		// 根据文件格式判断推流类型
		int nPushStreamType = 0;
		AVDictionary* avdic = nullptr;
		if (strFileFormat == "flv")
			nPushStreamType = 0;
		else if (strFileFormat == "mpegts")
			nPushStreamType = 1;
		else if (strFileFormat == "rtsp")
			nPushStreamType = 2;

		if (nPushStreamType == 0) {
			av_dict_set(&avdic, "rtmp_live", "1", 0);
			av_dict_set(&avdic, "live", "1", 0);
		} else if (nPushStreamType == 1) {
			av_dict_set(&avdic, "pkt_size", "1316", 0);
		} else if (nPushStreamType == 2) {
			av_dict_set(&avdic, "rtsp_transport", "tcp", 0);
		}

		// 打开输出流的 I/O 上下文，传递之前设置好的音频编码器配置参数
		nRet = avio_open2(&pOutputFormatCtx->pb, m_stPushStreamInfo.strAddress.toStdString().c_str(), AVIO_FLAG_WRITE, nullptr, &avdic);
		if (nRet < 0) {
			av_strerror(nRet, errBuf, ERRBUF_SIZE);
			av_log(nullptr, AV_LOG_ERROR, "Can't open io, %s\n", errBuf);
			if (pOutputVideoCodecCtx) {
				avcodec_free_context(&pOutputVideoCodecCtx);
			}
			if (pOutputAudioCodecCtx) {
				avcodec_free_context(&pOutputAudioCodecCtx);
			}
			avformat_free_context(pOutputFormatCtx);
			avformat_close_input(&m_pFormatCtx);
			return;
		}

		// 写入输出流的头部信息，初始化输出流并写入包含必要信息的头部
		nRet = avformat_write_header(pOutputFormatCtx, nullptr);
		if (nRet < 0) {
			avio_closep(&pOutputFormatCtx->pb);
			avformat_free_context(pOutputFormatCtx);
			avformat_close_input(&m_pFormatCtx);
			av_strerror(nRet, errBuf, ERRBUF_SIZE);
			av_log(nullptr, AV_LOG_ERROR, "Can't write header, %s\n", errBuf);
			return;
		}
		// 将推流所需的格式上下文，时间基准，以及视频和音频编码器上下文保存
		pairPushFormat = qMakePair(pOutputFormatCtx, std::make_tuple(calPts, pOutputVideoCodecCtx, pOutputAudioCodecCtx));

		// 启动推流线程
		m_pPushThread = new CLXPushThread(pOutputFormatCtx, m_videoPushPacketQueue, m_audioPushPacketQueue,
		                                  m_pushVideoWaitCondition, m_pushVideoMutex, m_pushAudioWaitCondition, m_pushAudioMutex,
		                                  LXPushStreamInfo::Video & m_stPushStreamInfo.eStream, LXPushStreamInfo::Audio & m_stPushStreamInfo.eStream);
		m_pPushThread->start();
	}
	// 如果存在视频流
	if (m_nVideoIndex >= 0) {
		// 设置解码模式
		m_eDecodeMode = m_bPicture ? eLXDecodeMode::eLXDecodeMode_CPU : eLXDecodeMode::eLXDecodeMode_GPU;
		// 创建包含 AVFormatContext 和相关信息元组的pair对象
		QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairVideoFormat;
		pairVideoFormat = qMakePair(pairPushFormat.first, std::make_tuple(std::get<0>(pairPushFormat.second), std::get<1>(pairPushFormat.second)));
		// 创建视频线程
		m_pVideoThread = new CLXVideoThread(m_videoPacketQueue, m_videoPushPacketQueue, m_videoWaitCondition,
		                                    m_videoMutex, m_pushVideoWaitCondition, m_pushVideoMutex, m_pFormatCtx, m_nVideoIndex, nVideoEncodeIndex,
		                                    pairVideoFormat,
		                                    m_eMode, m_nAudioIndex < 0, LXPushStreamInfo::Video & m_stPushStreamInfo.eStream, m_szPlay,
		                                    m_eDecodeMode);
		// 连接信号
		connect(m_pVideoThread, &CLXVideoThread::notifyImage, this, &CLXCodecThread::notifyImage);
		connect(m_pVideoThread, &CLXVideoThread::notifyCountDown, this, &CLXCodecThread::notifyCountDown);
		// 开启线程
		m_pVideoThread->start();
	}
	// 如果存在音频流或者是推流模式并且音频流是开启的
	if (m_nAudioIndex >= 0 || (CLXCodecThread::OpenMode::OpenMode_Push & m_eMode && LXPushStreamInfo::Audio & m_stPushStreamInfo.eStream)) {
		// 创建存储解码参数的 AVCodecParameters 对象
		AVCodecParameters decodePara;
		// 如果没有音频流，且是推流模式并且音频流是开启的
		if (m_nAudioIndex < 0 && CLXCodecThread::OpenMode::OpenMode_Push & m_eMode && LXPushStreamInfo::Audio & m_stPushStreamInfo.eStream) {
			// 创建一个包含音频编码参数的元组
			auto pairEncodeCtx = std::make_tuple(std::get<0>(pairPushFormat.second), std::get<2>(pairPushFormat.second));
			// 创建静音音频编码线程
			m_pEncodeMuteThread = new CLXEncodeMuteAudioThread(m_audioPacketQueue, m_audioWaitCondition, m_audioMutex, m_AVSyncWaitCondition,
			                                                   m_AVSyncMutex, nAudioEncodeIndex, pairEncodeCtx);
			m_pEncodeMuteThread->start();
			// 从推送格式的音频上下文中获取解码参数，并设置AAC的profile
			avcodec_parameters_from_context(&decodePara, std::get<2>(pairPushFormat.second));
			decodePara.profile = FF_PROFILE_AAC_LOW;
			memset(decodePara.extradata, 0, decodePara.extradata_size);
			decodePara.extradata_size = 0;
		}
		QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairAudioFormat;
		pairAudioFormat = qMakePair(pairPushFormat.first, std::make_tuple(std::get<0>(pairPushFormat.second), std::get<2>(pairPushFormat.second)));
		// 创建音频线程
		m_pAudioThread = new CLXAudioThread(m_audioPacketQueue, m_audioPushPacketQueue, m_audioWaitCondition, m_audioMutex, m_pushAudioWaitCondition,
		                                    m_pushAudioMutex, m_pFormatCtx, m_nAudioIndex, nAudioEncodeIndex, pairAudioFormat, m_eMode,
		                                    LXPushStreamInfo::Audio & m_stPushStreamInfo.eStream, decodePara);
		connect(m_pAudioThread, &CLXAudioThread::notifyAudio, this, &CLXCodecThread::notifyAudio);
		connect(m_pAudioThread, &CLXAudioThread::notifyCountDown, this, &CLXCodecThread::notifyCountDown);
		connect(m_pAudioThread, &CLXAudioThread::notifyAudioPara, this, &CLXCodecThread::notifyAudioPara);
		// 启动音频线程
		m_pAudioThread->start();
	}
	// 如果媒体文件的时长大于等于0，通过信号传递垫片的时长，单位：秒，否则传递0
	if (m_pFormatCtx->duration >= 0) {
		emit notifyClipInfo(m_pFormatCtx->duration / AV_TIME_BASE);
	} else {
		emit notifyClipInfo(0);
	}
	//视频准备读取
	AVPacket* packet = av_packet_alloc();

	//读取数据包
Loop:
	while (m_bRunning && av_read_frame(m_pFormatCtx, packet) >= 0) {
		// 如果处于暂停状态，等待编码线程唤醒
		if (m_bPause) {
			if (m_nVideoIndex >= 0) {
				QMutexLocker videoLocker(&m_videoMutex);
				m_videoWaitCondition.wait(&m_videoMutex);
			} else if (m_nAudioIndex >= 0) {
				QMutexLocker audioLocker(&m_audioMutex);
				m_audioWaitCondition.wait(&m_audioMutex);
			}
			av_packet_unref(packet);
			continue;
		}
		// 当前数据包属于视频流
		if (m_nVideoIndex == packet->stream_index) {
			// 如果是图像模式，循环将克隆的数据包推送到视频队列中，并唤醒相关线程
			if (m_bPicture) {
				while (m_bRunning) {
					AVPacket* pkt = av_packet_clone(packet);
					QMutexLocker locker(&m_videoMutex);
					if (m_videoPacketQueue.isFull()) {
						m_videoWaitCondition.wait(&m_videoMutex);
					}
					m_videoPacketQueue.push(pkt);
					m_videoWaitCondition.wakeOne();
					m_AVSyncWaitCondition.wakeOne();
				}
			}
			// 否则将克隆的数据包推送到视频队列，并唤醒相关线程
			else {
				AVPacket* pkt = av_packet_clone(packet);
				QMutexLocker locker(&m_videoMutex);
				if (m_videoPacketQueue.isFull()) {
					m_videoWaitCondition.wait(&m_videoMutex);
				}
				m_videoPacketQueue.push(pkt);
				m_AVSyncWaitCondition.wakeOne();
				m_videoWaitCondition.wakeOne();
			}
		}
		// 当前数据包属于音频流
		else if (m_nAudioIndex == packet->stream_index) {
			// 将克隆的数据包推送到音频队列，并唤醒相关线程
			AVPacket* pkt = av_packet_clone(packet);
			QMutexLocker locker(&m_audioMutex);
			if (m_audioPacketQueue.isFull()) {
				m_audioWaitCondition.wait(&m_audioMutex);
			}
			m_audioPacketQueue.push(pkt);
			m_audioWaitCondition.wakeOne();
		}
		// 释放当前数据包
		av_packet_unref(packet);
	}
	// 如果处于运行状态并且启用了循环播放
	if (m_bRunning && m_bLoop) {
		// 暂停，定位到起始时间，恢复播放
		pause();
		seek(m_pFormatCtx->start_time);
		resume();
		goto Loop;
	}
	// 清理内存
	clearMemory();
	// 如果存在推送格式的上下文，写入尾部，关闭IO，释放视频和音频编码上下文
	if (pairPushFormat.first) {
		av_write_trailer(pairPushFormat.first);
		avio_close(pairPushFormat.first->pb);
		if (std::get<1>(pairPushFormat.second)) {
			avcodec_close(std::get<1>(pairPushFormat.second));
			avcodec_free_context(&std::get<1>(pairPushFormat.second));
		}
		if (std::get<2>(pairPushFormat.second)) {
			avcodec_close(std::get<2>(pairPushFormat.second));
			avcodec_free_context(&std::get<2>(pairPushFormat.second));
		}
		avformat_free_context(pairPushFormat.first);
	}
	// 释放数据包和媒体格式上下文
	av_packet_free(&packet);
	avformat_close_input(&m_pFormatCtx);
}

void CLXCodecThread::LogCallBack(void* avcl, int level, const char* fmt, va_list vl) { // NOLINT
	QString strLogPath = qApp->applicationDirPath() + "/log/ffmpeg.log";               // NOLINT
	if (m_pLogFile == nullptr) {
		m_pLogFile = fopen(strLogPath.toStdString().c_str(), "w+"); // NOLINT
	}
	if (m_pLogFile) {
		// 构建时间戳格式
		char timeFmt[1024]{};
		sprintf(timeFmt, "%s     %s", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.z").toStdString().c_str(), fmt); // NOLINT
		// 将格式化后的日志信息写入文件
		vfprintf(m_pLogFile, timeFmt, vl);
		// 刷新缓冲区
		fflush(m_pLogFile);
	}
}

void CLXCodecThread::clearMemory() {
	if (m_pPushThread) {
		m_pushVideoWaitCondition.wakeAll();
		m_pushAudioWaitCondition.wakeAll();
		m_pPushThread->stop();
		SAFE_DELETE(m_pPushThread);
	}
	if (m_pVideoThread) {
		m_videoWaitCondition.wakeAll();
		m_pVideoThread->stop();
		SAFE_DELETE(m_pVideoThread);
	}
	if (m_pAudioThread) {
		m_audioWaitCondition.wakeAll();
		m_pAudioThread->stop();
		SAFE_DELETE(m_pAudioThread);
	}
	if (m_pEncodeMuteThread) {
		m_AVSyncWaitCondition.wakeAll();
		m_pEncodeMuteThread->stop();
		SAFE_DELETE(m_pEncodeMuteThread);
	}
	while (!m_videoPacketQueue.isEmpty()) {
		AVPacket* pkt = m_videoPacketQueue.pop();
		av_packet_unref(pkt);
		av_packet_free(&pkt);
	}
	m_videoPacketQueue.clear();
	while (!m_audioPacketQueue.isEmpty()) {
		AVPacket* pkt = m_audioPacketQueue.pop();
		av_packet_unref(pkt);
		av_packet_free(&pkt);
	}
	m_audioPacketQueue.clear();
	while (!m_videoPushPacketQueue.isEmpty()) {
		AVPacket* pkt = m_videoPushPacketQueue.pop();
		av_packet_unref(pkt);
		av_packet_free(&pkt);
	}
	m_videoPushPacketQueue.clear();
	while (!m_audioPushPacketQueue.isEmpty()) {
		AVPacket* pkt = m_audioPushPacketQueue.pop();
		av_packet_unref(pkt);
		av_packet_free(&pkt);
	}
	m_audioPushPacketQueue.clear();
}

//CLXVideoThread
CLXVideoThread::CLXVideoThread(CircularQueue<AVPacket*>& packetQueue, CircularQueue<AVPacket*>& pushPacketQueue, QWaitCondition& waitCondition,
                               QMutex& mutex,
                               QWaitCondition& videoWaitCondition, QMutex& videoMutex, AVFormatContext* pFormatCtx,
                               int nStreamIndex, int nEncodeStreamIndex,
                               QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairOutputCtx, CLXCodecThread::OpenMode eMode,
                               bool bSendCountDown, bool bPush, QSize szPlay, CLXCodecThread::eLXDecodeMode eDecodeMode, QObject* parent)
: QThread(parent), m_pFormatCtx(pFormatCtx), m_szPlay(szPlay), m_pairOutputCtx(std::move(pairOutputCtx)), m_nStreamIndex(nStreamIndex),
  m_nEncodeStreamIndex(nEncodeStreamIndex), m_bSendCountDown(bSendCountDown), m_bPush(bPush), m_packetQueue(packetQueue),
  m_pushPacketQueue(pushPacketQueue), m_decodeWaitCondition(waitCondition), m_decodeMutex(mutex), m_pushWaitCondition(videoWaitCondition),
  m_pushMutex(videoMutex), m_eMode(eMode), m_eDecodeMode(eDecodeMode) {
}

CLXVideoThread::~CLXVideoThread() {
	stop();
}

void CLXVideoThread::pause() {
	QMutexLocker locker(&m_decodeMutex);
	m_bPause = true;
	m_encodeWaitCondition.wakeAll();
	m_playWaitCondition.wakeAll();
	if (m_pEncodeThread) {
		m_pEncodeThread->pause();
	}
	if (m_pPlayThread) {
		m_pPlayThread->pause();
	}
}

void CLXVideoThread::resume() {
	QMutexLocker locker(&m_decodeMutex);
	m_bPause = false;
	if (m_pEncodeThread) {
		m_pEncodeThread->resume();
	}
	if (m_pPlayThread) {
		m_pPlayThread->resume();
	}
}

void CLXVideoThread::stop() {
	m_bRunning = false;
	// 唤醒所有线程
	m_encodeWaitCondition.wakeAll();
	m_playWaitCondition.wakeAll();
	if (m_pEncodeThread) {
		m_encodeWaitCondition.wakeAll();
		m_pEncodeThread->stop();
		SAFE_DELETE(m_pEncodeThread);
	}
	if (m_pPlayThread) {
		m_playWaitCondition.wakeAll();
		m_pPlayThread->stop();
		SAFE_DELETE(m_pPlayThread);
	}
	// 等待线程执行完毕
	wait();
	while (!m_decodeFrameQueue.isEmpty()) {
		AVFrame* frame = m_decodeFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_decodeFrameQueue.clear();
	while (!m_playFrameQueue.isEmpty()) {
		AVFrame* frame = m_playFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_playFrameQueue.clear();
}

void CLXVideoThread::setCurrentPts(int64_t nPts) {
	// 设置当前播放的时间戳
	if (m_pPlayThread) {
		m_pPlayThread->setCurrentPts(nPts);
	}
	// 清除解码后和播放的视频帧
	while (!m_decodeFrameQueue.isEmpty()) {
		AVFrame* frame = m_decodeFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_decodeFrameQueue.clear();
	while (!m_playFrameQueue.isEmpty()) {
		AVFrame* frame = m_playFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_playFrameQueue.clear();
}

enum AVPixelFormat CLXVideoThread::hw_pix_fmt = AV_PIX_FMT_NONE;

// 确定编码器上下文应使用的硬件加速像素格式
enum AVPixelFormat CLXVideoThread::get_hw_format(AVCodecContext* ctx,
                                                 const enum AVPixelFormat* pix_fmts) {
	const enum AVPixelFormat* p = nullptr;

	for (p = pix_fmts; *p != -1; p++) {
		if (*p == hw_pix_fmt)
			return *p;
	}

	fprintf(stderr, "Failed to get HW surface format.\n");
	return AV_PIX_FMT_NONE;
}

void CLXVideoThread::run() {
	int nRet = -1;
	char errBuf[ERRBUF_SIZE]{};
	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	const AVCodec* pVideoCodec = nullptr;
	AVBufferRef* hw_device_ctx = nullptr;

	auto codec = [&]() {
		// 将视频流的参数拷贝到解码器上下文
		avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_nStreamIndex]->codecpar);
		// 根据解码器的 id 找到对应的视频解码器
		pVideoCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	};
	// GPU解码
	if (m_eDecodeMode == CLXCodecThread::eLXDecodeMode::eLXDecodeMode_GPU) {
#ifdef Q_OS_WIN
		// 选择视频流中的最佳解码器
		av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVideoCodec, 0);
		// 遍历解码器的硬件配置，找到与指定硬件设备类型匹配的解码器配置
		enum AVHWDeviceType type = AV_HWDEVICE_TYPE_DXVA2;
		for (int i = 0;; i++) {
			const AVCodecHWConfig* config = avcodec_get_hw_config(pVideoCodec, i);
			if (!config) {
				fprintf(stderr, "Decoder %s does not support device type %s.\n",
				        pVideoCodec->name, av_hwdevice_get_type_name(type));
				return;
			}
			if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
			    config->device_type == type) {
				// 设置解码器的像素格式
				hw_pix_fmt = config->pix_fmt;
				break;
			}
		}
		// 将视频流的参数拷贝到解码上下文
		avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_nStreamIndex]->codecpar);
		// 设置解码器的回调函数为 get_hw_format
		m_pCodecCtx->get_format = get_hw_format;
		if (av_hwdevice_ctx_create(&hw_device_ctx, type,
		                           nullptr, nullptr, 0) < 0) {
			fprintf(stderr, "Failed to create specified HW device.\n");
			return;
		}

		// 将硬解码设备上下文绑定到解码器上下文
		m_pCodecCtx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
#else
        codec();
#endif
	} else {
		codec();
	}
	// 打开解码器 m_pCodecCtx，将解码器与其上下文关联
	nRet = avcodec_open2(m_pCodecCtx, pVideoCodec, nullptr);
	if (nRet < 0) {
		av_strerror(nRet, errBuf, ERRBUF_SIZE);
		av_log(nullptr, AV_LOG_ERROR, "Can't open decoder, %s\n", errBuf);
		avcodec_free_context(&m_pCodecCtx);
		return;
	}
	AVFrame* pFrame = nullptr;
	// 分配 pFrame 结构
	pFrame = av_frame_alloc();
	// 如果是推流并且需要推流
	if (CLXCodecThread::OpenMode::OpenMode_Push & m_eMode && m_bPush) {
		QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairEncodeCtx = qMakePair(m_pCodecCtx, m_pairOutputCtx.second);
		// 创建编码线程，用于将解码后的帧推送到编码器并打包成推流所需的数据包
		m_pEncodeThread = new CLXEncodeVideoThread(m_decodeFrameQueue, m_pushPacketQueue, m_encodeWaitCondition, m_encodeMutex,
		                                           m_pushWaitCondition, m_pushMutex, m_nEncodeStreamIndex, pairEncodeCtx, m_eDecodeMode);
		m_pEncodeThread->start();
	}
	// 如果是播放模式
	if (CLXCodecThread::OpenMode::OpenMode_Play & m_eMode) {
		// 创建播放线程，该线程用于将解码后的帧进行播放或显示
		m_pPlayThread = new CLXVideoPlayThread(m_playFrameQueue, m_playWaitCondition, m_playMutex, m_pCodecCtx,
		                                       m_pFormatCtx->streams[m_nStreamIndex]->time_base, m_bSendCountDown, this, m_szPlay, m_eDecodeMode);
		m_pPlayThread->start();
	}
	// sw_frame 存储从 GPU 到 CPU 的数据
	AVFrame* sw_frame = av_frame_alloc();
	// tmp_frame 用于在 GPU 解码时判断是否需要进行数据转移
	AVFrame* tmp_frame = nullptr;
	while (m_bRunning) {
		if (m_bPause) {
			continue;
		}
		// 记录当前时间，方便后续统计解码耗时
		QElapsedTimer ti;
		ti.start();
		// 使用互斥锁对解码相关的操作进行保护
		m_decodeMutex.lock();
		// 如果解码队列为空，唤醒等待条件，并等待新的数据包
		if (m_packetQueue.isEmpty()) {
			m_decodeWaitCondition.wakeOne();
			m_decodeWaitCondition.wait(&m_decodeMutex);
		}
		// 从解码队列中取出一个数据包
		AVPacket* packet = m_packetQueue.pop();
		if (Q_LIKELY(packet)) {
			// 将数据包送入解码器
			nRet = avcodec_send_packet(m_pCodecCtx, packet);
			if (nRet < 0) {
				av_strerror(nRet, errBuf, ERRBUF_SIZE);
				av_log(nullptr, AV_LOG_ERROR, "Can't send video packet, %s\n", errBuf);
				av_packet_unref(packet);
				av_packet_free(&packet);
				m_decodeMutex.unlock();
				continue;
			}
			// 通过 avcodec_receive_frame 循环获取解码后的视频帧
			while (avcodec_receive_frame(m_pCodecCtx, pFrame) >= 0) {
				// 如果解码器输出的帧格式是 GPU 处理的方式，则尝试将数据从 GPU 转移到 CPU
				if (pFrame->format == hw_pix_fmt) {
					/* retrieve data from GPU to CPU */
					if (av_hwframe_transfer_data(sw_frame, pFrame, 0) < 0) {
						av_strerror(nRet, errBuf, ERRBUF_SIZE);
						av_log(nullptr, AV_LOG_ERROR, "Error transferring the data to system memory, %s\n", errBuf);
						av_packet_unref(packet);
						av_packet_free(&packet);
						m_decodeMutex.unlock();
						continue;
					}
					// 将 tmp_frame 指向 sw_frame
					tmp_frame = sw_frame;
					tmp_frame->pts = pFrame->pts;
					tmp_frame->pkt_dts = pFrame->pkt_dts;
				}
				// 将 tmp_frame 指向 sw_frame
				else
					tmp_frame = pFrame;

				// 判断是否推流
				if (CLXCodecThread::OpenMode::OpenMode_Push & m_eMode && m_bPush) {
					// 使用互斥锁 m_encodeMutex 对推送队列进行保护
					// 并在队列满时等待条件
					m_encodeMutex.lock();
					if (m_decodeFrameQueue.isFull()) {
						m_encodeWaitCondition.wait(&m_encodeMutex);
					}
					if (AVFrame* pCopyFrame = av_frame_clone(tmp_frame)) {
						m_decodeFrameQueue.push(pCopyFrame);
						m_encodeWaitCondition.wakeOne();
					}
					m_encodeMutex.unlock();
				}
				// 判断是否播放
				if (CLXCodecThread::OpenMode::OpenMode_Play & m_eMode) {
					// 使用互斥锁 m_playMutex 对推送队列进行保护
					// 并在队列满时等待条件
					m_playMutex.lock();
					if (m_playFrameQueue.isFull()) {
						m_playWaitCondition.wait(&m_playMutex);
					}
					if (AVFrame* pCopyFrame = av_frame_clone(tmp_frame)) {
						m_playFrameQueue.push(pCopyFrame);
						m_playWaitCondition.wakeOne();
					}
					m_playMutex.unlock();
				}
			}
			// 释放解码使用的资源，包括数据包，解码后的帧
			av_packet_unref(packet);
			av_packet_free(&packet);
			av_frame_unref(pFrame);
		}
		// 解锁解码互斥锁
		m_decodeMutex.unlock();
	}
	// 释放通过 av_frame_alloc 分配的解码后的帧结构体 pFrame
	av_frame_free(&pFrame);
	// 释放 GPU 到 CPU 转移的帧结构体 sw_frame
	av_frame_free(&sw_frame);
	// 判断是否存在硬件设备上下文
	if (hw_device_ctx) {
		av_buffer_unref(&hw_device_ctx);
	}
	// 关闭解码器
	avcodec_close(m_pCodecCtx);
	// 释放解码器
	avcodec_free_context(&m_pCodecCtx);
}

//CLXAudioThread
CLXAudioThread::CLXAudioThread(CircularQueue<AVPacket*>& packetQueue, CircularQueue<AVPacket*>& pushPacketQueue, QWaitCondition& waitCondition,
                               QMutex& mutex, QWaitCondition& audioWaitCondition, QMutex& audioMutex, AVFormatContext* pFormatCtx,
                               int nStreamIndex, int nEncodeStreamIndex,
                               QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairOutputCtx, CLXCodecThread::OpenMode eMode,
                               bool bPush, const AVCodecParameters& decodePara, QObject* parent)
: QThread(parent), m_pFormatCtx(pFormatCtx), m_decodePara(decodePara), m_pairOutputCtx(std::move(pairOutputCtx)), m_nStreamIndex(nStreamIndex),
  m_nEncodeStreamIndex(nEncodeStreamIndex), m_bPush(bPush), m_packetQueue(packetQueue), m_pushPacketQueue(pushPacketQueue),
  m_decodeWaitCondition(waitCondition), m_decodeMutex(mutex), m_pushWaitCondition(audioWaitCondition), m_pushMutex(audioMutex), m_eMode(eMode) {
}

CLXAudioThread::~CLXAudioThread() {
	stop();
}

void CLXAudioThread::pause() {
	QMutexLocker locker(&m_decodeMutex);
	m_encodeWaitCondition.wakeAll();
	m_playWaitCondition.wakeAll();
	if (m_pEncodeThread) {
		m_pEncodeThread->pause();
	}
	if (m_pPlayThread) {
		m_pPlayThread->pause();
	}
	m_bPause = true;
}

void CLXAudioThread::resume() {
	QMutexLocker locker(&m_decodeMutex);
	m_bPause = false;
	if (m_pEncodeThread) {
		m_pEncodeThread->resume();
	}
	if (m_pPlayThread) {
		m_pPlayThread->resume();
	}
}

void CLXAudioThread::stop() {
	m_bRunning = false;
	m_encodeWaitCondition.wakeAll();
	m_playWaitCondition.wakeAll();
	if (m_pEncodeThread) {
		m_encodeWaitCondition.wakeAll();
		m_pEncodeThread->stop();
		SAFE_DELETE(m_pEncodeThread);
	}
	if (m_pPlayThread) {
		m_playWaitCondition.wakeAll();
		m_pPlayThread->stop();
		SAFE_DELETE(m_pPlayThread);
	}
	wait();
	while (!m_decodeFrameQueue.isEmpty()) {
		AVFrame* frame = m_decodeFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_decodeFrameQueue.clear();
	while (!m_playFrameQueue.isEmpty()) {
		AVFrame* frame = m_playFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_playFrameQueue.clear();
}

void CLXAudioThread::setCurrentPts(int64_t nPts) {
	if (m_pPlayThread) {
		m_pPlayThread->setCurrentPts(nPts);
	}
	while (!m_decodeFrameQueue.isEmpty()) {
		AVFrame* frame = m_decodeFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_decodeFrameQueue.clear();
	while (!m_playFrameQueue.isEmpty()) {
		AVFrame* frame = m_playFrameQueue.pop();
		av_frame_unref(frame);
		av_frame_free(&frame);
	}
	m_playFrameQueue.clear();
}

void CLXAudioThread::run() {
	int nRet = -1;
	char errBuf[ERRBUF_SIZE]{};
	// 分配并初始化音频解码器
	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	// 如果是推流并且无推流索引
	if (m_bPush && m_nStreamIndex < 0) {
		// 使用传入的解码参数
		avcodec_parameters_to_context(m_pCodecCtx, &m_decodePara);
	} else {
		// 使用流的解码参数
		avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_nStreamIndex]->codecpar);
	}

	// 通过 id 找到音频解码器
	const AVCodec* pAudioCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	// 打开音频解码器
	nRet = avcodec_open2(m_pCodecCtx, pAudioCodec, nullptr);
	if (nRet < 0) {
		av_strerror(nRet, errBuf, ERRBUF_SIZE);
		av_log(nullptr, AV_LOG_ERROR, "Can't open audio decoder, %s\n", errBuf);
		avcodec_free_context(&m_pCodecCtx);
		return;
	}
	// 发送音频参数通知
	emit notifyAudioPara(m_pCodecCtx->sample_rate, m_pCodecCtx->channels);
	// 判断为推送还是播放，并且创建其对应的线程
	if (CLXCodecThread::OpenMode::OpenMode_Push & m_eMode && m_bPush) {
		QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairEncodeCtx = qMakePair(m_pCodecCtx, m_pairOutputCtx.second);
		m_pEncodeThread = new CLXEncodeAudioThread(m_decodeFrameQueue, m_pushPacketQueue, m_encodeWaitCondition, m_encodeMutex,
		                                           m_pushWaitCondition, m_pushMutex, m_nEncodeStreamIndex, pairEncodeCtx);

		m_pEncodeThread->start();
	}
	if (CLXCodecThread::OpenMode::OpenMode_Play & m_eMode) {
		m_pPlayThread = new CLXAudioPlayThread(m_playFrameQueue, m_playWaitCondition, m_playMutex, m_pCodecCtx,
		                                       m_nStreamIndex >= 0 ? m_pFormatCtx->streams[m_nStreamIndex]->time_base : m_pCodecCtx->time_base, this);
		m_pPlayThread->start();
	}

	AVFrame* pFrame = nullptr;
	pFrame = av_frame_alloc();
	while (m_bRunning) {
		if (m_bPause) {
			continue;
		}
		m_decodeMutex.lock();
		// 如果音频包队列为空，唤醒并等待
		if (m_packetQueue.isEmpty()) {
			m_decodeWaitCondition.wakeOne();
			m_decodeWaitCondition.wait(&m_decodeMutex);
		}
		// 从音频包队列中获取一个音频包
		AVPacket* packet = m_packetQueue.pop();
		if (Q_LIKELY(packet)) {
			// 通过 avcodec_send_packet 发送音频包进行解码
			nRet = avcodec_send_packet(m_pCodecCtx, packet);
			if (nRet < 0) {
				av_strerror(nRet, errBuf, ERRBUF_SIZE);
				av_log(nullptr, AV_LOG_ERROR, "Can't send audio packet, %s\n", errBuf);
				av_packet_unref(packet);
				av_packet_free(&packet);
				m_decodeMutex.unlock();
				continue;
			}
			// 接收解码后的音频帧
			while (avcodec_receive_frame(m_pCodecCtx, pFrame) >= 0) {
				// 判断是推送还是播放模式，并将解码后的音频帧放入相应的队列当中
				if (CLXCodecThread::OpenMode::OpenMode_Push & m_eMode && m_bPush) {
					m_encodeMutex.lock();
					if (m_decodeFrameQueue.isFull()) {
						m_encodeWaitCondition.wait(&m_encodeMutex);
					}
					if (AVFrame* pCopyFrame = av_frame_clone(pFrame)) {
						m_decodeFrameQueue.push(pCopyFrame);
						m_encodeWaitCondition.wakeOne();
					}
					m_encodeMutex.unlock();
				}
				if (CLXCodecThread::OpenMode::OpenMode_Play & m_eMode) {
					m_playMutex.lock();
					if (m_playFrameQueue.isFull()) {
						m_playWaitCondition.wait(&m_playMutex);
					}
					if (AVFrame* pCopyFrame = av_frame_clone(pFrame)) {
						m_playFrameQueue.push(pCopyFrame);
						m_playWaitCondition.wakeOne();
					}
					m_playMutex.unlock();
				}
			}
			// 释放音频包和数据帧
			av_packet_unref(packet);
			av_packet_free(&packet);
			av_frame_unref(pFrame);
		}
		m_decodeMutex.unlock();
	}
	// 释放最后一个音频帧
	av_frame_free(&pFrame);
	// 关闭音频解码器
	avcodec_close(m_pCodecCtx);
	// 释放音频解码器
	avcodec_free_context(&m_pCodecCtx);
}

//CLXVideoPlayThread
CLXVideoPlayThread::CLXVideoPlayThread(CircularQueue<AVFrame*>& decodeFrameQueue, QWaitCondition& waitCondition, QMutex& mutex,
                                       AVCodecContext* pCodecCtx, const AVRational& timeBase, bool bSendCountDown, CLXVideoThread* videoThread,
                                       QSize szPlay,
                                       CLXCodecThread::eLXDecodeMode eDecodeMode, QObject* parent)
: QThread(parent), m_pCodecCtx(pCodecCtx), m_bSendCountDown(bSendCountDown), m_timeBase(timeBase), m_videoThread(videoThread),
  m_playFrameQueue(decodeFrameQueue), m_playWaitCondition(waitCondition), m_playMutex(mutex), m_szPlay(szPlay), m_eDecodeMode(eDecodeMode) {
}

CLXVideoPlayThread::~CLXVideoPlayThread() = default;

void CLXVideoPlayThread::pause() {
	QMutexLocker locker(&m_playMutex);
	m_bPause = true;
}

void CLXVideoPlayThread::resume() {
	QMutexLocker locker(&m_playMutex);
	m_bPause = false;
	m_nLastTime = av_gettime();
}

void CLXVideoPlayThread::stop() {
	m_bRunning = false;
	m_playWaitCondition.wakeOne();
	wait();
}

void CLXVideoPlayThread::setCurrentPts(int64_t nPts) {
	if (m_pCodecCtx) {
		avcodec_flush_buffers(m_pCodecCtx);
		m_nLastPts = nPts;
	}
}

void CLXVideoPlayThread::run() {
	// 分配用于存储 RGB 格式帧的 AVFrame
	AVFrame* pFrameRGB = nullptr;
	pFrameRGB = av_frame_alloc();
	// 分配 RGB 图像的数据缓冲区
	int iSize = av_image_alloc(pFrameRGB->data, pFrameRGB->linesize, m_szPlay.width(), m_szPlay.height(), AV_PIX_FMT_RGB32, 1); // NOLINT
	// 创建用于图像转换的 SwsContext（图像转换器
#ifdef Q_OS_LINUX
    enum AVPixelFormat srcFormat = m_pCodecCtx->pix_fmt;
#else
	enum AVPixelFormat srcFormat = m_eDecodeMode == CLXCodecThread::eLXDecodeMode::eLXDecodeMode_CPU ? m_pCodecCtx->pix_fmt : AV_PIX_FMT_NV12;
#endif
	struct SwsContext* img_decode_convert_ctx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, srcFormat, m_szPlay.width(),
	                                                           m_szPlay.height(), AV_PIX_FMT_RGB32, SWS_BICUBIC, nullptr, nullptr, nullptr);
	// 初始化播放时间戳
	m_nLastTime = av_gettime();
	m_nLastPts = 0;
	// 循环播放视频帧
	while (m_bRunning) {
		// 如果暂停，则继续进行下一轮循环
		if (m_bPause) {
			continue;
		}
		// 上锁，访问播放队列
		m_playMutex.lock();
		// 如果播放队列为空，唤醒并等待
		if (m_playFrameQueue.isEmpty()) {
			m_playWaitCondition.wakeOne();
			m_playWaitCondition.wait(&m_playMutex);
		}
		// 从播放帧队列中取出一帧
		AVFrame* pFrame = m_playFrameQueue.pop();
		if (Q_LIKELY(pFrame)) {
			// 发送倒计时通知
			if (m_bSendCountDown) {
				int64_t nCurrentTimeStamp = pFrame->pts * av_q2d(m_timeBase); // NOLINT
				emit m_videoThread->notifyCountDown(nCurrentTimeStamp);
			}
			// 计算时间偏移
			int64_t nTimeStampOffset = (pFrame->pts - m_nLastPts) * AV_TIME_BASE * av_q2d(m_timeBase); // NOLINT
			// 将数据帧转换为 RGB 格式
			int nH = sws_scale(img_decode_convert_ctx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameRGB->data, // NOLINT
			                   pFrameRGB->linesize);
			// 创建QImage对象，用于显示
			QImage tmpImg(static_cast<const uchar*>(pFrameRGB->data[0]), m_szPlay.width(), m_szPlay.height(), QImage::Format_RGB32);
			tmpImg.detach();
			// 首先发送给 CLXVideoThread，再由 CLXVideoThread 发送给 CLXPlaybackWidget 呈现出来
			emit m_videoThread->notifyImage(QPixmap::fromImage(tmpImg));
			// 计算时间偏移并休眠
			int64_t nTimeOffset = nTimeStampOffset - av_gettime() + m_nLastTime;
			if (nTimeOffset > 0) {
				av_usleep(nTimeOffset);
			}
			// 释放帧数据
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
		}
		m_playMutex.unlock();
	}
	// 释放 RGB 图像数据缓冲区
	av_freep(&pFrameRGB->data[0]);
	// 释放 RGB 帧
	av_frame_free(&pFrameRGB);
	// 释放图像转换器
	sws_freeContext(img_decode_convert_ctx);
}

//CLXAudioPlayThread
CLXAudioPlayThread::CLXAudioPlayThread(CircularQueue<AVFrame*>& decodeFrameQueue, QWaitCondition& waitCondition,
                                       QMutex& mutex, AVCodecContext* pCodecCtx, const AVRational& timeBase,
                                       CLXAudioThread* audioThread, QObject* parent)
: QThread(parent), m_pCodecCtx(pCodecCtx), m_timeBase(timeBase), m_audioThread(audioThread),
  m_playFrameQueue(decodeFrameQueue), m_playWaitCondition(waitCondition), m_playMutex(mutex) {
}

CLXAudioPlayThread::~CLXAudioPlayThread() {
	stop();
}

void CLXAudioPlayThread::pause() {
	QMutexLocker locker(&m_playMutex);
	m_bPause = true;
}

void CLXAudioPlayThread::resume() {
	QMutexLocker locker(&m_playMutex);
	m_nLastTime = av_gettime();
	m_bPause = false;
}

void CLXAudioPlayThread::stop() {
	m_bRunning = false;
	m_playWaitCondition.wakeOne();
	wait();
}

void CLXAudioPlayThread::setCurrentPts(int64_t nPts) {
	if (m_pCodecCtx) {
		avcodec_flush_buffers(m_pCodecCtx);
		m_nLastPts = nPts;
	}
}

void CLXAudioPlayThread::run() {
	//音频准备读取
	int out_channel_nb = 0;
	out_channel_nb = av_get_channel_layout_nb_channels(m_pCodecCtx->channel_layout);
	// 分配用于存储转换后音频数据的缓冲区
	auto audio_decode_buffer = static_cast<uint8_t*>(av_malloc(m_pCodecCtx->channels * m_pCodecCtx->sample_rate));
	// 创建音频转换器 audio_decode_swrCtx
	auto audio_decode_swrCtx = swr_alloc_set_opts(nullptr, static_cast<int64_t>(m_pCodecCtx->channel_layout), AV_SAMPLE_FMT_S16,
	                                              m_pCodecCtx->sample_rate, static_cast<int64_t>(m_pCodecCtx->channel_layout),
	                                              m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr);
	// 初始化音频转换器
	swr_init(audio_decode_swrCtx);
	// 初始化播放时间戳
	m_nLastTime = av_gettime();
	m_nLastPts = 0;
	// 循环播放音频帧
	while (m_bRunning) {
		if (m_bPause) {
			continue;
		}
		// 上锁，访问播放帧队列
		m_playMutex.lock();
		// 如果播放帧队列为空，唤醒并等待
		if (m_playFrameQueue.isEmpty()) {
			m_playWaitCondition.wakeOne();
			m_playWaitCondition.wait(&m_playMutex);
		}
		int64_t nTimeStampOffset = 0;
		// 从播放队列中取出一帧
		AVFrame* pFrame = m_playFrameQueue.pop();
		// 如果帧存在
		if (Q_LIKELY(pFrame)) {
			// 发送倒计时通知
			int64_t nCurrentTimeStamp = pFrame->pts * av_q2d(m_timeBase); // NOLINT
			emit m_audioThread->notifyCountDown(nCurrentTimeStamp);
			// 计算时间戳偏移
			nTimeStampOffset = (pFrame->pts - m_nLastPts) * AV_TIME_BASE * av_q2d(m_timeBase); // NOLINT
			// 执行音频转换
			swr_convert(audio_decode_swrCtx, &audio_decode_buffer, m_pCodecCtx->channels * m_pCodecCtx->sample_rate,
			            const_cast<const uint8_t**>(pFrame->data), pFrame->nb_samples);
			// 计算输出音频数据大小
			int out_audio_buffer_size = av_samples_get_buffer_size(nullptr, out_channel_nb, pFrame->nb_samples, AV_SAMPLE_FMT_S16, 1);
			// 将音频数据转换为 QByteArray 并发送给主线程
			QByteArray data(reinterpret_cast<const char*>(audio_decode_buffer), out_audio_buffer_size);
			// 首先发送给 CLXCodecThread 线程，再由 CLXCodecThread 线程发送给 CLXPlaybackWidget，将音频数据写入 m_audioByteBuffer，再写入 m_pAudioDevice，
			// 由 m_pAudioDevice = m_pAudioOutput->start(); 启动播放
			emit m_audioThread->notifyAudio(data);
			// 释放帧数据
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
		}
		// 解锁
		m_playMutex.unlock();
		// 计算时间偏移并休眠，确保按照音频的时间戳进行播放
		int64_t nTimeOffset = nTimeStampOffset - av_gettime() + m_nLastTime;
		if (nTimeOffset > 0) {
			av_usleep(nTimeOffset);
		}
	}
	// 释放音频缓冲区
	av_freep(&audio_decode_buffer);
	// 释放音频转换器
	swr_free(&audio_decode_swrCtx);
}

CLXEncodeVideoThread::CLXEncodeVideoThread(CircularQueue<AVFrame*>& encodeFrameQueue, CircularQueue<AVPacket*>& pushPacketQueue,
                                           QWaitCondition& encodeWaitCondition,
                                           QMutex& encodeMutex, QWaitCondition& pushWaitCondition, QMutex& pushMutex, int nEncodeStreamIndex,
                                           QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairEncodeCtx,
                                           CLXCodecThread::eLXDecodeMode eEncodeMode,
                                           QObject* parent)
: QThread(parent), m_encodeFrameQueue(encodeFrameQueue), m_pushPacketQueue(pushPacketQueue), m_nEncodeStreamIndex(nEncodeStreamIndex),
  m_encodeWaitCondition(encodeWaitCondition), m_encodeMutex(encodeMutex), m_pushWaitCondition(pushWaitCondition), m_pushMutex(pushMutex),
  m_pairEncodeCtx(std::move(pairEncodeCtx)), m_eEncodeMode(eEncodeMode) {
}

CLXEncodeVideoThread::~CLXEncodeVideoThread() {
	stop();
}

void CLXEncodeVideoThread::pause() {
	QMutexLocker locker(&m_encodeMutex);
	m_bPause = true;
}

void CLXEncodeVideoThread::resume() {
	QMutexLocker locker(&m_encodeMutex);
	m_bPause = false;
}

void CLXEncodeVideoThread::stop() {
	m_bRunning = false;
	m_encodeWaitCondition.wakeOne();
	wait();
}

void CLXEncodeVideoThread::run() {
	char errBuf[ERRBUF_SIZE]{};
	// 获取编码器
	if (AVCodecContext* pEncodeCtx = std::get<1>(m_pairEncodeCtx.second)) {
		int frame_index = 0;
		int nRet = 0;
		// 分配用于存储 YUV420P 格式帧的AVFrame
		AVFrame* pFrameYUV420P;
		pFrameYUV420P = av_frame_alloc();
		//av_image_alloc(pFrameYUV420P->data, pFrameYUV420P->linesize, pEncodeCtx->width, pEncodeCtx->height, AV_PIX_FMT_YUV420P, 1);
		// 分配用于图像转换的缓冲区
		auto video_convert_buffer = static_cast<uint8_t*>(av_malloc(
			av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pEncodeCtx->width, pEncodeCtx->height, 1)));
		av_image_fill_arrays(pFrameYUV420P->data, pFrameYUV420P->linesize, video_convert_buffer, AV_PIX_FMT_YUV420P, pEncodeCtx->width,
		                     pEncodeCtx->height, 1);
		// 创建图像转换器
#ifdef Q_OS_WIN
		struct SwsContext* img_encode_convert_ctx = sws_getContext(m_pairEncodeCtx.first->width, m_pairEncodeCtx.first->height,
		                                                           m_eEncodeMode == CLXCodecThread::eLXDecodeMode::eLXDecodeMode_CPU
			                                                           ? m_pairEncodeCtx.first->pix_fmt : AV_PIX_FMT_NV12, pEncodeCtx->width,
		                                                           pEncodeCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);
#elif defined(Q_OS_LINUX)
        struct SwsContext* img_encode_convert_ctx = sws_getContext(
                m_pairEncodeCtx.first->width, m_pairEncodeCtx.first->height, m_pairEncodeCtx.first->pix_fmt,
                pEncodeCtx->width, pEncodeCtx->height, AV_PIX_FMT_YUV420P,
                SWS_ACCURATE_RND | SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
#endif
		// 设置AVFrame的格式为 YUV420P
		pFrameYUV420P->format = AV_PIX_FMT_YUV420P;

		// 分配用于存储编码后数据的 AVPacket
		AVPacket* pPushPacket = av_packet_alloc();

		// 在线程运行期间循环执行
		while (m_bRunning) {
			// 如果线程被暂停，则继续下一轮循环
			if (m_bPause) {
				continue;
			}
			// 上锁访问编码帧队列
			m_encodeMutex.lock();
			// 如果编码帧队列为空，唤醒并等待
			if (m_encodeFrameQueue.isEmpty()) {
				m_encodeWaitCondition.wakeOne();
				m_encodeWaitCondition.wait(&m_encodeMutex);
			}
			// 从编码帧队列中取出一帧
			AVFrame* pFrame = m_encodeFrameQueue.pop();
			// 如果帧有效
			if (Q_LIKELY(pFrame)) {
				// 获取计算 PTS 的对象
				const CCalcPtsDur& calPts = std::get<0>(m_pairEncodeCtx.second);
				// 默认使用原始帧进行编码
				AVFrame* pEncodeFrame = pFrame;
				// 如果编码器不支持 NV12 格式，或者帧的分辨率不符合要求，进行格式转换
				if (AV_PIX_FMT_YUV420P != m_pairEncodeCtx.first->pix_fmt || m_pairEncodeCtx.first->width != pEncodeCtx->width ||
				    m_pairEncodeCtx.first->height != pEncodeCtx->height) {
					// 进行图像转换
					nRet = sws_scale(img_encode_convert_ctx, pFrame->data, pFrame->linesize, 0, m_pairEncodeCtx.first->height, pFrameYUV420P->data,
					                 pFrameYUV420P->linesize);
					// 如果转换失败，记录警告并继续下一轮循环
					if (nRet < 0) {
						av_strerror(nRet, errBuf, ERRBUF_SIZE);
						av_log(nullptr, AV_LOG_WARNING, "video sws_scale yuv420p fail, %s\n", errBuf);
						av_packet_unref(pPushPacket);
						m_encodeMutex.unlock();
						continue;
					}
					// 设置 YUV420P 帧的宽和高
					pFrameYUV420P->width = pEncodeCtx->width;
					pFrameYUV420P->height = pEncodeCtx->height;
					// 使用转换后的帧进行编码
					pEncodeFrame = av_frame_clone(pFrameYUV420P);
				}
				// 设置帧的 PTS 并发送给编码器
				pEncodeFrame->pts = calPts.GetVideoPts(frame_index);
				nRet = avcodec_send_frame(pEncodeCtx, pEncodeFrame);
				// 如果发送失败，记录警告并继续下一轮循环
				if (nRet < 0) {
					av_strerror(nRet, errBuf, ERRBUF_SIZE);
					av_log(nullptr, AV_LOG_WARNING, "video avcodec_send_frame fail, %s\n", errBuf);
					av_packet_unref(pPushPacket);
					m_encodeMutex.unlock();
					continue;
				}
				// 接收编码后的数据包
				nRet = avcodec_receive_packet(pEncodeCtx, pPushPacket);
				if (nRet < 0) {
					av_strerror(nRet, errBuf, ERRBUF_SIZE);
					av_log(nullptr, AV_LOG_WARNING, "video avcodec_receive_packet fail, %s\n", errBuf);
					av_packet_unref(pPushPacket);
					m_encodeMutex.unlock();
					continue;
				}
				// 设置推送包流索引
				pPushPacket->stream_index = m_nEncodeStreamIndex;
				m_pushMutex.lock();
				// 如果推送队列已满，等待
				if (m_pushPacketQueue.isFull()) {
					m_pushWaitCondition.wait(&m_pushMutex);
				}
				// 将数据包克隆一份并推送
				m_pushPacketQueue.push(av_packet_clone(pPushPacket));
				m_pushWaitCondition.wakeOne();
				m_pushMutex.unlock();
				// 释放原始帧的资源
				av_frame_unref(pFrame);
				// 释放编码帧的资源
				av_frame_unref(pEncodeFrame);
				av_frame_free(&pEncodeFrame);
				// 释放推送包的资源
				av_packet_unref(pPushPacket);
				// 帧索引递增
				frame_index++;
			}
			m_encodeMutex.unlock();
		}
		// 释放资源
		av_frame_free(&pFrameYUV420P);
		av_packet_free(&pPushPacket);
		av_freep(&video_convert_buffer);
		sws_freeContext(img_encode_convert_ctx);
	}
}

//CLXEncodeAudioThread
CLXEncodeAudioThread::CLXEncodeAudioThread(CircularQueue<AVFrame*>& encodeFrameQueue, CircularQueue<AVPacket*>& pushPacketQueue,
                                           QWaitCondition& encodeWaitCondition,
                                           QMutex& encodeMutex, QWaitCondition& pushWaitCondition, QMutex& pushMutex, int nEncodeStreamIndex,
                                           QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairEncodeCtx, QObject* parent)
: QThread(parent), m_encodeFrameQueue(encodeFrameQueue), m_pushPacketQueue(pushPacketQueue), m_nEncodeStreamIndex(nEncodeStreamIndex),
  m_encodeWaitCondition(encodeWaitCondition), m_encodeMutex(encodeMutex), m_pushWaitCondition(pushWaitCondition), m_pushMutex(pushMutex),
  m_pairEncodeCtx(std::move(pairEncodeCtx)) {
}

CLXEncodeAudioThread::~CLXEncodeAudioThread() {
	stop();
}

void CLXEncodeAudioThread::pause() {
	QMutexLocker locker(&m_encodeMutex);
	m_bPause = true;
}

void CLXEncodeAudioThread::resume() {
	QMutexLocker locker(&m_encodeMutex);
	m_bPause = false;
}

void CLXEncodeAudioThread::stop() {
	m_bRunning = false;
	m_encodeWaitCondition.wakeOne();
	wait();
}

void CLXEncodeAudioThread::run() {
	char errBuf[ERRBUF_SIZE]{}; // 存储错误信息缓冲区
	// 获取编码器
	if (AVCodecContext* pEncodeCtx = std::get<1>(m_pairEncodeCtx.second)) {
		int frame_index = 0;
		int nRet = 0;
		// 分配用于存储编码前音频帧的空间
		AVFrame* pFrameAAC;
		pFrameAAC = av_frame_alloc();
		// 创建音频重采样上下文并设置参数
		auto audio_encode_swrCtx = swr_alloc_set_opts(nullptr, pEncodeCtx->channel_layout, AV_SAMPLE_FMT_FLT, pEncodeCtx->sample_rate, // NOLINT
		                                              m_pairEncodeCtx.first->channel_layout, m_pairEncodeCtx.first->sample_fmt,        // NOLINT
		                                              m_pairEncodeCtx.first->sample_rate, 0, nullptr);

		swr_init(audio_encode_swrCtx);
		// 分配并初始化推送数据包
		AVPacket* pPushPacket = av_packet_alloc();
		while (m_bRunning) {
			if (m_bPause) {
				continue;
			}
			m_encodeMutex.lock();
			if (m_encodeFrameQueue.isEmpty()) {
				m_encodeWaitCondition.wakeOne();
				m_encodeWaitCondition.wait(&m_encodeMutex);
			}
			// 获取音频帧队列中的一帧
			AVFrame* pFrame = m_encodeFrameQueue.pop();
			if (Q_LIKELY(pFrame)) {
				// 处理音频帧
				const CCalcPtsDur& calPts = std::get<0>(m_pairEncodeCtx.second);
				AVFrame* pEncodeFrame = pFrame;
				// 如果音频格式不是 AV_SAMPLE_FMT_FLTP，则进行音频重采样
				if (AV_SAMPLE_FMT_FLTP != m_pairEncodeCtx.first->sample_fmt) {
					// 重采样失败，记录错误信息并释放资源
					nRet = swr_convert_frame(audio_encode_swrCtx, pFrameAAC, pFrame);
					if (nRet < 0) {
						av_strerror(nRet, errBuf, ERRBUF_SIZE);
						av_log(nullptr, AV_LOG_WARNING, "audio swr_convert fail, %s\n", errBuf);
						av_packet_unref(pPushPacket);
						m_encodeMutex.unlock();
						continue;
					}
					// 克隆重采样后的音频帧
					pEncodeFrame = av_frame_clone(pFrameAAC);
				}
				// 设置音频帧的时间戳
				pEncodeFrame->pts = calPts.GetAudioPts(frame_index, pEncodeCtx->sample_rate);
				// 将音频帧发送给编码器
				nRet = avcodec_send_frame(pEncodeCtx, pEncodeFrame);
				if (nRet < 0) {
					// 发送失败，记录错误信息并释放资源
					av_strerror(nRet, errBuf, ERRBUF_SIZE);
					av_log(nullptr, AV_LOG_WARNING, "audio avcodec_receive_packet fail, %s\n", errBuf);
					av_packet_unref(pPushPacket);
					m_encodeMutex.unlock();
					continue;
				}
				// 接收编码后的音频数据包
				nRet = avcodec_receive_packet(pEncodeCtx, pPushPacket);
				if (nRet < 0) {
					// 接收失败，记录错误信息并释放资源
					av_strerror(nRet, errBuf, ERRBUF_SIZE);
					av_log(nullptr, AV_LOG_WARNING, "audio avcodec_receive_packet fail, %s\n", errBuf);
					av_packet_unref(pPushPacket);
					m_encodeMutex.unlock();
					continue;
				}
				// 设置推送包流索引
				pPushPacket->stream_index = m_nEncodeStreamIndex;
				m_pushMutex.lock();
				if (m_pushPacketQueue.isFull()) {
					// 队列满，等待并唤醒
					m_pushWaitCondition.wait(&m_pushMutex);
				}
				// 将克隆后的数据包推送到队列
				m_pushPacketQueue.push(av_packet_clone(pPushPacket));
				m_pushWaitCondition.wakeOne();
				m_pushMutex.unlock();
				// 释放帧和数据包
				av_frame_unref(pFrame);
				av_frame_unref(pEncodeFrame);
				av_frame_free(&pEncodeFrame);
				av_packet_unref(pPushPacket);
				// 递增帧索引
				frame_index++;
			}
			m_encodeMutex.unlock();
		}
		// 释放资源
		av_frame_free(&pFrameAAC);
		av_packet_free(&pPushPacket);
		swr_free(&audio_encode_swrCtx);
	}
}

//CLXEncodeMuteAudioThread
CLXEncodeMuteAudioThread::CLXEncodeMuteAudioThread(CircularQueue<AVPacket*>& pushPacketQueue, QWaitCondition& encodeWaitCondition,
                                                   QMutex& encodeMutex, QWaitCondition& syncWaitCondition, QMutex& syncMutex, int nEncodeStreamIndex,
                                                   std::tuple<CCalcPtsDur, AVCodecContext*> pairEncodeCtx, QObject* parent)
: QThread(parent), m_encodePacketQueue(pushPacketQueue), m_nEncodeStreamIndex(nEncodeStreamIndex),
  m_encodeWaitCondition(encodeWaitCondition), m_encodeMutex(encodeMutex), m_syncWaitCondition(syncWaitCondition), m_syncMutex(syncMutex),
  m_pairEncodeCtx(std::move(pairEncodeCtx)) {
}

CLXEncodeMuteAudioThread::~CLXEncodeMuteAudioThread() {
	stop();
}

void CLXEncodeMuteAudioThread::pause() {
	QMutexLocker locker(&m_encodeMutex);
	m_bPause = true;
}

void CLXEncodeMuteAudioThread::resume() {
	QMutexLocker locker(&m_encodeMutex);
	m_bPause = false;
}

void CLXEncodeMuteAudioThread::stop() {
	m_bRunning = false;
	wait();
}

void CLXEncodeMuteAudioThread::run() {
	char errBuf[ERRBUF_SIZE]{}; // 错误信息缓冲区
	// 获取输入编码器
	if (AVCodecContext* pEncodeCtx = std::get<1>(m_pairEncodeCtx)) {
		int nRet = 0;
		// 查找 AAC 编码器
		const AVCodec* out_AudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
		// 分配输出音频编码器上下文
		AVCodecContext* pOutputAudioCodecCtx = avcodec_alloc_context3(nullptr);
		// 配置输出音频编码器上下文参数
		pOutputAudioCodecCtx->profile = FF_PROFILE_AAC_LOW;    // 编码协议
		pOutputAudioCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO; // 音频编码
		pOutputAudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
		pOutputAudioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
		pOutputAudioCodecCtx->channels = 2;
		pOutputAudioCodecCtx->sample_rate = pEncodeCtx->sample_rate;
		pOutputAudioCodecCtx->bit_rate = pEncodeCtx->bit_rate;

		// 设置编码器参数
		AVDictionary* param = nullptr;
		av_dict_set(&param, "preset", "superfast", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);

		// av_dict_set(&param, "rtsp_transport", "tcp", 0);
		// 打开输出音频编码器
		nRet = avcodec_open2(pOutputAudioCodecCtx, out_AudioCodec, &param);
		if (nRet < 0) {
			av_strerror(nRet, errBuf, ERRBUF_SIZE);
			av_log(nullptr, AV_LOG_ERROR, "Can't open audio encoder\n");
			avcodec_free_context(&pOutputAudioCodecCtx);
			return;
		}
		// 判断输出音频编码器上下文是否存在
		if (pOutputAudioCodecCtx) {
			int frame_index = 0;
			// 分配输出音频编码后的包
			AVPacket* pEncodePacket = av_packet_alloc();
			// 分配用于静音的音频帧
			AVFrame* pMuteFrame = av_frame_alloc();
			pMuteFrame->sample_rate = pEncodeCtx->sample_rate;
			pMuteFrame->channel_layout = pEncodeCtx->channel_layout;
			pMuteFrame->nb_samples = 1024; /*默认的sample大小:1024*/
			pMuteFrame->channels = 2;
			pMuteFrame->format = pEncodeCtx->sample_fmt;
			// 获取静音音频帧的数据缓冲区
			nRet = av_frame_get_buffer(pMuteFrame, 0);
			if (nRet < 0) {
				av_frame_free(&pMuteFrame);
				return;
			}
			// 将静音音频帧的数据设置为静音
			av_samples_set_silence(pMuteFrame->data, 0, pMuteFrame->nb_samples, pMuteFrame->channels, pEncodeCtx->sample_fmt);
			// 获取计算时间戳和时长的对象
			const CCalcPtsDur& calPts = std::get<0>(m_pairEncodeCtx);
			// 编码静音音频帧
			while (m_bRunning) {
				// 如果处于暂停状态，继续下一轮循环
				if (m_bPause) {
					continue;
				}
				// 使用互斥锁锁住推送线程的互斥锁
				QMutexLocker locker(&m_syncMutex);
				// 等待推送线程的信号，即等待音频推送线程通知可以开始编码下一帧静音音频
				m_syncWaitCondition.wait(&m_syncMutex);
				// 循环编码两帧静音音频
				for (int i = 0; i < 2; ++i) {
					// 克隆静音音频帧
					AVFrame* pCopyFrame = av_frame_clone(pMuteFrame);
					// 设置静音音频时间戳
					pCopyFrame->pts = calPts.GetAudioPts(frame_index, pEncodeCtx->sample_rate);
					// 发送静音音频帧进行编码
					nRet = avcodec_send_frame(pOutputAudioCodecCtx, pCopyFrame);
					if (nRet < 0) {
						av_strerror(nRet, errBuf, ERRBUF_SIZE);
						av_log(nullptr, AV_LOG_WARNING, "audio avcodec_receive_packet fail, %s\n", errBuf);
						av_frame_unref(pCopyFrame);
						av_packet_unref(pEncodePacket);
						continue;
					}
					// 接收编码后的音频包
					nRet = avcodec_receive_packet(pOutputAudioCodecCtx, pEncodePacket);
					if (nRet < 0) {
						av_strerror(nRet, errBuf, ERRBUF_SIZE);
						av_log(nullptr, AV_LOG_WARNING, "audio avcodec_receive_packet fail, %s\n", errBuf);
						av_frame_unref(pCopyFrame);
						av_packet_unref(pEncodePacket);
						continue;
					}
					// 设置音频包的流索引
					pEncodePacket->stream_index = m_nEncodeStreamIndex;
					// 使用互斥锁锁住编码队列的互斥锁
					m_encodeMutex.lock();
					// 如果队列已满，等待编码队列的信号
					if (m_encodePacketQueue.isFull()) {
						m_encodeWaitCondition.wait(&m_encodeMutex);
					}
					// 将编码后的音频包推入编码队列
					m_encodePacketQueue.push(av_packet_clone(pEncodePacket));
					// 通知编码线程可以开始编码下一帧
					m_encodeWaitCondition.wakeOne();
					// 释放静音音频帧的资源
					av_frame_unref(pCopyFrame);
					av_frame_free(&pCopyFrame);
					// 释放编码后的音频包资源
					av_packet_unref(pEncodePacket);
					// 递增帧索引
					frame_index++;
					m_encodeMutex.unlock();
				}
			}
			// 释放资源
			av_frame_free(&pMuteFrame);
			av_packet_free(&pEncodePacket);
			avcodec_close(pOutputAudioCodecCtx);
			avcodec_free_context(&pOutputAudioCodecCtx);
		}
	}
}

//CLXPushThread
CLXPushThread::CLXPushThread(AVFormatContext* pOutputFormatCtx, CircularQueue<AVPacket*>& videoPacketQueue,
                             CircularQueue<AVPacket*>& audioPacketQueue, QWaitCondition& videoWaitCondition, QMutex& videoMutex,
                             QWaitCondition& audioWaitCondition, QMutex& audioMutex, bool bPushVideo, bool bPushAudio, QObject* parent)
: QThread(parent), m_videoPacketQueue(videoPacketQueue), m_audioPacketQueue(audioPacketQueue), m_bPushVideo(bPushVideo),
  m_bPushAudio(bPushAudio), m_videoWaitCondition(videoWaitCondition), m_videoMutex(videoMutex), m_audioWaitCondition(audioWaitCondition),
  m_audioMutex(audioMutex), m_pOutputFormatCtx(pOutputFormatCtx) {
}

CLXPushThread::~CLXPushThread() {
	stop();
}

void CLXPushThread::pause() {
	QMutexLocker videoLocker(&m_videoMutex);
	QMutexLocker audioLocker(&m_audioMutex);
	m_bPause = true;
}

void CLXPushThread::resume() {
	QMutexLocker videoLocker(&m_videoMutex);
	QMutexLocker audioLocker(&m_audioMutex);
	m_bPause = false;
}

void CLXPushThread::stop() {
	m_bRunning = false;
	m_videoWaitCondition.wakeAll();
	m_audioWaitCondition.wakeAll();
	wait();
}

void CLXPushThread::run() {
	//QFile file(qApp->applicationDirPath() + "PTS_logger.txt");
	//file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Truncate);
	//QTextStream textStream(&file);
	char errBuf[ERRBUF_SIZE]{};
	while (m_bRunning) {
		if (m_bPause) {
			continue;
		}
		int nVideoPts = 0;
		if (m_bPushVideo) {
			QMutexLocker locker(&m_videoMutex);
			if (m_videoPacketQueue.isEmpty()) {
				m_videoWaitCondition.wakeOne();
				m_videoWaitCondition.wait(&m_videoMutex);
			}
			// 取出视频队列头的数据包
			AVPacket* pVideoPacket = m_videoPacketQueue.pop();
			if (pVideoPacket && pVideoPacket->pts >= 0) {
				//QString curTime = "\nvideo:\t" + QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()).toString("yyyy-MM-dd hh:mm:ss.zzz");
				//textStream << curTime << "\n\r";
				//OutputDebugStringA(curTime.toStdString().c_str());
				// qDebug() << "video" << pVideoPacket->pts;
				nVideoPts = pVideoPacket->pts; // NOLINT
				// 将视频数据包写入输出上下文
				int nRet = av_interleaved_write_frame(m_pOutputFormatCtx, pVideoPacket);
				if (nRet < 0) {
					av_strerror(nRet, errBuf, ERRBUF_SIZE);
					av_log(nullptr, AV_LOG_WARNING, "video push error, %s\n", errBuf);
					av_packet_unref(pVideoPacket);
					av_packet_free(&pVideoPacket);
					continue;
				}
				av_packet_unref(pVideoPacket);
				av_packet_free(&pVideoPacket);
			}
		}
		if (m_bPushAudio) {
			QMutexLocker locker(&m_audioMutex);
			if (m_audioPacketQueue.isEmpty()) {
				m_audioWaitCondition.wakeOne();
				m_audioWaitCondition.wait(&m_audioMutex);
			}
			// 从音频队列头部取出数据包
			AVPacket* pAudioPacket = m_audioPacketQueue.pop();
			while (pAudioPacket && pAudioPacket->buf && pAudioPacket->pts >= 0 && (!m_bPushVideo || pAudioPacket->pts <= nVideoPts)) {
				//QString curTime = "\naudio:\t" + QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()).toString("yyyy-MM-dd hh:mm:ss.zzz");
				//textStream << curTime << "\n\r";
				//OutputDebugStringA(curTime.toStdString().c_str());
				//qDebug() << "audio" << pAudioPacket->pts;
				// 写入输出上下文
				int nRet = av_interleaved_write_frame(m_pOutputFormatCtx, pAudioPacket);
				if (nRet < 0) {
					av_strerror(nRet, errBuf, ERRBUF_SIZE);
					av_log(nullptr, AV_LOG_WARNING, "audio push error, %s\n", errBuf);
					av_packet_unref(pAudioPacket);
					av_packet_free(&pAudioPacket);
					// 如果队列非空，且下一个音频数据包的 PTS 小于视频数据包的 PTS，继续读取下一个
					if (!m_audioPacketQueue.isEmpty()) {
						if (m_audioPacketQueue.first()->pts < nVideoPts) {
							pAudioPacket = m_audioPacketQueue.pop();
						}
					}
					continue;
				}
				av_packet_unref(pAudioPacket);
				av_packet_free(&pAudioPacket);
				// 如果队列非空，且下一个音频数据包的 PTS 小于视频数据包的 PTS，继续读取下一个
				if (!m_audioPacketQueue.isEmpty()) {
					if (!m_bPushVideo || m_audioPacketQueue.first()->pts < nVideoPts) {
						pAudioPacket = m_audioPacketQueue.pop();
					}
				}
			}
		}
	}
}

//CLXRecvThread
CLXRecvThread::CLXRecvThread(QString strPath, QObject* parent): QThread(parent), m_strPath(std::move(strPath)) {
	setAutoDelete(false);
	m_bRunning = true;
}

CLXRecvThread::~CLXRecvThread() = default;

void CLXRecvThread::stop() {
	m_bRunning = false;
	wait();
}

QString CLXRecvThread::path() const {
	return m_strPath;
}

// 返回视频和音频的编解码参数
QPair<AVCodecParameters*, AVCodecParameters*> CLXRecvThread::getContext() const {
	return qMakePair(m_pVideoCodecPara, m_pAudioCodecPara);
}

void CLXRecvThread::run() {
	int nRet = -1;
	char errBuf[ERRBUF_SIZE]{};
	//查找视频流和音频流
	// 保存媒体文件的格式相关信息
	nRet = avformat_open_input(&m_pFormatCtx, m_strPath.toStdString().c_str(), nullptr, nullptr);
	// 打开失败，记录错误信息，返回
	if (nRet < 0) {
		av_strerror(nRet, errBuf, ERRBUF_SIZE);
		av_log(nullptr, AV_LOG_ERROR, "Can't open input, %s\n", errBuf);
		return;
	}
	// 查找媒体文件中的流信息
	nRet = avformat_find_stream_info(m_pFormatCtx, nullptr);
	// 打开失败，记录错误信息，关闭输入文件，返回
	if (nRet < 0) {
		av_strerror(nRet, errBuf, ERRBUF_SIZE);
		av_log(nullptr, AV_LOG_ERROR, "Can't find stream info, %s\n", errBuf);
		avformat_close_input(&m_pFormatCtx);
		return;
	}
	// 打印格式信息
	av_dump_format(m_pFormatCtx, 0, m_strPath.toStdString().c_str(), 0);
	// 查找视频和音频流索引
	int nVideoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	int nAudioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	// 打开失败，记录错误信息，关闭输入文件，返回
	if (-1 == nVideoIndex && -1 == nAudioIndex) {
		av_log(nullptr, AV_LOG_ERROR, "Can't find video and audio stream, %s\n");
		avformat_close_input(&m_pFormatCtx);
		return;
	}
	// 复制视频和音频编解码参数
	if (AVERROR_STREAM_NOT_FOUND != nVideoIndex) {
		m_pVideoCodecPara = avcodec_parameters_alloc();
		avcodec_parameters_copy(m_pVideoCodecPara, m_pFormatCtx->streams[nVideoIndex]->codecpar);
	}
	if (AVERROR_STREAM_NOT_FOUND != nAudioIndex) {
		m_pAudioCodecPara = avcodec_parameters_alloc();
		avcodec_parameters_copy(m_pAudioCodecPara, m_pFormatCtx->streams[nAudioIndex]->codecpar);
	}
	avformat_close_input(&m_pFormatCtx);
}
