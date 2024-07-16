#pragma once

#include "frame/uvframe.hpp"
#include "global/uvmedia.hpp"

#define DEFAULT_FPS         25
#define DEFAULT_FRAME_CACHE 5

enum {
	SOFTWARE_DECODE        = 1,
	UVARDWARE_DECODE_QSV   = 2,
	UVARDWARE_DECODE_CUVID = 3,
};

#define DEFAULT_DECODE_MODE UVARDWARE_DECODE_CUVID

enum uvplayer_event_e {
	UVPLAYER_OPEN_FAILED,
	UVPLAYER_OPENED,
	UVPLAYER_EOF,
	UVPLAYER_CLOSED,
	UVPLAYER_ERROR,
};

typedef int (*uvplayer_event_cb)(uvplayer_event_e e, void* userdata);


class CUVVideoPlayer {
public:
	CUVVideoPlayer() {
		set_frame_cache(DEFAULT_FRAME_CACHE);
		fps = DEFAULT_FPS;
		decode_mode = DEFAULT_DECODE_MODE;

		width = 0;
		height = 0;
		duration = 0;
		start_time = 0;
		eof = 0;
		error = 0;
		event_cb = nullptr;
	}

	virtual ~CUVVideoPlayer() = default;

	virtual int start() = 0;
	virtual int stop() = 0;
	virtual int pause() = 0;
	virtual int resume() = 0;

	virtual int seek(int64_t ms) {
		return 0;
	}

	void set_media(const CUVMedia& media) {
		this->media = media;
	}

	void set_decode_mode(const int mode) {
		decode_mode = mode;
	}

	[[nodiscard]] FrameStats get_frame_stats() const {
		return frame_buf.frame_stats;
	}

	[[nodiscard]] FrameInfo get_frame_info() const {
		return frame_buf.frame_info;
	}

	void set_frame_cache(const int cache) {
		frame_buf.setCache(cache);
	}

	void clear_frame_cache() {
		frame_buf.clear();
	}

	int push_frame(CUVFrame* pFrame) {
		return frame_buf.push(pFrame);
	}

	int pop_frame(CUVFrame* pFrame) {
		return frame_buf.pop(pFrame);
	}

	void set_event_callback(const uvplayer_event_cb cb, void* userdata) {
		event_cb = cb;
		event_cb_userdata = userdata;
	}

	void event_callback(uvplayer_event_e e) const {
		if (event_cb) {
			event_cb(e, event_cb_userdata);
		}
	}

	CUVMedia media{};
	int fps{};
	int decode_mode{};
	int real_decode_mode{};

	int32_t width{};
	int32_t height{};

	int64_t duration{};   // ms
	int64_t start_time{}; // ms
	int eof{};
	int error{};

protected:
	CUVFrameBuf frame_buf;
	uvplayer_event_cb event_cb;
	void* event_cb_userdata{};
};
