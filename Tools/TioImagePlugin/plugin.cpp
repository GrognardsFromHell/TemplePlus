
#include <infrastructure/images.h>

#include <QtCore/QSize>
#include <QtCore/QVariant>
#include <QtGui/QImage>
#include <QtGui/QImageIOHandler>

#ifdef NDEBUG
#define QT_NO_DEBUG
#endif

class QTioHandler : public QImageIOHandler
{
public:
	QTioHandler();
	~QTioHandler();

	bool canRead() const;
	bool read(QImage *image);

	QByteArray name() const;

	static bool canRead(QIODevice *device);

	QVariant option(ImageOption option) const;
	void setOption(ImageOption option, const QVariant &value);
	bool supportsOption(ImageOption option) const;

private:
	mutable bool mHasRead = false;
	mutable gfx::DecodedImage mImage;

	QImage::Format GetImageFormat() const {
		if (mImage.info.hasAlpha) {
			return QImage::Format_ARGB32;
		} else {
			return QImage::Format_RGB32;
		}
	}
};


QTioHandler::QTioHandler() : QImageIOHandler()
{
}

QTioHandler::~QTioHandler()
{
}

bool QTioHandler::canRead() const
{

	if (!mHasRead) {
		// TGA reader implementation needs a seekable QIODevice, so
		// sequential devices are not supported
		if (device()->isSequential())
			return false;

		qint64 pos = device()->pos();

		QByteArray imgData = device()->readAll();
		mImage = gfx::DecodeImage({ (uint8_t*)imgData.data(), imgData.size() });
		mHasRead = true;
		device()->seek(pos);
	}
	
	return mImage.info.format != gfx::ImageFileFormat::Unknown;
}

bool QTioHandler::canRead(QIODevice *device)
{
	if (!device) {
		qWarning("QTioHandler::canRead() called with no device");
		return false;
	}

	// TGA reader implementation needs a seekable QIODevice, so
	// sequential devices are not supported
	if (device->isSequential())
		return false;

	qint64 pos = device->pos();

	bool isValid;
	{
		QByteArray imgData = device->readAll();
		auto fileInfo = gfx::DetectImageFormat({ (uint8_t*) imgData.data(), imgData.size() });		
		isValid = (fileInfo.format != gfx::ImageFileFormat::Unknown);
	}
	device->seek(pos);
	return isValid;
}

bool QTioHandler::read(QImage *image)
{
	if (!canRead())
		return false;

	*image = QImage(mImage.info.width, mImage.info.height, GetImageFormat());

	auto dest = image->bits();
	auto src = mImage.data.get();
	for (int y = 0; y < mImage.info.height; y++) {
		memcpy(dest, src, 4 * mImage.info.width);
		src += mImage.info.width * 4;
		dest += image->bytesPerLine();
	}

	return true;
}

QByteArray QTioHandler::name() const
{
	return "tga";
}

QVariant QTioHandler::option(ImageOption option) const
{
	if (option == Size && canRead()) {
		return QSize(mImage.info.width, mImage.info.height);
	}
	else if (option == ImageFormat) {
		return QImage::Format_ARGB32;
	}
	return QVariant();
}

void QTioHandler::setOption(ImageOption option, const QVariant &value)
{
	Q_UNUSED(option);
	Q_UNUSED(value);
}

bool QTioHandler::supportsOption(ImageOption option) const
{
	return option == CompressionRatio
		|| option == Size
		|| option == ImageFormat;
}

class QTioImagePlugin : public QImageIOPlugin
{
	Q_OBJECT
		Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "tio.json")

public:
	Capabilities capabilities(QIODevice * device, const QByteArray & format) const;
	QImageIOHandler * create(QIODevice * device, const QByteArray & format = QByteArray()) const;
	QStringList keys() const;
};

QImageIOPlugin::Capabilities QTioImagePlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
	if (format == "tga")
		return Capabilities(CanRead);
	if (!format.isEmpty())
		return 0;
	if (!device->isOpen())
		return 0;

	Capabilities cap;
	if (device->isReadable() && QTioHandler::canRead(device))
		cap |= CanRead;
	return cap;
}

QImageIOHandler* QTioImagePlugin::create(QIODevice *device, const QByteArray &format) const
{
	QImageIOHandler *tioHandler = new QTioHandler();
	tioHandler->setDevice(device);
	tioHandler->setFormat(format);
	return tioHandler;
}

QStringList QTioImagePlugin::keys() const
{
	return QStringList() << QLatin1String("tio");
}

#include "moc.h"
