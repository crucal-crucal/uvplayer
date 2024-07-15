#pragma once

#include <atomic>
#include <iostream>
#include <QtGlobal>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(n)  ((n) > 0 ? (n) : -(n))
#endif

typedef struct uvbuf_s {
	char* base{};
	size_t len{};

#ifdef __cplusplus

	uvbuf_s() {
		base = nullptr;
		len = 0;
	}

	uvbuf_s(void* data, const size_t& size) {
		this->base = static_cast<char*>(data);
		this->len = size;
	}
#endif
} uvbuf_t;

typedef struct offset_buf_s {
	char* base{};
	size_t len{};
	size_t offset{};

#ifdef __cplusplus
	offset_buf_s() {
		base = nullptr;
		len = offset = 0;
	}

	offset_buf_s(void* data, const size_t len) {
		this->base = static_cast<char*>(data);
		this->len = len;
	}
#endif
} offset_buf_t;

static volatile long s_alloc_cnt = ATOMIC_VAR_INIT(0);
static volatile long s_free_cnt = ATOMIC_VAR_INIT(0);

inline void* safe_realloc(void* oldptr, const size_t newSize, const size_t oldSize) {
	_InterlockedIncrement(&s_alloc_cnt);
	_InterlockedIncrement(&s_free_cnt);
	void* ptr = realloc(oldptr, newSize);
	if (!ptr) {
		std::cerr << "realloc failed" << std::endl;
		exit(-1);
	}
	if (newSize > oldSize) {
		memset(static_cast<char*>(ptr) + oldSize, 0, newSize - oldSize);
	}
	return ptr;
}

inline void* safe_zalloc(const size_t size) {
	_InterlockedIncrement(&s_alloc_cnt);
	void* ptr = malloc(size);
	if (!ptr) {
		std::cerr << "malloc failed" << std::endl;
		exit(-1);
	}
	memset(ptr, 0, size);
	return ptr;
}

inline void safe_free(void* ptr) {
	if (ptr) {
		free(ptr);
		_InterlockedIncrement(&s_free_cnt);
	}
}

#ifdef __cplusplus
class CUVBuf : public uvbuf_t {
public:
	explicit CUVBuf() {
		_cleanup = false;
	}

	explicit CUVBuf(void* data, const size_t& size) : uvbuf_t(data, size) {
		_cleanup = false;
	}

	explicit CUVBuf(const size_t& size) {
	}

	virtual ~CUVBuf() {
		cleanup();
	}

	[[nodiscard]] void* data() const { return base; }

	[[nodiscard]] virtual size_t size() const { return len; }

	bool isNull() { return base == nullptr || len == 0; }

	void cleanup() {
		if (_cleanup) {
			if (base) {
				safe_free(base);
				base = nullptr;
			}
			len = 0;
			_cleanup = false;
		}
	}

	void resize(const size_t& size) {
		if (size == len) return;

		if (!base) {
			*reinterpret_cast<void**>(&(base)) = safe_zalloc(size);
		} else {
			base = static_cast<char*>(safe_realloc(base, size, len));
		}
		len = size;
		_cleanup = true;
	}

	void copy(const void* data, const size_t len) {
		resize(len);
		memcpy(base, data, len);
	}

	void copy(uvbuf_t* buf) {
		copy(buf->base, buf->len);
	}

private:
	bool _cleanup{ false };
};

// VL: Variable Length
class CUVVLBuf : public CUVBuf {
public:
	explicit CUVVLBuf() {
		_offset = _size = 0;
	}

	explicit CUVVLBuf(void* data, size_t len) : CUVBuf(data, len) {
		_offset = 0;
		_size = len;
	}

	explicit CUVVLBuf(const size_t cap) : CUVBuf(cap) { _offset = _size = 0; }

	~CUVVLBuf() override = default;

	[[nodiscard]] char* data() const { // NOLINT
		return base + _offset;         // NOLINT
	}

	[[nodiscard]] size_t size() const override { return _size; }

	void push_front(const void* ptr, const size_t len) {
		if (len > this->len - _size) {
			size_t newsize = MAX(this->len, len) * 2;
			base = static_cast<char*>(safe_realloc(base, newsize, this->len));
			this->len = newsize;
		}

		if (_offset < len) {
			// move => end
			memmove(base + this->len - _size, data(), _size);
			_offset = this->len - _size;
		}

		memcpy(data() - len, ptr, len);
		_offset -= len;
		_size += len;
	}

	void push_back(const void* ptr, const size_t len) {
		if (len > this->len - _size) {
			const size_t newsize = MAX(this->len, len) * 2;
			base = static_cast<char*>(safe_realloc(base, newsize, this->len));
			this->len = newsize;
		} else if (len > this->len - _offset - _size) {
			// move => start
			memmove(base, data(), _size);
			_offset = 0;
		}
		memcpy(data() + _size, ptr, len);
		_size += len;
	}

	void pop_front(void* ptr, const size_t len) {
		if (len <= _size) {
			if (ptr) {
				memcpy(ptr, data(), len);
			}
			_offset += len;
			if (_offset >= len) _offset = 0;
			_size -= len;
		}
	}

	void pop_back(void* ptr, const size_t len) {
		if (len <= _size) {
			if (ptr) {
				memcpy(ptr, data() + _size - len, len);
			}
			_size -= len;
		}
	}

	void clear() {
		_offset = _size = 0;
	}

	void prepend(void* ptr, const size_t len) {
		push_front(ptr, len);
	}

	void append(void* ptr, const size_t len) {
		push_back(ptr, len);
	}

	void insert(void* ptr, const size_t len) {
		push_back(ptr, len);
	}

	void remove(const size_t len) {
		pop_front(nullptr, len);
	}

private:
	size_t _offset;
	size_t _size;
};

class CUVRingBuf : public CUVBuf {
public:
	CUVRingBuf() { _head = _tail = _size = 0; }

	explicit CUVRingBuf(const size_t cap) : CUVBuf(cap) { _head = _tail = _size = 0; }

	~CUVRingBuf() override = default;

	char* alloc(const size_t len) {
		char* ret = nullptr;
		if (_head < _tail || _size == 0) {
			// [_tail, this->len) && [0, _head)
			if (this->len - _tail >= len) {
				ret = base + _tail;
				_tail += len;
				if (_tail == this->len) _tail = 0;
			} else if (_head >= len) {
				ret = base;
				_tail = len;
			}
		} else {
			// [_tail, _head)
			if (_head - _tail >= len) {
				ret = base + _tail;
				_tail += len;
			}
		}
		_size += ret ? len : 0;
		return ret;
	}

	void free(const size_t len) {
		_size -= len;
		if (len <= this->len - _head) {
			_head += len;
			if (_head == this->len) _head = 0;
		} else {
			_head = len;
		}
	}

	virtual void clear() { _head = _tail = _size = 0; }

	[[nodiscard]] size_t size() const override { return _size; }

private:
	size_t _head{};
	size_t _tail{};
	size_t _size{};
};
#endif
