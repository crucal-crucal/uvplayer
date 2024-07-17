#include "uvsdl2Wnd.hpp"

#include <QDebug>

#include "def/avdef.hpp"

/**
 * class CUVSDL2Wnd
 */
std::atomic_flag CUVSDL2Wnd::s_sdl_init = ATOMIC_FLAG_INIT;

static SDL_PixelFormatEnum SDL_pix_fmt(const int type) {
	switch (type) {
		case PIX_FMT_IYUV: return SDL_PIXELFORMAT_IYUV;
		case PIX_FMT_YV12: return SDL_PIXELFORMAT_YV12;
		case PIX_FMT_NV12: return SDL_PIXELFORMAT_NV12;
		case PIX_FMT_NV21: return SDL_PIXELFORMAT_NV21;
		case PIX_FMT_YUY2: return SDL_PIXELFORMAT_YUY2;
		case PIX_FMT_YVYU: return SDL_PIXELFORMAT_YVYU;
		case PIX_FMT_UYVY: return SDL_PIXELFORMAT_UYVY;
		case PIX_FMT_RGB: return SDL_PIXELFORMAT_RGB24;
		case PIX_FMT_BGR: return SDL_PIXELFORMAT_BGR24;
		case PIX_FMT_RGBA: return SDL_PIXELFORMAT_RGBA8888;
		case PIX_FMT_ARGB: return SDL_PIXELFORMAT_ARGB8888;
		case PIX_FMT_ABGR: return SDL_PIXELFORMAT_ABGR8888;
		default: break;
	}
	return SDL_PIXELFORMAT_UNKNOWN;
}

CUVSDL2Wnd::CUVSDL2Wnd(QWidget* parent) : CUVVideoWnd(parent), QWidget(parent) {
	if (!s_sdl_init.test_and_set()) {
		SDL_Init(SDL_INIT_VIDEO);
	}

	setAttribute(Qt::WA_NativeWindow);
	m_sdl_window = SDL_CreateWindowFrom(reinterpret_cast<void*>(winId()));
	m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!m_sdl_renderer) {
		qWarning("Failed to initialize a hardware accelerated renderer: %s", SDL_GetError());
		m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, 0);
	}
	if (m_sdl_renderer) {
		memset(&m_sdl_renderer_info, 0, sizeof(SDL_RendererInfo));
		if (!SDL_GetRendererInfo(m_sdl_renderer, &m_sdl_renderer_info)) {
			qDebug() << "Renderer: " << m_sdl_renderer_info.name;
		}
	}

	if (!m_sdl_window || !m_sdl_renderer || !m_sdl_renderer_info.num_texture_formats) {
		qCritical("Failed to create a SDL window or renderer: %s", SDL_GetError());
		exit(-1);
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	m_sdl_texture = nullptr;
	m_tex_w = m_tex_h = 0;
}

CUVSDL2Wnd::~CUVSDL2Wnd() {
	if (m_sdl_texture) {
		SDL_DestroyTexture(m_sdl_texture);
		m_sdl_texture = nullptr;
	}

	if (m_sdl_renderer) {
		SDL_DestroyRenderer(m_sdl_renderer);
		m_sdl_renderer = nullptr;
	}

	if (m_sdl_window) {
		SDL_DestroyWindow(m_sdl_window);
		m_sdl_window = nullptr;
	}
}

void CUVSDL2Wnd::setgeometry(const QRect& rc) {
	QWidget::setGeometry(rc);
}

void CUVSDL2Wnd::Update() {
	paintEvent(nullptr);
}

void CUVSDL2Wnd::paintEvent(QPaintEvent* event) {
	calcFPS();
	SDL_SetRenderDrawColor(m_sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_sdl_renderer);

	if (!last_frame.isNull()) {
		if (!m_sdl_texture || m_tex_w != last_frame.w || m_tex_h != last_frame.h || m_tex_pix_fmt != last_frame.type) {
			if (m_sdl_texture) {
				SDL_DestroyTexture(m_sdl_texture);
				m_sdl_texture = nullptr;
			}
			m_sdl_pix_fmt = SDL_pix_fmt(last_frame.type);
			m_sdl_texture = SDL_CreateTexture(m_sdl_renderer, m_sdl_pix_fmt, SDL_TEXTUREACCESS_STREAMING, last_frame.w, last_frame.h);
			if (!m_sdl_texture) {
				qCritical() << "SDL_CreateTexture failed: " << SDL_GetError();
				return;
			}
			void* pixels{};
			int pitch{};
			if (SDL_LockTexture(m_sdl_texture, nullptr, &pixels, &pitch) < 0) {
				return;
			}
			memset(pixels, 0, pitch * last_frame.h);
			SDL_UnlockTexture(m_sdl_texture);

			m_tex_w = last_frame.w;
			m_tex_h = last_frame.h;
			m_tex_pix_fmt = last_frame.type;
			m_tex_bpp = pix_fmt_bpp(last_frame.type);
			m_tex_pitch = pitch;
			qDebug("SDL_Texture: w = %d, h = %d, bpp = %d, pitch = %d, pix_fmt = %d", m_tex_w, m_tex_h, m_tex_bpp, m_tex_pitch, m_tex_pix_fmt);
		}
		SDL_UpdateTexture(m_sdl_texture, nullptr, last_frame.buf.base, m_tex_pitch);
		SDL_RenderCopy(m_sdl_renderer, m_sdl_texture, nullptr, nullptr);
	}

	SDL_RenderPresent(m_sdl_renderer);
}

void CUVSDL2Wnd::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
}
