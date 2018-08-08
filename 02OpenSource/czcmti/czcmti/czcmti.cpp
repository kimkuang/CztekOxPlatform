#include "ui/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMetaType>
#include <QDir>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QCryptographicHash> 
#include <QMessageBox>
#include <signal.h>
#ifdef __linux__
#include <execinfo.h>     /* for backtrace() */
#endif
#include "common/Global.h"
#include "common/logger.h"
#include <QtSolutions/QtSingleApplication>
#include "utils/utils.h"
#include "CustomEvent.h"
#include "ui/AboutDialog.h"
#include "ui/LoginDialog.h"
#include "conf/MachineSettings.h"

static MainWindow *gMainWindowInstance = NULL;
#define BACKTRACE_SIZE   16

bool IsUnlock(void)
{
    QString czcmtiInI = GlobalVars::APP_PATH + "/" + "czcmti.ini";
    QFileInfo fileCzcmtiIni(czcmtiInI);
    if (false == fileCzcmtiIni.exists())
    {
        QDir dir(GlobalVars::PROJECT_PATH);
        if (true == dir.exists())
        {
            QFileInfoList ltFileInfo = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
            if (0 == ltFileInfo.size())
            {
                return true;
            }
        }
        else
        {
            return true;
        }

        return false;
    }

    SystemSettings *sysSetting = SystemSettings::GetInstance();
    if ((int)UiDefs::Operator_Operator == sysSetting->Operator)
    {
        for (int i = 0; i < sysSetting->CameraNumber; ++i)
        {
            QString strModuleInI = GlobalVars::PROJECT_PATH + sysSetting->ProjectName + "/" + QString("module%1.ini").arg(i);
            QFile file(strModuleInI);
            if (false == file.open(QIODevice::ReadOnly))
            {
                return false;
            }

            QByteArray btMd5 = QCryptographicHash::hash(file.readAll() + "OfTester", QCryptographicHash::Md5).toHex();
            if (0 != sysSetting->vtMd5MoudleInI[i].compare(QString::fromStdString(btMd5.toStdString())))
            {
                file.close();
                return false;
            }

            file.close();
        }
    }

    //czcmti.ini system section is not modify by outside
    QString systemSection = "";
    systemSection.sprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", sysSetting->WorkMode, sysSetting->Operator,
        sysSetting ->ShowLoginDialog, sysSetting ->CameraNumber, sysSetting ->AntiShakeDelay,
        sysSetting ->AlarmMsgThreshold, sysSetting ->StopTestingThreshold, sysSetting ->SensorSelected,
        sysSetting ->ShowStatusBar, sysSetting ->ShowToolBar, sysSetting ->ShowTextPanel,
        sysSetting ->ShowTestItemPanel, sysSetting ->ShowTestResultPanel);
    for (int i = 0; i < sysSetting->CameraNumber; ++i)
    {
        systemSection += sysSetting->vtMd5MoudleInI[i];
    }

    QByteArray btSysSection = QCryptographicHash::hash(systemSection.toLatin1(), QCryptographicHash::Md5).toHex();
    if (0 != sysSetting->md5SystemSection.compare(QString::fromStdString(btSysSection.toStdString())))
    {
        return false;
    }

    return true;
}

#ifdef __linux__
static void dump()
{
    int j, nptrs;
    void *buffers[BACKTRACE_SIZE];
    char **strings;

    nptrs = backtrace(buffers, BACKTRACE_SIZE);
    qDebug("+++++++++++++++++++ DUMP +++++++++++++++++++");
    qDebug("backtrace() returned %d addresses", nptrs);

    strings = backtrace_symbols(buffers, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }
    for (j = 0; j < nptrs; j++)
        qDebug("  [%02d] %s\n", j, strings[j]);
    qDebug("--------------------------------------------");

    free(strings);
}
#endif

static void signalHandler(int signo)
{
    qDebug("Handlering signal [%d]...", signo);
    switch (signo) {
    case SIGSEGV: // 11
        qCritical()<<"SIGSEGV catched!!!";
        dump();
        exit(-1);
        break;
#ifdef __linux__
    case SIGIO:
    case SIGKILL:
    case SIGQUIT:
#endif
    case SIGTERM: // 15
    case SIGINT:  // 2
        if (gMainWindowInstance) {
            qApp->postEvent(gMainWindowInstance, new ExceptionEvent(ExceptionEvent::Exception_SystemSignal, signo));
        }
        break;
    }
}

int main(int argc, char *argv[])
{
    if ((argc == 2) && ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0))) {
        QString version;
        AboutDialog::GetVersion(version);
        printf("%s %s\n", argv[0], version.toStdString().c_str());
        return 0;
    }

    qDebug()<<QCoreApplication::libraryPaths();
    QtSingleApplication app("czcmti", argc, argv);
    if (app.isRunning()) {
        if (app.sendMessage("Wake up!", 1000)) {
            qDebug()<<"Has woke up!";
            return 0;
        }
    }

    qDebug()<<"Application initialization...";
    GlobalVars::InitVars();
    /*if (false == IsUnlock())
    {
        QMessageBox::critical(nullptr, "Critical", "Program is Locked, Before Config is Modified By Outside!", "exit");
        return ERR_Failed;
    }*/

    GlobalVars::InitApplicationDirectory();
    Utils::ClearOldFiles(GlobalVars::LOG_PATH, QStringList()<<"log*.log", 5);
    Utils::ClearOldFiles(GlobalVars::PIC_PATH, QStringList()<<"pic*", 5);
    Utils::ClearOldFiles(GlobalVars::MOV_PATH, QStringList()<<"mov*", 5);

#ifdef QT_NO_DEBUG // turn off debug logging in RELEASE mode
    QLoggingCategory::setFilterRules("*.debug=false");
#endif
    qInstallMessageHandler(Logger::StandardFileOutput);

    signal(SIGSEGV, signalHandler);
#ifdef __linux__
    signal(SIGKILL, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGIO, signalHandler); // overcurrent
#endif
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    app.processEvents();
    QFile qssFile(GlobalVars::APP_PATH + "style.qss");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
        qDebug("set style sheet...");
        QString qss = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(qss);
        qssFile.close();
    }

    app.processEvents();
    QTranslator qtTranslator;
    QString qmFile = QString("qt_%1.qm").arg(QLocale::system().name());
    if (!qtTranslator.load(qmFile, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        qWarning()<<QString("Loading %1 failed.").arg(qmFile);
    app.installTranslator(&qtTranslator);

    QTranslator appTranslator;
    qmFile = QString("%1_%2.qm").arg(qApp->applicationName()).arg(QLocale::system().name());
    if (!appTranslator.load(qmFile, app.applicationDirPath()))
        qWarning()<<QString("Loading %1 failed.").arg(qmFile);
    app.installTranslator(&appTranslator);

    app.processEvents();
    qDebug("Creating database connections ...");
    if (!SqlDatabaseUtil::CreateDbConnections("system", "DriverName=QSQLITE;DatabaseName=system.db3", false)) {
        qCritical("Creating db connection failed.");
        exit(-1);
    }
    SystemSettings *sysSetting = SystemSettings::GetInstance();

    app.processEvents();  
    bool flag = true;
    if (sysSetting->ShowLoginDialog) {
        LoginDialog *loginDlg = LoginDialog::GetInstance();
        flag = (loginDlg->exec() == QDialog::Accepted);
    }

    int ec = 0;
    if (flag) {
        MainWindow win;
        gMainWindowInstance = &win;
        app.setActivationWindow(&win); // active window
        QObject::connect(&app, &QtSingleApplication::messageReceived, &win, &MainWindow::app_messageReceived);
        win.show();
        ec = app.exec();

        ChannelController::FreeAllInstances(); // free before TestItemService
    }
    LoginDialog::FreeInstance();
    SystemSettings::DestroyAllInstances(); // all ISettings
    return ec;
}
