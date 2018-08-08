#ifndef STRINGUTILS_H
#define STRINGUTILS_H
#include <string>
#include "library_global.h"

class LIBRARY_API StringUtils
{
public:
    static char *strupr(char *str);
    static std::string &StdstringFormat(std::string &str, const char *format, ...);
};

#endif // STRINGUTILS_H
