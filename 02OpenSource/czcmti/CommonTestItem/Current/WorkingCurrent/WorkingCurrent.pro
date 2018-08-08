#-------------------------------------------------
#
# Project created by QtCreator 2017-12-26T18:06:09
#
#-------------------------------------------------

QT       += widgets

CONFIG += c++11 plugin

QT       -= gui

TARGET = WorkingCurrent
TEMPLATE = lib

SOURCES += WorkingCurrent.cpp \
        ConfWidget.cpp \
        Option.cpp

HEADERS += WorkingCurrent.h \
        ConfWidget.h \
        Option.h

FORMS += ConfWidget.ui

include(../common.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}
