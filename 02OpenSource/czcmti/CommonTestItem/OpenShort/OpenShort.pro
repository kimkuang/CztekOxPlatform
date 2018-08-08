#-------------------------------------------------
#
# Project created by QtCreator 2017-10-13T13:41:38
#
#-------------------------------------------------

QT       += gui widgets

TARGET = OpenShort
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

DEFINES += CZPLUGIN_EXPORTS DISABLE_CONTINUE_WHEN_FAILED

SOURCES += OpenShort.cpp \
    ConfWidget.cpp \
    Option.cpp \
    EnumNameMap.cpp \
    GeneralRoTableModel.cpp \
    $$PWD/../utility/StdStringUtils.cpp

HEADERS += OpenShort.h \
    ConfWidget.h \
    Option.h \
    EnumNameMap.h \
    GeneralRoTableModel.h \
    $$PWD/../utility/StdStringUtils.h

FORMS += \
    ConfWidget.ui

INCLUDEPATH += $$PWD/../utility/
INCLUDEPATH += $$PWD/../../includes/
INCLUDEPATH += $$PWD/../../../includes/

unix {
    target.path = /usr/lib
    INSTALLS += target
}
