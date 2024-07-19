#pragma once

#include <QtWidgets/QWidget>

#ifdef CUVMATERIALSHARED_LIB
#define CUVMATERIALSHARED_EXPORT Q_DECL_EXPORT
#else
#define CUVMATERIALSHARED_EXPORT Q_DECL_IMPORT
#endif

/*
 * @brief 在父组件上创建一个覆盖层，并根据父组件的移动和调整大小事件实时更新覆盖层的位置和大小
 */
class CUVMATERIALSHARED_EXPORT CUVMaterialOverlayWidget : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CUVMaterialOverlayWidget)

public:
	explicit CUVMaterialOverlayWidget(QWidget* parent = nullptr);
	~CUVMaterialOverlayWidget() override;

protected:
	bool event(QEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;

	[[nodiscard]] virtual QRect overlayGeometry() const;
};
