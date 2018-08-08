#ifndef ENUMNAMEMAP_H
#define ENUMNAMEMAP_H
#include <vector>
#include <map>
#include <string>

class EnumNameMap
{
public:
    EnumNameMap();

    const std::map<int, std::string> &GetPinNameMap() const;
    int FindPinIdx(const std::string &name);
    std::string FindPinName(int idx);

private:
    std::map<int, std::string> m_pinNameMap;
    void initPinNameMap();
};

#endif // ENUMNAMEMAP_H
