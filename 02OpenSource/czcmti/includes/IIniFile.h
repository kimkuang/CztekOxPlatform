#ifndef __IINIFILE_H__
#define __IINIFILE_H__
#include <QString>
#include <QMap>

class IIniFile
{
public:
    virtual QString ReadString(const QString &sectionName, const QString &key, const QString defaultValue = "") = 0;
    virtual bool WriteString(const QString &sectionName, const QString &key, const QString &value) = 0;
    virtual int ReadInteger(const QString &sectionName, const QString &key, int defaultValue = 0) = 0;
    virtual int WriteInteger(const QString &sectionName, const QString &key, int value) = 0;
    virtual float ReadFloat(const QString &sectionName, const QString &key, float defaultValue = 0.0f) = 0;
    virtual bool WriteFloat(const QString &sectionName, const QString &key, float value) = 0;
    virtual bool ReadBool(const QString &sectionName, const QString &key, bool defaultValue = false) = 0;
    virtual bool WriteBool(const QString &sectionName, const QString &key, bool value) = 0;
    virtual void ReadSection(const QString &section, QMap<QString, QString> &configurations) = 0;
    virtual int WriteSection(const QString &section, const QMap<QString, QString> &configurations) = 0;
};

#endif /* __IINIFILE_H__ */
