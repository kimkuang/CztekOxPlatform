#-------------------------------------------------
#
# Project created by QtCreator 2017-12-12T08:54:26
#
#-------------------------------------------------

QT       += core gui

TARGET = VcmDriver
TEMPLATE = lib

DEFINES += VCMDRIVER_EXPORTS

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

SOURCES += \
    VcmDriver.cpp \
    VcmDriver_AD5823.cpp \
    VcmDriver_A1457.cpp \
    VcmDriver_AK7371.cpp \
    VcmDriver_AK7374.cpp \
    VcmDriver_BU64241.cpp \
    VcmDriver_DW9718.cpp \
    VcmDriver_DW9761.cpp \
    VcmDriver_LV8498.cpp \
    VcmDriver_DW9800.cpp \
    VcmDriver_DW9763.cpp \
    VcmDriver_DW9767.cpp \
    VcmDriver_LC898217.cpp \
    VcmDriver_LC898219.cpp \
    VcmDriver_DW9714.cpp \
    VcmDriver_ZC524.cpp

HEADERS += \
    VcmDriver.h \
    VcmDriver_AD5823.h \
    VcmDriver_A1457.h \
    VcmDriver_AK7371.h \
    VcmDriver_AK7374.h \
    VcmDriver_BU64241.h \
    VcmDriver_DW9718.h \
    VcmDriver_DW9761.h \
    VcmDriver_LV8498.h \
    VcmDriver_DW9800.h \
    VcmDriver_DW9763.h \
    VcmDriver_DW9767.h \
    VcmDriver_LC898217.h \
    VcmDriver_LC898219.h \
    VcmDriver_DW9714.h \
    VcmDriver_ZC524.h

unix {
    # install
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../includes
INCLUDEPATH += $$PWD/../../includes
