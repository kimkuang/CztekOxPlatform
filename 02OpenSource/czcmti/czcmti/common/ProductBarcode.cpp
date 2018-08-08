#include "ProductBarcode.h"
#include <QFile>
#include <QTextStream>
#include "controller/ChannelController.h"

bool ProductBarcode::GetProductBarcode(T_Barcode &barcode)
{
    barcode.OriginalString = ChannelController::GetInstance(0)->GetBarcode();
    barcode.HalVersion = ChannelController::GetInstance(0)->GetHalVersion();
    barcode.DeviceVersion = ChannelController::GetInstance(0)->GetDeviceVersion();
    return true;
}

void ProductBarcode::GetFeatureFromBarcode(const T_Barcode &barcode, ProductFeatures &productFeatures)
{
    (void)barcode;
    productFeatures = ProductBarcode::PF_All;
}

bool ProductBarcode::GetRootfsVersion(QString &rootfsVer)
{
    rootfsVer = "";

    return true;
}

bool ProductBarcode::GetRootfsPatchVersion(QString &patchVer)
{
    patchVer = "";
    QString fileName = "/etc/cztek/patches_version.txt"; // cztek definition
    if (!QFile::exists(fileName)) {
        return false;
    }
    QFile file(fileName);
    QStringList strList;
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString line;
        do {
            line = stream.readLine();
            if (!line.isEmpty())
                strList.append(line);
        } while (!stream.atEnd());
    }
    file.close();

    if (strList.count() < 1)
        return false;
    patchVer = strList[strList.count() - 1].trimmed();
    return true;
}
