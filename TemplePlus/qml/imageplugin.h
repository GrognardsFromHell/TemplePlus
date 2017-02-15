
#pragma once

#define QT_STATICPLUGIN

#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtGui/QImageIOHandler>
#include <QtCore/QSize>
#include <QtGui/QImage>

class TPImagePlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "tpimageplugin.json")

public:
    Capabilities capabilities(QIODevice * device, const QByteArray & format) const;
    QImageIOHandler * create(QIODevice * device, const QByteArray & format = QByteArray()) const;
    QStringList keys() const;
};
