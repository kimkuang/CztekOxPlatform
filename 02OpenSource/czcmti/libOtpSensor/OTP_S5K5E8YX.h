#ifndef __OTP_S5K5E8YX_H__
#define __OTP_S5K5E8YX_H__
#include "OtpSensor.h"

#ifdef OTP_DRV_S5K5E8YX
#define EEPROMSTARTADDR                                                 0x0000
#define EEPROMENDADDR                                                   0x1FFF

#define REG_OP_MODE_ADDR                                                0x0100  //Operating Mode Registers---0:Software Standby, 1:Streaming(Active Video)--I2C Communiction with sensor is possible, Core is power on.
    #define SENSOR_OP_MODE_MASK                                             0x01
    #define SENSOR_SOFTWARE_STANDBY                                         (0x00 << 0)
    #define SENSOR_STREAMING_ON                                             (0x01 << 0)

#define REG_CTL_RW_ADDR                                                 0x0A00
    #define INIT_CMD                                                        (0x00)
    #define R_CMD                                                           (0x01 << 0)
    #define W_CMD                                                           (0x03 << 0)
    #define PENDING_ERROR                                                   (0x01 << 2)

#define REG_ERR_FLGA_ADDR                                               0x0A01
    #define R_RDY                                                           (0x01 << 0)
    #define W_RDY                                                           (0x01 << 1)
    #define DATA_CORRUPTED                                                  (0x01 << 2)
    #define IMPROPY_USAGE                                                   (0x01 << 3)

#define REG_PAGE_SEL_ADDR                                               0x0A02

#define REG_DATA_START_ADDR                                             0x0A04
#define REG_DATA_END_ADDR                                               0x0A43
    #define PAGE_LEN                                                        (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

#define REG_OTP_WR_DISABLE_ADDR                                         0x3B40
    #define OTP_WR_DISABLE_OFF                                              (0x00 << 0)
    #define OTP_WR_DISABLE_ON                                               (0x01 << 0)

#define REG_OTP_WR_TIME_MODE_ADDR                                       0x3B45
    #define WR_TIME_UNLIMITED_MODE                                           (0x01 << 0)

#endif

class OTP_S5K5E8YX : public OtpSensor
{
public:
    OTP_S5K5E8YX(uint i2cAddr);
    ~OTP_S5K5E8YX(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);
};

#endif
