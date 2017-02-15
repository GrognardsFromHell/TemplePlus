
#pragma once

#include <QImageIOHandler>
#include <QImage>

#include <infrastructure/images.h>

class TPImageHandler : public QImageIOHandler
{
public:
	TPImageHandler();
	~TPImageHandler();

	bool canRead() const override;
	bool read(QImage *image) override;

	static bool canRead(QIODevice *device);

	QVariant option(ImageOption option) const override;
	void setOption(ImageOption option, const QVariant &value) override;
	bool supportsOption(ImageOption option) const override;

	static void EncodeDecodedImage(const gfx::DecodedImage &image, QIODevice *out);
	static bool IsEncodedDecodedImage(QIODevice *device);
	static bool DecodeEncodedDecodedImage(QIODevice *in, gfx::DecodedImage *image);

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
