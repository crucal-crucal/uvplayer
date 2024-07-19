#pragma once

#include <QMetaType>

typedef enum {
	PIX_FMT_NONE = 0,

	PIX_FMT_GRAY, // YYYYYYYY

	PIX_FMT_YUV_FIRST        = 100,
	PIX_FMT_YUV_PLANAR_FIRST = 200,
	PIX_FMT_IYUV, // YYYYYYYYUUVV
	PIX_FMT_YV12, // YYYYVVYYVVUU
	PIX_FMT_NV12, // YYUVYYYYUVUV
	PIX_FMT_NV21, // YYVUYYYYVUVU
	PIX_FMT_YUV_PLANAR_LAST,
	PIX_FMT_YUV_PACKED_FIRST = 300,
	PIX_FMT_YUY2, // YUYVYUYV
	PIX_FMT_YVYU, // YVYUYVYU
	PIX_FMT_UYVY, // UYVYUYVY
	PIX_FMT_YUV_PACKED_LAST,
	PIX_FMT_YUV_LAST,

	PIX_FMT_RGB_FIRST = 400,
	PIX_FMT_RGB,  // RGBRGB
	PIX_FMT_BGR,  // BGRBGR
	PIX_FMT_RGBA, // RGBARGBA
	PIX_FMT_BGRA, // BGRABGRA
	PIX_FMT_ARGB, // ARGBARGB
	PIX_FMT_ABGR, // ABGRABGR
	PIX_FMT_RGB_LAST,
} pix_fmt_e;

static bool pix_fmt_is_yuv(const int type) { // NOLINT
	return type > PIX_FMT_YUV_FIRST && type < PIX_FMT_YUV_LAST;
}

static bool pix_fmt_is_planar_yuv(const int type) {
	return type > PIX_FMT_YUV_PLANAR_FIRST && type < PIX_FMT_YUV_PLANAR_LAST;
}

static bool pix_fmt_is_packet_yuv(const int type) {
	return type > PIX_FMT_YUV_PACKED_FIRST && type < PIX_FMT_YUV_PACKED_LAST;
}

static bool pix_fmt_is_rgb(const int type) {
	return type > PIX_FMT_RGB_FIRST && type < PIX_FMT_RGB_LAST;
}

static int pix_fmt_bpp(const int type) {
	if (pix_fmt_is_yuv(type)) {
		return 12;
	}
	switch (type) {
		case PIX_FMT_RGB:
		case PIX_FMT_BGR:
			return 24;
		case PIX_FMT_RGBA:
		case PIX_FMT_BGRA:
		case PIX_FMT_ARGB:
		case PIX_FMT_ABGR:
			return 32;
		case PIX_FMT_GRAY:
			return 8;
		default: break;
	}
	return 0;
}

typedef enum {
	MEDIA_TYPE_FILE = 0,
	MEDIA_TYPE_NETWORK,
	MEDIA_TYPE_CAPTURE,
	MEDIA_TYPE_NB
} media_type_e;

#define DEFAULT_MEDIA_TYPE  MEDIA_TYPE_CAPTURE
#define MEDIA_TYPE_NONE     MEDIA_TYPE_NB

typedef enum {
	AVSTREAM_TYPE_VIDEO,
	AVSTREAM_TYPE_AUDIO,
	AVSTREAM_TYPE_SUBTITLE,
	AVSTREAM_TYPE_NB
} avstream_type_e;

typedef enum {
	ASPECT_FULL,           // 100%
	ASPECT_PERCENT,        // 50%
	ASPECT_ORIGINAL_RATIO, // w:h
	ASPECT_ORIGINAL_SIZE,  // wxh
	ASPECT_CUSTOM_RATIO,   // 4:3 16:9 ...
	ASPECT_CUSTOM_SIZE,    // 1280x720 640*480 ...
} aspect_ratio_e;

#define DEFAULT_ASPECT_RATIO    ASPECT_FULL

typedef struct aspect_ratio_s {
	aspect_ratio_e type{};
	int w{}, h{};
} aspect_ratio_t;

Q_DECLARE_METATYPE(aspect_ratio_t)

//----------------video-----------------------

#define ERRBUF_SIZE  4096

// 进程状态
enum eProcessState {
	eProcessState_Unknown = -1, // 未知
	eProcessState_Online,       // 在线
	eProcessState_Online_Push,  // 在线推送
	eProcessState_Offline,      // 离线
};

// 推送流信息
struct LXPushStreamInfo {
	// 推送流类型
	enum PushStream {
		Video = 0x1,
		Audio = 0x2
	};

	QString strAddress{ "" };                                     // 推送地址
	float fVideoBitRate{ 0.0 };                                   // 视频比特率
	PushStream eStream{ static_cast<PushStream>(Video | Audio) }; // 推流类型
	int nWidth{ 0 };                                              // 视频宽度
	int nHeight{ 0 };                                             // 视频高度
	int nFrameRateNum{ 1 };                                       // 视频帧率分子
	int nFrameRateDen{ 25 };                                      // 视频帧率分母
	int nColorDepth{ 24 };                                        // 视频颜色深度
	int nAudioBitRate{ 0 };                                       // 音频比特率
	int nAudioSampleRate{ 0 };                                    // 音频采样率
};
