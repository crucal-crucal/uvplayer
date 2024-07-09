#pragma once

#include <QLabel>
#include <QPushButton>
#include <QSlider>

class CUVVideoToolbar : public QFrame {
	Q_OBJECT

public:
	explicit CUVVideoToolbar(QWidget* parent = nullptr);
	~CUVVideoToolbar() override;

	QSlider* sldProgress() { return m_pSldProgress; }

	QLabel* lblDuration() { return m_pLbDuration; }

	QPushButton* btnStart() { return m_pBtnStart; }

	QPushButton* btnPause() { return m_pBtnPause; }

signals:
	void sigStart();
	void sigPause();
	void sigStop();

protected:
	void init();
	void initConnect();

private:
	QPushButton* m_pBtnStart{ nullptr };
	QPushButton* m_pBtnPause{ nullptr };

	QPushButton* m_pBtnPrev{ nullptr };
	QPushButton* m_pBtnStop{ nullptr };
	QPushButton* m_pBtnNext{ nullptr };

	QSlider* m_pSldProgress{ nullptr };
	QLabel* m_pLbDuration{ nullptr };
};
