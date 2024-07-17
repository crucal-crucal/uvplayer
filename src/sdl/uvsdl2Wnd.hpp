#pragma once

#include "util/uvsdl_util.hpp"
#include "interface/uvvideownd.hpp"

class CUVSDL2Wnd : public CUVVideoWnd, QWidget {
public:
	explicit CUVSDL2Wnd(QWidget* parent = nullptr);
	~CUVSDL2Wnd() override;

	void setgeometry(const QRect& rc) override;
	void Update() override;

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

	static std::atomic_flag s_sdl_init;
	SDL_Window* m_sdl_window{ nullptr };
	SDL_Renderer* m_sdl_renderer{ nullptr };
	SDL_Texture* m_sdl_texture{ nullptr };
	SDL_RendererInfo m_sdl_renderer_info{ nullptr };
	int m_tex_w{};
	int m_tex_h{};
	int m_tex_pix_fmt{};
	int m_tex_bpp{};
	int m_tex_pitch{};
	SDL_PixelFormatEnum m_sdl_pix_fmt{};
};
