#pragma once

#include <QLabel>
#include <QPushButton>

#include "uvmaterialslider/uvmaterialslider.hpp"

class CUVVideoToolbar final : public QFrame {
	Q_OBJECT

public:
	explicit CUVVideoToolbar(QWidget* parent = nullptr);
	~CUVVideoToolbar() override;

	[[nodiscard]] QLabel* lblDuration() const { return m_pLbDuration; }

	[[nodiscard]] QLabel* lbCurDuration() const { return m_pLbCurDuration; }

	[[nodiscard]] QPushButton* btnStart() const { return m_pBtnStart; }

	[[nodiscard]] QPushButton* btnPause() const { return m_pBtnPause; }

signals:
	void sigStart();
	void sigPause();
	void sigStop();

protected:
	void init();
	void initConnect();

public:
	QPushButton* m_pBtnStart{ nullptr };
	QPushButton* m_pBtnPause{ nullptr };

	QPushButton* m_pBtnPrev{ nullptr };
	QPushButton* m_pBtnStop{ nullptr };
	QPushButton* m_pBtnNext{ nullptr };

	QLabel* m_pLbCurDuration{ nullptr };
	CUVMaterialSlider* sldProgress{ nullptr };
	QLabel* m_pLbDuration{ nullptr };
};
