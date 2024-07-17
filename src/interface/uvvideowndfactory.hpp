#pragma once

#include "uvvideownd.hpp"
#include "gl/uvglwnd.hpp"
#include "sdl/uvsdl2Wnd.hpp"

enum renderer_type_e {
	RENDERER_TYPE_OPENGL,
	RENDERER_TYPE_SDL
};

#define DEFAULT_RENDERER_TYPE RENDERER_TYPE_OPENGL

class CUVVideoWndFactory {
public:
	static CUVVideoWnd* create(const renderer_type_e& type = DEFAULT_RENDERER_TYPE, QWidget* parent = nullptr) {
		switch (type) {
			case RENDERER_TYPE_OPENGL:
				return new CUVGLWnd(parent);
			case RENDERER_TYPE_SDL:
				return new CUVSDL2Wnd(parent);
			default:
				return nullptr;
		}
	}
};
