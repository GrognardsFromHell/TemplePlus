
#include "imagehandler.h"

#include <infrastructure/images.h>

#include <QDataStream>
#include <QVariant>

// Used to mark a verbatim encoded IMG file
static const qint64 MAGIC = 0x2ba72df42a9d92c4;

TPImageHandler::TPImageHandler() : QImageIOHandler()
{
}

TPImageHandler::~TPImageHandler()
{
}

bool TPImageHandler::canRead() const
{

	if (!mHasRead) {
		// TGA reader implementation needs a seekable QIODevice, so
		// sequential devices are not supported
		if (device()->isSequential())
			return false;

		qint64 pos = device()->pos();

		if (DecodeEncodedDecodedImage(device(), &mImage)) {
			setFormat("img");
		}
		else {
			device()->seek(pos);

			QByteArray imgData = device()->readAll();
			mImage = gfx::DecodeImage({ (uint8_t*)imgData.data(), imgData.size() });
			setFormat("tga");
		}

		mHasRead = true;
		device()->seek(pos);
	}

	return mImage.info.format != gfx::ImageFileFormat::Unknown;
}

bool TPImageHandler::canRead(QIODevice *device)
{
	if (!device) {
		qWarning("TPImageHandler::canRead() called with no device");
		return false;
	}

	// TGA reader implementation needs a seekable QIODevice, so
	// sequential devices are not supported
	if (device->isSequential())
		return false;

	// check the IO device
	if (IsEncodedDecodedImage(device)) {
		return true;
	}

	qint64 pos = device->pos();

	// No sensible image is below this size
	if (device->size() < 16) {
		return false;
	}

	bool isValid;
	{
		QByteArray imgData = device->readAll();
		auto fileInfo = gfx::DetectImageFormat({ (uint8_t*)imgData.data(), imgData.size() });
		isValid = (fileInfo.format != gfx::ImageFileFormat::Unknown);
	}
	device->seek(pos);
	return isValid;
}

bool TPImageHandler::read(QImage *image)
{
	if (!canRead())
		return false;

	*image = QImage(mImage.info.width, mImage.info.height, GetImageFormat());

	const auto bpl = 4 * mImage.info.width;
	Q_ASSERT(image->bytesPerLine() >= bpl);
	auto src = mImage.data.get();
	for (int y = 0; y < mImage.info.height; y++) {
		auto dest = image->scanLine(y);
		memcpy(dest, src, bpl);
		src += bpl;
	}

	return true;
}

QVariant TPImageHandler::option(ImageOption option) const
{
	if (option == Size && canRead()) {
		return QSize(mImage.info.width, mImage.info.height);
	}
	else if (option == ImageFormat) {
		return GetImageFormat();
	}
	return QVariant();
}

void TPImageHandler::setOption(ImageOption option, const QVariant &value)
{
	Q_UNUSED(option);
	Q_UNUSED(value);
}

bool TPImageHandler::supportsOption(ImageOption option) const
{
	return option == CompressionRatio
		|| option == Size
		|| option == ImageFormat;
}

void TPImageHandler::EncodeDecodedImage(const gfx::DecodedImage &image, QIODevice *out)
{
	QDataStream streamOut(out);
	streamOut << MAGIC
		<< image.info.hasAlpha
		<< image.info.width
		<< image.info.height;
	auto fullSize = image.info.width * image.info.height * 4;
	out->write((const char*)image.data.get(), fullSize);
}

bool TPImageHandler::IsEncodedDecodedImage(QIODevice * device)
{
	Q_ASSERT(!device->isSequential());
	auto pos = device->pos();
	QDataStream checkStr(device);
	qint64 magic;
	checkStr >> magic;
	auto valid = (MAGIC == magic);
	device->seek(pos);
	return valid;
}

bool TPImageHandler::DecodeEncodedDecodedImage(QIODevice *device, gfx::DecodedImage * image)
{
	Q_ASSERT(!device->isSequential());

	qint64 magic;
	QDataStream in(device);
	in >> magic;

	if (magic != MAGIC) {
		return false;
	}

	in >> image->info.hasAlpha
		>> image->info.width
		>> image->info.height;
	uint fullSize = image->info.width * image->info.height * 4;
	image->data = std::make_unique<uint8_t[]>(fullSize);
	if (device->read((char*)image->data.get(), fullSize) != fullSize) {
		return false;
	}

	image->info.format = gfx::ImageFileFormat::IMG;

	return true;
}
