#pragma once

#include <QSlider>

#ifdef CUVSLIDER_LIB
#define CUVSLIDER_EXPORT Q_DECL_EXPORT
#else
#define CUVSLIDER_EXPORT Q_DECL_IMPORT
#endif

class CUVSLIDER_EXPORT CUVSlider final : public QSlider {
	Q_OBJECT

public:
	explicit CUVSlider(QWidget* parent = nullptr);
	explicit CUVSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
	~CUVSlider() override;
};
