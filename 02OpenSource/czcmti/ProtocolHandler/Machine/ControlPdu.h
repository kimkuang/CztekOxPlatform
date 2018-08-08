#ifndef __CONTROL_PDU_H__
#define __CONTROL_PDU_H__
#include <string>
#include "ICzPlugin.h"

class ControlPdu
{
public:
    enum E_ErrorCode {
        ERR_NoError = 0,
        ERR_PartialPacket = -1,
        ERR_InvalidHeader = -2,
        ERR_InvalidPacket = -3,
        ERR_InvalidCommand = -4,
    };
    enum E_Receiver {
        Receiver_Handler = 0,
        Receiver_Tester = 1,
        Receiver_TestItem,
    };

    enum {
        MIN_TO_TESTER_PDU_BYTES = 7,
        MIN_TO_HANDLER_PDU_BYTES = 9,
        MIN_TO_TESTITEM_PDU_BYTES = 5,
        CONTROL_PORT = 3956,
        GVCP_CMD_TIMEOUT = 500,
    };

public:
    ControlPdu();
    ControlPdu(const ControlPdu &other);
    ControlPdu& operator = (const ControlPdu &other);
    inline bool operator== (const ControlPdu &other)
    {
        return (Command == other.Command) && (SubStation == other.SubStation) &&
               (CameraIndex == other.CameraIndex);
    }
    virtual ~ControlPdu();

    int Pack(std::string &packet);
    int Unpack(std::string &packet);

public:
    std::string Packet;
    E_Receiver Receiver;
    E_MachineCommand Command;
    int SubStation;
    int CameraIndex;
    std::string Parameter;
    int ErrorCode; // Empty if Request
};

#endif // __CONTROL_PDU_H__
