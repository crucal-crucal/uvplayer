#pragma once


#include "uvvideownd.hpp"
#include "uvglwidget.hpp"

class CUVGLWnd : public CUVVideoWnd, CUVGLWidget {
public:
	explicit CUVGLWnd(QWidget* parent = nullptr);
	~CUVGLWnd() override;

	void setgeometry(const QRect& rc) override;
	void Update() override;

protected:
	void paintGL() override;
	void drawTime();
	void drawFPS();
	void drawResolution();
};
