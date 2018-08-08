#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"

class Option
{
public:

public:
    static Option* GetInstance();
    static void FreeInstance();
    void RestoreDefaults();

private:
    static Option* m_instance;
    Option();
    ~Option();
    Option(const Option &) = delete;
    Option& operator = (const Option&) = delete;
};

#endif // OPTION_H
