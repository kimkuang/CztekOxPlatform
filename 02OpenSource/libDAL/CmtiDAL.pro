#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T08:48:56
#
#-------------------------------------------------

QT       += sql

CONFIG += c++11

TEMPLATE = lib
CONFIG += plugin

TARGET = CmtiDal

CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    CONFIG_DIR = Debug
} else {
    CONFIG_DIR = Release
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
    DESTDIR = $$PWD/../czcmti/lib/$${CPU_ARCH}/$${CONFIG_DIR}
}
else {
    DESTDIR = $$PWD/../czcmti/lib/$${CONFIG_DIR}
}

#win32:VERSION = 1.0.1.0 # major.minor.patch.build
#else:VERSION = 1.0.1    # major.minor.patch

DEFINES += LIBRARY_EXPORTS

SOURCES += \
    bll/sensorbll.cpp \
    conf/sensorini.cpp \
    dal/sqldatabaseutil.cpp \
    dal/sensordal.cpp \
    dal/versiondal.cpp \
    cmtidal.cpp

HEADERS += \
    bll/sensorbll.h \
    bo/sensorbo.h \
    conf/sensorini.h \
    dal/sqldatabaseutil.h \
    dal/sensordal.h \
    dal/versiondal.h \
    entities/sensor.h \
    entities/version.h \
    cmtidal.h

win32 {
    RC_FILE += version.rc
    QMAKE_CXXFLAGS += /wd"4819"
}

INCLUDEPATH += $$PWD/../includes
