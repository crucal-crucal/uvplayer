#pragma once

typedef unsigned int CUVColor; // 0xAARRGGBB

#define CLR_B(c)    (c         & 0xff)
#define CLR_G(c)    ((c >> 8)  & 0xff)
#define CLR_R(c)    ((c >> 16) & 0xff)
#define CLR_A(c)    ((c >> 24) & 0xff)
#define ARGB(a, r, g, b) MAKE_FOURCC(a, r, g, b)

typedef struct cuvpoint_s {
	int x;
	int y;

#ifdef __cplusplus
	cuvpoint_s() {
		x = y = 0;
	}

	cuvpoint_s(const int x, const int y) {
		this->x = x;
		this->y = y;
	}
#endif
} CUVPoint;

typedef struct cuvsize_s {
	int w;
	int h;

#ifdef __cplusplus
	cuvsize_s() {
		w = h = 0;
	}

	cuvsize_s(const int w, const int h) {
		this->w = w;
		this->h = h;
	}
#endif
} CUVSize;

typedef struct cuvrect_s {
	int x;
	int y;
	int w;
	int h;

#ifdef __cplusplus
	cuvrect_s() {
		x = y = w = h = 0;
	}

	cuvrect_s(const int x, const int y, const int w, const int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}

	[[nodiscard]] int left() const { return x; }

	[[nodiscard]] int right() const { return x + w; }

	[[nodiscard]] int top() const { return y; }

	[[nodiscard]] int bottom() const { return y + h; }
#endif
} CUVRect;
