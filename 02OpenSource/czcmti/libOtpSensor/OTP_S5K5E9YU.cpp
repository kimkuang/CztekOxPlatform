
#define OTP_DRV_S5K5E9YU

#include "OTP_S5K5E9YU.h"

OTP_S5K5E9YU::OTP_S5K5E9YU(uint i2cAddr)
            : OtpSensor(i2cAddr)
{

}

OTP_S5K5E9YU::~OTP_S5K5E9YU(void)
{

}

int OTP_S5K5E9YU::OtpRead(uint startAddr, uint endAddr, uchar *Buff, ushort page)
{
    QString strLog = "";
    if (NULL == Buff)
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s Error---Buff:0x%p is NULL\r\n", __FUNCTION__, Buff);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (startAddr > endAddr)
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s Error---startAddr:%d is Larger than endAddr:%d\r\n", __FUNCTION__, startAddr, endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /** Step 1:Stream On **/
    USHORT stram_flag = 0;
    if (ERR_NoError != I2cRead(REG_OP_MODE_ADDR, stram_flag, RB_ADDR16_DATA8))
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cRead Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (SENSOR_SOFTWARE_STANDBY == (stram_flag & SENSOR_OP_MODE_MASK))
    {
        if (ERR_NoError != I2cWrite(REG_OP_MODE_ADDR, SENSOR_STREAMING_ON, RB_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_OP_MODE_ADDR Error\r\n", __FUNCTION__);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        QThread::msleep(50);
    }

    /** Step 2:Set Read Mode and Page No **/
    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, PENDING_ERROR, RB_ADDR16_DATA8)) //Make initial State
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR[PENDING_ERROR] Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(REG_PAGE_SEL_ADDR, page, RB_ADDR16_DATA8))    //Set Page Num
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_PAGE_SEL_ADDR Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, R_CMD, RB_ADDR16_DATA8)) //otp enable and read start
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_PAGE_SEL_ADDR Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /** Step 3:Wait for Sensor transfer 1page data from OTP to buffer, about 47us, **/
    unsigned int iCnt = 0;
    while (1)
    {
        ushort uTmp = 0;

        QThread::msleep(5);
        if (ERR_NoError != I2cRead(REG_ERR_FLGA_ADDR, uTmp, RB_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cRead REG_ERR_FLGA_ADDR Error---iCnt:%d\r\n", __FUNCTION__, iCnt);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }

        if (uTmp & R_RDY)
        {
            break;
        }

        iCnt++;
        if (iCnt >= 3)
        {
            strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cRead REG_ERR_FLGA_ADDR Error---iCnt:%d so Timeout\r\n", __FUNCTION__, iCnt);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }

    int nLen = endAddr - startAddr + 1;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(m_i2cAddr, 400, startAddr, 2, Buff, nLen))
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, PENDING_ERROR, RB_ADDR16_DATA8))  //Make initial State
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, INIT_CMD, RB_ADDR16_DATA8))  //Disable NVM Controller
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OTP_S5K5E9YU::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    QString strLog = "";

    if (NULL == buf || endAddr < startAddr)
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s Error---buf:0x%p is NULL or startAddr:%d > endAddr:%d\r\n", __FUNCTION__, buf, startAddr, endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /** Step 1:Stream On **/
    USHORT stram_flag = 0;
    if (ERR_NoError != I2cRead(REG_OP_MODE_ADDR, stram_flag, RB_ADDR16_DATA8))
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cRead Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (SENSOR_SOFTWARE_STANDBY == (stram_flag & SENSOR_OP_MODE_MASK))
    {
        if (ERR_NoError != I2cWrite(REG_OP_MODE_ADDR, SENSOR_STREAMING_ON, RB_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_OP_MODE_ADDR Error\r\n", __FUNCTION__);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        QThread::msleep(50);
    }

    /** Step 2:OTP Write Enable, and Set Write Mode and Page No **/
    if (ERR_NoError != I2cWrite(REG_OTP_WR_DISABLE_ADDR, OTP_WR_DISABLE_OFF, RB_ADDR16_DATA8)) //Make initial State
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_OTP_WR_DISABLE_ADDR[OTP_WR_DISABLE_OFF] Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(REG_OTP_WR_TIME_MODE_ADDR, WR_TIME_UNLIMITED_MODE, RB_ADDR16_DATA8))    //Set Page Num
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_OTP_WR_TIME_MODE_ADDR[WR_TIME_UNLIMITED_MODE] Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, PENDING_ERROR, RB_ADDR16_DATA8)) //Make initial State
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR[PENDING_ERROR] Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(REG_PAGE_SEL_ADDR, page, RB_ADDR16_DATA8))    //Set Page Num
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_PAGE_SEL_ADDR Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, W_CMD, RB_ADDR16_DATA8)) //otp enable and read start
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_PAGE_SEL_ADDR Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /** Step 3:Write Data to OTP Zone **/
    int nLen = endAddr - startAddr + 1;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(m_i2cAddr, 400, startAddr, 2, buf, nLen))
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /** Step 4:Wait for Sensor transfer 64bytes data from buffer to OTP, about (T > 36ms, 36ms = 550us(1byte write time)*64), **/
    unsigned int iCnt = 0;
    while (1)
    {
        ushort uTmp = 0;

        QThread::msleep(5);
        if (ERR_NoError != I2cRead(REG_ERR_FLGA_ADDR, uTmp, RB_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cRead REG_ERR_FLGA_ADDR Error---iCnt:%d\r\n", __FUNCTION__, iCnt);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }

        if (uTmp & W_RDY)
        {
            break;
        }

        iCnt++;
        if (iCnt >= 8)  //wait (T > 36ms, 36ms = 550us(1byte write time)*64) : the time to transfer 64bytes data from buffer to OTP - or check if 0x0A01[1] is high. 0x0A01[1] is the notice bit of write ending.
        {
            strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cRead REG_ERR_FLGA_ADDR Error---iCnt:%d so Timeout\r\n", __FUNCTION__, iCnt);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }

    /** Step 4:End of Write Process **/
    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, PENDING_ERROR, RB_ADDR16_DATA8))  //Make initial State
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(REG_CTL_RW_ADDR, INIT_CMD, RB_ADDR16_DATA8))  //Disable NVM Controller
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /** Step 5:Write Disable **/
    if (ERR_NoError != I2cWrite(REG_OTP_WR_DISABLE_ADDR, OTP_WR_DISABLE_ON, RB_ADDR16_DATA8)) //Make initial State
    {
        strLog = QString::asprintf("OTP_S5K5E9YU::%s---I2cWrite REG_OTP_WR_DISABLE_ADDR[OTP_WR_DISABLE_OFF] Error---page:%d\r\n", __FUNCTION__, page);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}
