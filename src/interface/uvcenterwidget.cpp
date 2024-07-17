#include "uvcenterwidget.hpp"

#include <QHBoxLayout>
#include <QSplitter>

#include "def/uvdef.hpp"

CUVCenterWidget::CUVCenterWidget(QWidget* parent) : QWidget(parent) {
	init();
	initConnect();
}

CUVCenterWidget::~CUVCenterWidget() = default;

void CUVCenterWidget::init() {
	lside = new QWidget(this);
	mv = new CUVMultiView(this);
	rside = new QWidget(this);

	auto split = new QSplitter(Qt::Horizontal);
	split->addWidget(lside);
	split->addWidget(mv);
	split->addWidget(rside);

	lside->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	lside->setMinimumWidth(LSIDE_MIN_WIDTH);
	lside->setMaximumWidth(LSIDE_MAX_WIDTH);

	mv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mv->setMinimumWidth(MV_MIN_WIDTH);
	mv->setMinimumHeight(MV_MIN_HEIGHT);

	rside->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	rside->setMinimumWidth(RSIDE_MIN_WIDTH);
	rside->setMaximumWidth(RSIDE_MAX_WIDTH);

	split->setStretchFactor(0, 1);
	split->setStretchFactor(1, 1);
	split->setStretchFactor(2, 1);

	auto hbox = new QHBoxLayout();
	hbox->setContentsMargins(10,1,10,1);
	hbox->setSpacing(1);
	hbox->addWidget(split);
	setLayout(hbox);
}

void CUVCenterWidget::initConnect() {
}
