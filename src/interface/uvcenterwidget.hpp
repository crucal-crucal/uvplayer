#pragma once

#include "uvmultiview.hpp"

class CUVCenterWidget final : public QWidget {
	Q_OBJECT

public:
	explicit CUVCenterWidget(QWidget* parent = nullptr);
	~CUVCenterWidget() override;

protected:
	void init();
	void initConnect();

public:
	QWidget* lside{ nullptr };
	CUVMultiView* mv{ nullptr };
	QWidget* rside{ nullptr };
};
