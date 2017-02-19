
#include <QtCore/QMimeDatabase>
#include <QtCore/QBuffer>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <infrastructure/vfs.h>
#include <infrastructure/images.h>

#include "networkaccessmanager.h"
#include "imagehandler.h"

QNetworkAccessManager * TPNetworkAccessManagerFactory::create(QObject * parent)
{
	return new TPNetworkAccessManager(parent);
}

TPNetworkAccessManager::TPNetworkAccessManager(QObject * parent) : QNetworkAccessManager(parent) {
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

QNetworkReply *TPNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
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

	QString contentType;
	qint64 size;

	// .img files need to be returned as a bundle since we cannot decode them otherwise
	// Image IO plugins will only get the single data stream, and not access to the original
	// path or filename
	if (filename.endsWith(".img", Qt::CaseInsensitive)) {
		try {
			auto imgContent(vfs->ReadAsBinary(localName));

			auto combinedImg = gfx::DecodeCombinedImage(localName, { (uint8_t*)imgContent.data(), (int) imgContent.size() });

			// write the decoded image raw to the network device, so it can be picked up 1:1 by the imageio plugin
			mDecodedData.open(QIODevice::WriteOnly);
			mDecodedData.seek(0);
			TPImageHandler::EncodeDecodedImage(combinedImg, &mDecodedData);
			mDecodedData.seek(0);
			mDecodedData.open(QIODevice::ReadOnly);

			contentType = "templeplus/img";
			size = mDecodedData.size();
		} catch (const std::exception &e) {
			const QString msg = QString("Unable to read %1: %2").arg(url.toString()).arg(e.what());
			setError(QNetworkReply::ProtocolFailure, msg);
			QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
				Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ProtocolFailure));
			QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
			return;
		}
	} else {

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
		contentType = mimeDatabase.mimeTypeForFileNameAndData(filename, payload).name();
		size = payload.size();

		mDecodedData.close();
		mDecodedData.setData(payload);
		mDecodedData.open(QIODevice::ReadOnly);
	}
	
	setHeader(QNetworkRequest::ContentTypeHeader, contentType);
	setHeader(QNetworkRequest::ContentLengthHeader, size);
	QMetaObject::invokeMethod(this, "metaDataChanged", Qt::QueuedConnection);

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
