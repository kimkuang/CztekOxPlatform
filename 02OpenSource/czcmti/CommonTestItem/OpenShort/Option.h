#ifndef OPTION_H
#define OPTION_H
#include <vector>

class Option
{
public:
    unsigned int SupplyVol_mV;
    unsigned int SupplyCur_uA;
    unsigned int OpenThreshold_mV;
    unsigned int ShortThreshold_mV;
    std::vector<int> TestPinVector;
    bool ContinueWhenFailed;

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
