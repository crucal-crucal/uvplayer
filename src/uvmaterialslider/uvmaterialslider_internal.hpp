#pragma once

#include <QStateMachine>

#include "uvshared/uvmaterialoverlaywidget.hpp"

class CUVMaterialSlider;
class CUVMaterialSliderThumb;
class CUVMaterialSliderTrack;

class CUVMaterialSliderStateMachine final : public QStateMachine {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialSliderStateMachine)

public:
	explicit CUVMaterialSliderStateMachine(CUVMaterialSlider* slider, CUVMaterialSliderThumb* thumb, CUVMaterialSliderTrack* track);
	~CUVMaterialSliderStateMachine() override;

	void setupProperties() const;

private:
	CUVMaterialSlider* const m_slider{ nullptr };
	CUVMaterialSliderThumb* const m_thumb{ nullptr };
	CUVMaterialSliderTrack* const m_track{ nullptr };
	QState* const m_topState{ nullptr };
	QState* const m_fstState{ nullptr };
	QState* const m_sndState{ nullptr };
	QState* const m_inactiveState{ nullptr };
	QState* const m_focusState{ nullptr };
	QState* const m_slidingState{ nullptr };
	QState* const m_pulseOutState{ nullptr };
	QState* const m_pulseInState{ nullptr };
	QState* const m_minState{ nullptr };
	QState* const m_normalState{ nullptr };
};

class CUVMaterialSliderThumb final : public CUVMaterialOverlayWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialSliderThumb)

	Q_PROPERTY(qreal diameter WRITE setDiameter READ diameter)
	Q_PROPERTY(qreal borderWidth WRITE setBorderWidth READ borderWidth)
	Q_PROPERTY(QColor borderColor WRITE setBorderColor READ borderColor)
	Q_PROPERTY(QColor fillColor WRITE setFillColor READ fillColor)
	Q_PROPERTY(qreal haloSize WRITE setHaloSize READ haloSize)
	Q_PROPERTY(QColor haloColor WRITE setHaloColor READ haloColor)

public:
	explicit CUVMaterialSliderThumb(CUVMaterialSlider* slider);
	~CUVMaterialSliderThumb() override;

	inline void setDiameter(qreal diameter);
	[[nodiscard]] inline qreal diameter() const;

	inline void setBorderWidth(qreal width);
	[[nodiscard]] inline qreal borderWidth() const;

	inline void setBorderColor(const QColor& color);
	[[nodiscard]] inline QColor borderColor() const;

	inline void setFillColor(const QColor& color);
	[[nodiscard]] inline QColor fillColor() const;

	inline void setHaloSize(qreal size);
	[[nodiscard]] inline qreal haloSize() const;

	inline void setHaloColor(const QColor& color);
	[[nodiscard]] inline QColor haloColor() const;

	inline void setOffset(int offset);
	[[nodiscard]] inline int offset() const;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

private:
	const CUVMaterialSlider* const m_slider{ nullptr };
	QColor m_borderColor{};
	QColor m_fillColor{};
	QColor m_haloColor{};
	qreal m_diameter = 30;
	qreal m_borderWidth{ 2 };
	qreal m_haloSize = 50;
	int m_offset{};
};

inline void CUVMaterialSliderThumb::setDiameter(const qreal diameter) {
	m_diameter = diameter;
	update();
}

inline qreal CUVMaterialSliderThumb::diameter() const {
	return m_diameter;
}

inline void CUVMaterialSliderThumb::setBorderWidth(const qreal width) {
	m_borderWidth = width;
	update();
}

inline qreal CUVMaterialSliderThumb::borderWidth() const {
	return m_borderWidth;
}

inline void CUVMaterialSliderThumb::setBorderColor(const QColor& color) {
	m_borderColor = color;
	update();
}

inline QColor CUVMaterialSliderThumb::borderColor() const {
	return m_borderColor;
}

inline void CUVMaterialSliderThumb::setFillColor(const QColor& color) {
	m_fillColor = color;
	update();
}

inline QColor CUVMaterialSliderThumb::fillColor() const {
	return m_fillColor;
}

inline void CUVMaterialSliderThumb::setHaloSize(const qreal size) {
	m_haloSize = size;
	update();
}

inline qreal CUVMaterialSliderThumb::haloSize() const {
	return m_haloSize;
}

inline void CUVMaterialSliderThumb::setHaloColor(const QColor& color) {
	m_haloColor = color;
	update();
}

inline QColor CUVMaterialSliderThumb::haloColor() const {
	return m_haloColor;
}

inline void CUVMaterialSliderThumb::setOffset(const int offset) {
	m_offset = offset;
	update();
}

inline int CUVMaterialSliderThumb::offset() const {
	return m_offset;
}

class CUVMaterialSliderTrack final : public CUVMaterialOverlayWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialSliderTrack)

	Q_PROPERTY(QColor fillColor WRITE setFillColor READ fillColor)

public:
	explicit CUVMaterialSliderTrack(CUVMaterialSliderThumb* thumb, CUVMaterialSlider* slider);
	~CUVMaterialSliderTrack() override;

	inline void setFillColor(const QColor& color);
	[[nodiscard]] inline QColor fillColor() const;

	inline void setTrackWidth(int width);
	[[nodiscard]] inline int trackWidth() const;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

private:
	const CUVMaterialSlider* const m_slider{ nullptr };
	CUVMaterialSliderThumb* const m_thumb{ nullptr };
	QColor m_fillColor{};
	int m_trackWidth{};
};

inline void CUVMaterialSliderTrack::setFillColor(const QColor& color) {
	m_fillColor = color;
	update();
}

inline QColor CUVMaterialSliderTrack::fillColor() const {
	return m_fillColor;
}

void CUVMaterialSliderTrack::setTrackWidth(const int width) {
	m_trackWidth = width;
	update();
}

int CUVMaterialSliderTrack::trackWidth() const {
	return m_trackWidth;
}
