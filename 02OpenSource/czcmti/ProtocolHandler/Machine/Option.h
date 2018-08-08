#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <QString>
#include <QList>

class Option
{
public:
    int ListenPort;
    int ForwardPort;
    QString ListenIp;  // for any: 127.0.0.1
    QString ForwardIp;
    QString HandshakeRequest;
    QString HandshakeResponse;

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
