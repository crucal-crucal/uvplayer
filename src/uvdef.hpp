#pragma once

#include <QRect>
#include <QtGlobal>
#include <QApplication>
#include <QDesktopWidget>

#ifdef Q_OS_WIN
#include <Windows.h>
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
