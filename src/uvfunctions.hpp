#pragma once

#include <QPushButton>
#include <QPixmap>

inline QPushButton* getPushButton(const QPixmap& pixmap, const QString& tooltip = QString(), QSize sz = QSize(0, 0), QWidget* parent = nullptr) {
	auto btn = new QPushButton(parent);
	btn->setFlat(true);
	if (sz.isEmpty()) {
		sz = pixmap.size();
	}
	btn->setFixedSize(sz);
	btn->setIconSize(sz);
	btn->setIcon(pixmap.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	btn->setToolTip(tooltip);
	return btn;
}
