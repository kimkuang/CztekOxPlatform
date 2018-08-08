#-------------------------------------------------
#
# Project created by QtCreator 2017-12-25T20:38:12
#
#-------------------------------------------------

QT       += gui widgets

CONFIG += c++11 plugin

TARGET = AF_MoveToPos
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

DEFINES += TESTITEM_EXPORTS

SOURCES += \
    ConfWidget.cpp \
    Option.cpp \
    AFMoveToPos.cpp

HEADERS += \
    ConfWidget.h \
    Option.h \
    AFMoveToPos.h

FORMS += \
    ConfWidget.ui

INCLUDEPATH += $$PWD/../../includes/
INCLUDEPATH += $$PWD/../../../includes/

# algorithm module
#LIBS += -lCmtiAlgorithm # -Wl,-rpath=.
#INCLUDEPATH += $$PWD/../../libAlgorithm/

unix {
    target.path = /usr/lib
    INSTALLS += target
}