#ifndef __QTINIFILE_H__
#define __QTINIFILE_H__
#include "IIniFile.h"
#include "fileio/IniFile.h"
#include <QVector>

class QtIniFile : public IIniFile
{
public:
    QString GetFileName() const { return QString::fromStdString(m_iniFile.GetFileName()); }
    void SetCommentSign(const QVector<QString> &vecCommentSign);
    void LoadFile(const QString &fileName);
    void SaveFile();
    void SaveAsFile(const QString &fileName);
public:
    /// IIniFile
    /// After all Write* functions, please call SaveFile()/SaveAsFile() function to save.
    QString ReadString(const QString &sectionName, const QString &key, const QString defaultValue = "");
    bool WriteString(const QString &sectionName, const QString &key, const QString &value);
    int ReadInteger(const QString &sectionName, const QString &key, int defaultValue);
    int WriteInteger(const QString &sectionName, const QString &key, int value);
    float ReadFloat(const QString &sectionName, const QString &key, float defaultValue = 0.0f);
    bool WriteFloat(const QString &sectionName, const QString &key, float value);
    bool ReadBool(const QString &sectionName, const QString &key, bool defaultValue = false);
    bool WriteBool(const QString &sectionName, const QString &key, bool value);
    void ReadSection(const QString &section, QMap<QString, QString> &configurations);
    int WriteSection(const QString &section, const QMap<QString, QString> &configurations);
    bool ReadSection(const std::string &section, std::vector<IniFile::T_LineConf> &sectionLines);
    bool WriteSection(const std::string &section, const std::vector<IniFile::T_LineConf> &sectionLines);
    // obsoleted
    void ReadSection(const std::string &section, std::map<std::string, std::string> &configurations);
    int WriteSection(const std::string &section, const std::map<std::string, std::string> &configurations);
private:
    IniFile m_iniFile;
};

#endif /* __QTINIFILE_H__ */
