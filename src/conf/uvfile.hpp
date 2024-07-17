#pragma once

#include <string>

#include "def/uvdef.hpp"
#include "util/uvbuf.hpp"

#define MAX_PATH 200

class CUVFile {
public:
	explicit CUVFile() {
		m_file = nullptr;
	}

	~CUVFile() {
		close();
	}

	int open(const char* filepath, const char* mode) {
		close();
		strncpy_s(this->filepath, filepath, MAX_PATH);
		return fopen_s(&m_file, filepath, mode);
	}

	void close() {
		if (m_file) {
			fclose(m_file);
			m_file = nullptr;
		}
	}

	size_t read(void* ptr, const size_t& len) const {
		return fread(ptr, 1, len, m_file);
	}

	size_t write(const void* ptr, const size_t& len) const {
		return fwrite(ptr, 1, len, m_file);
	}

	[[nodiscard]] size_t size() const {
		struct stat st{};
		memset(&st, 0, sizeof(st));
		stat(filepath, &st);
		return st.st_size;
	}

	size_t readAll(CUVBuf& buf) const {
		const auto filesize = size();
		buf.resize(filesize);
		return fread(buf.base, 1, filesize, m_file);
	}

	size_t readAll(std::string& str) const {
		const auto filesize = size();
		str.resize(filesize);
		return fread(static_cast<void*>(str.data()), 1, filesize, m_file);
	}

	bool readline(std::string& str) const {
		str.clear();
		char ch;
		while (fread(&ch, 1, 1, m_file)) {
			if (ch == LF) {
				return true;
			}
			if (ch == CR) {
				if (fread(&ch, 1, 1, m_file) && ch != LF) {
					fseek(m_file, -1, SEEK_CUR);
				}
				return true;
			}
			str += ch;
		}

		return !str.empty();
	}

private:
	char filepath[MAX_PATH]{};
	FILE* m_file{ nullptr };
};
