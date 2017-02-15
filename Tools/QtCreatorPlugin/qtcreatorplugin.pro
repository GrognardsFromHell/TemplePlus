
TEMPLATE = lib
TARGET = TioIntegration
QT += qml

MAKE_CXXFLAGS+=/Zi
QMAKE_LFLAGS+= /INCREMENTAL:NO /Debug

INCLUDEPATH += ../../Infrastructure/include ../../Temple/include
# LIBS += -lgdi32 -luser32
LIBS += ../../Release/Infrastructure.lib -lgdi32 -luser32 ../../dependencies/lib/turbojpeg-static.lib ../../Release/Temple.lib

SOURCES += plugin.cpp ../../TemplePlus/ui/ui_qtquick_nam.cpp ../../TemplePlus/ui/ui_qtquick_images.cpp

HEADERS += \
    plugin.h \
    ../../TemplePlus/ui/ui_qtquick_nam.h \
    ../../TemplePlus/ui/ui_qtquick_images.h \
    stdafx.h

DISTFILES += \
    qmldir

