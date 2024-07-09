#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>

#include "uvmedia.hpp"

class CUVFileTab : public QWidget {
	Q_OBJECT

public:
	explicit CUVFileTab(QWidget* parent = nullptr);
	~CUVFileTab() override;

	[[nodiscard]] QLineEdit* edit() { return m_pLeFileName; }

private:
	QLineEdit* m_pLeFileName{ nullptr };
	QPushButton* m_pBtnBrowse{ nullptr };
};

class CUVNetWorkTab : public QWidget {
	Q_OBJECT

public:
	explicit CUVNetWorkTab(QWidget* parent = nullptr);
	~CUVNetWorkTab() override;

	[[nodiscard]] QLineEdit* edit() { return m_pLeURL; }

private:
	QLineEdit* m_pLeURL{ nullptr };
};

class CUVCaptureTab : public QWidget {
	Q_OBJECT

public:
	explicit CUVCaptureTab(QWidget* parent = nullptr);
	~CUVCaptureTab() override;

	[[nodiscard]] QComboBox* cmb() { return m_pCbCaptureDevice; }

private:
	QComboBox* m_pCbCaptureDevice{ nullptr };
};


class CUVOpenMediaDlg : public QDialog {
	Q_OBJECT

public:
	explicit CUVOpenMediaDlg(QWidget* parent = nullptr);
	~CUVOpenMediaDlg() override;

	CUVMedia& media() { return m_media; }

	QTabWidget* tab() { return m_pTab; }

public slots:
	void accept() override;

protected:
	void init();

private:
	QTabWidget* m_pTab{ nullptr };
	CUVMedia m_media{};
};
