#pragma once

#include <atomic>

#include "uvthread.hpp"
#include "video/uvvideoplayer.hpp"

extern "C" {
#include <libavutil/time.h>
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
}

class CUVFFPlayer final : public CUVVideoPlayer, public CUVThread {
public:
	CUVFFPlayer();

	~CUVFFPlayer() override;

	int start() override {
		quit = 0;
		return CUVThread::start();
	}

	int stop() override {
		quit = 1;
		return CUVThread::stop();
	}

	int pause() override { return CUVThread::pause(); }

	int resume() override { return CUVThread::resume(); }

	int seek(int64_t ms) override;

private:
	bool doPrepare() override;
	void doTask() override;
	bool doFinish() override;
	int open();
	int close();

public:
	int64_t block_starttime{};
	int64_t block_timeout{};
	int quit{};

private:
	static std::atomic_flag s_ffmpeg_init;

	AVDictionary* fmt_opts{ nullptr };
	AVDictionary* codec_opts{ nullptr };

	AVFormatContext* fmt_ctx{ nullptr };

	AVCodecContext* video_codec_ctx{ nullptr };
	AVPacket* video_packet{ nullptr };
	AVFrame* video_frame{ nullptr };

#if 0
    AVCodecContext* audio_codec_ctx{ nullptr };
    AVPacket* audio_packet{ nullptr };
    AVFrame* audio_frame{ nullptr };

    AVCodecContext* subtitle_codec_ctx{ nullptr };
    AVPacket* subtitle_packet{ nullptr };
    AVSubtitle* subtitle{ nullptr };
#endif

	int video_stream_index{};
	int audio_stream_index{};
	int subtitle_stream_index{};

	int video_time_base_num{};
	int video_time_base_den{};

	// for scale
	AVPixelFormat src_pix_fmt{};
	AVPixelFormat dst_pix_fmt{};
	SwsContext* sws_ctx{ nullptr };
	uint8_t* data[4]{ nullptr };
	int linesize[4]{};
	CUVFrame m_frame{};
};
