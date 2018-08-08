#ifndef __MACHINESETTINGS_H__
#define __MACHINESETTINGS_H__
#include "ISettings.h"

class MachineSettings : public ISettings<MachineSettings>
{
public:
    static MachineSettings *GetInstance();

    // General
    QString MachineName;

public:
    bool WriteSettings();
    bool ReadSettings();
    void InitSettings();

private:
    static QString MACHINE_FILE_NAME;
    friend class ISettings<MachineSettings>;
    MachineSettings();
    MachineSettings(const std::string &fileName);
    MachineSettings(const MachineSettings&) = delete;
    MachineSettings &operator=(const MachineSettings&) = delete;
    ~MachineSettings();
};

#endif // __MACHINESETTINGS_H__
