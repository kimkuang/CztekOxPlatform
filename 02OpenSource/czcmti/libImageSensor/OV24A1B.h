
#ifndef __OV24A1B_H__
#define __OV24A1B_H__

#include "ImageSensor.h"

#ifdef IMG_DRV_OV24A1B
#define STANDBY_MODE_CTRL                                               0x0100
    #define STANDBY_MODE_CTRL_PD                                            (0x00 << 0)
    #define STANDBY_MODE_CTRL_PWR_UP                                        (0x01 << 0)

#define SOFT_RESET_MODE_CTRL                                            0x0103
    #define SOFT_RESET_EN                                                   (0x01 << 0)

#define TEMPERATURE_CTL_REG_ADDR                                        0x4D12
#define TEMPERATURE_VAL_REG_ADDR                                        0x4D13

/*#define AWB_GAIN_R_REG_H_ADDR                                           0x5106
#define AWB_GAIN_R_REG_L_ADDR                                           0x5107

#define AWB_GAIN_GB_REG_H_ADDR                                          0x5102
#define AWB_GAIN_GB_REG_L_ADDR                                          0x5103

#define AWB_GAIN_GR_REG_H_ADDR                                          0x5104
#define AWB_GAIN_GR_REG_L_ADDR                                          0x5105

#define AWB_GAIN_B_REG_H_ADDR                                           0x5100
#define AWB_GAIN_B_REG_L_ADDR                                           0x5101*/

#define EXPOSURE_REG_H_ADDR                                             0x3501
#define EXPOSURE_REG_L_ADDR                                             0x3502

#define SENSOR_GAIN_REG_H_ADDR                                          0x3508
#define SENSOR_GAIN_REG_L_ADDR                                          0x3509

/** OTP Memmory Ctrl Reg **/
#define OTP_PROG_CTRL                                                   0x3D80
    #define OTP_PROG_EN_WO                                                  (0x01 << 0) //Writing 0 to 1 starts Programing
    #define OTP_WR_BUSY_RO                                                  (0x01 << 7) //Programing ongoing

#define OTP_LOAD_CTRL                                                   0x3D81
    #define OTP_LOAD_EN_WO                                                  (0x01 << 0) //Writing to this bit will starts loading data
    #define OTP_AUTO_LOAD_MODE_EN_WO                                        (0x01 << 1) //
    #define OTP_BIST_RES_CLR_WO                                             (0x01 << 2) //
    #define OTP_BIST_DONE_RO                                                (0x01 << 4) //
    #define OTP_BIST_ERR_RO                                                 (0x01 << 5) //
    #define OTP_IN_LOADING_RO                                               (0x01 << 7) //Loading ongoing

#define OTP_MODE_CTRL                                                   0x3D84
    #define OTP_MODE_MANUAL_RW                                              (0x01 << 6) //Manual Mode
    #define OTP_MODE_PROG_DIS_RW                                            (0x01 << 7) //Program Disable

#define OTP_LOAD_EN_CTRL                                                0x3D85
    #define OTP_SW_LOAD_SETTING_EN_RW                                       (0x01 << 0) //OTP SW Load setting enable
    #define OTP_HW_LOAD_SETTING_EN_RW                                       (0x01 << 1) //OTP HW Load setting enable
    #define OTP_PWR_UP_LOAD_DATA_EN_RW                                      (0x01 << 3) //OTP Power up load data enable
    #define OTP_BIST_EN_RW                                                  (0x01 << 4) //OTP Bist Enable
    #define OTP_BIST_SEL_RW                                                 (0x01 << 5) //OTP Bist, 0:Compare with SRAM, 1:Compare with zero or one
    #define OTP_BIST_COMP_RW                                                (0x01 << 6) //OTP Bist comp value

#define OTP_START_ADDR_HIGH                                             0x3D88  //Start high Addr for manual mode
#define OTP_START_ADDR_LOW                                              0x3D89  //Start low Addr for manual mode
#define OTP_END_ADDR_HIGH                                               0x3D8A  //End high Addr for manual mode
#define OTP_END_ADDR_LOW                                                0x3D8B  //End low Addr for manual mode
#endif /* IMG_DRV_OV24A1B */

class OV24A1B :	public ImageSensor
{
public:
    OV24A1B(const T_SensorSetting &sensorSetting);

    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    virtual int GetTemperature(int &temperature);
    virtual int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    virtual int SetSensorExposure(uint value);
    virtual int SetSensorGain(uint value, bool bMultiple=false);
    virtual int GetSensorExposure(uint &value);
    virtual int GetSensorGain(uint &value);

private:
    virtual int ReadReg(ushort startAddr, ushort endAddr, ushort *buf, ushort page = 0);
    virtual int WriteReg(ushort startAddr, ushort endAddr, ushort *buf, ushort page = 0);
};

#endif  // __OV24A1B_H__
