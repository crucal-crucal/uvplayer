#include "uvvideownd.hpp"

#include "conf/uvconf.hpp"

#ifdef Q_OS_WIN
#include <windows.h> // NOLINT
#include <sysinfoapi.h>
#endif

CUVVideoWnd::CUVVideoWnd(QWidget*) {
	fps = 0;
	framecnt = 0;
	tick = 0;
	draw_time = g_confile->get<bool>("draw_time", "ui");
	draw_fps = g_confile->get<bool>("draw_fps", "ui");
	draw_resolution = g_confile->get<bool>("draw_resolution", "ui");
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
