#include "uvstring.hpp"

#include <cstdarg>


static inline int vscprintf(const char* fmt, va_list ap) { // NOLINT
	return vsnprintf(nullptr, 0, fmt, ap);
}

namespace uvstring {
	std::string asprintf(const char* format, ...) {
		va_list ap;
		va_start(ap, format);
		const int len = vscprintf(format, ap);
		va_end(ap);

		std::string str(len, '\0');
		va_start(ap, format);
		vsnprintf(&str[0], len + 1, format, ap);
		va_end(ap);

		return str;
	}

	stringList split(const std::string& str, const char delim) {
		const char* p = str.c_str();
		const char* value = p;
		stringList list;
		while (*p != '\0') {
			if (*p == delim) {
				list.emplace_back(value, p - value);
				value = p + 1;
			}
			++p;
		}
		list.emplace_back(value);
		return list;
	}

	keyValueMap splitKV(const std::string& str, const char kv_kv, const char k_v) {
		enum {
			s_key,
			s_value
		} state = s_key;

		const char* p = str.c_str();
		const char* key = p;
		const char* value = nullptr;
		int key_len = 0;
		int value_len = 0;
		keyValueMap kvs;
		while (*p != '\0') {
			if (*p == kv_kv) {
				if (key_len && value_len) {
					kvs[std::string(key, key_len)] = std::string(value, value_len);
				}
				state = s_key;
				key = p + 1;
				key_len = 0;
				value_len = 0;
			} else if (*p == k_v) {
				state = s_value;
				value = p + 1;
				value_len = 0;
			} else {
				state == s_key ? ++key_len : ++value_len;
			}
			++p;
		}

		if (key_len && value_len) {
			kvs[std::string(key, key_len)] = std::string(value, value_len);
		}

		return kvs;
	}

	std::string trim(const std::string& str, const std::string& trim_chars) {
		const auto pos1 = str.find_first_not_of(trim_chars);
		if (pos1 == std::string::npos) {
			return "";
		}

		const auto pos2 = str.find_last_not_of(trim_chars);
		return str.substr(pos1, pos2 - pos1 + 1);
	}

	std::string trimL(const std::string& str, const char* chars) {
		const auto pos = str.find_first_not_of(chars);
		return pos == std::string::npos ? "" : str.substr(pos);
	}

	std::string trimR(const std::string& str, const char* chars) {
		const auto pos = str.find_last_not_of(chars);
		return str.substr(0, pos + 1);
	}

	std::string trim_pairs(const std::string& str, const char* pairs) {
		if (str.empty()) return str;

		const char* s = str.c_str();
		const char* e = s + str.size() - 1;
		const char* p = pairs;
		bool is_pair = false;
		while (*p != '\0' && *(p + 1) != '\0') {
			if (*s == *p && *e == *(p + 1)) {
				is_pair = true;
				break;
			}
			p += 2;
		}

		return is_pair ? str.substr(1, str.size() - 2) : str;
	}

	std::string replace(const std::string& str, const std::string& from, const std::string& to) {
		std::string result = str;

		std::string::size_type pos = 0;
		while ((pos = result.find(from, pos)) != std::string::npos) {
			result.replace(pos, from.length(), to);
			pos += to.length();
		}

		return result;
	}

	std::string basename(const std::string& path) {
		const auto pos1 = path.find_last_not_of("/\\");
		if (pos1 == std::string::npos) {
			return "/";
		}

		const auto pos2 = path.find_last_of("/\\", pos1);

		return pos2 == std::string::npos ? path.substr(0, pos1 + 1) : path.substr(pos2 + 1, pos1 - pos2);
	}

	std::string dirname(const std::string& path) {
		const auto pos1 = path.find_last_not_of("/\\");
		if (pos1 == std::string::npos) {
			return "/";
		}

		const auto pos2 = path.find_last_of("/\\", pos1);

		return pos2 == std::string::npos ? "." : path.substr(0, pos2 + 1);
	}

	std::string filename(const std::string& path) {
		auto pos1 = path.find_last_of("/\\");
		if (pos1 == std::string::npos) {
			pos1 = 0;
		} else {
			pos1++;
		}

		std::string file = path.substr(pos1, -1);
		const auto pos2 = file.find_last_of('.');

		return pos2 == std::string::npos ? file : file.substr(0, pos2);
	}

	std::string suffixname(const std::string& path) {
		auto pos1 = path.find_last_of("/\\");
		if (pos1 == std::string::npos) {
			pos1 = 0;
		} else {
			pos1++;
		}

		const std::string file = path.substr(pos1, -1);
		const auto pos2 = file.find_last_of('.');

		return pos2 == std::string::npos ? "" : file.substr(pos2 + 1);
	}
}
