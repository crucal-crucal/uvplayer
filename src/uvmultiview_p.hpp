#pragma once


#include "uvmultiview.hpp"
#include "uvtable.hpp"
#include "uvvideowidget.hpp"

class CUVMultiView;

class CUVMultiViewPrivate {
	Q_DISABLE_COPY(CUVMultiViewPrivate)
	Q_DECLARE_PUBLIC(CUVMultiView)

public:
	explicit CUVMultiViewPrivate(CUVMultiView* q);
	~CUVMultiViewPrivate();

	CUVMultiView* const q_ptr{ nullptr };

	void initUI();
	void initConnect();
	void updateUI();

	void saveLayout();
	void restoreLayout();

	void mergeCells(int lt, int rb);
	static void exchangeCells(CUVVideoWidget* player1, CUVVideoWidget* player2);
	void stretch(QWidget* wdg);

	CUVVideoWidget* getPlayerByID(int playerid);
	CUVVideoWidget* getPlayerByPos(const QPoint& pt);
	CUVVideoWidget* getIdlePlayer();

	CUVTable table{};
	CUVTable prev_table{};
	QVector<QWidget*> views{};
	QLabel* labRect{ nullptr };
	QLabel* labDrag{ nullptr };

	QPoint ptMousePress{};
	uint64_t tsMousePress{};
	CUVMultiView::Action action{};
	bool bStretch{};
};
