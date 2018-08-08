#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <vector>
#include <string>

class Option
{
public:

    int m_LeftSL;   //不计算POD的区域
    int m_RightSL;  //不计算POD的区域
    int m_TopSL;    //不计算POD的区域
    int m_BottomSL; //不计算POD的区域
    int m_AreaWidth;    //不计算POD的区域
    int m_AreaHeight;   //不计算POD的区域
    int m_DeadUnit;
    int m_DeadSpec;
    int m_DeadType;     //1 for%, 0 for pixel value;
    int m_DefultExposure;
    int m_DefultGain;
    bool m_SaveImage;
    int m_ErrorCode;

public:
    Option();
    ~Option();
    void RestoreDefaults();

private:
    Option(const Option &) = delete;
    Option& operator = (const Option&) = delete;
};

#endif // OPTION_H
