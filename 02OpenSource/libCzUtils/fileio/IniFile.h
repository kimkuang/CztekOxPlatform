#ifndef __INIFILE_H__
#define __INIFILE_H__

#include "library_global.h"
#include <vector>
#include <string>

class LIBRARY_API IniFile
{
public:
    struct T_LineConf
    {
        std::string Key;
        std::string Value;
        std::string Comment;
        std::string Content; // Not standard key value pair, it is the part except comment
        T_LineConf()
        {
            Key = Value = Comment = Content = "";
        }
        T_LineConf(const std::string &key, const std::string &value)
        {
            Key = key;
            Value = value;
            Comment = "";
            Content = key + "=" + value;
        }
        T_LineConf(const std::string &key, const std::string &value, const std::string &comment, const std::string &content)
        {
            Key = key;
            Value = value;
            Comment = comment;
            Content = content;
        }
    };
    struct T_Section
    {
        std::string SectionName;
        std::vector<T_LineConf> SectionLines;
        T_Section()
        {
            SectionName = "";
            SectionLines.clear();
        }
        T_Section(const std::string &sectionName)
        {
            SectionName = sectionName;
            SectionLines.clear();
        }
    };
public:
    IniFile();
    virtual ~IniFile();

    std::string GetFileName() const { return m_fileName; }
    void SetCommentSign(const std::vector<std::string> &vecCommentSign);
    void LoadFile(const std::string &fileName);
    void SaveFile();
    void SaveAsFile(const std::string &fileName);
    std::string ReadSection(const std::string &sectionName);
    void ReadSection(const std::string &sectionName, std::vector<T_LineConf> &sectionLines);
    int ReadInteger(const std::string &sectionName, const std::string &key, int defaultValue = 0);
    float ReadFloat(const std::string &sectionName, const std::string &key, float defaultValue = 0.0f);
    bool ReadBool(const std::string &sectionName, const std::string &key, bool defaultValue = false);
    std::string ReadString(const std::string &sectionName, const std::string &key, const std::string &defaultVal = "");
    bool WriteSection(const std::string &sectionName, const std::vector<T_LineConf> &sectionLines);
    bool WriteInteger(const std::string &sectionName, const std::string &key, int value);
    bool WriteFloat(const std::string &sectionName, const std::string &key, float value);
    bool WriteBool(const std::string &sectionName, const std::string &key, bool value);
    bool WriteString(const std::string &sectionName, const std::string &key, const std::string &value);
    bool ClearSection(const std::string &sectionName);

protected:
    std::vector<T_Section> m_iniContent;
    std::vector<std::string> m_vecCommentSign;
    std::string m_fileName;
    bool m_modified;

    std::string& ltrim(std::string &s);
    std::string& rtrim(std::string &s);
    std::string& trim(std::string &s);
    std::vector<std::string> split(const std::string &str, const std::string &spliter, bool skipEmptyParts = true);
};

#endif /* __INIFILE_H__ */
