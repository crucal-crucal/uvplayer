#pragma once

extern "C" {
#include <libavutil/time.h>
#include "libavutil/imgutils.h"
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
#include "libavutil/channel_layout.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}