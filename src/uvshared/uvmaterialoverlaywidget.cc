#include "uvmaterialoverlaywidget.hpp"

#include <QEvent>

/*!
 *  \class CUVMaterialOverlayWidget
 */
CUVMaterialOverlayWidget::CUVMaterialOverlayWidget(QWidget* parent): QWidget(parent) {
	if (parent) {
		parent->installEventFilter(this);
	}
}

CUVMaterialOverlayWidget::~CUVMaterialOverlayWidget() = default;

bool CUVMaterialOverlayWidget::event(QEvent* event) {
	if (!parent()) {
		return QWidget::event(event);
	}
	switch (event->type()) {
		case QEvent::ParentChange: {
			parent()->installEventFilter(this);
			setGeometry(overlayGeometry());
			break;
		}
		case QEvent::ParentAboutToChange: {
			parent()->removeEventFilter(this);
			break;
		}
		default:
			break;
	}
	return QWidget::event(event);
}

bool CUVMaterialOverlayWidget::eventFilter(QObject* watched, QEvent* event) {
	switch (event->type()) {
		case QEvent::Move:
		case QEvent::Resize:
			setGeometry(overlayGeometry());
			break;
		default:
			break;
	}
	return QWidget::eventFilter(watched, event);
}

QRect CUVMaterialOverlayWidget::overlayGeometry() const {
	const QWidget* widget = parentWidget();
	if (!widget) {
		return {};
	}
	return widget->rect();
}
