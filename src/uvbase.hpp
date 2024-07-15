#pragma once

#include <QtGlobal> // NOLINT

inline bool getboolean(const char* str) {
	if (!str) return false;

	switch (strlen(str)) {
		case 1:
			return *str == '1' || *str == 'y' || *str == 'Y';
		case 2:
			return _stricmp(str, "on") == 0;
		case 3:
			return _stricmp(str, "yes") == 0;
		case 4:
			return _stricmp(str, "true") == 0;
		case 5:
			return _stricmp(str, "enable") == 0;
		default: break;
	}

	return false;
}

inline char* strrchr_dir(const char* filepath) {
	auto p = const_cast<char*>(filepath);
	while (*p) ++p;
	while (--p >= filepath) {
#ifdef Q_OS_WIN
		if (*p == '/' || *p == '\\')
#else
		if (*p == '/')
#endif
			return p;
	}
	return nullptr;
}
