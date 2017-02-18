
#pragma once

#include <QTranslator>
#include <QHash>

class MesTranslator : public QTranslator {
public:
	MesTranslator(QObject *parent);
	
	QString translate(const char *context, const char *sourceText,
		const char *disambiguation = Q_NULLPTR, int n = -1) const override;

	bool isEmpty() const override;

private:
	QHash<QString, QHash<int, QString>> mMesFiles;

	void LoadMesFile(QString alias, const std::string &filename);
};
