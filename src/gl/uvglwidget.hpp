#pragma once

// Must be included before any Qt header
#include "GL/glew.h"

#include <atomic>
#include <QOpenGLWidget>

#include "util/uvframe.hpp"
#include "util/uvgl.hpp"
#include "util/uvgui.hpp"

void bindTexture(GLTexture* tex, QImage* img);

class CUVGLWidget : public QOpenGLWidget {
public:
	explicit CUVGLWidget(QWidget* parent = nullptr);
	~CUVGLWidget() override;

	// ratio = 0 means spread
	void setAspectRatio(double ratio);

	void drawFrame(const CUVFrame* pFrame) const;
	void drawTexture(const CUVRect& rc, const GLTexture* tex) const;
	void drawRect(const CUVRect& rc, CUVColor clr, int line_width = 1, bool bFill = false) const;
	void drawText(const QPoint& lb, const char* text, int fontsize, const QColor& clr);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void setVertices(double ratio);
	void setVertices(const QRect& rc);

	static void loadYUVShader();
	void initVAO() const;
	void initYUV();
	static void checkShaderCompileStatus(GLuint shader, const std::string& name);
	static void checkProgramLinkStatus(GLuint program);
	void drawYUV(const CUVFrame* pFrame) const;

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
