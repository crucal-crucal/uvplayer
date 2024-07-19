#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>

#include "global/uvmedia.hpp"

class CUVFileTab final : public QWidget {
	Q_OBJECT

public:
	explicit CUVFileTab(QWidget* parent = nullptr);
	~CUVFileTab() override;

	QLineEdit* lineEdit{ nullptr };
	QPushButton* m_pBtnBrowse{ nullptr };
};

class CUVNetWorkTab final : public QWidget {
	Q_OBJECT

public:
	explicit CUVNetWorkTab(QWidget* parent = nullptr);
	~CUVNetWorkTab() override;

	QLineEdit* lineEdit{ nullptr };
};

class CUVCaptureTab final : public QWidget {
	Q_OBJECT

public:
	explicit CUVCaptureTab(QWidget* parent = nullptr);
	~CUVCaptureTab() override;

	QComboBox* comboBox{ nullptr };
};


class CUVOpenMediaDlg final : public QDialog {
	Q_OBJECT

public:
	explicit CUVOpenMediaDlg(QWidget* parent = nullptr);
	~CUVOpenMediaDlg() override;

public slots:
	void accept() override;

protected:
	void init();

public:
	QTabWidget* tab{ nullptr };
	CUVMedia media{};
};
