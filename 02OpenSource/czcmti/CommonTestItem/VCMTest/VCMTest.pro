#-------------------------------------------------
#
# Project created by QtCreator 2017-12-25T20:38:12
#
#-------------------------------------------------

QT       += gui widgets

CONFIG += c++11 plugin

TARGET = VCMTest
TEMPLATE = lib

CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    DIST_DIR = Debug
} else {
    DIST_DIR = Release
    QMAKE_POST_LINK = $(STRIP) $(DESTDIR)$(TARGET)
}

win32 {
    contains(QT_ARCH, i386) {
        message("32-bit")
        CPU_ARCH = x86
    } else {
        message("64-bit")
        CPU_ARCH = x64
    }
    DESTDIR = $$PWD/../lib/$${CPU_ARCH}/$${DIST_DIR}
}
else {
    DESTDIR = $$PWD/../lib/$${DIST_DIR}
}

DEFINES += CZPLUGIN_EXPORTS

SOURCES += \
    ConfWidget.cpp \
    Option.cpp \
    VCMTest.cpp

HEADERS += \
    ConfWidget.h \
    Option.h \
    VCMTest.h

FORMS += \
    ConfWidget.ui

INCLUDEPATH += $$PWD/../../includes/
INCLUDEPATH += $$PWD/../../../includes/

unix {
    target.path = /usr/lib
    INSTALLS += target
}
