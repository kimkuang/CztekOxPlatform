#include "StdStringUtils.h"

std::string& StdStringUtils::ltrim(std::string &s)
{
    if (s.empty())
        return s;
    const char EMPTY_CHARS[] = { '\x20', '\x0d', '\x0a', '\x09' };
    int pos = 0;
    for (unsigned int i = 0; i < sizeof(EMPTY_CHARS) / sizeof(char); i++)
    {
        int t = s.find_first_not_of(EMPTY_CHARS[i]);
        if (t > pos)
            pos = t;
    }
    s.erase(0, pos);
    return s;
}

std::string& StdStringUtils::rtrim(std::string &s)
{
    if (s.empty())
        return s;
    const char EMPTY_CHARS[] = { '\x20', '\x0d', '\x0a', '\x09' };
    int pos = s.size() - 1;
    for (unsigned int i = 0; i < sizeof(EMPTY_CHARS) / sizeof(char); i++)
    {
        int t = s.find_last_not_of(EMPTY_CHARS[i]);
        if (t < pos)
            pos = t;
    }
    s.erase(pos + 1);
    return s;
}

std::string& StdStringUtils::trim(std::string &s)
{
    s = rtrim(ltrim(s));
    return s;
}

std::vector<std::string> StdStringUtils::split(const std::string &str, const std::string &spliter, bool skipEmptyParts)
{
    std::vector<std::string> slTemp;
    std::string strTemp = str;
    std::string::size_type pos;
    do
    {
        pos = strTemp.find(spliter);
        std::string strSection = strTemp.substr(0, pos);
        strSection = trim(strSection);
        bool flag = !strSection.empty() || (strSection.empty() && !skipEmptyParts);
        if (flag)
            slTemp.push_back(strSection);
        if (pos != std::string::npos)
            strTemp = strTemp.substr(pos + 1);
    } while (pos != std::string::npos);
    return slTemp;
}