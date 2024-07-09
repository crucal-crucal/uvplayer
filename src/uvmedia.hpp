#pragma once

#include <string>

#include "avdef.hpp"

typedef struct media_s {
	media_type_e type{};
	std::string src;
	std::string decscr{};
	int index{};

	media_s() {
		type = MEDIA_TYPE_NONE;
		index = -1;
	}
} CUVMedia;
