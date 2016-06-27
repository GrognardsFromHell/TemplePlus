
#include "stdafx.h"

#include <QtCore/QMimeDatabase>
#include <QtCore/QBuffer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <infrastructure/vfs.h>

#include "ui_qtquick_nam.h"

/**
 * This extends the network access manager to retrieve content from TIO.
 */
class TioNetworkAccessManager : public QNetworkAccessManager {
public:
	TioNetworkAccessManager(QObject *parent);
protected:
	QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData) override;
};

QNetworkAccessManager * CustomNAMFactory::create(QObject * parent)
{
	return new TioNetworkAccessManager(parent);
}

TioNetworkAccessManager::TioNetworkAccessManager(QObject * parent) : QNetworkAccessManager(parent) {
}

class TioNetworkReply : public QNetworkReply
{
public:
	TioNetworkReply(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op);
	~TioNetworkReply();
	virtual void abort() override;

	// reimplemented from QNetworkReply
	virtual qint64 bytesAvailable() const override;
	virtual bool isSequential() const override;
	qint64 size() const override;

	virtual qint64 readData(char *data, qint64 maxlen) override;

private:
	QBuffer mDecodedData;
};

QNetworkReply *TioNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
	auto scheme = req.url().scheme();

	// We redirect GETs on local files through TIO
	if (op == QNetworkAccessManager::GetOperation || op == QNetworkAccessManager::HeadOperation) {
		if (scheme == QLatin1String("tio")) {
			return new TioNetworkReply(this, req, op);
		}
	}

	return QNetworkAccessManager::createRequest(op, req, outgoingData);
}

TioNetworkReply::TioNetworkReply(QObject * parent, const QNetworkRequest & req, const QNetworkAccessManager::Operation op) : QNetworkReply(parent)
{
	setRequest(req);
	setUrl(req.url());
	setOperation(op);
	setFinished(true);
	QNetworkReply::open(QIODevice::ReadOnly);

	QUrl url = req.url();
	
	auto filename = url.path();
	if (filename.startsWith('/')) {
		filename = filename.right(filename.size() - 1);
	}

	auto localName = filename.toStdString();
	
	auto fh = vfs->Open(localName.c_str(), "rb");
	if (!fh) {
		const QString msg = QString("Unable to open %1").arg(url.toString());
		setError(QNetworkReply::ContentNotFoundError, msg);
		QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
			Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ContentNotFoundError));
		QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
		return;
	}

	auto len = vfs->Length(fh);
	QByteArray payload(len, Qt::Uninitialized);

	if (vfs->Read(payload.data(), len, fh) != len) {
		vfs->Close(fh);
		const QString msg = QString("Unable to read %1").arg(url.toString());
		setError(QNetworkReply::ProtocolFailure, msg);
		QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
			Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ProtocolFailure));
		QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
		return;
	}

	vfs->Close(fh);
	
	QMimeDatabase mimeDatabase;
	auto mimeType = mimeDatabase.mimeTypeForFileNameAndData(filename, payload);
	qint64 size = payload.size();
	setHeader(QNetworkRequest::ContentTypeHeader, mimeType.name());
	setHeader(QNetworkRequest::ContentLengthHeader, size);
	QMetaObject::invokeMethod(this, "metaDataChanged", Qt::QueuedConnection);

	mDecodedData.setData(payload);
	mDecodedData.open(QIODevice::ReadOnly);

	QMetaObject::invokeMethod(this, "downloadProgress", Qt::QueuedConnection,
		Q_ARG(qint64, size), Q_ARG(qint64, size));
	QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
	QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
	

}

TioNetworkReply::~TioNetworkReply()
{
}

void TioNetworkReply::abort()
{
	QNetworkReply::close();
}

qint64 TioNetworkReply::bytesAvailable() const
{
	return QNetworkReply::bytesAvailable() + mDecodedData.bytesAvailable();
}

bool TioNetworkReply::isSequential() const
{
	return true;
}

qint64 TioNetworkReply::size() const
{
	return mDecodedData.size();
}

qint64 TioNetworkReply::readData(char *data, qint64 maxlen)
{
	return mDecodedData.read(data, maxlen);
}
