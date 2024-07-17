#pragma once

class CUVCustomEvent {
public:
	enum Type {
		User = 10000,
		OpenMediaSucceed,
		OpenMediaFailed,
		PlayerEOF,
		PlayerError,
	};
};
