#pragma once

#include <atomic>

#include "uvthread.hpp"
#include "interface/uvvideoplayer.hpp"
#include "util/uvffmpeg_util.hpp"

#define AV_DEFAULT_LOGLEVEL AV_LOG_TRACE

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
	static void logCallBack(void* ptr, int level, const char* fmt, va_list vl);
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
	static FILE* m_pLogFile;
	static QString ff_logPath;
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
	AVFrame* pFrame{};
	int linesize[4]{};
	CUVFrame m_frame{};
};
