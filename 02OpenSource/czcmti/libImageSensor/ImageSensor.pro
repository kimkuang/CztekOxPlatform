#-------------------------------------------------
#
# Project created by QtCreator 2017-12-12T08:54:26
#
#-------------------------------------------------

QT       += core gui

TARGET = ImageSensor
TEMPLATE = lib

DEFINES += IMAGESENSOR_EXPORTS

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
    ImageSensor.cpp \
    GC2375.cpp \
    GC5025.cpp \
    OV12A10.cpp \
    OV02A10.cpp \
    OV08A10.cpp \
    HI1333.cpp \
    IMX258_0AQH5.cpp \
    IMX350.cpp \
    IMX351.cpp \
    IMX363.cpp \
    IMX376.cpp \
    IMX380.cpp \
    IMX398.cpp \
    IMX476.cpp \
    IMX486.cpp \
    IMX498.cpp \
    IMX519.cpp \
    IMX550.cpp \
    IMX576.cpp \
    IMX600.cpp \
    S5K2L7SA.cpp \
    S5K3T1SP03.cpp \
    S5K4H7YX.cpp \
    S5K5E8YX.cpp \
    S5K5E9YU.cpp \
    OV13A40.cpp \
    OV16B10.cpp \
    OV16885.cpp \
    OV24A1B.cpp \
    IMX355.cpp

HEADERS += \
    ImageSensor.h \
    GC2375.h \
    GC5025.h \
    OV12A10.h \
    OV02A10.h \
    OV08A10.h \
    HI1333.h \
    IMX258_0AQH5.h \
    IMX350.h \
    IMX351.h \
    IMX363.h \
    IMX376.h \
    IMX380.h \
    IMX398.h \
    IMX476.h \
    IMX486.h \
    IMX498.h \
    IMX519.h \
    IMX550.h \
    IMX576.h \
    IMX600.h \
    S5K2L7SA.h \
    S5K3T1SP03.h \
    S5K4H7YX.h \
    S5K5E8YX.h \
    S5K5E9YU.h \
    OV13A40.h \
    OV16B10.h \
    OV16885.h \
    OV24A1B.h \
    IMX355.h

unix {
    # install
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../includes
INCLUDEPATH += $$PWD/../../includes
