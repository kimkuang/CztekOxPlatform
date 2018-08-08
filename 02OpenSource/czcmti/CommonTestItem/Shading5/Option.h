#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <vector>
#include <string>

class Option
{
public:
    struct T_RoiConf {
        bool Enable;
        uint XPercent;
        uint YPercent;
        T_RoiConf() {
            XPercent = YPercent = 0;
        }
    };
public:
    Option();
    ~Option();
    void RestoreDefaults();

    T_RoiConf m_RoiConfs[5]; // 0 == center
    bool m_Continue;
    int m_BLC;
    int m_YDiff;
    int m_YMax;
    int m_YMin;
    int m_BGMin;
    int m_BGMax;
    int m_RGMax;
    int m_RGMin;
    int m_RoiHeightPercent;
    int m_RoiWidthPercent;

private:
    Option(const Option &) = delete;
    Option& operator = (const Option&) = delete;
};

#endif // OPTION_H
