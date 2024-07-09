#pragma once

class CUVCenterWidget;

class CUVCenterWidgetPrivate {
public:
	explicit CUVCenterWidgetPrivate(CUVCenterWidget* parent);
	~CUVCenterWidgetPrivate();

	CUVCenterWidget* m_pParent;
};
