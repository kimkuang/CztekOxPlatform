#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <vector>
#include <string>
#include <QRect>

class Option
{
public:
    bool m_bSaveAfCode;
    float m_fEFL;
    float m_fInifiDistance;
    float m_fMacroDistance;
    float m_fMoveDistance;
    ushort m_uInifiAddrH;
    ushort m_uInifiAddrL;
    ushort m_uMacroAddrH;
    ushort m_uMacroAddrL;
    ushort m_uOtherCode;
    int m_MoveToPosMethod;
    uint m_uVcmDelay;

    enum E_MoveToPosMethod {
        Move_OtherPos = 0,
        Move_InfinityPos,
        Move_MacroPos,
        Move_MiddlePos,
        Move_OtherCode,
    };

public:
    Option();
    ~Option();
    void RestoreDefaults();

private:
    Option(const Option &) = delete;
    Option& operator = (const Option&) = delete;
};

#endif // OPTION_H
