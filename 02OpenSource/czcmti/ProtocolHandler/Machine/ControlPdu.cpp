#include "ControlPdu.h"
//#include <QDebug>

ControlPdu::ControlPdu()
{
    Parameter = "";
}

ControlPdu::ControlPdu(const ControlPdu &other)
{
    this->Packet = other.Packet;
    this->Receiver = other.Receiver;
    this->Command = other.Command;
    this->SubStation = other.SubStation;
    this->CameraIndex = other.CameraIndex;
    this->Parameter = other.Parameter;
    this->ErrorCode = other.ErrorCode;
}

ControlPdu &ControlPdu::operator =(const ControlPdu &other)
{
    this->Packet = other.Packet;
    this->Receiver = other.Receiver;
    this->Command = other.Command;
    this->SubStation = other.SubStation;
    this->CameraIndex = other.CameraIndex;
    this->Parameter = other.Parameter;
    this->ErrorCode = other.ErrorCode;
    return *this;
}

ControlPdu::~ControlPdu()
{
}

int ControlPdu::Pack(std::string &packet)
{
    packet = "";
    if (this->Receiver == Receiver_Tester)
        packet += "T";
    else
        packet += "H";
    char buffer[10];
    sprintf(buffer, "%02d", this->Command);
    packet += std::string(buffer);

    sprintf(buffer, "%02d", this->SubStation);
    packet += std::string(buffer);

    sprintf(buffer, "%02d", this->CameraIndex);
    packet += std::string(buffer);

    if (this->Receiver == Receiver_Tester) {
        if (this->Command == MachineCommand_BindCode) {
            packet += (":" + this->Parameter);
        }
    }
    else {
        sprintf(buffer, "%02d", this->ErrorCode);
        packet += std::string(buffer);
    }
    this->Packet = packet;
    return ERR_NoError;
}

int ControlPdu::Unpack(std::string &packet)
{
    int canRemoveLen = 0;
    int leftIdx = 0;
    int rightIdx = 0;
    if ((leftIdx = packet.find_first_of('T')) != -1) { // find first 'T'
        this->Receiver = Receiver_Tester;
        canRemoveLen = leftIdx + 1;
        this->Packet = packet.substr(leftIdx, packet.size() - leftIdx);
    }
    else if ((leftIdx = packet.find_first_of('H')) != -1) { // find first 'H'
        this->Receiver = Receiver_Handler;
        canRemoveLen = leftIdx + 1;
        this->Packet = packet.substr(leftIdx, packet.size() - leftIdx);
    }
    else if ((leftIdx = packet.find_first_of('S')) != -1) {
        this->Receiver = Receiver_TestItem;
        canRemoveLen = leftIdx + 1;
        this->Packet = packet.substr(leftIdx, packet.size() - leftIdx);
    }
    else
        return ERR_InvalidPacket;

    if ((Receiver_Tester == this->Receiver) || (Receiver_Handler == this->Receiver)) {
        std::string strTemp = this->Packet.substr(1, 2);
        canRemoveLen += 2;
        if (strTemp == "00")
            this->Command = MachineCommand_OpenCamera;
        else if (strTemp == "01")
            this->Command = MachineCommand_Testing;
        else if (strTemp == "02")
            this->Command = MachineCommand_CloseCamera;
        else if (strTemp == "03")
            this->Command = MachineCommand_RefreshUi;
        else if (strTemp == "04")
            this->Command = MachineCommand_BindCode;
        else {
            packet = packet.substr(canRemoveLen, packet.size() - canRemoveLen);
            return ERR_InvalidCommand;
        }

        strTemp = this->Packet.substr(3, 2);
        canRemoveLen += 2;
        this->SubStation = std::stoi(strTemp);

        strTemp = this->Packet.substr(5, 2);
        canRemoveLen += 2;
        this->CameraIndex = std::stoi(strTemp);

        this->Parameter = "";
        if (this->Receiver == Receiver_Tester) {
            if (this->Command == MachineCommand_BindCode) {
                strTemp = this->Packet.substr(7, this->Packet.size() - 7);
                canRemoveLen += (this->Packet.size() - 7);
                leftIdx = strTemp.find_first_of(':', 0);
                if (leftIdx != 0) {
                    packet = packet.substr(canRemoveLen, packet.size() - canRemoveLen);
                    return ERR_InvalidPacket;
                }

                QStringList ltBindCode = QString::fromStdString(strTemp).split(':', QString::SkipEmptyParts);
                if (ltBindCode.size() < 2)
                {
                    return ERR_InvalidPacket;
                }

                QString runCard = ltBindCode[0];
                QString bindCode = ltBindCode[1];
                if (2 != ltBindCode.size())
                {
                       bindCode = ltBindCode[1].left(ltBindCode[1].length() - 7);
                }

                this->Parameter = strTemp.substr(leftIdx + 1, runCard.length() + bindCode.length() + 1);
                canRemoveLen += runCard.length() + bindCode.length() + 2;
            }
        }
        else {
            strTemp = this->Packet.substr(7, 2);
            canRemoveLen += 2;
            this->ErrorCode = std::stoi(strTemp);
        }
    }
    else {
        canRemoveLen += 4;
    }
//    qDebug()<<canRemoveLen<<QString::fromStdString(packet);
    packet = packet.substr(canRemoveLen, packet.size() - canRemoveLen);
//    qDebug()<<canRemoveLen<<QString::fromStdString(packet);
    return ERR_NoError;
}
