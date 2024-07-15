#pragma once

#include <QWidget>

#include "uvframe.hpp"

class CUVVideoWnd {
public:
	explicit CUVVideoWnd(QWidget* parent = nullptr);
	virtual ~CUVVideoWnd() = default;

	virtual void setgeometry(const QRect& rc) = 0;
	virtual void Update() = 0;

protected:
	void calcFPS();

public:
	CUVFrame last_frame{};
	int fps{};
	bool draw_time{};
	bool draw_fps{};
	bool draw_resolution{};

protected:
	// for calFPS
	uint64_t tick;
	int framecnt;
};
