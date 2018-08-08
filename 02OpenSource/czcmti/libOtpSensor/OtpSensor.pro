#-------------------------------------------------
#
# Project created by QtCreator 2017-12-12T08:54:26
#
#-------------------------------------------------

QT       += core

TARGET = OtpSensor
TEMPLATE = lib

DEFINES += OTPSENSOR_EXPORTS

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
    OtpSensor.cpp \
    EFLASH_DW9763.cpp \
    EFLASH_DW9767.cpp \
    OTP_S5K4H7YX.cpp \
    OTP_S5K5E8YX.cpp \
    OTP_S5K5E9YU.cpp \
    EEPROM_GT24C32.cpp \
    EEPROM_GT24C16.cpp \
    EEPROM_GT24C64.cpp \
    EEPROM_GT24P64A.cpp \
    EEPROM_GT24P64B.cpp \
    EEPROM_GT24C128.cpp \
    EEPROM_Normal.cpp \
    EEPROM_GT24P128B.cpp \
    EEPROM_GT24P128C.cpp \
    EEPROM_BL24SA64.cpp

HEADERS += \
    OtpSensor.h \
    EFLASH_DW9763.h \
    EFLASH_DW9767.h \
    OTP_S5K4H7YX.h \
    OTP_S5K5E8YX.h \
    OTP_S5K5E9YU.h \
    EEPROM_GT24C32.h \
    EEPROM_GT24C16.h \
    EEPROM_GT24C64.h \
    EEPROM_GT24P64A.h \
    EEPROM_GT24P64B.h \
    EEPROM_GT24C128.h \
    EEPROM_Normal.h \
    EEPROM_GT24P128B.h \
    EEPROM_GT24P128C.h \
    EEPROM_BL24SA64.h

unix {
    # install
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../includes
INCLUDEPATH += $$PWD/../../includes
