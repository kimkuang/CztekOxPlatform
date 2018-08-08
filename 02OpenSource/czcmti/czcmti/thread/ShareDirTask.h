#ifndef SHAREDIRTASK_H
#define SHAREDIRTASK_H
#include <QRunnable>
#include <QFile>
#include "CzUtilsDefs.h"

class ShareDirTask : public QRunnable
{
public:
    struct T_ShellParam {
        enum E_CommandType {
            Command_Mount,
            Command_Umount,
        };
        E_CommandType CommandType;
        QString UserName;
        QString Password;
        QString SourceUrl;
        QString MountedUrl;
    };

    ShareDirTask(const T_ShellParam &shellParam) :
        m_shellParam(shellParam)
    {}

protected:
    void run()
    {
        //qDebug("tid: %#x FUNC: %s LINE: %d", (unsigned int)QThread::currentThreadId(), __FUNCTION__, __LINE__);
        QString shellFileName = "mount_disk.sh";
        QFile file(QString(":/shell/res/%1").arg(shellFileName));
        if (QFile::exists(QString("/tmp/%1").arg(shellFileName)))
            QFile::remove(QString("/tmp/%1").arg(shellFileName));
        if (!file.copy(QString("/tmp/%1").arg(shellFileName))) {
            qCritical()<<QString("Copying %1 failed.").arg(shellFileName);
            return;
        }
        QString command;
        if (T_ShellParam::Command_Mount == m_shellParam.CommandType) {
            command = QString("bash /tmp/%1 mount \"%2\" \"%3\" %4 %5").arg(shellFileName)
                    .arg(m_shellParam.UserName).arg(m_shellParam.Password)
                    .arg(m_shellParam.SourceUrl).arg(m_shellParam.MountedUrl);
        }
        else {
            command = QString("bash /tmp/%1 umount %2").arg(shellFileName).arg(m_shellParam.MountedUrl);
        }
        QString stdOutput;
        QString stdError;
        qDebug().noquote()<<command;
        bool flag = Utils::ExecShellCommand(command, stdOutput, stdError, 1000, -1);
        if (!flag) {
            qCritical()<<"Share dir failed!"<<stdError;
        }
//        qDebug()<<stdOutput.toLatin1().constData();
//        qDebug()<<stdError.toLatin1().constData();
        QFile::remove(QString("/tmp/%1").arg(shellFileName));
    }

private:
    T_ShellParam m_shellParam;
};

#endif // SHAREDIRTASK_H
