#pragma once

#include <QLabel>
#include <QPushButton>

class CUVVideoTitlebar : public QFrame {
	Q_OBJECT

public:
	explicit CUVVideoTitlebar(QWidget* parent = nullptr);
	~CUVVideoTitlebar() override;

protected:
	void init();

public:
	QLabel* labTitle{ nullptr };
	QPushButton* btnClose{ nullptr };
};
