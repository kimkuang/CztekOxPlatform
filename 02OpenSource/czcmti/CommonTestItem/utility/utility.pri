INCLUDEPATH += $$PWD/../../includes/
#INCLUDEPATH += $$PWD/../../../includes/
#INCLUDEPATH += $$PWD/Common/

LIBS += -lCzUtils
INCLUDEPATH += $$PWD/../../../libCzUtils

SOURCES += $$PWD/StdStringUtils.cpp \
    $$PWD/QtIniFile.cpp

HEADERS += $$PWD/StdStringUtils.h \
    $$PWD/QtIniFile.h
