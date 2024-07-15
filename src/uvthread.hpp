#pragma once

#include <QtGlobal>

#ifdef Q_OS_WIN
#define uv_getpid (long)GetCurrentProcessId
#else
#define uv_getpid (long)getpid
#endif

#ifdef Q_OS_WIN
#define uv_gettid   (long)GetCurrentThreadId
#elif defined(Q_OS_UNIX)
#include <sys/syscall.h>
#define uv_gettid syscall(SYS_gettid)
#endif

#ifdef Q_OS_WIN
#include "Windows.h"
typedef HANDLE uvthread_t;
typedef DWORD (WINAPI*uvthread_routine)(void*);
#define UVTHREAD_RETTYPE DWORD
#define UVTHREAD_ROUTINE(fname) DWORD WINAPI fname(void* userdata)

static uvthread_t uvthread_create(uvthread_routine fn, void* userdata) { // NOLINT
	return CreateThread(nullptr, 0, fn, userdata, 0, nullptr);
}

static int uvthread_join(const uvthread_t th) {
	WaitForSingleObject(th, INFINITE);
	CloseHandle(th);
	return 0;
}

#elif defined(Q_OS_UNIX)
typedef pthread_t   uvthread_t;
typedef void* (*uvthread_routine)(void*);
#define	UVTHREAD_RETTYPE void*
#define UVTHREAD_ROUTINE(fname) void* fname(void* userdata)
static inline uvthread_t uvthread_create(uvthread_routine fn, void* userdata) {
	pthread_t th;
	pthread_create(&th, NULL, fn, userdata);
	return th;
}

static inline int uvthread_join(uvthread_t th) {
	return pthread_join(th, NULL);
}
#endif

#ifdef __cplusplus
#include <thread>
#include <atomic>
#include <chrono>

class CUVThread {
public:
	enum Status {
		STOP,
		RUNNING,
		PAUSE
	};

	enum SleepPolicy {
		YIELD,
		SLEEP_FOR,
		SLEEP_UNTIL,
		NO_SLEEP,
	};

	CUVThread() {
		status = STOP;
		status_changed = false;
		dotask_cnt = 0;
		sleep_policy = YIELD;
		sleep_ms = 0;
	}

	virtual ~CUVThread() = default;

	void setStatus(const Status& stat) {
		status_changed = true;
		status = stat;
	}

	void setSleepPolicy(const SleepPolicy& policy, const uint32_t ms = 0) {
		sleep_policy = policy;
		sleep_ms = ms;
		setStatus(status);
	}

	virtual int start() {
		if (status == STOP) {
			thread = std::thread([this] {
				if (!doPrepare()) {
					return;
				}
				setStatus(RUNNING);
				run();
				setStatus(STOP);
				if (!doFinish()) {
					return;
				}
			});
		}
		return 0;
	}

	virtual int stop() {
		if (status != STOP) {
			setStatus(STOP);
		}
		if (thread.joinable()) {
			thread.join(); // wait thread exit
		}
		return 0;
	}

	virtual int pause() {
		if (status == RUNNING) {
			setStatus(PAUSE);
		}
		return 0;
	}

	virtual int resume() {
		if (status == PAUSE) {
			setStatus(RUNNING);
		}
		return 0;
	}

	virtual void run() {
		while (status != STOP) {
			while (status == PAUSE) {
				std::this_thread::yield();
			}

			doTask();
			++dotask_cnt;

			sleep();
		}
	}

	virtual bool doPrepare() { return true; }

	virtual void doTask() {
	}

	virtual bool doFinish() { return true; }


	std::thread thread;
	std::atomic<Status> status;
	uint32_t dotask_cnt;

protected:
	void sleep() {
		switch (sleep_policy) {
			case YIELD:
				std::this_thread::yield();
				break;
			case SLEEP_FOR:
				std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
				break;
			case SLEEP_UNTIL: {
				if (status_changed) {
					status_changed = false;
					base_tp = std::chrono::system_clock::now();
				}
				base_tp += std::chrono::milliseconds(sleep_ms);
				std::this_thread::sleep_until(base_tp);
			}
			break;
			default: // donothing, go all out.
				break;
		}
	}

	SleepPolicy sleep_policy;
	uint32_t sleep_ms;
	// for SLEEP_UNTIL
	std::atomic<bool> status_changed;
	std::chrono::system_clock::time_point base_tp;
};
#endif
