#pragma once

#include <QtWidgets/QAbstractSlider>
#include <QScopedPointer>

#define CUV_MATERIAL_SLIDER_MARGIN 30

class CUVMaterialSliderPrivate;

#ifdef CUVMATERIALSLIDER_LIB
#define CUVMATERIALSLIDER_EXPORT Q_DECL_EXPORT
#else
#define CUVMATERIALSLIDER_EXPORT Q_DECL_IMPORT
#endif

class CUVMATERIALSLIDER_EXPORT CUVMaterialSlider : public QAbstractSlider {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialSlider)
	Q_DECLARE_PRIVATE(CUVMaterialSlider)

	Q_PROPERTY(QColor thumbColor WRITE setThumbColor READ thumbColor)
	Q_PROPERTY(QColor trackColor WRITE setTrackColor READ trackColor)
	Q_PROPERTY(QColor disabledColor WRITE setDisabledColor READ disabledColor)

public:
	explicit CUVMaterialSlider(QWidget* parent = nullptr);
	~CUVMaterialSlider() override;

	void setUseThemeColors(bool value);
	[[nodiscard]] bool useThemeColors() const;

	void setThumbColor(const QColor& color);
	[[nodiscard]] QColor thumbColor() const;

	void setThumbDiameter(qreal diameter);
	[[nodiscard]] qreal thumbDiameter() const;

	void setThumbHaloSize(qreal size);
	[[nodiscard]] qreal thumbHaloSize() const;

	void setTrackColor(const QColor& color);
	[[nodiscard]] QColor trackColor() const;

	void setTrackHeight(int height);
	[[nodiscard]] int trackHeight() const;

	void setDisabledColor(const QColor& color);
	[[nodiscard]] QColor disabledColor() const;

	void setPageStepMode(bool pageStep);
	[[nodiscard]] bool pageStepMode() const;

	[[nodiscard]] QSize minimumSizeHint() const override;

	void setInvertedAppearance(bool value); // NOLINT

	void custom_hide();
	void custom_show();

	void setvisible(bool visible);

protected:
	void sliderChange(SliderChange change) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;

	void updateThumbOffset();

	const QScopedPointer<CUVMaterialSliderPrivate> d_ptr{ nullptr };
};
