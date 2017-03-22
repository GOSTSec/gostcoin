#-------------------------------------------------
#
# Project created by QtCreator 2013-01-06T03:07:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = i2psam
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -Wall

SOURCES += i2psam.cpp

HEADERS += i2psam.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
