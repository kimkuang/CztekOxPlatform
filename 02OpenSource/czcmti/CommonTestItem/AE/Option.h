#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <vector>
#include <string>
#include <QRect>

class Option
{
public:
    bool m_b10BitCal;
    int  m_iSensorType;
    int  m_iRefMode;
    int  m_iAECalMode;
    int  m_iAEMinVal;
    int  m_iAEMaxVal;
    int  m_iRoiW;
    int  m_iRoiH;
    int  m_iBlc;
    int  m_iStartX;
    int  m_iStartY;
    int  m_iAETimes;
    int  m_iAEInfiVal;
    int  m_iAERememberVal;
    bool m_bRememberAE;
    int m_iAnalogGain;
    int m_iAEFrame;
    int m_iWaitTime;

    enum {
        NORMAL = 0,     //常用sensor
        TWOPD,          //2PD sensor
        FOURCELL,       //4Cell sensor
    };

    enum {
        AVGPIXEL = 0,   //取大于平均值pixel
        ONE28PIXEL,     //取大于128 pixel
        CHLPIXEL,       //取单通道平均值
    };

    enum {
        RTARGET = 0,
        GRTARGET,
        GBTARGET,
        BTARGET,
        YTARGET,
        GGTARGET,
        MAXTARGET,
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
