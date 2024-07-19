#include "uvmaterialcheckable_internal.hpp"

#include <QIcon>
#include <QPainter>
#include <QTransform>
#include <utility>

#include "uvmaterialcheckable.hpp"

/*!
 *  \class CUVMaterialCheckableIcon
 *  \internal
 */
CUVMaterialCheckableIcon::CUVMaterialCheckableIcon(QIcon icon, CUVMaterialCheckable* parent)
: QWidget(parent), m_checkable(parent), m_color(Qt::black), m_icon(std::move(icon)), m_iconSize(24), m_opacity(1.0) {
	Q_ASSERT(parent);

	setAttribute(Qt::WA_TransparentForMouseEvents);
}

CUVMaterialCheckableIcon::~CUVMaterialCheckableIcon() = default;

QSize CUVMaterialCheckableIcon::sizeHint() const {
	return { static_cast<int>(m_iconSize), static_cast<int>(m_iconSize) };
}

void CUVMaterialCheckableIcon::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event)

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setOpacity(m_opacity);

	QPixmap pixmap = icon().pixmap(24, 24);

	if (!pixmap.isNull()) {
		const qreal p = static_cast<qreal>((height()) - m_iconSize) / 2;
		const qreal z = m_iconSize / 24;

		QTransform t;
		if (CUVMaterialCheckable::LabelPositionLeft == m_checkable->labelPosition()) {
			t.translate(p + width() - 42, p);
		} else {
			t.translate(p, p);
		}
		t.scale(z, z);
		painter.setTransform(t);

		QPainter icon(&pixmap);
		icon.setCompositionMode(QPainter::CompositionMode_SourceIn);
		icon.fillRect(pixmap.rect(), color());
		painter.drawPixmap(0, 0, pixmap);
	}
}
