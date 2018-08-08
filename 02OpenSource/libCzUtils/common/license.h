#ifndef LICENSE_H
#define LICENSE_H

#include <QDateTime>
#include <QDataStream>
#include <QDebug>
#include "library_global.h"

class LIBRARY_API License
{
public:
    struct LicenseFile {
        QString MachineId;
        QDateTime LastTime;
        QDateTime ExpiredTime;
        QString HashDigest;
    };
    static int EXPIRED_DAYS;
    static QString LICENSE_FILE_NAME;

    License(const QString &machineId);
    ~License();

    bool InitLicense();
    int CheckLicense(QString &errorMessage);
    bool UpdateLicense();
private:

    const QString SALT_NOISE_STRING;

    LicenseFile m_licFile;
    int m_leftDays;
    QString calcLicenseHash(QString &strHash, const LicenseFile &licFile);
};

// 重载自定义对象的输入
inline QDataStream &operator<<(QDataStream &out, const License::LicenseFile &lic)
{
//    qDebug()<<lic.MachineId<<lic.LastTime<<lic.ExpiredTime<<lic.HashDigest;
    out<<lic.MachineId<<lic.LastTime<<lic.ExpiredTime<<lic.HashDigest;
    return out;
}

// 重载自定义对象的输出
inline QDataStream &operator>>(QDataStream &in, License::LicenseFile &lic)
{
    in>>lic.MachineId>>lic.LastTime>>lic.ExpiredTime>>lic.HashDigest;
    qDebug()<<lic.MachineId<<lic.LastTime<<lic.ExpiredTime;//<<lic.HashDigest;
    return in;
}

#endif // LICENSE_H
