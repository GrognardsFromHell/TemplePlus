
#include <infrastructure/mesparser.h>
#include <infrastructure/vfs.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <infrastructure/exception.h>

#include "mestranslator.h"

MesTranslator::MesTranslator(QObject * parent) : QTranslator(parent)
{

	auto mesFilesStr = vfs->ReadAsString("ui/mesfiles.json");

	QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(mesFilesStr));
	auto rootObj = jsonDoc.object();

	for (auto alias : rootObj.keys()) {
		auto filename = rootObj.take(alias).toString();
		LoadMesFile(alias, filename.toStdString());
	}

}

QString MesTranslator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const {

	// Split incoming source Text
	auto sep = strchr(sourceText, ':');
	if (!sep) {
		return QString::fromLatin1(sourceText);
	}

	QString mesFileAlias = QString::fromLatin1(sourceText, sep - sourceText);

	auto it = mMesFiles.constFind(mesFileAlias);

	if (it == mMesFiles.end()) {
		qDebug() << "Mes file with alias" << mesFileAlias << "not registered.";
		return QString::fromLatin1(sourceText);
	}

	int key = atoi(sep + 1);
	
	auto it2 = it->constFind(key);

	if (it2 == it->constEnd()) {
		qDebug() << "Unknown mes-file key" << key << "in mes-file alias" << mesFileAlias << "used.";
		return QString::fromLatin1(sourceText);
	}

	return it2.value();

}

bool MesTranslator::isEmpty() const {
	return false;
}

void MesTranslator::LoadMesFile(QString alias, const std::string & filename)
{	
	if (mMesFiles.contains(alias)) {
		throw TempleException("MES File {} already loaded", alias.toStdString());
	}

	auto &translations = mMesFiles[alias];

	auto content = MesFile::ParseFile(filename);

	for (auto &entry : content) {
		translations[entry.first] = QString::fromStdString(entry.second);
	}

}
