#include "QtIniFile.h"
#include "czcmtidefs.h"

void QtIniFile::SetCommentSign(const QVector<QString> &vecCommentSign)
{
    std::vector<std::string> stdVector;
    foreach (QString comment, vecCommentSign) {
        stdVector.push_back(comment.toStdString());
    }
    m_iniFile.SetCommentSign(stdVector);
}

void QtIniFile::LoadFile(const QString &fileName)
{
    m_iniFile.LoadFile(fileName.toStdString());
}

void QtIniFile::SaveFile()
{
    m_iniFile.SaveFile();
}

void QtIniFile::SaveAsFile(const QString &fileName)
{
    m_iniFile.SaveAsFile(fileName.toStdString());
}

QString QtIniFile::ReadString(const QString &sectionName, const QString &key, const QString defaultValue)
{
    std::string value = m_iniFile.ReadString(sectionName.toStdString(), key.toStdString(), defaultValue.toStdString());
    return QString::fromStdString(value);
}

bool QtIniFile::WriteString(const QString &sectionName, const QString &key, const QString &value)
{
    return m_iniFile.WriteString(sectionName.toStdString(), key.toStdString(), value.toStdString());
}

int QtIniFile::ReadInteger(const QString &sectionName, const QString &key, int defaultValue)
{
    return m_iniFile.ReadInteger(sectionName.toStdString(), key.toStdString(), defaultValue);
}

int QtIniFile::WriteInteger(const QString &sectionName, const QString &key, int value)
{
    return m_iniFile.WriteInteger(sectionName.toStdString(), key.toStdString(), value);
}

float QtIniFile::ReadFloat(const QString &sectionName, const QString &key, float defaultValue)
{
    return m_iniFile.ReadFloat(sectionName.toStdString(), key.toStdString(), defaultValue);
}

bool QtIniFile::WriteFloat(const QString &sectionName, const QString &key, float value)
{
    return m_iniFile.WriteFloat(sectionName.toStdString(), key.toStdString(), value);
}

bool QtIniFile::ReadBool(const QString &sectionName, const QString &key, bool defaultValue)
{
    return m_iniFile.ReadBool(sectionName.toStdString(), key.toStdString(), defaultValue);
}

bool QtIniFile::WriteBool(const QString &sectionName, const QString &key, bool value)
{
    return m_iniFile.WriteBool(sectionName.toStdString(), key.toStdString(), value);
}

void QtIniFile::ReadSection(const QString &section, QMap<QString, QString> &configurations)
{
    configurations.clear();
    std::vector<IniFile::T_LineConf> sectionLines;
    m_iniFile.ReadSection(section.toStdString(), sectionLines);
    for (auto it = sectionLines.begin(); it != sectionLines.end(); ++it) {
        if (it->Key.empty())
            continue;
        configurations.insert(QString::fromStdString(it->Key), QString::fromStdString(it->Value));
    }
}

int QtIniFile::WriteSection(const QString &section, const QMap<QString, QString> &configurations)
{
    std::vector<IniFile::T_LineConf> sectionLines;
    QMapIterator<QString, QString> it(configurations);
    while (it.hasNext()) {
        it.next();
        sectionLines.push_back(IniFile::T_LineConf(it.key().toStdString(), it.value().toStdString()));
//            qDebug()<<section<<it.key()<<it.value();
    }
    if (m_iniFile.WriteSection(section.toStdString(), sectionLines)) {
        m_iniFile.SaveFile();
        return ERR_NoError;
    }
    return ERR_Failed;
}

bool QtIniFile::ReadSection(const std::string &section, std::vector<IniFile::T_LineConf> &sectionLines)
{
    m_iniFile.ReadSection(section, sectionLines);
    return true;
}

bool QtIniFile::WriteSection(const std::string &section, const std::vector<IniFile::T_LineConf> &sectionLines)
{
    if (m_iniFile.WriteSection(section, sectionLines)) {
        m_iniFile.SaveFile();
        return true;
    }
    return false;
}

void QtIniFile::ReadSection(const std::string &section, std::map<std::string, std::string> &configurations)
{
    configurations.clear();
    std::vector<IniFile::T_LineConf> sectionLines;
    m_iniFile.ReadSection(section, sectionLines);
    for (auto it = sectionLines.begin(); it != sectionLines.end(); ++it) {
        if (it->Key.empty())
            continue;
        configurations.insert(std::make_pair(it->Key, it->Value));
    }
}

int QtIniFile::WriteSection(const std::string &section, const std::map<std::string, std::string> &configurations)
{
    std::vector<IniFile::T_LineConf> sectionLines;
    for (auto it = configurations.begin(); it != configurations.end(); ++it) {
        sectionLines.push_back(IniFile::T_LineConf(it->first, it->second));
//            qDebug()<<QString::fromStdString(section)<<QString::fromStdString(it->first)<<QString::fromStdString(it->second);
    }
    if (m_iniFile.WriteSection(section, sectionLines)) {
        m_iniFile.SaveFile();
        return ERR_NoError;
    }
    return ERR_Failed;
}
