#include "uvglwnd.hpp"

#include <sstream>
#include <iomanip>
#include <QPainter>

CUVGLWnd::CUVGLWnd(QWidget* parent) : CUVVideoWnd(parent), CUVGLWidget(parent) {
}

CUVGLWnd::~CUVGLWnd() = default;

void CUVGLWnd::setgeometry(const QRect& rc) {
	setGeometry(rc);
}

void CUVGLWnd::Update() {
	update();
}

/**
 * @note: 帧绘制、时间、FPS和分辨率的绘制
 */
void CUVGLWnd::paintGL() {
	calcFPS();
	CUVGLWidget::paintGL();

	if (last_frame.isNull()) {
//		 QPoint pt = rect().center() - QPoint(80, -10);
//		 drawText(pt, "NO VIDEO", 16, Qt::white);
//
//		 QPainter painter(this);
//		 QPixmap pixmap(":/image/media_bk.png");
//		 int w = pixmap.width();
//		 int h = pixmap.height();
//		 QRect rc((width() - w) / 2, (height() - h) / 2, w, h);
//		 painter.drawPixmap(rc, pixmap);
	} else {
		drawFrame(&last_frame);
		if (draw_time) {
			drawTime();
		}
		if (draw_fps) {
			drawFPS();
		}
		if (draw_resolution) {
			drawResolution();
		}
	}
}

void CUVGLWnd::drawTime() {
	std::ostringstream oss;
	int sec = last_frame.ts / 1000; // NOLINT
	int m = sec / 60;
	const int s = sec % 60;
	const int h = m / 60;
	m = m % 60;
	oss << std::setfill('0') << std::setw(2) << h << ":"
			<< std::setfill('0') << std::setw(2) << m << ":"
			<< std::setfill('0') << std::setw(2) << s;
	const std::string szTime = oss.str();
	// Left Top
	constexpr QPoint pt(10, 40);
	drawText(pt, szTime.c_str(), 14, Qt::red);
}

void CUVGLWnd::drawFPS() {
	std::ostringstream oss;
	oss << "FPS:" << fps;
	const std::string szFPS = oss.str();
	// Right Top
	const QPoint pt(width() - 100, 40);
	drawText(pt, szFPS.c_str(), 14, Qt::red);
}

void CUVGLWnd::drawResolution() {
	std::ostringstream oss;
	oss << last_frame.w << " X " << last_frame.h;
	const std::string szResolution = oss.str();
	// Left Bottom
	const QPoint pt(10, height() - 10);
	drawText(pt, szResolution.c_str(), 14, Qt::red);
}
