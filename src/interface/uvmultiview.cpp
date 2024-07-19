#include "uvmultiview.hpp"

#include "uvmultiview_p.hpp"
#include "conf/uvconf.hpp"
#include "def/uvdef.hpp"
#include "framelessMessageBox/uvmessagebox.hpp"

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

	const int row = g_confile->get<int>("mv_row", "ui", MV_STYLE_ROW);
	const int col = g_confile->get<int>("mv_col", "ui", MV_STYLE_COL);
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

	const int row = table.row, col = table.col;
	if (row == 0 || col == 0) return;
	const int cell_w = q->width() / col, cell_h = q->height() / row;

	const int margin_x = (q->width() - cell_w * col) / 2, margin_y = (q->height() - cell_h * row) / 2;
	int x = margin_x, y = margin_y;
	for (const auto& view: views) {
		view->hide();
	}

	int cnt = 0;
	CUVTableCell cell{};
	for (int r = 0; r < row; ++r) {
		for (int c = 0; c < col; ++c) {
			if (const int id = r * col + c + 1; table.getTableCell(id, cell)) {
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
	std::swap(prev_table, table);
	updateUI();
}

void CUVMultiViewPrivate::mergeCells(const int lt, const int rb) {
	// find first non-stop player as lt
	for (int i = lt; i <= rb; ++i) {
		if (CUVVideoWidget* player = getPlayerByID(i); player && player->status != CUVVideoWidget::STOP) {
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

	const QRect rcTmp = player1->geometry();
	const int idTmp = player1->playerid;

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
		for (const auto& view: views) {
			view->hide();
		}
		wdg->setGeometry(q->rect());
		wdg->show();
		bStretch = true;
	}
}

CUVVideoWidget* CUVMultiViewPrivate::getPlayerByID(const int playerid) {
	for (const auto& view: views) {
		if (const auto player = dynamic_cast<CUVVideoWidget*>(view); player->playerid == playerid) {
			return player;
		}
	}
	return nullptr;
}

CUVVideoWidget* CUVMultiViewPrivate::getPlayerByPos(const QPoint& point) {
	for (const auto& wdg: views) {
		if (wdg->isVisible() && wdg->geometry().contains(point)) {
			return dynamic_cast<CUVVideoWidget*>(wdg);
		}
	}
	return nullptr;
}

CUVVideoWidget* CUVMultiViewPrivate::getIdlePlayer() {
	for (const auto& view: views) {
		if (const auto player = dynamic_cast<CUVVideoWidget*>(view); player->isVisible() && player->status == CUVVideoWidget::STOP) {
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

void CUVMultiView::setLayout(const int row, const int col) {
	Q_D(CUVMultiView);

	d->saveLayout();
	d->table.init(row, col);
	d->updateUI();
	g_confile->set<int>("mv_row", row, "ui");
	g_confile->set<int>("mv_col", col, "ui");
}

void CUVMultiView::play(const CUVMedia& media) {
	Q_D(CUVMultiView);

	if (CUVVideoWidget* player = d->getIdlePlayer(); !player) {
		UVMessageBox::CUVMessageBox::information(this, tr("Info"), tr("No spare player, please stop one and try agian!"));
	} else {
		player->open(media);
	}
}

void CUVMultiView::resizeEvent(QResizeEvent* event) {
	Q_D(CUVMultiView);

	d->updateUI();
}

void CUVMultiView::mousePressEvent(QMouseEvent* event) {
	Q_D(CUVMultiView);

	d->ptMousePress = event->pos();
	d->tsMousePress = gettick();
}

void CUVMultiView::mouseReleaseEvent(QMouseEvent* event) {
	Q_D(CUVMultiView);

	if (d->action == EXCHANGE) {
		CUVVideoWidget* player1 = d->getPlayerByPos(d->ptMousePress);
		if (CUVVideoWidget* player2 = d->getPlayerByPos(event->pos()); player1 && player2 && player1 != player2) {
			CUVMultiViewPrivate::exchangeCells(player1, player2);
		}
	} else if (d->action == MERGE) {
		const QRect rc = adjustRect(d->ptMousePress, event->pos());
		const CUVVideoWidget* player1 = d->getPlayerByPos(rc.topLeft());
		if (const CUVVideoWidget* player2 = d->getPlayerByPos(rc.bottomRight()); player1 && player2 && player1 != player2) {
			d->mergeCells(player1->playerid, player2->playerid);
		}
	}

	d->labRect->setVisible(false);
	d->labDrag->setVisible(false);
	setCursor(Qt::ArrowCursor);
}

void CUVMultiView::mouseMoveEvent(QMouseEvent* event) {
	Q_D(CUVMultiView);

	CUVVideoWidget* player = d->getPlayerByPos(d->ptMousePress);
	if (player == nullptr) {
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		if (!d->labDrag->isVisible()) {
			if (gettick() - d->tsMousePress < 300) return;
			d->action = EXCHANGE;
			setCursor(Qt::OpenHandCursor);
			d->labDrag->setPixmap(player->grab().scaled(d->labDrag->size()));
			d->labDrag->setVisible(true);
		}
		if (d->labDrag->isVisible()) {
			d->labDrag->move(event->pos() - QPoint(d->labDrag->width() / 2, d->labDrag->height()));
		}
	} else if (event->buttons() == Qt::RightButton) {
		if (!d->labRect->isVisible()) {
			d->action = MERGE;
			setCursor(Qt::CrossCursor);
			d->labRect->setVisible(true);
		}
		if (d->labRect->isVisible()) {
			d->labRect->setGeometry(adjustRect(d->ptMousePress, event->pos()));
		}
	}
}

void CUVMultiView::mouseDoubleClickEvent(QMouseEvent* event) {
	Q_D(CUVMultiView);

	if (CUVVideoWidget* player = d->getPlayerByPos(event->pos())) {
		d->stretch(player);
	}
}
