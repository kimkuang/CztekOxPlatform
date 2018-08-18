QT += core gui

CONFIG += c++11

TARGET = AlgorithmTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    CONFIG_DIR = Debug
} else {
    CONFIG_DIR = Release
}

win32 {
    contains(QT_ARCH, i386) {
        message("32-bit")
        CPU_ARCH = x86
    } else {
        message("64-bit")
        CPU_ARCH = x64
    }
    DESTDIR = $$PWD/../bin/$${CPU_ARCH}/$${CONFIG_DIR}
}
else {
    DESTDIR = $$PWD/../bin/$${CONFIG_DIR}
}

HEADERS += \
    common/FileIo.h \
    Test_SFR.h

SOURCES += \
    common/FileIo.cpp \
    main.cpp \
    Test_SFR.cpp

win32 {
    LIBS += -L$$PWD/../czcmti/lib/$${CPU_ARCH}/$${CONFIG_DIR}
}
else {
    LIBS += -L$$PWD/../czcmti/lib/$${CONFIG_DIR}
}

INCLUDEPATH += $$PWD/../includes

LIBS += -lCmtiAlgorithm
INCLUDEPATH += $$PWD/../libAlgorithm
