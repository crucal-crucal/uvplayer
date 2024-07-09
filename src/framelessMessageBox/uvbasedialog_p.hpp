#pragma once

#include <QDialogButtonBox>
#include <QVBoxLayout>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QPainter;
class CUVBaseDialog;

class CUVBaseDialogPrivate final : public QObject {
	Q_OBJECT

	Q_DISABLE_COPY(CUVBaseDialogPrivate)
	Q_DECLARE_PUBLIC(CUVBaseDialog)

public:
	explicit CUVBaseDialogPrivate(CUVBaseDialog* q);
	~CUVBaseDialogPrivate() override;

public slots:
	void switchSize();

public:
	void init();

	CUVBaseDialog* const q_ptr{ nullptr };

	QDialogButtonBox* m_pDialogBtnBox{ nullptr };
	QVBoxLayout* m_plyVTotal{ nullptr };
	QHBoxLayout* m_plyHTitle{ nullptr };
	QHBoxLayout* m_plyHContent{ nullptr };

	QString m_strTitle{};
	QString m_strIconPath{};
	QPixmap m_icon{};
	QPoint m_PressTitlePoint{};
	QRect m_rtTitle{};
	bool m_bPressTitle{ false };
	bool m_bResizeable{ false };
	bool m_bIconScaled{ false };
	bool m_bEscEnable{ false };
	bool m_bMoveEnable{ true };
	bool m_bTitleVisible{ true };
	bool m_bShadowVisible{ true };
	bool m_bEnterEnable{ true };
	QHash<CUVBaseDialog::TitleButtonRole, QAbstractButton*> m_hashTitleButtons;
};
