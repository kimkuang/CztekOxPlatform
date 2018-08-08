#ifndef PATCHEXECUTOR_H
#define PATCHEXECUTOR_H
#include <QRunnable>
#include <QDir>
#include <QDebug>
#include "common/Global.h"
#include "utils/utils.h"
#ifdef __linux__
#include <unistd.h>
#endif

/******************patch.ini******************
// upgrader
upgrader=/usr/bin/
// autodisplay
autodisplay=/home/root/autodisplay/
autodisplay_zh_CN.qm=/home/root/autodisplay/
autodisplay_en_US.qm=/home/root/autodisplay/
autodisplay.desktop=/usr/share/applications/
**********************************************/
class PatchExecutor : public QRunnable
{
protected:
    void run()
    {
        QDir dir(GlobalVars::PATCH_PATH);
//        if (GlobalVars::ThisProductBarcode.HardwareVersion == ProductBarcode::HV_Invalid) {
//            qDebug("Cannot support patch function dut to invalid seraial number. Deleting patch dir...");
//            dir.removeRecursively();
//#ifdef __linux__
//            sync();
//#endif
//            return;
//        }
        if(!dir.exists()) {
            qDebug()<<"No patches to execute...";
            return;
        }

        QString iniFileName = GlobalVars::PATCH_PATH + GlobalVars::PATCH_INI_FILE_NAME;
        QFile file(iniFileName);
        if (!file.exists()) {
            qDebug()<<"No patches to execute...";
            return;
        }

        QString content;
        if (file.open(QFile::Text | QFile::ReadOnly)) {
            qDebug()<<QString("Found patch config file: %1").arg(iniFileName);
            QTextStream textStream(&file);
            content = textStream.readAll();
            file.close();
            file.remove();
        }
        else {
            qCritical()<<QString("Cannot open file: %1!").arg(iniFileName);
            return;
        }
        Utils::RemoveLineComment(content, content, "//");

        QStringList tmpSl = content.split(QChar('\n'), QString::SkipEmptyParts);
        QString fileName, dirName;
        foreach (QString s, tmpSl) {
            int pos = s.indexOf(QChar('='));
            if (pos < 0)
                continue;
            fileName = s.mid(0, pos).trimmed();
            dirName = s.mid(pos + 1).trimmed();
            if (!dirName.endsWith(QChar('/')))
                dirName = dirName + "/";
            if (!QFileInfo::exists(dirName)) {
                qCritical()<<QString("Destination dir %1 does not exist.").arg(dirName);
                continue;
            }

            QString srcFileName = GlobalVars::PATCH_PATH + fileName;
            QString destFileName = dirName + fileName;
            if (!QFile::exists(srcFileName)) {
                qCritical()<<QString("Cannot find patch file: %1").arg(fileName);
                continue;
            }
            else {
                if (QFile::exists(destFileName))
                    QFile::remove(destFileName);
                if (QFile::copy(srcFileName, destFileName)) {
                    QFile::remove(srcFileName);
                    qDebug()<<QString("Patching file %1 to %2 succeed.").arg(fileName).arg(dirName);
                }
                else
                    qCritical()<<QString("Patching file %1 to %2 failed.").arg(fileName).arg(dirName);
            }
        }
        dir.removeRecursively();
#ifdef __linux__
        sync();
#endif

        qDebug("Patch executor finished.");
    }
};

#endif // PATCHEXECUTOR_H
