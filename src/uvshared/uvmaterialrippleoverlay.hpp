#pragma once

#include <QPainterPath>

#include "uvmaterialoverlaywidget.hpp"

class CUVMaterialRipple;

#ifdef CUVMATERIALSHARED_LIB
#define CUVMATERIALSHARED_EXPORT Q_DECL_EXPORT
#else
#define CUVMATERIALSHARED_EXPORT Q_DECL_IMPORT
#endif

class CUVMATERIALSHARED_EXPORT CUVMaterialRippleOverlay : public CUVMaterialOverlayWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialRippleOverlay)

public:
	explicit CUVMaterialRippleOverlay(QWidget* parent = nullptr);
	~CUVMaterialRippleOverlay() override;

	void addRipple(CUVMaterialRipple* ripple);
	void addRipple(const QPoint& position, qreal radius = 300);

	void removeRipple(CUVMaterialRipple* ripple);

	inline void setClipping(bool enable);
	[[nodiscard]] inline bool hasClipping() const;

	inline void setClipPath(const QPainterPath& path);

protected:
	void paintEvent(QPaintEvent* event) override;

	[[nodiscard]] inline QList<CUVMaterialRipple*> ripples() const;

private:
	static void paintRipple(QPainter* painter, const CUVMaterialRipple* ripple);

	QList<CUVMaterialRipple*> m_ripples{};
	QPainterPath m_clipPath{};
	bool m_useClip{};
};

inline void CUVMaterialRippleOverlay::setClipping(const bool enable) {
	m_useClip = enable;
	update();
}

inline bool CUVMaterialRippleOverlay::hasClipping() const {
	return m_useClip;
}

inline void CUVMaterialRippleOverlay::setClipPath(const QPainterPath& path) {
	m_clipPath = path;
	update();
}

inline QList<CUVMaterialRipple*> CUVMaterialRippleOverlay::ripples() const {
	return m_ripples;
}
