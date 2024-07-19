#pragma once

#include <QIcon>
#include <QtWidgets/QWidget>

class CUVMaterialCheckable;

#ifdef CUVMATERIALSHARED_LIB
#define CUVMATERIALSHARED_EXPORT Q_DECL_EXPORT
#else
#define CUVMATERIALSHARED_EXPORT Q_DECL_IMPORT
#endif

class CUVMATERIALSHARED_EXPORT CUVMaterialCheckableIcon final : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialCheckableIcon)

	Q_PROPERTY(QColor color READ color WRITE setColor)
	Q_PROPERTY(qreal iconSize READ iconSize WRITE setIconSize)
	Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
	CUVMaterialCheckableIcon(QIcon icon, CUVMaterialCheckable* parent);
	~CUVMaterialCheckableIcon() override;

	[[nodiscard]] QSize sizeHint() const override;

	inline void setIcon(const QIcon& icon);
	[[nodiscard]] inline QIcon icon() const;

	inline void setColor(const QColor& color);
	[[nodiscard]] inline QColor color() const;

	inline void setIconSize(qreal size);
	[[nodiscard]] inline qreal iconSize() const;

	inline void setOpacity(qreal opacity);
	[[nodiscard]] inline qreal opacity() const;

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	CUVMaterialCheckable* const m_checkable{ nullptr };
	QColor m_color{};
	QIcon m_icon{};
	qreal m_iconSize{};
	qreal m_opacity{};
};

inline void CUVMaterialCheckableIcon::setIcon(const QIcon& icon) {
	m_icon = icon;
	update();
}

inline QIcon CUVMaterialCheckableIcon::icon() const {
	return m_icon;
}

inline void CUVMaterialCheckableIcon::setColor(const QColor& color) {
	m_color = color;
	update();
}

inline QColor CUVMaterialCheckableIcon::color() const {
	return m_color;
}

inline void CUVMaterialCheckableIcon::setIconSize(const qreal size) {
	m_iconSize = size;
	update();
}

inline qreal CUVMaterialCheckableIcon::iconSize() const {
	return m_iconSize;
}

inline void CUVMaterialCheckableIcon::setOpacity(const qreal opacity) {
	m_opacity = opacity;
	update();
}

inline qreal CUVMaterialCheckableIcon::opacity() const {
	return m_opacity;
}
