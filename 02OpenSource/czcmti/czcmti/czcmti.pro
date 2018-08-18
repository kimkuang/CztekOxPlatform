#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T11:24:19
#
#-------------------------------------------------

QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT       += network

TEMPLATE = app
CONFIG += c++11

TARGET = czcmti

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

SOURCES += \
    common/FileIo.cpp \
    common/Global.cpp \
    common/ProductBarcode.cpp \
    common/TlvParser.cpp \
    ui/BoolFrame.cpp \
    ui/AboutDialog.cpp \
    ui/PasswordDialog.cpp \
    ui/SensorDetailDialog.cpp \
    ui/EditSensorDialog.cpp \
    ui/CameraChildWindow.cpp \
    ui/MainWindow.cpp \
    ui/LoginDialog.cpp \
    ui/HardwareTestDialog.cpp \
    ui/TestItemManagementDialog.cpp \
    ui/DatabaseSettingWidget.cpp \
    ui/SelectSensorDialog.cpp \
    ui/TestItemSettingDialog.cpp \
    ui/SensorPropertyBrowserWidget.cpp \
    ui/GeneralSettingsWidget.cpp \
    ui/MiddlewareSettingsWidget.cpp \
    ui/IpcSettingsWidget.cpp \
    ui/GlobalSettingsDialog.cpp \
    ui/TestItemButton.cpp \
    ui/DecoderDialog.cpp \
    ui/MachineSettingsDialog.cpp \
    ui/ShareDirWidget.cpp \
    ui/UpgradeDialog.cpp \
    conf/SystemSettings.cpp \
    conf/ModuleSettings.cpp \
    conf/MachineSettings.cpp \
    controller/ChannelController.cpp \
    service/DecoderService.cpp \
    service/FileTransferService.cpp \
    thread/CaptureThread.cpp \
    service/TestItemService.cpp \
    service/MachineService.cpp \
    service/IpcService.cpp \
    czcmti.cpp \
    ui/I2cDebugDialog.cpp \
    service/PluginLibPool.cpp \
    ui/RawOptionDlg.cpp \
    common/ImageProc.cpp \
    ui/UNumberLabel.cpp

HEADERS  += \
    common/FileIo.h \
    common/Global.h \
    common/ProductBarcode.h \
    common/TlvParser.h \
    common/UdevTypes.h \
    conf/ISettings.h \
    conf/version.h \
    thread/patchexecutor.h \
    thread/ShareDirTask.h \
    ui/BoolFrame.h \
    ui/AboutDialog.h \
    ui/PasswordDialog.h \
    ui/SensorDetailDialog.h \
    ui/EditSensorDialog.h \
    ui/CameraChildWindow.h \
    ui/MainWindow.h \
    ui/LoginDialog.h \
    ui/HardwareTestDialog.h \
    ui/TestItemManagementDialog.h \
    ui/DatabaseSettingWidget.h \
    ui/SelectSensorDialog.h \
    ui/TestItemSettingDialog.h \
    ui/SensorPropertyBrowserWidget.h \
    ui/GeneralSettingsWidget.h \
    ui/MiddlewareSettingsWidget.h \
    ui/IpcSettingsWidget.h \
    ui/GlobalSettingsDialog.h \
    ui/ISettingsWidget.h \
    ui/DecoderDialog.h \
    ui/TestItemButton.h \
    ui/MachineSettingsDialog.h \
    ui/ShareDirWidget.h \
    ui/UpgradeDialog.h \
    conf/SystemSettings.h \
    conf/ModuleSettings.h \
    conf/MachineSettings.h \
    controller/ChannelController.h \
    service/DecoderService.h \
    service/FileTransferService.h \
    thread/CaptureThread.h \
    service/TestItemService.h \
    service/MachineService.h \
    ui/I2cDebugDialog.h \
    service/PluginLibPool.h \
    service/IpcService.h \
    ui/RawOptionDlg.h \
    common/ImageProc.h \
    ui/UNumberLabel.h

FORMS    += \
    ui/PasswordDialog.ui \
    ui/SensorDetailDialog.ui \
    ui/EditSensorDialog.ui \
    ui/AboutDialog.ui \
    ui/CameraChildWindow.ui \
    ui/MainWindow.ui \
    ui/LoginDialog.ui \
    ui/HardwareTestDialog.ui \
    ui/TestItemManagementDialog.ui \
    ui/DatabaseSettingWidget.ui \
    ui/SelectSensorDialog.ui \
    ui/TestItemSettingDialog.ui \
    ui/GeneralSettingsWidget.ui \
    ui/MiddlewareSettingsWidget.ui \
    ui/IpcSettingsWidget.ui \
    ui/GlobalSettingsDialog.ui \
    ui/DecoderDialog.ui \
    ui/MachineSettingsDialog.ui \
    ui/I2cDebugDialog.ui \
    ui/ShareDirWidget.ui \
    ui/UpgradeDialog.ui \
    ui/RawOptionDlg.ui

INCLUDEPATH += $$PWD/../../includes
INCLUDEPATH += $$PWD/../includes
INCLUDEPATH += $$PWD/../

win32 {
    RC_FILE += czcmti.rc # Only validates in windows.

    #QMAKE_CXXFLAGS += /wd"4819"

    LIBS += -L$$PWD/../../lib/$${CPU_ARCH}/$${CONFIG_DIR}
    OPENCV_DIR = "D:/opencv/opencv2410/build/"
    INCLUDEPATH += $${OPENCV_DIR}/include
}
else {
    SOURCES += \
        common/UdevManager.cpp \
        thread/UdevMonitor.cpp
    HEADERS  += \
        common/UdevManager.h \
        thread/UdevMonitor.h

    LIBS += -L$$PWD/../lib/$${CONFIG_DIR} -Wl,-rpath=.
}

LIBS += -lCzUtils
INCLUDEPATH += $$PWD/../../libCzUtils
LIBS += -lUiUtils
INCLUDEPATH += $$PWD/../libUiUtils
LIBS += -lImageSensor
INCLUDEPATH += $$PWD/../libImageSensor
LIBS += -lOtpSensor
INCLUDEPATH += $$PWD/../libOtpSensor
LIBS += -lVcmDriver
INCLUDEPATH += $$PWD/../libVcmDriver

# database access module
LIBS += -lCmtiDal
INCLUDEPATH += $$PWD/../../libDAL

# hal module
DEFINES += CAMERA=2 # BASLER=0 Rat=1 Ox=2
LIBS += -lHal_Ox
INCLUDEPATH += $$PWD/../../libHal

# QtSolutions
LIBS += -lQt5PropertyBrowser -lQt5SingleApplication

# network transfer module
LIBS += -lnetwork_trans -Wl,-rpath=.
LIBS += -Wl,-rpath=./plugins/testitem/

#DEFINES += RDDBG
contains( DEFINES, RDDBG ) {
    message( "Configuring rd debug version..." );
}

#DEFINES += SYNC_QSEMAPHORE # use QSemaphore instead of QWaitCondition

RESOURCES += \
    czcmti.qrc \
    shaders.qrc

DBFILES = system.db3

DEFINES  += SQLITE_HAS_CODEC QT_MESSAGELOGCONTEX

TRANSLATIONS += czcmti_zh_CN.ts

unix {
    LIBS     += -lsqlite3 -ludev
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
    # install
    others.path = ~/rootfs/home/root/czcamera_client
    # 想要不当作可执行文件安装，需要保证文件属性没有可执行权限(如：644)
    others.files = system.db3 \
        $${TARGET}_*.qm

    target.path = ~/rootfs/home/root/czcamera_client
    INSTALLS += target \
        others
}

DISTFILES += \
    style.qss
