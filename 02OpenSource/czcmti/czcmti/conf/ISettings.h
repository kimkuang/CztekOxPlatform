#pragma once
#include <map>
#include <mutex>
#include <QDebug>
#include "common/Global.h"
#include "IIniFile.h"
#include "CzUtilsDefs.h"

template <typename T>
class ISettings : public IIniFile
{
public:
    static T *GetInstance(const std::string &fileName)
    {
        T *t = NULL;
        if (m_instanceHashTable.find(fileName) == m_instanceHashTable.end())
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            Q_UNUSED(locker);
            if (m_instanceHashTable.find(fileName) == m_instanceHashTable.end()) {
                t = new T(fileName);
                if (t) {
                    t->InitSettings();
                    m_instanceHashTable.insert(std::make_pair(fileName, t));
                    if (!t->ReadSettings()) {
                        qCritical("Loading paramters failed!!! [%s]", fileName.c_str());
                        t->InitSettings();
                    }
                }
                else {
                    qCritical("Creating instance failed.");
                    return t;
                }
            }
        }
        t = m_instanceHashTable[fileName];
        return t;
    }

    static void DestroyInstance(const std::string &fileName)
    {
        if (m_instanceHashTable.find(fileName) != m_instanceHashTable.end()) {
            T *instance = m_instanceHashTable[fileName];
            delete instance; instance = NULL;

            m_instanceHashTable.erase(fileName);
        }
    }

    static void DestroyAllInstances()
    {
        auto it = m_instanceHashTable.begin();
        for (; it != m_instanceHashTable.end(); ++it) {
            delete it->second;
        }
        m_instanceHashTable.clear();
    }

public:
    virtual bool ReadSettings() = 0;
    virtual bool WriteSettings() = 0;
    virtual void InitSettings() = 0;

    QString ReadString(const QString &sectionName, const QString &key, const QString defaultValue)
    {
        std::string value = m_iniFile->ReadString(sectionName.toStdString(), key.toStdString(), defaultValue.toStdString());
        return QString::fromStdString(value);
    }

    bool WriteString(const QString &sectionName, const QString &key, const QString &value)
    {
        return m_iniFile->WriteString(sectionName.toStdString(), key.toStdString(), value.toStdString());
    }

    int ReadInteger(const QString &sectionName, const QString &key, int defaultValue)
    {
        return m_iniFile->ReadInteger(sectionName.toStdString(), key.toStdString(), defaultValue);
    }

    int WriteInteger(const QString &sectionName, const QString &key, int value)
    {
        return m_iniFile->WriteInteger(sectionName.toStdString(), key.toStdString(), value);
    }

    float ReadFloat(const QString &sectionName, const QString &key, float defaultValue)
    {
        return m_iniFile->ReadFloat(sectionName.toStdString(), key.toStdString(), defaultValue);
    }

    bool WriteFloat(const QString &sectionName, const QString &key, float value)
    {
        return m_iniFile->WriteFloat(sectionName.toStdString(), key.toStdString(), value);
    }

    bool ReadBool(const QString &sectionName, const QString &key, bool defaultValue)
    {
        return m_iniFile->ReadBool(sectionName.toStdString(), key.toStdString(), defaultValue);
    }

    bool WriteBool(const QString &sectionName, const QString &key, bool value)
    {
        return m_iniFile->WriteBool(sectionName.toStdString(), key.toStdString(), value);
    }

    void ReadSection(const QString &section, QMap<QString, QString> &configurations)
    {
        configurations.clear();
        std::vector<IniFile::T_LineConf> sectionLines;
        m_iniFile->ReadSection(section.toStdString(), sectionLines);
        for (auto it = sectionLines.begin(); it != sectionLines.end(); ++it) {
            if (it->Key.empty())
                continue;
            configurations.insert(QString::fromStdString(it->Key), QString::fromStdString(it->Value));
        }
    }

    int WriteSection(const QString &section, const QMap<QString, QString> &configurations)
    {
        std::vector<IniFile::T_LineConf> sectionLines;
        QMapIterator<QString, QString> it(configurations);
        while (it.hasNext()) {
            it.next();
            sectionLines.push_back(IniFile::T_LineConf(it.key().toStdString(), it.value().toStdString()));
//            qDebug()<<section<<it.key()<<it.value();
        }
        if (m_iniFile->WriteSection(section.toStdString(), sectionLines)) {
            m_iniFile->SaveFile();
            return ERR_NoError;
        }
        return ERR_Failed;
    }

    void ReadSection(const std::string &section, std::map<std::string, std::string> &configurations)
    {
        configurations.clear();
        std::vector<IniFile::T_LineConf> sectionLines;
        m_iniFile->ReadSection(section, sectionLines);
        for (auto it = sectionLines.begin(); it != sectionLines.end(); ++it) {
            if (it->Key.empty())
                continue;
            configurations.insert(std::make_pair(it->Key, it->Value));
        }
    }

    int WriteSection(const std::string &section, const std::map<std::string, std::string> &configurations)
    {
        std::vector<IniFile::T_LineConf> sectionLines;
        for (auto it = configurations.begin(); it != configurations.end(); ++it) {
            sectionLines.push_back(IniFile::T_LineConf(it->first, it->second));
//            qDebug()<<QString::fromStdString(section)<<QString::fromStdString(it->first)<<QString::fromStdString(it->second);
        }
        if (m_iniFile->WriteSection(section, sectionLines)) {
            m_iniFile->SaveFile();
            return ERR_NoError;
        }
        return ERR_Failed;
    }

    void ReadSection(const std::string &section, std::vector<IniFile::T_LineConf> &sectionLines)
    {
        m_iniFile->ReadSection(section, sectionLines);
    }

    int WriteSection(const std::string &section, const std::vector<IniFile::T_LineConf> &sectionLines)
    {
        if (m_iniFile->WriteSection(section, sectionLines)) {
            m_iniFile->SaveFile();
            return ERR_NoError;
        }
        return ERR_Failed;
    }

protected:
    IniFile *m_iniFile;

    ISettings()
    {
        m_iniFile = new IniFile();
    }
    ISettings(const std::string &fileName)
    {
        std::string fullFileName;
        if (fileName.rfind('/') == std::string::npos)
        {
            fullFileName = GlobalVars::APP_PATH.toStdString() + fileName;
        }
        else
            fullFileName = fileName;
        qDebug("%s", fullFileName.c_str());
        m_iniFile = new IniFile();
        m_iniFile->LoadFile(fullFileName);
    }
    virtual ~ISettings()
    {
        if (m_iniFile != nullptr) {
            delete m_iniFile; m_iniFile = nullptr;
        }
    }

private:
    ISettings(const ISettings&) = delete;
    ISettings& operator = (const ISettings&) = delete;
    static std::mutex m_mutex;
    static std::map<std::string, T*> m_instanceHashTable;
};
template <typename T>
std::map<std::string, T*> ISettings<T>::m_instanceHashTable;
template <typename T>
std::mutex ISettings<T>::m_mutex;

