#pragma once

// Must be included before any Qt header
#include "glew.h"

#include <atomic>
#include <QOpenGLWidget>

#include "uvframe.hpp"
#include "uvgl.hpp"
#include "uvgui.hpp"

void bindTexture(GLTexture* tex, QImage* img);

class CUVGLWidget : public QOpenGLWidget {
public:
	explicit CUVGLWidget(QWidget* parent = nullptr);
	~CUVGLWidget() override;

	// ratio = 0 means spread
	void setAspectRatio(double ratio);

	void drawFrame(CUVFrame* pFrame);
	void drawTexture(CUVRect rc, GLTexture* tex);
	void drawRect(CUVRect rc, CUVColor clr, int line_width = 1, bool bFill = false);
	void drawText(QPoint lb, const char* text, int fontsize, QColor clr);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void setVertices(double ratio);
	void setVertices(QRect rc);

	static void loadYUVShader();
	void initVAO();
	void initYUV();

	void drawYUV(CUVFrame* pFrame);

	static std::atomic_flag s_glew_init;
	static GLuint prog_yuv;
	static GLuint texUniformY;
	static GLuint texUniformU;
	static GLuint texUniformV;
	GLuint tex_yuv[3]{};

	double aspect_ratio{};
	GLfloat vertices[8]{};
	GLfloat textures[8]{};

	// NOTE: QPainter used 3 VertexAttribArray
	enum VER_ATTR {
		VER_ATTR_VER = 3,
		VER_ATTR_TEX,
	};
};
