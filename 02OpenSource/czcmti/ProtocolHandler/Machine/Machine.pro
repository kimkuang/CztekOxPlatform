#-------------------------------------------------
#
# Project created by QtCreator 2018-01-03T15:38:30
#
#-------------------------------------------------

QT       += network widgets

QT       -= gui

TARGET = Machine
TEMPLATE = lib

CONFIG += c++11 plugin
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
    ControlPdu.cpp \
    MachineHandler.cpp \
    ConfWidget.cpp \
    Option.cpp \
    TcpServer.cpp \
    TcpClient.cpp

HEADERS += \
    ControlPdu.h \
    MachineHandler.h \
    ConfWidget.h \
    Option.h \
    TcpServer.h \
    TcpClient.h

FORMS += \
    ConfWidget.ui

INCLUDEPATH += $$PWD/../../includes/
INCLUDEPATH += $$PWD/../../../includes/

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -lCzUtils
INCLUDEPATH += $$PWD/../../../libCzUtils

