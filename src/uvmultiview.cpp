#include "uvmultiview.hpp"

#include <QMouseEvent>

#include "uvdef.hpp"
#include "framelessMessageBox/uvmessagebox.hpp"

#define SEPARATOR_LINE_WIDTH 1

CUVMultiView::CUVMultiView(QWidget* parent) : QWidget(parent) {
	initUI();
	initConnect();
	bStretch = false;
}

CUVMultiView::~CUVMultiView() = default;

CUVVideoWidget* CUVMultiView::getPlayerByID(int playerid) {
	for (auto& view: views) {
		auto player = dynamic_cast<CUVVideoWidget*>(view);
		if (player->playerid == playerid) {
			return player;
		}
	}
	return nullptr;
}

CUVVideoWidget* CUVMultiView::getPlayerByPos(QPoint pt) {
	for (auto wdg: views) {
		if (wdg->isVisible() && wdg->geometry().contains(pt)) {
			return dynamic_cast<CUVVideoWidget*>(wdg);
		}
	}
	return nullptr;
}

CUVVideoWidget* CUVMultiView::getIdlePlayer() {
	for (auto& view: views) {
		auto player = dynamic_cast<CUVVideoWidget*>(view);
		if (player->isVisible() && player->status == CUVVideoWidget::STOP) {
			return player;
		}
	}
	return nullptr;
}

void CUVMultiView::setLayout(int row, int col) {
	saveLayout();
	table.init(row, col);
	updateUI();
	// g_confile->Set<int>("mv_row", row, "ui");
	// g_confile->Set<int>("mv_col", col, "ui");
}

void CUVMultiView::saveLayout() {
	prev_table = table;
}

void CUVMultiView::restoreLayout() {
	CUVTable tmp = table;
	table = prev_table;
	prev_table = tmp;
	updateUI();
}

void CUVMultiView::mergeCells(int lt, int rb) {
	// find first non-stop player as lt
	for (int i = lt; i <= rb; ++i) {
		CUVVideoWidget* player = getPlayerByID(i);
		if (player && player->status != CUVVideoWidget::STOP) {
			exchangeCells(player, getPlayerByID(lt));
			break;
		}
	}

	saveLayout();
	table.merge(lt, rb);
	updateUI();
}

void CUVMultiView::exchangeCells(CUVVideoWidget* player1, CUVVideoWidget* player2) {
	qDebug("exchange %d<=>%d", player1->playerid, player2->playerid);

	QRect rcTmp = player1->geometry();
	int idTmp = player1->playerid;

	player1->setGeometry(player2->geometry());
	player1->playerid = player2->playerid;

	player2->setGeometry(rcTmp);
	player2->playerid = idTmp;
}

void CUVMultiView::stretch(QWidget* wdg) {
	if (table.row == 1 && table.col == 1) return;
	if (bStretch) {
		restoreLayout();
		bStretch = false;
	} else {
		saveLayout();
		for (auto& view: views) {
			view->hide();
		}
		wdg->setGeometry(rect());
		wdg->show();
		bStretch = true;
	}
}

void CUVMultiView::play(CUVMedia& media) {
	CUVVideoWidget* player = getIdlePlayer();
	if (!player) {
		UVMessageBox::CUVMessageBox::information(this, tr("Info"), tr("No spare player, please stop one and try agian!"));
	} else {
		player->open(media);
	}
}

void CUVMultiView::initUI() {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	for (int i = 0; i < MV_STYLE_MAXNUM; ++i) {
		auto player = new CUVVideoWidget(this);
		player->playerid = i + 1;
		views.push_back(player);
	}

	// int row = g_confile->Get<int>("mv_row", "ui", MV_STYLE_ROW);
	// int col = g_confile->Get<int>("mv_col", "ui", MV_STYLE_COL);
	setLayout(3, 3);

	labDrag = new QLabel(this);
	labDrag->setFixedSize(160, 160);
	labDrag->hide();
	labDrag->setStyleSheet("border:3px groove #FF8C00");

	labRect = new QLabel(this);
	labRect->hide();
	labRect->setStyleSheet("border:2px solid red");
}

void CUVMultiView::initConnect() {
}

void CUVMultiView::updateUI() {
	int row = table.row;
	int col = table.col;
	if (row == 0 || col == 0) return;
	int cell_w = width() / col;
	int cell_h = height() / row;

	int margin_x = (width() - cell_w * col) / 2;
	int margin_y = (height() - cell_h * row) / 2;
	int x = margin_x;
	int y = margin_y;
	for (auto& view: views) {
		view->hide();
	}

	int cnt = 0;
	CUVTableCell cell;
	for (int r = 0; r < row; ++r) {
		for (int c = 0; c < col; ++c) {
			int id = r * col + c + 1;
			if (table.getTableCell(id, cell)) {
				QWidget* wdg = getPlayerByID(id);
				if (wdg) {
					wdg->setGeometry(x, y, cell_w * cell.colspan() - SEPARATOR_LINE_WIDTH, cell_h * cell.rowspan() - SEPARATOR_LINE_WIDTH);
					wdg->show();
					++cnt;
				}
			}
			x += cell_w;
		}
		x = margin_x;
		y += cell_h;
	}

	bStretch = (cnt == 1);
}

void CUVMultiView::resizeEvent(QResizeEvent* e) {
	updateUI();
}

void CUVMultiView::mousePressEvent(QMouseEvent* e) {
	ptMousePress = e->pos();
	tsMousePress = gettick();
}

void CUVMultiView::mouseReleaseEvent(QMouseEvent* e) {
	if (action == EXCHANGE) {
		CUVVideoWidget* player1 = getPlayerByPos(ptMousePress);
		CUVVideoWidget* player2 = getPlayerByPos(e->pos());
		if (player1 && player2 && player1 != player2) {
			exchangeCells(player1, player2);
		}
	} else if (action == MERGE) {
		QRect rc = adjustRect(ptMousePress, e->pos());
		CUVVideoWidget* player1 = getPlayerByPos(rc.topLeft());
		CUVVideoWidget* player2 = getPlayerByPos(rc.bottomRight());
		if (player1 && player2 && player1 != player2) {
			mergeCells(player1->playerid, player2->playerid);
		}
	}

	labRect->setVisible(false);
	labDrag->setVisible(false);
	setCursor(Qt::ArrowCursor);
}

void CUVMultiView::mouseMoveEvent(QMouseEvent* e) {
	CUVVideoWidget* player = getPlayerByPos(ptMousePress);
	if (player == nullptr) {
		return;
	}

	if (e->buttons() == Qt::LeftButton) {
		if (!labDrag->isVisible()) {
			if (gettick() - tsMousePress < 300) return;
			action = EXCHANGE;
			setCursor(Qt::OpenHandCursor);
			labDrag->setPixmap(player->grab().scaled(labDrag->size()));
			labDrag->setVisible(true);
		}
		if (labDrag->isVisible()) {
			labDrag->move(e->pos() - QPoint(labDrag->width() / 2, labDrag->height()));
		}
	} else if (e->buttons() == Qt::RightButton) {
		if (!labRect->isVisible()) {
			action = MERGE;
			setCursor(Qt::CrossCursor);
			labRect->setVisible(true);
		}
		if (labRect->isVisible()) {
			labRect->setGeometry(adjustRect(ptMousePress, e->pos()));
		}
	}
}

void CUVMultiView::mouseDoubleClickEvent(QMouseEvent* e) {
	CUVVideoWidget* player = getPlayerByPos(e->pos());
	if (player) {
		stretch(player);
	}
}
