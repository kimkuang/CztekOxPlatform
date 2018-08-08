#ifndef WORKINGCURRENTOPTION_H
#define WORKINGCURRENTOPTION_H

class Option
{
public:
    bool DvddEnabled;
    bool AvddEnabled;
    bool DovddEnabled;
    bool AfvccEnabled;
    bool VppEnabled;
    bool TotalEnabled;

    int DvddRange;
    int AvddRange;
    int DovddRange;
    int AfvccRange;
    int VppRange;
    int TotalRange;

    int DvddLowerLimit;
    int AvddLowerLimit;
    int DovddLowerLimit;
    int AfvccLowerLimit;
    int VppLowerLimit;
    int TotalLowerLimit;

    int DvddUpperLimit;
    int AvddUpperLimit;
    int DovddUpperLimit;
    int AfvccUpperLimit;
    int VppUpperLimit;
    int TotalUpperLimit;

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

#endif // WORKINGCURRENTOPTION_H
