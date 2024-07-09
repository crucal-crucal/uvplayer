#pragma once

#include <QFileDialog>

class CUVFileBase;

class CUVFileBasePrivate : public QObject {
	Q_OBJECT

	Q_DISABLE_COPY(CUVFileBasePrivate)
	Q_DECLARE_PUBLIC(CUVFileBase)

public:
	explicit CUVFileBasePrivate(CUVFileBase* q);
	~CUVFileBasePrivate() override;

	CUVFileBase* const q_ptr{ nullptr };
	void init(const QString& strRegisterName, const QString& caption, const QString& directory, const QString& filter);

	QFileDialog* m_pFileDialog{ nullptr };
	QString m_strRegisterName{ "" };
};
