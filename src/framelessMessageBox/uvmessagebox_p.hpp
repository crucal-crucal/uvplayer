#pragma once

#include <QMessageBox>

namespace UVMessageBox {
class CUVMessageBox;
class CUVCountdownMessageBox;
}

class CUVMessageBoxPrivate {
	Q_DISABLE_COPY(CUVMessageBoxPrivate)
	Q_DECLARE_PUBLIC(UVMessageBox::CUVMessageBox)

public:
	explicit CUVMessageBoxPrivate(UVMessageBox::CUVMessageBox* q);
	~CUVMessageBoxPrivate();

	UVMessageBox::CUVMessageBox* const q_ptr{ nullptr };

	static QMessageBox::ButtonRole standardConvertToRole(QMessageBox::StandardButton button);

	QMessageBox* m_pMessageBox{ nullptr };
	QMessageBox::StandardButtons buttons_;
	Qt::WindowFlags flags_;
};
