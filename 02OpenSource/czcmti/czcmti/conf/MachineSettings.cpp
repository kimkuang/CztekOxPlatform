#include "MachineSettings.h"

QString MachineSettings::MACHINE_FILE_NAME = "machine.ini";
MachineSettings *MachineSettings::GetInstance()
{
    return ISettings<MachineSettings>::GetInstance(MACHINE_FILE_NAME.toStdString());
}

MachineSettings::MachineSettings()
{
}

MachineSettings::MachineSettings(const std::string &fileName) :
    ISettings<MachineSettings>(fileName)
{}

MachineSettings::~MachineSettings()
{}

bool MachineSettings::WriteSettings()
{
    return true;
}

bool MachineSettings::ReadSettings()
{
    return true;
}

void MachineSettings::InitSettings()
{
}
