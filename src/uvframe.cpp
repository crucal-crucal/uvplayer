#include "uvframe.hpp"

int CUVFrameBuf::push(CUVFrame* pFrame) {
	if (pFrame->isNull())
		return -10;

	frame_stats.push_cnt++;

	std::lock_guard<std::mutex> locker(mutex);

	if (frames.size() >= (size_t) cache_num) {
		// std::cout << ("frame cache full!");
		if (policy == CUVFrameBuf::DISCARD) {
			return -20; // note: cache full, discard frame
		}

		CUVFrame& frame = frames.front();
		frames.pop_front();
		free(frame.buf.len);
		if (frame.userdata) {
			// std::cout << ("free userdata");
			::free(frame.userdata);
			frame.userdata = nullptr;
		}
	}

	int ret = 0;
	if (isNull()) {
		resize(pFrame->buf.len * cache_num);
		ret = 1; // note: first push

		frame_info.w = pFrame->w;
		frame_info.h = pFrame->h;
		frame_info.type = pFrame->type;
		frame_info.bpp = pFrame->bpp;
	}

	CUVFrame frame;
	frame.buf.base = alloc(pFrame->buf.len);
	frame.buf.len = pFrame->buf.len;
	frame.copy(*pFrame);
	frames.push_back(frame);
	frame_stats.push_ok_cnt++;

	return ret;
}

int CUVFrameBuf::pop(CUVFrame* pFrame) {
	frame_stats.pop_cnt++;

	std::lock_guard<std::mutex> locker(mutex);

	if (isNull())
		return -10;

	if (frames.empty()) {
		// std::cout << ("frame cache empty!");
		return -20;
	}

	CUVFrame& frame = frames.front();
	frames.pop_front();
	free(frame.buf.len);

	if (frame.isNull())
		return -30;

	pFrame->copy(frame);
	frame_stats.pop_ok_cnt++;

	return 0;
}

void CUVFrameBuf::clear() {
	std::lock_guard<std::mutex> locker(mutex);
	frames.clear();
	CUVRingBuf::clear();
}
