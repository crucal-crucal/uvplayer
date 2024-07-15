#pragma once

#include <functional>

typedef std::function<void()> Function;

// same as golang defer
class Defer {
public:
	explicit Defer(Function&& fn) : _fn(std::move(fn)) {
	}

	~Defer() { if (_fn) _fn(); }

private:
	Function _fn;
};

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define defer(code) Defer CONCAT(_defer_, __LINE__)([&](){code;});

class ScopeCleanup {
public:
	template<typename Fn, typename... Args>
	explicit ScopeCleanup(Fn&& fn, Args&&... args) {
		_cleanup = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	~ScopeCleanup() {
		_cleanup();
	}

private:
	Function _cleanup;
};

template<typename T>
class ScopeFree {
public:
	explicit ScopeFree(T* p) : _p(p) {
	}

	~ScopeFree() { SAFE_FREE(_p); }

private:
	T* _p;
};

template<typename T>
class ScopeDelete {
public:
	explicit ScopeDelete(T* p) : _p(p) {
	}

	~ScopeDelete() { SAFE_DELETE(_p); }

private:
	T* _p;
};

template<typename T>
class ScopeDeleteArray {
public:
	explicit ScopeDeleteArray(T* p) : _p(p) {
	}

	~ScopeDeleteArray() { SAFE_DELETE_ARRAY(_p); }

private:
	T* _p;
};

template<typename T>
class ScopeRelease {
public:
	explicit ScopeRelease(T* p) : _p(p) {
	}

	~ScopeRelease() { SAFE_RELEASE(_p); }

private:
	T* _p;
};

template<typename T>
class ScopeLock {
public:
	explicit ScopeLock(T& mutex) : _mutex(mutex) { _mutex.lock(); }

	~ScopeLock() { _mutex.unlock(); }

private:
	T& _mutex;
};
