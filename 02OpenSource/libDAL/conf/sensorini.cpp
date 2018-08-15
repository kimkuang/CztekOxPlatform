#include "sensorini.h"
#include <QStringList>
#include <QDebug>

SensorIni::SensorIni(QObject *parent) : QObject(parent)
{
}

SensorIni::~SensorIni()
{
}

bool SensorIni::SaveTemplateToFile(const QString &fileName)
{
    QString content;
    generateCommentPart(content);
    QStringList sl;
    // Sensor section
    sl.append("[Sensor]");
    sl.append(QString("SchemeName=%1").arg(""));
    sl.append(QString("Description=%1").arg("SensorTemplate"));
    sl.append(QString("VendorName=%1").arg(""));
    sl.append(QString("ChipName=%1").arg(""));
    sl.append(QString("InterfaceType=%1").arg(""));
    sl.append(QString("Lanes=%1").arg(""));
    sl.append(QString("MipiFreq=%1").arg(""));
    sl.append(QString("Mclk=%1").arg(""));
    sl.append(QString("DataWidth=%1").arg(""));
    sl.append(QString("ImageFormat=%1").arg(""));
    sl.append(QString("ImageMode=%1").arg(""));
    sl.append(QString("PixelWidth=%1").arg(""));
    sl.append(QString("PixelHeight=%1").arg(""));
    sl.append(QString("QuickWidth=%1").arg(""));
    sl.append(QString("QuickHeight=%1").arg(""));
    sl.append(QString("CropParam=%1").arg(""));
    sl.append(QString("LvdsParam=%1").arg(""));
    sl.append(QString("RegBitsMode=%1").arg(""));
    sl.append(QString("CommIntfType=%1").arg(""));
    sl.append(QString("CommSpeed=%1").arg(""));
    sl.append(QString("CommAddr=0x%1").arg(""));
    sl.append(QString("CommExtraParam=%1").arg(""));
    sl.append(QString("PclkPol=%1").arg(""));
    sl.append(QString("DataPol=%1").arg(""));
    sl.append(QString("HsyncPol=%1").arg(""));
    sl.append(QString("VsyncPol=%1").arg(""));
    sl.append(QString("Pwdn=%1").arg(""));
    sl.append(QString("PwdnParam=%1").arg(""));
    sl.append(QString("Reset=%1").arg(""));
    sl.append(QString("ResetParam=%1").arg(""));
    sl.append(QString("FocusParam=%1").arg(""));
    sl.append(QString("AppType=%1").arg(""));
    sl.append("\n");

    sl.append("[FullModeParams]");
    sl.append("\n");

    sl.append("[OtpInitParams]");
    sl.append("\n");

    sl.append("[QuickModeParams]");
    sl.append("\n");

    sl.append("[SleepParams]");
    sl.append("\n");

    sl.append("[AfInitParams]");
    sl.append("\n");

    sl.append("[AfAutoParams]");
    sl.append("\n");

    sl.append("[AfFarParams]");
    sl.append("\n");

    sl.append("[AfNearParams]");
    sl.append("\n");

    sl.append("[ExposureParams]");
    sl.append("\n");

    sl.append("[GainParams]");
    sl.append("\n");

    sl.append("[Voltages]");
    sl.append("\n");

    sl.append("[FlagRegisters]");
    sl.append("\n");

    content = content + sl.join(QChar('\n'));

    m_file.setFileName(fileName);
    if (m_file.open(QFile::Text | QFile::WriteOnly | QFile::Truncate)) {
        m_textStream.setDevice(&m_file);
        m_textStream<<content<<flush;
        m_file.close();
        return true;
    }
    else
        return false;
}

bool SensorIni::SaveToFile(const Sensor &sensor, const QString &fileName)
{
    QString content;
    generateCommentPart(content);
    QStringList sl;
    // Sensor section
    sl.append("[Sensor]");
    sl.append(QString("SchemeName=%1").arg(sensor.SchemeName));
    sl.append(QString("Description=%1").arg(sensor.Description));
    sl.append(QString("VendorName=%1").arg(sensor.VendorName));
    sl.append(QString("ChipName=%1").arg(sensor.ChipName));
    sl.append(QString("InterfaceType=%1").arg(sensor.InterfaceType));
    sl.append(QString("Lanes=%1").arg(sensor.Lanes));
    sl.append(QString("MipiFreq=%1").arg(sensor.MipiFreq));
    sl.append(QString("Mclk=%1").arg(sensor.Mclk));
    sl.append(QString("DataWidth=%1").arg(sensor.DataWidth));
    sl.append(QString("ImageFormat=%1").arg(sensor.ImageFormat));
    sl.append(QString("ImageMode=%1").arg(sensor.ImageMode));
    sl.append(QString("PixelWidth=%1").arg(sensor.PixelWidth));
    sl.append(QString("PixelHeight=%1").arg(sensor.PixelHeight));
    sl.append(QString("QuickWidth=%1").arg(sensor.QuickWidth));
    sl.append(QString("QuickHeight=%1").arg(sensor.QuickHeight));
    sl.append(QString("CropParam=%1").arg(sensor.CropParam));
    sl.append(QString("LvdsParam=%1").arg(sensor.LvdsParam));
    sl.append(QString("RegBitsMode=%1").arg(sensor.RegBitsMode));
    sl.append(QString("CommIntfType=%1").arg(sensor.CommIntfType));
    sl.append(QString("CommSpeed=%1").arg(sensor.CommSpeed));
    sl.append(QString("CommAddr=0x%1").arg(sensor.CommAddr, 0, 16));
    sl.append(QString("CommExtraParam=%1").arg(sensor.CommExtraParam));
    sl.append(QString("PclkPol=%1").arg(sensor.PclkPol));
    sl.append(QString("DataPol=%1").arg(sensor.DataPol));
    sl.append(QString("HsyncPol=%1").arg(sensor.HsyncPol));
    sl.append(QString("VsyncPol=%1").arg(sensor.VsyncPol));
    sl.append(QString("Pwdn=%1").arg(sensor.Pwdn));
    sl.append(QString("PwdnParam=%1").arg(sensor.PwdnParam));
    sl.append(QString("Reset=%1").arg(sensor.Reset));
    sl.append(QString("ResetParam=%1").arg(sensor.ResetParam));
    sl.append(QString("FocusParam=%1").arg(sensor.FocusParam));
    sl.append(QString("AppType=%1").arg(sensor.AppType));
    sl.append("\n");

    QString tmpStr;
    sl.append("[FullModeParams]");
    QStringList tmpSl = sensor.FullModeParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[OtpInitParams]");
    tmpSl = sensor.OtpInitParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[QuickModeParams]");
    tmpSl = sensor.QuickModeParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[SleepParams]");
    tmpSl = sensor.SleepParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[AfInitParams]");
    tmpSl = sensor.AfInitParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[AfAutoParams]");
    tmpSl = sensor.AfAutoParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[AfFarParams]");
    tmpSl = sensor.AfFarParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[AfNearParams]");
    tmpSl = sensor.AfNearParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[ExposureParams]");
    tmpSl = sensor.ExposureParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[GainParams]");
    tmpSl = sensor.GainParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[OtpAlgParams]");
    tmpSl = sensor.OtpAlgParams.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[Voltages]");
    tmpSl = sensor.Voltage.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    sl.append("[FlagRegisters]");
    tmpSl = sensor.FlagRegister.split(QChar('\n'), QString::SkipEmptyParts);
    foreach (tmpStr, tmpSl) {
        sl.append(tmpStr.trimmed());
    }
    sl.append("\n");

    content = content + sl.join(QChar('\n'));

    m_file.setFileName(fileName);
    if (m_file.open(QFile::Text | QFile::WriteOnly | QFile::Truncate)) {
        m_textStream.setDevice(&m_file);
        m_textStream<<content<<flush;
        m_file.close();
        return true;
    }
    else
        return false;
}

bool SensorIni::LoadFromFile(Sensor &sensor, const QString &fileName)
{
    QString content;
    m_file.setFileName(fileName);
    if (m_file.open(QFile::Text | QFile::ReadOnly)) {
        m_textStream.setDevice(&m_file);
        content = m_textStream.readAll();
        m_file.close();
    }
    else {
        qCritical()<<QString("Cannot open file: %1!").arg(fileName);
        return false;
    }

    int idx = content.indexOf("CZTEK", 0, Qt::CaseSensitive);
    QString version = content.mid(idx + 6, 3); // Length("CZTEK ") = 6
    removeLineComment(content, content, "//");
    QString tmpString;
    // parse sensor section
    if (!getSection(content, "Sensor", tmpString)) {
        return false;
    }
    QStringList tmpSl = tmpString.split(QChar('\n'), QString::SkipEmptyParts);
    bool ok;
    QString key, value;
    foreach (QString s, tmpSl) {
        int pos = s.indexOf(QChar('='));
        if (pos < 0)
            continue;
        key = s.mid(0, pos).trimmed();
        value = s.mid(pos + 1).trimmed();
//        qDebug()<<key<<value;
        if (key == "SchemeName") {
            sensor.SchemeName = value;
        }
        if (key == "Description") {
            sensor.Description = value;
        }
        if (key == "VendorName") {
            sensor.VendorName = value;
        }
        if (key == "ChipName") {
            sensor.ChipName = value;
        }
        if (key == "InterfaceType") {
            sensor.InterfaceType = (E_InterfaceType)value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[value] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if ((key == "MipiLanes") || (key == "Lanes")) {
            sensor.Lanes = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "MipiFreq") {
            sensor.MipiFreqDef = sensor.MipiFreq = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "Mclk") {
            sensor.MclkDef = sensor.Mclk = value.toFloat(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "DataWidth") {
            sensor.DataWidth = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "ImageFormat") {
            sensor.ImageFormat = (E_ImageFormat)value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "ImageMode") {
            sensor.ImageModeDef = sensor.ImageMode = (E_ImageMode)value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "PixelWidth") {
            sensor.PixelWidth = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "PixelHeight") {
            sensor.PixelHeight = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "QuickWidth") {
            sensor.QuickWidth = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "QuickHeight") {
            sensor.QuickHeight = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "CropParam") {
            sensor.CropParam = value;
        }
        if (key == "LvdsParam") {
            qDebug()<<"LvdsParam:"<<value;
            sensor.LvdsParam = value;
        }
        if ((key == "I2CMode") || (key == "RegBitsMode")) {
            sensor.RegBitsMode = (E_RegBitsMode)value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "CommIntfType") {
            sensor.CommIntfType = (T_CommIntfConf::E_IntfType)value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if ((key == "I2CSpeed") || (key == "CommSpeed")) {
            sensor.CommSpeedDef = sensor.CommSpeed = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
            if (version == "1.0") {
                if (sensor.CommSpeed == 0)
                    sensor.CommSpeedDef = sensor.CommSpeed = 1; // 100kHz
                else
                    sensor.CommSpeedDef = sensor.CommSpeed = 4; // 400kHz
            }
        }
        if ((key == "I2CAddress") || (key == "CommAddr")) {
            sensor.CommAddr = value.toInt(&ok, 16);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "CommExtraParam") {
            sensor.CommExtraParam = value;
        }
        if (key == "PclkPol") {
            sensor.PclkPolDef = sensor.PclkPol = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "DataPol") {
            sensor.DataPolDef = sensor.DataPol = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "HsyncPol") {
            sensor.HsyncPolDef = sensor.HsyncPol = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "VsyncPol") {
            sensor.VsyncPolDef = sensor.VsyncPol = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "Pwdn") {
            sensor.PwdnDef = sensor.Pwdn = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "PwdnParam") {
            sensor.PwdnParam = value;
        }
        if (key == "Reset") {
            sensor.ResetDef = sensor.Reset = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
        if (key == "ResetParam") {
            sensor.ResetParam = value;
        }
        if (key == "FocusParam") {
            sensor.FocusParam = value;
        }
        if (key == "AppType") {
            sensor.AppType = value.toInt(&ok);
            if (!ok) {
                qCritical()<<QString("%1[%2] value is invalid!").arg(key).arg(value);
                return false;
            }
        }
    }

    if (getSection(content, "FullModeParams", tmpString)) {
        sensor.FullModeParams = tmpString.trimmed();
    }
    else {
        return false;
    }

    if (getSection(content, "OtpInitParams", tmpString)) {
        sensor.OtpInitParams = tmpString.trimmed();
    }

    if (getSection(content, "QuickModeParams", tmpString)) {
        sensor.QuickModeParams = tmpString.trimmed();
    }

    if (getSection(content, "SleepParams", tmpString)) {
        sensor.SleepParams = tmpString.trimmed();
    }

    if (getSection(content, "AfInitParams", tmpString)) {
        sensor.AfInitParams = tmpString.trimmed();
    }

    if (getSection(content, "AfAutoParams", tmpString)) {
        sensor.AfAutoParams = tmpString.trimmed();
    }

    if (getSection(content, "AfFarParams", tmpString)) {
        sensor.AfFarParams = tmpString.trimmed();
    }

    if (getSection(content, "AfNearParams", tmpString)) {
        sensor.AfNearParams = tmpString.trimmed();
    }

    if (getSection(content, "ExposureParams", tmpString)) {
        sensor.ExposureParams = tmpString.trimmed();
    }

    if (getSection(content, "GainParams", tmpString)) {
        sensor.GainParams = tmpString.trimmed();
    }

    if (getSection(content, "OtpAlgParams", tmpString)) {
        sensor.OtpAlgParams = tmpString.trimmed();
    }

    if (getSection(content, "Voltages", tmpString)) {
        sensor.VoltageDef = sensor.Voltage = tmpString.trimmed();
    }
    else {
        return false;
    }

    if (getSection(content, "FlagRegisters", tmpString)) {
        sensor.FlagRegister = tmpString.trimmed();
    }
    else {
        return false;
    }
    sensor.SystemRecord = 0;

    return true;
}

void SensorIni::FileVersion(QString &fileVersion)
{
    fileVersion = m_fileVersion;
}

void SensorIni::removeLineComment(QString &newContent, const QString &oldContent, const QString &commentMark)
{
    QStringList oldLineList = oldContent.split(QChar('\n'), QString::SkipEmptyParts);
    QStringList newLineList;
    foreach (QString line, oldLineList) {
        int commentIdx = line.indexOf(commentMark);
        if (commentIdx > -1)
            line.truncate(commentIdx); // 移除注释
        newLineList.append(line);
    }
    newContent = newLineList.join(QChar('\n'));
}

bool SensorIni::generateCommentPart(QString &commentPart)
{
    QStringList sl;
    sl.append("// CZTEK 1.1");
    sl.append("// www.cztek.cn");
    sl.append("// InterfaceType: 0-MIPI; 1-DVP; 2-MTK; 3-SPI; 4-HISPI; 5-SPREADTRUM; 6-TV; 7-UVC; 8-LVDS");
    sl.append("// ImageFormat: 0-RAW8; 1-RAW10; 2-RAW12; 3-RAW16; 4-RGB8; 5-RGB16; 6-RGB24; 7-RGB32; 8-YUV422; 9-YUV420");
    sl.append("// ImageMode: 0-YCbYCr/RGGB; 1-YCrYCb/GRBG; 2-CbYCrY/GBRG; 3-CrYCbY/BGGR");
    sl.append("// RegBitsMode: 0-Normal Mode; 1-ADDR8_DATA8; 2-ADDR8_DATA16; 3-ADDR16_DATA8; 4-ADDR16_DATA16");
    sl.append("// CommIntfConf: 0-IIC 1-SPI");
    sl.append("// CommSpeed: unit is 100kHz");
    sl.append("// CommAddr(HEX): if IIC, slave address; if SPI, chip id");
    sl.append("// CommExtraParam: if IIC, TBD; if spi, [mode(HEX), databits, isBigEndian]");
    sl.append("// LvdsParam: seaName, sedName, Version(HEX), Mode(HEX), V_Total(Dec), H_Total(Dec), V_H_Blank(Dec), Crop_Top");
    sl.append("// AppType: 0-czcmstd 1-cmotp");
    commentPart = sl.join("\n");
    commentPart += "\n\n";
    return true;
}

bool SensorIni::getSection(const QString &fileContent, const QString &sectionName, QString &sectionContent)
{
    sectionContent = "";
    QString sectionNameStr = QString("[%1]").arg(sectionName);
    int startIdx = fileContent.indexOf(sectionNameStr, 0, Qt::CaseInsensitive);
    if (startIdx > -1) {
        startIdx += sectionNameStr.length();
        int endIdx = fileContent.indexOf("[", startIdx, Qt::CaseInsensitive);
        sectionContent = fileContent.mid(startIdx, endIdx == -1 ? -1 : endIdx - startIdx).trimmed();
//        qDebug()<<QString("%1: %2").arg(sectionNameStr).arg(sectionContent);
        return true;
    }
    else {
        qWarning()<<QString("Cannot find section %1!").arg(sectionNameStr);
        return false;
    }
}


