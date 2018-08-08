#ifndef VCMDRIVER_H
#define VCMDRIVER_H
#include "IVcmDriver.h"

//I2C Mode:
//      0:Normal 8Addr,8Data;
//		1:Samsung 8 Addr,8Data;
//		2:Micron 8 Addr,16Data
//		3:Stmicro 16Addr,8Data;
//		4:Micron2 16 Addr,16Data
typedef enum
{
    I2C_MODE_ADDR8_DATA8_NORMAL = RB_ADDR8_DATA8,
    I2C_MODE_ADDR8_DATA8_SAMSUNG,
    I2C_MODE_ADDR8_DATA16,
    I2C_MODE_ADDR16_DATA8,
    I2C_MODE_ADDR16_DATA16,

    I2C_MODE_MAX
}_I2C_MODE_;

#define MSB(x)                                              (unsigned char)( ((x) >> 8) & 0xFF )
#define LSB(x)                                              (unsigned char)( (x) & 0xFF )
#ifdef MAKEWORD
#undef MAKEWORD
#endif
#define MAKEWORD(HBy, LBy)                                  (unsigned short)( ((unsigned short)((HBy) & 0xFF) << 8) | ((LBy) & 0xFF) )
#define GetBit(x, pos)                                      ( (x) & (0x01 << (pos)) )
#define ClrBit(x, pos)                                      ( (x) & (~(0x01 << (pos))) )

class VcmDriver : public IVcmDriver
{
public:
    VcmDriver(uint i2cAddr);
    virtual ~VcmDriver();
    int VcmInitialize();
    int BindChannelContext(T_ChannelContext *context);
    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
    int VcmFinalize();

protected:
    uint m_i2cAddr;
    T_ChannelContext *m_channelContext;
    IChannelController *m_channelController;
};

typedef VcmDriver* (*PGET_VcmDrv)(uint &);
typedef struct
{
    const char *pchName;
    PGET_VcmDrv pGetVcmDrv;
}_VCM_DRV_TABLE_;

#endif // VCMDRIVER_H
