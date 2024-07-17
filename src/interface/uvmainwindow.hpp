#pragma once

#include <QMainWindow>

#include "def/appdef.hpp" // NOLINT

class CUVMainWindowPrivate;

class CUVMainWindow : public QMainWindow {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMainWindow)
	Q_DECLARE_PRIVATE(CUVMainWindow)

public:
	explicit CUVMainWindow(QWidget* parent = nullptr);
	~CUVMainWindow() override;

	enum window_state_e {
		NORMAL = 0,
		MINIMIZED,
		MAXIMIZED,
		FULLSCREEN,
	} window_state{};

public slots:
	void about();
	void fullScreen();
	void onMVStyleSelected(int id);
	void mv_fullScreen();
	void openMediaDlg(int index);

protected:
	QScopedPointer<CUVMainWindowPrivate> d_ptr{ nullptr };

	void keyPressEvent(QKeyEvent* event) override;
	void changeEvent(QEvent*) override;
};
