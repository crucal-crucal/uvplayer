#include "uvvideotitlebar.hpp"

#include <QHBoxLayout>

#include <uvfunctions.hpp>

CUVVideoTitlebar::CUVVideoTitlebar(QWidget* parent) : QFrame(parent) {
	init();
}

CUVVideoTitlebar::~CUVVideoTitlebar() = default;

void CUVVideoTitlebar::init() {
	setFixedHeight(50);

	labTitle = new QLabel(this);
	btnClose = getPushButton(QPixmap(":/image/close.png"), tr("Close"), {}, this);

	const auto hbox = new QHBoxLayout;
	hbox->setContentsMargins(10,1,10,1);
	hbox->setSpacing(1);
	hbox->addWidget(labTitle);
	hbox->addStretch();
	hbox->addWidget(btnClose);

	setLayout(hbox);
}
