#include "uvvideotitlebar.hpp"

#include <QHBoxLayout>

CUVVideoTitlebar::CUVVideoTitlebar(QWidget* parent) : QFrame(parent) {
	init();
}

CUVVideoTitlebar::~CUVVideoTitlebar() = default;

void CUVVideoTitlebar::init() {
	setFixedHeight(50);

	m_pLbTitle = new QLabel;
	m_pBtnClose = new QPushButton(tr("close"), this);

	const auto hbox = new QHBoxLayout;
	hbox->addWidget(m_pLbTitle);
	hbox->addStretch();
	hbox->addWidget(m_pBtnClose);
}
