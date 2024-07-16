#include "uvglwidget.hpp"

#include "def/avdef.hpp"

#include <QPainter>
#include <array>
#include <sstream>
#include <iomanip>

static int glPixFmt(const int type) {
	switch (type) {
		case PIX_FMT_BGR: return GL_BGR;
		case PIX_FMT_RGB: return GL_RGB;
		case PIX_FMT_BGRA: return GL_BGRA;
		case PIX_FMT_RGBA: return GL_RGBA;
		default: return -1;
	}
}

void bindTexture(GLTexture* tex, QImage* img) {
	if (img->format() != QImage::Format_ARGB32)
		return;

	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	tex->frame.w = img->width();
	tex->frame.h = img->height();
	tex->frame.type = GL_BGRA;
	tex->frame.bpp = img->bitPlaneCount();
	// gluBuild2DMipmaps(GL_TEXTURE_2D, tex->frame.bpp/8, tex->frame.w, tex->frame.h, tex->frame.type, GL_UNSIGNED_BYTE, img->bits());
	glTexImage2D(GL_TEXTURE_2D, 0, tex->frame.bpp / 8, tex->frame.w, tex->frame.h, 0, tex->frame.type, GL_UNSIGNED_BYTE, img->bits());
}

/**
 * CUVGLWidget static member initialization
 */
std::atomic_flag CUVGLWidget::s_glew_init = ATOMIC_FLAG_INIT;
GLuint CUVGLWidget::prog_yuv{};
GLuint CUVGLWidget::texUniformY{};
GLuint CUVGLWidget::texUniformU{};
GLuint CUVGLWidget::texUniformV{};

/**
 * class CUVGLWidget
 * @param parent
 */
CUVGLWidget::CUVGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
	aspect_ratio = 0.0;
	setVertices(1.0);

	std::array<GLfloat, 8> tmp = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	};
	std::copy(tmp.begin(), tmp.end(), textures);
}

CUVGLWidget::~CUVGLWidget() = default;

void CUVGLWidget::setAspectRatio(const double ratio) {
	aspect_ratio = ratio;
	if (ABS(ratio) < 1e-6) {
		setVertices(1.0);
	} else {
		setVertices(static_cast<double>(height()) / width() * aspect_ratio);
	}
}

void CUVGLWidget::drawFrame(const CUVFrame* pFrame) const {
	if (pFrame->type == PIX_FMT_IYUV || pFrame->type == PIX_FMT_YV12) {
		drawYUV(pFrame);
	} else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glRasterPos3f(-1.0f, 1.0f, 0);
		glPixelZoom(static_cast<GLfloat>(width()) / static_cast<GLfloat>(pFrame->w), static_cast<GLfloat>(-height()) / static_cast<GLfloat>(pFrame->h));
		glDrawPixels(pFrame->w, pFrame->h, glPixFmt(pFrame->type), GL_UNSIGNED_BYTE, pFrame->buf.base);
	}
}

void CUVGLWidget::drawTexture(const CUVRect& rc, const GLTexture* tex) const {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width(), height(), 0.0, -1.0, 1.0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->id);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);
	glVertex2i(rc.left(), rc.top());
	glTexCoord2d(1, 0);
	glVertex2i(rc.right(), rc.top());
	glTexCoord2d(1, 1);
	glVertex2i(rc.right(), rc.bottom());
	glTexCoord2d(0, 1);
	glVertex2i(rc.left(), rc.bottom());
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void CUVGLWidget::drawRect(const CUVRect& rc, const CUVColor clr, const int line_width, const bool bFill) const {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width(), height(), 0.0, -1.0, 1.0);

	glPolygonMode(GL_FRONT_AND_BACK, bFill ? GL_FILL : GL_LINE);

	glLineWidth(static_cast<GLfloat>(line_width));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ub(CLR_R(clr), CLR_G(clr), CLR_B(clr), CLR_A(clr));
	glRecti(rc.left(), rc.top(), rc.right(), rc.bottom());
	glColor4ub(255, 255, 255, 255);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_BLEND);
}

void CUVGLWidget::drawText(const QPoint& lb, const char* text, const int fontsize, const QColor& clr) {
	QPainter painter(this);
	QFont font = painter.font();
	font.setPointSize(fontsize);
	painter.setFont(font);
	painter.setPen(clr);
	painter.drawText(lb, text);
}

void CUVGLWidget::initializeGL() {
	if (!s_glew_init.test_and_set()) {
		if (glewInit() != GLEW_OK) {
			s_glew_init.clear();
			qFatal("glewInit failed");
			return;
		}
	}
	initVAO();
	loadYUVShader();
	initYUV();
}

void CUVGLWidget::resizeGL(const int w, const int h) {
	glViewport(0, 0, w, h);
	setAspectRatio(aspect_ratio);
}

void CUVGLWidget::paintGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void CUVGLWidget::setVertices(const double ratio) {
	GLfloat w = 1.0, h = 1.0;
	if (ratio < 1.0) {
		w = static_cast<GLfloat>(ratio);
	} else {
		h = 1.0f / static_cast<GLfloat>(ratio);
	}

	std::array<GLfloat, 8> tmp = {
		-w, -h,
		w, -h,
		-w, h,
		w, h
	};
	std::copy(tmp.begin(), tmp.end(), vertices);
}

void CUVGLWidget::setVertices(const QRect& rc) {
	const auto wnd_w = static_cast<GLfloat>(width());
	const auto wnd_h = static_cast<GLfloat>(height());
	if (wnd_w <= 0 || wnd_h <= 0) {
		return;
	}
	const GLfloat left = static_cast<GLfloat>(rc.left()) * 2 / wnd_w - 1;
	const GLfloat right = static_cast<GLfloat>(rc.right() + 1) * 2 / wnd_w - 1;
	const GLfloat top = 1 - static_cast<GLfloat>(rc.top()) * 2 / wnd_h;
	const GLfloat bottom = 1 - static_cast<GLfloat>(rc.bottom() + 1) * 2 / wnd_h;

	qDebug("l=%d r=%d t=%d b=%d", rc.left(), rc.right(), rc.top(), rc.bottom());
	qDebug("l=%f r=%f t=%f b=%f", left, right, top, bottom);

	std::array<GLfloat, 8> tmp = {
		left, bottom,
		right, bottom,
		left, top,
		right, top
	};
	std::copy(tmp.begin(), tmp.end(), vertices);
}

void CUVGLWidget::loadYUVShader() {
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	const auto szVS = R"(
    attribute vec4 verIn;
    attribute vec2 texIn;
    varying vec2 texOut;

    void main(){
        gl_Position = verIn;
        texOut = texIn;
    }
    )";
	glShaderSource(vs, 1, &szVS, nullptr);

	const auto szFS = R"(
    varying vec2 texOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;

    void main(){
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y, texOut).r;
        yuv.y = texture2D(tex_u, texOut).r - 0.5;
        yuv.z = texture2D(tex_v, texOut).r - 0.5;
        rgb = mat3(
				1, 1, 1,
				0, -0.39465, 2.03211,
				1.13983, -0.58060, 0)
		* yuv;
        gl_FragColor = vec4(rgb, 1);
    }
    )";
	glShaderSource(fs, 1, &szFS, nullptr);

	glCompileShader(vs);
	glCompileShader(fs);

#ifdef _DEBUG
	GLint iRet = 0;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &iRet);
	qDebug("vs::GL_COMPILE_STATUS=%d", iRet);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &iRet);
	qDebug("fs::GL_COMPILE_STATUS=%d", iRet);
#endif

	checkShaderCompileStatus(vs, "Vertex Shader");
	checkShaderCompileStatus(fs, "Fragment Shader");

	prog_yuv = glCreateProgram();
	glAttachShader(prog_yuv, vs);
	glAttachShader(prog_yuv, fs);
	glBindAttribLocation(prog_yuv, VER_ATTR_VER, "verIn");
	glBindAttribLocation(prog_yuv, VER_ATTR_TEX, "texIn");
	glLinkProgram(prog_yuv);
	checkProgramLinkStatus(prog_yuv);

#ifdef _DEBUG
	glGetProgramiv(prog_yuv, GL_LINK_STATUS, &iRet);
	qDebug("prog_yuv=%d GL_LINK_STATUS=%d", prog_yuv, iRet);
#endif

	glValidateProgram(prog_yuv);

	texUniformY = glGetUniformLocation(prog_yuv, "tex_y");
	texUniformU = glGetUniformLocation(prog_yuv, "tex_u");
	texUniformV = glGetUniformLocation(prog_yuv, "tex_v");

	qDebug("loadYUVShader ok");
}

void CUVGLWidget::initVAO() const {
	glVertexAttribPointer(VER_ATTR_VER, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(VER_ATTR_VER);

	glVertexAttribPointer(VER_ATTR_TEX, 2, GL_FLOAT, GL_FALSE, 0, textures);
	glEnableVertexAttribArray(VER_ATTR_TEX);
}

void CUVGLWidget::initYUV() {
	glGenTextures(3, tex_yuv);
	for (const auto& tex: tex_yuv) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}

void CUVGLWidget::checkShaderCompileStatus(const GLuint shader, const std::string& name) {
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength);
		glGetShaderInfoLog(shader, logLength, nullptr, log.data());
		std::stringstream ss;
		ss << "Error compiling " << name << ": " << log.data();
		qFatal("%s", ss.str().c_str());
	}
}

void CUVGLWidget::checkProgramLinkStatus(const GLuint program) {
	GLint status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength);
		glGetProgramInfoLog(program, logLength, nullptr, log.data());
		std::stringstream ss;
		ss << "Error linking program: " << log.data();
		qFatal("%s", ss.str().c_str());
	}
}

void CUVGLWidget::drawYUV(const CUVFrame* pFrame) const {
	assert(pFrame->type == PIX_FMT_IYUV || pFrame->type == PIX_FMT_YV12);

	const int w = pFrame->w;
	const int h = pFrame->h;
	const int y_size = w * h;
	const auto y = reinterpret_cast<GLubyte*>(pFrame->buf.base);
	GLubyte* u = y + y_size;
	GLubyte* v = u + (y_size >> 2);
	if (pFrame->type == PIX_FMT_YV12) {
		std::swap(u, v);
	}

	glUseProgram(prog_yuv);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_yuv[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);
	glUniform1i(static_cast<GLint>(texUniformY), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_yuv[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w / 2, h / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);
	glUniform1i(static_cast<GLint>(texUniformU), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_yuv[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w / 2, h / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
	glUniform1i(static_cast<GLint>(texUniformV), 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glUseProgram(0);
}
