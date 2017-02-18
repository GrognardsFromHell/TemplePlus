TEMPLATE = lib
CONFIG += plugin
TARGET = QtCreatorPlugin

DEFINES += QTCREATORPLUGIN

QT += qml quick quick-private network

BASEDIR = $$PWD/../../

TPSOURCES = $$BASEDIR/TemplePlus/qml/

CONFIG(debug, debug|release) {
    CONFIG_NAME = debug
	LIBS += $$BASEDIR/dependencies/lib/turbojpeg-static_d.lib
} else {
    CONFIG_NAME = release	
	LIBS += $$BASEDIR/dependencies/lib/turbojpeg-static.lib
}

TARGET_DIR = $$BASEDIR/$$CONFIG_NAME
DESTDIR = $$TARGET_DIR/TemplePlus

INCLUDEPATH += $$BASEDIR/Temple/include
INCLUDEPATH += $$BASEDIR/Infrastructure/include

LIBS += -lole32 -lgdi32 -luser32 $$TARGET_DIR/Infrastructure.lib $$TARGET_DIR/Temple.lib

SOURCES += $$TPSOURCES/imagehandler.cpp \
	$$TPSOURCES/imageplugin.cpp \
	$$TPSOURCES/legacytextitem.cpp \
	$$TPSOURCES/legacytextrenderer.cpp \
	$$TPSOURCES/networkaccessmanager.cpp \
	$$TPSOURCES/qmlplugin.cpp

HEADERS += $$TPSOURCES/imagehandler.h \
	$$TPSOURCES/imageplugin.h \
	$$TPSOURCES/legacytextitem.h \
	$$TPSOURCES/legacytextrenderer.h \
	$$TPSOURCES/networkaccessmanager.h \
	$$TPSOURCES/qmlplugin.h
