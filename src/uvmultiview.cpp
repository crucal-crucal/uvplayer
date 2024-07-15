#include "uvmultiview.hpp"

#include "uvconf.hpp"
#include "uvdef.hpp"
#include "framelessMessageBox/uvmessagebox.hpp"
#include "uvmultiview_p.hpp"

#define SEPARATOR_LINE_WIDTH 1

/**
 * class CUVMultiViewPrivate
 * @param q
 * Internal class for CUVMultiView
 */
CUVMultiViewPrivate::CUVMultiViewPrivate(CUVMultiView* q) : q_ptr(q) {
	bStretch = false;
}

CUVMultiViewPrivate::~CUVMultiViewPrivate() = default;

void CUVMultiViewPrivate::initUI() {
	Q_Q(CUVMultiView);

	q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	for (int i = 0; i < MV_STYLE_MAXNUM; ++i) {
		const auto player = new CUVVideoWidget(q);
		player->playerid = i + 1;
		views.push_back(player);
	}

	int row = g_confile->get<int>("mv_row", "ui", MV_STYLE_ROW);
	int col = g_confile->get<int>("mv_col", "ui", MV_STYLE_COL);
	q->setLayout(row, col);

	labDrag = new QLabel(q);
	labDrag->setFixedSize(160, 160);
	labDrag->hide();
	labDrag->setStyleSheet("border:3px groove #FF8C00");

	labRect = new QLabel(q);
	labRect->hide();
	labRect->setStyleSheet("border:2px solid red");
}

void CUVMultiViewPrivate::initConnect() { // NOLINT
}

void CUVMultiViewPrivate::updateUI() {
	Q_Q(CUVMultiView);

	int row = table.row;
	int col = table.col;
	if (row == 0 || col == 0) return;
	int cell_w = q->width() / col;
	int cell_h = q->height() / row;

	int margin_x = (q->width() - cell_w * col) / 2;
	int margin_y = (q->height() - cell_h * row) / 2;
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
				if (QWidget* wdg = getPlayerByID(id)) {
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

void CUVMultiViewPrivate::saveLayout() {
	prev_table = table;
}

void CUVMultiViewPrivate::restoreLayout() {
	CUVTable tmp = table;
	table = prev_table;
	prev_table = tmp;
	updateUI();
}

void CUVMultiViewPrivate::mergeCells(int lt, int rb) {
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

void CUVMultiViewPrivate::exchangeCells(CUVVideoWidget* player1, CUVVideoWidget* player2) {
	qDebug("exchange %d<=>%d", player1->playerid, player2->playerid);

	QRect rcTmp = player1->geometry();
	int idTmp = player1->playerid;

	player1->setGeometry(player2->geometry());
	player1->playerid = player2->playerid;

	player2->setGeometry(rcTmp);
	player2->playerid = idTmp;
}

void CUVMultiViewPrivate::stretch(QWidget* wdg) {
	Q_Q(CUVMultiView);

	if (table.row == 1 && table.col == 1) {
		return;
	}
	if (bStretch) {
		restoreLayout();
		bStretch = false;
	} else {
		saveLayout();
		for (auto& view: views) {
			view->hide();
		}
		wdg->setGeometry(q->rect());
		wdg->show();
		bStretch = true;
	}
}

CUVVideoWidget* CUVMultiViewPrivate::getPlayerByID(int playerid) {
	for (auto& view: views) {
		auto player = dynamic_cast<CUVVideoWidget*>(view);
		if (player->playerid == playerid) {
			return player;
		}
	}
	return nullptr;
}

CUVVideoWidget* CUVMultiViewPrivate::getPlayerByPos(const QPoint& pt) {
	for (auto wdg: views) {
		if (wdg->isVisible() && wdg->geometry().contains(pt)) {
			return dynamic_cast<CUVVideoWidget*>(wdg);
		}
	}
	return nullptr;
}

CUVVideoWidget* CUVMultiViewPrivate::getIdlePlayer() {
	for (auto& view: views) {
		auto player = dynamic_cast<CUVVideoWidget*>(view);
		if (player->isVisible() && player->status == CUVVideoWidget::STOP) {
			return player;
		}
	}
	return nullptr;
}


/**
 * class CUVMultiView
 * @param parent
 */
CUVMultiView::CUVMultiView(QWidget* parent) : QWidget(parent), d_ptr(new CUVMultiViewPrivate(this)) {
	d_func()->initUI();
	d_func()->initConnect();
}

CUVMultiView::~CUVMultiView() = default;

void CUVMultiView::setLayout(int row, int col) {
	Q_D(CUVMultiView);

	d->saveLayout();
	d->table.init(row, col);
	d->updateUI();
	g_confile->set<int>("mv_row", row, "ui");
	g_confile->set<int>("mv_col", col, "ui");
}

void CUVMultiView::play(CUVMedia& media) {
	Q_D(CUVMultiView);

	CUVVideoWidget* player = d->getIdlePlayer();
	if (!player) {
		UVMessageBox::CUVMessageBox::information(this, tr("Info"), tr("No spare player, please stop one and try agian!"));
	} else {
		player->open(media);
	}
}

void CUVMultiView::resizeEvent(QResizeEvent* e) {
	Q_D(CUVMultiView);

	d->updateUI();
}

void CUVMultiView::mousePressEvent(QMouseEvent* e) {
	Q_D(CUVMultiView);

	d->ptMousePress = e->pos();
	d->tsMousePress = gettick();
}

void CUVMultiView::mouseReleaseEvent(QMouseEvent* e) {
	Q_D(CUVMultiView);

	if (d->action == EXCHANGE) {
		CUVVideoWidget* player1 = d->getPlayerByPos(d->ptMousePress);
		CUVVideoWidget* player2 = d->getPlayerByPos(e->pos());
		if (player1 && player2 && player1 != player2) {
			CUVMultiViewPrivate::exchangeCells(player1, player2);
		}
	} else if (d->action == MERGE) {
		QRect rc = adjustRect(d->ptMousePress, e->pos());
		CUVVideoWidget* player1 = d->getPlayerByPos(rc.topLeft());
		CUVVideoWidget* player2 = d->getPlayerByPos(rc.bottomRight());
		if (player1 && player2 && player1 != player2) {
			d->mergeCells(player1->playerid, player2->playerid);
		}
	}

	d->labRect->setVisible(false);
	d->labDrag->setVisible(false);
	setCursor(Qt::ArrowCursor);
}

void CUVMultiView::mouseMoveEvent(QMouseEvent* e) {
	Q_D(CUVMultiView);

	CUVVideoWidget* player = d->getPlayerByPos(d->ptMousePress);
	if (player == nullptr) {
		return;
	}

	if (e->buttons() == Qt::LeftButton) {
		if (!d->labDrag->isVisible()) {
			if (gettick() - d->tsMousePress < 300) return;
			d->action = EXCHANGE;
			setCursor(Qt::OpenHandCursor);
			d->labDrag->setPixmap(player->grab().scaled(d->labDrag->size()));
			d->labDrag->setVisible(true);
		}
		if (d->labDrag->isVisible()) {
			d->labDrag->move(e->pos() - QPoint(d->labDrag->width() / 2, d->labDrag->height()));
		}
	} else if (e->buttons() == Qt::RightButton) {
		if (!d->labRect->isVisible()) {
			d->action = MERGE;
			setCursor(Qt::CrossCursor);
			d->labRect->setVisible(true);
		}
		if (d->labRect->isVisible()) {
			d->labRect->setGeometry(adjustRect(d->ptMousePress, e->pos()));
		}
	}
}

void CUVMultiView::mouseDoubleClickEvent(QMouseEvent* e) {
	Q_D(CUVMultiView);

	if (CUVVideoWidget* player = d->getPlayerByPos(e->pos())) {
		d->stretch(player);
		setLayout(1, 1);
	}
}
