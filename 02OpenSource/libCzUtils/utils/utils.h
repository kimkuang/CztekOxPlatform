#ifndef UTILS_H
#define UTILS_H
#include "library_global.h"
#include <stdint.h>
#include <QSemaphore>
#include <QString>
#include <QMap>

class LIBRARY_API Utils
{
public:
    static void InitSemaphore(QSemaphore *semaphore, int n);
    static bool TryAcquireWithPooling(QSemaphore *semaphore, int n, int timeout);
    static void GetSystemErrorInfo(QString &errInfo, const QString &hintMsg, int errNo);
    static void AdjustValueByRange(int &val, int min, int max);
    static void AdjustRangeByValue(int val, int &min, int &max);
    static void ClearOldFiles(const QString &path, const QStringList &nameFilters, int keepDays);
    static bool GetMountPointFromDevNode(QString &mountPoint, const QString &devNode);
    static bool ExecShellFile(const QString &shellName, const QStringList &arguments, const QString &workDir = "");
    static bool ExecPipeShellCommand(const QString &command1, const QString &command2, QString &stdOutput,
                                     QString &stdError, int startTimeout = 1000, int finishTimeout = 5000);
    static bool ExecShellCommand(const QString &command, QString &stdOutput, QString &stdError,
                                 int startTimeout = 1000, int finishTimeout = 5000);
    static void RemoveLineComment(QString &newContent, const QString &oldContent, const QString &commentMark = "//");
    static int PageCount(int totalNum, int pageNum);
    static void GetAppVersion(const QString &appName, QString &appVersion);
    static unsigned char ReverseByte(unsigned char b);
    static void Md5SumFile(QString &md5, const QString &fileName);
    static void FormatOutputDataBuffer(const uchar *buffer, int len);
    static int ParseBitList(QList<uint> &bitList, uint val, int bitCnt = 32);
    static void ParseStringToKeyValueMap(QMap<QString, QString> &keyValMap, const QString &str, const QString &sep = ";");
    static bool GetDiskSizeInfo(const QString &diskName, uint64_t &total, uint64_t &available);
    static bool GetDiskSizeInfoByFsType(const QString &fsType, uint64_t &total, uint64_t &available);
};

#endif // UTILS_H
