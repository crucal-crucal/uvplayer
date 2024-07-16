#include "uvvideownd.hpp"

#ifdef Q_OS_WIN
#include <windows.h> // NOLINT
#include <sysinfoapi.h>
#endif

CUVVideoWnd::CUVVideoWnd(QWidget*) {
	fps = 0;
	framecnt = 0;
	tick = 0;
	draw_time = true;
	draw_fps = true;
	draw_resolution = true;
}

void CUVVideoWnd::calcFPS() {
	if (GetTickCount() - tick > 1000) {
		fps = framecnt;
		framecnt = 0;
		tick = GetTickCount();
	} else {
		++framecnt;
	}
}
