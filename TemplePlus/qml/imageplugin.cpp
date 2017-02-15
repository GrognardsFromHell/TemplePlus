
/*
	Required to have the plugin contained in this file be statically linked
*/

#include "imageplugin.h"
#include "imagehandler.h"

QImageIOPlugin::Capabilities TPImagePlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
	if (format == "tga")
		return Capabilities(CanRead);
	if (!format.isEmpty())
		return 0;
	if (!device->isOpen())
		return 0;

	Capabilities cap = 0;
	if (device->isReadable() && TPImageHandler::canRead(device))
		cap |= CanRead;
	return cap;
}

QImageIOHandler* TPImagePlugin::create(QIODevice *device, const QByteArray &format) const
{
	QImageIOHandler *tioHandler = new TPImageHandler();
	tioHandler->setDevice(device);
	tioHandler->setFormat(format);
	return tioHandler;
}

QStringList TPImagePlugin::keys() const
{
	return QStringList() << QLatin1String("tpimageplugin");
}


Q_IMPORT_PLUGIN(TPImagePlugin);
