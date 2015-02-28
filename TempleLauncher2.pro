#-------------------------------------------------
#
# Project created by QtCreator 2015-02-28T14:38:15
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = TempleLauncher2
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += minhook-1.3/include

SOURCES += main.cpp \
    minhook-1.3/src/buffer.c \
    minhook-1.3/src/hook.c \
    minhook-1.3/src/trampoline.c \
    minhook-1.3/src/HDE/hde32.c \
    minhook-1.3/src/HDE/hde64.c \
    temple_functions.cpp \
    libraryholder.cpp

HEADERS += \
    temple_functions.h \
    libraryholder.h \
    system.h
