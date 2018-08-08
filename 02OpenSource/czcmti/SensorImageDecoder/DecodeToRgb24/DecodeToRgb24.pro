#-------------------------------------------------
#
# Project created by QtCreator 2017-12-20T10:08:41
#
#-------------------------------------------------

QT       += core gui widgets

CONFIG += c++11 plugin

TARGET = DecodeToRgb24
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
    DecodeToRgb24.cpp \
    SensorImageDecoder.cpp \
    BayerRawDecoder.cpp \
    Rgb16Decoder.cpp \
    YuvDecoder.cpp \
    ConfWidget.cpp \
    Option.cpp

HEADERS += \
    DecodeToRgb24.h \
    SensorImageDecoder.h \
    BayerRawDecoder.h \
    Rgb16Decoder.h \
    YuvDecoder.h \
    ConfWidget.h \
    Option.h

FORMS += \
    ConfWidget.ui

win32 {
    OPENCV_DIR = $$(OPENCV2410) #"D:/Program Files/opencv/build"
    OPENCV_VER = 2410
    INCLUDEPATH += $${OPENCV_DIR}/include
    LIBS += -L$${OPENCV_DIR}/$${CPU_ARCH}/vc12/lib -lopencv_core$${OPENCV_VER} -lopencv_imgproc$${OPENCV_VER} -lopencv_highgui$${OPENCV_VER}
}
else {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv

#    INCLUDEPATH += /usr/local/cuda/include
#    LIBS += -lnppi

    # install
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../../includes/
INCLUDEPATH += $$PWD/../../../includes/
