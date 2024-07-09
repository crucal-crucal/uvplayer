#pragma once

#include <QLabel>
#include <QPushButton>

class CUVVideoTitlebar : public QFrame {
	Q_OBJECT

public:
	explicit CUVVideoTitlebar(QWidget* parent = nullptr);
	~CUVVideoTitlebar() override;

	QPushButton* btnClose() { return m_pBtnClose; }
	QLabel* labTitle() { return m_pLbTitle; }

protected:
	void init();

private:
	QLabel* m_pLbTitle{ nullptr };
	QPushButton* m_pBtnClose{ nullptr };
};
