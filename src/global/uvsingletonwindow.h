#pragma once

#include <mutex>

template<typename T>
class CUVSingletoWindow {
  public:
	static T* Instance() {
		static std::once_flag flag;
		static T* inst = nullptr;

		// 确保只会被调用一次
		std::call_once(flag, []() {
			inst = new T;
		});

		return inst;
	}
};