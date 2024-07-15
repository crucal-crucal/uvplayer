#pragma once

#include <QObject>
#include <QVector>
#include <QToolBar>

class QAction;
class CUVMainWindow;
class CUVCenterWidget;

class CUVMainWindowPrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMainWindowPrivate)
	Q_DECLARE_PUBLIC(CUVMainWindow)

public:
	explicit CUVMainWindowPrivate(CUVMainWindow* q);
	~CUVMainWindowPrivate() override;

	CUVMainWindow* const q_ptr{ nullptr };

	void init();
	void initConnect();
	void initMenu();

	QAction* m_pActMenuBar{ nullptr };
	QAction* m_pActFullScreen{ nullptr };
	QAction* m_pActMvFullScreen{ nullptr };

	QVector<QToolBar*> m_toolBars{ };
	CUVCenterWidget* m_pCenterWidget{ nullptr };
};
