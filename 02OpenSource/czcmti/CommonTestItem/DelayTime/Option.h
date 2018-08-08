#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <vector>
#include <string>
#include <QString>

class Option
{

public:
    Option();
    ~Option();
    void RestoreDefaults();
    int m_DelayTime;
    bool m_Enable;

private:
    Option(const Option &) = delete;
    Option& operator = (const Option&) = delete;
};

#endif // OPTION_H
