#pragma once

#include <deque>
#include <mutex>
#include <QMutexLocker>

#include "uvbuf.hpp"

class CUVFrame {
public:
	CUVBuf buf{};
	int w{}, h{}, bpp{}, type{};
	uint64_t ts{};
	int64_t useridx{};
	void* userdata{};

	CUVFrame() {
		w = h = bpp = type = 0;
		ts = 0;
		useridx = -1;
		userdata = nullptr;
	}

	[[nodiscard]] bool isNull() {
		return w == 0 || h == 0 || buf.isNull();
	}

	void copy(const CUVFrame& rhs) {
		w = rhs.w;
		h = rhs.h;
		bpp = rhs.bpp;
		type = rhs.type;
		ts = rhs.ts;
		useridx = rhs.useridx;
		userdata = rhs.userdata;
		buf.copy(rhs.buf.base, rhs.buf.len);
	}
};

typedef struct frame_info_s {
	int w;
	int h;
	int type;
	int bpp;
} FrameInfo;

typedef struct frame_stats_s {
	int push_cnt;
	int pop_cnt;

	int push_ok_cnt;
	int pop_ok_cnt;

	frame_stats_s() {
		push_cnt = pop_cnt = push_ok_cnt = pop_ok_cnt = 0;
	}
} FrameStats;

#define DEFAULT_FRAME_CACHENUM  10

class CUVFrameBuf final : public CUVRingBuf {
public:
	enum CacheFullPolicy {
		SQUEEZE,
		DISCARD,
	} policy;

	CUVFrameBuf() {
		cache_num = DEFAULT_FRAME_CACHENUM;
		policy = SQUEEZE;
	}

	void setCache(const int num) { cache_num = num; }

	void setPolicy(const CacheFullPolicy& policy) { this->policy = policy; }

	int push(CUVFrame* pFrame);
	int pop(CUVFrame* pFrame);
	void clear() override;

	int cache_num;
	FrameStats frame_stats;
	FrameInfo frame_info{};
	std::deque<CUVFrame> frames;

	QMutex mutex;
};
