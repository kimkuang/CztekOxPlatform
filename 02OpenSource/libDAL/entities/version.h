#ifndef __VERSION_H__
#define __VERSION_H__

#include <QString>

class Version
{
public:
    int Id;
    QString ObjectName;
    QString BinVersion;
    QString BinDate;
    QString DbVersion;
    QString DbDate;
};

#endif // __VERSION_H__

