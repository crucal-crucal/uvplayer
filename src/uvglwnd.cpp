#include "uvglwnd.hpp"

CUVGLWnd::CUVGLWnd(QWidget* parent) : CUVVideoWnd(parent), CUVGLWidget(parent) {
}

CUVGLWnd::~CUVGLWnd() = default;

void CUVGLWnd::setgeometry(const QRect& rc) {
	setGeometry(rc);
}

void CUVGLWnd::Update() {
	update();
}

void CUVGLWnd::paintGL() {
	calcFPS();
	CUVGLWidget::paintGL();

	if (last_frame.isNull()) {
		/*
		QPoint pt = rect().center() - QPoint(80, -10);
		drawText(pt, "NO VIDEO", 16, Qt::white);

		QPainter painter(this);
		QPixmap pixmap(":/image/media_bk.png");
		int w = pixmap.width();
		int h = pixmap.height();
		QRect rc((width()-w)/2, (height()-h)/2, w, h);
		painter.drawPixmap(rc, pixmap);
		*/
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
	char szTime[12];
	int sec = last_frame.ts / 1000;
	int h, m, s;
	m = sec / 60;
	s = sec % 60;
	h = m / 60;
	m = m % 60;
	sprintf(szTime, "%02d:%02d:%02d", h, m, s);
	// Left Top
	QPoint pt(10, 40);
	drawText(pt, szTime, 14, Qt::white);
}

void CUVGLWnd::drawFPS() {
	char szFPS[16];
	sprintf(szFPS, "FPS:%d", fps);
	// Right Top
	QPoint pt(width() - 100, 40);
	drawText(pt, szFPS, 14, Qt::blue);
}

void CUVGLWnd::drawResolution() {
	char szResolution[16];
	sprintf(szResolution, "%d X %d", last_frame.w, last_frame.h);
	// Left Bottom
	QPoint pt(10, height() - 10);
	drawText(pt, szResolution, 14, Qt::blue);
}
