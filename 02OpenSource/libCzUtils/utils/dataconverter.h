#ifndef DATACONVERTER_H
#define DATACONVERTER_H
#include <QtGlobal>
#include "library_global.h"

class LIBRARY_API DataConverter
{
public:
    static void String2HexString(uchar *hexStr, const uchar *str, int bytes);
    static void HexString2String(uchar *str, const uchar *hexStr, int bytes);
};

#endif // DATACONVERTER_H
