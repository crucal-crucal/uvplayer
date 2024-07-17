#pragma once

#include <QRect>
#include <QtGlobal>
#include <QApplication>
#include <QDesktopWidget>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

//----------------Be common-----------------------

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(n)  ((n) > 0 ? (n) : -(n))
#endif

#ifndef NABS
#define NABS(n) ((n) < 0 ? (n) : -(n))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#endif

#ifndef BITSET
#define BITSET(p, n) (*(p) |= (1u << (n)))
#endif

#ifndef BITCLR
#define BITCLR(p, n) (*(p) &= ~(1u << (n)))
#endif

#ifndef BITGET
#define BITGET(i, n) ((i) & (1u << (n)))
#endif

#ifndef CR
#define CR      '\r'
#endif

#ifndef LF
#define LF      '\n'
#endif

#ifndef CRLF
#define CRLF    "\r\n"
#endif

#define FLOAT_PRECISION     1e-6
#define FLOAT_EQUAL_ZERO(f) (ABS(f) < FLOAT_PRECISION)

#ifndef INFINITE
#define INFINITE    (uint32_t)-1
#endif

/*
ASCII:
[0, 0x20)    control-charaters
[0x20, 0x7F) printable-charaters

0x0A => LF
0x0D => CR
0x20 => SPACE
0x7F => DEL

[0x09, 0x0D] => \t\n\v\f\r
[0x30, 0x39] => 0~9
[0x41, 0x5A] => A~Z
[0x61, 0x7A] => a~z
*/
#ifndef IS_ALPHA
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#endif

#ifndef IS_NUM
#define IS_NUM(c)   ((c) >= '0' && (c) <= '9')
#endif

#ifndef IS_ALPHANUM
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_NUM(c))
#endif

#ifndef IS_CNTRL
#define IS_CNTRL(c) ((c) >= 0 && (c) < 0x20)
#endif

#ifndef IS_GRAPH
#define IS_GRAPH(c) ((c) >= 0x20 && (c) < 0x7F)
#endif

#ifndef IS_HEX
#define IS_HEX(c) (IS_NUM(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#endif

#ifndef IS_LOWER
#define IS_LOWER(c) (((c) >= 'a' && (c) <= 'z'))
#endif

#ifndef IS_UPPER
#define IS_UPPER(c) (((c) >= 'A' && (c) <= 'Z'))
#endif

#ifndef LOWER
#define LOWER(c)    ((c) | 0x20)
#endif

#ifndef UPPER
#define UPPER(c)    ((c) & ~0x20)
#endif

#ifndef SAFE_ALLOC
#define SAFE_ALLOC(p, size) \
do { \
void* ptr = malloc(size); \
if (!ptr) { \
qFatal("Out of memory"); \
exit(-1); \
}\
memset(ptr, 0, size); \
*(void**)&(p) = ptr; \
} while(0)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p) do { if (p) { free(p); (p) = nullptr; } } while(0)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) do { if (p) { delete (p); (p) = nullptr; } } while(0)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) do { if (p) { delete[] (p); (p) = nullptr; } } while(0)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) do { if (p) { (p)->release(); (p) = nullptr; } } while(0)
#endif


//----------------mutable-----------------------
#define LSIDE_VISIBLE       false
#define RSIDE_VISIBLE       false

#define LSIDE_MIN_WIDTH     200
#define LSIDE_MAX_WIDTH     400

#define MV_MIN_WIDTH        640
#define MV_MIN_HEIGHT       480

#define RSIDE_MIN_WIDTH     200
#define RSIDE_MAX_WIDTH     400

#define WITH_MV_STYLE       1
#define MV_STYLE_ROW        3
#define MV_STYLE_COL        3

#define DESKTOP_WIDTH   qApp->desktop()->availableGeometry().width()
#define DESKTOP_HEIGHT  qApp->desktop()->availableGeometry().height()

inline QRect adjustRect(QPoint pt1, QPoint pt2) {
	int x1 = qMin(pt1.x(), pt2.x());
	int x2 = qMax(pt1.x(), pt2.x());
	int y1 = qMin(pt1.y(), pt2.y());
	int y2 = qMax(pt1.y(), pt2.y());
	return QRect(QPoint(x1, y1), QPoint(x2, y2));
}

inline void centerWidget(QWidget* wdg) {
	int w = wdg->width();
	int h = wdg->height();
	if (w < DESKTOP_WIDTH && h < DESKTOP_HEIGHT) {
		wdg->setGeometry((DESKTOP_WIDTH - w) / 2, (DESKTOP_HEIGHT - h) / 2, w, h);
	}
}

inline unsigned int gettick() {
#ifdef Q_OS_WIN
	return GetTickCount();
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

//----------------scope-----------------------
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
