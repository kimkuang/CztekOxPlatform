#ifndef PRODUCTBARCODE_H
#define PRODUCTBARCODE_H
#include <QObject> // QFlags->QObject FIX: ISO C++ forbids declaration of 'Q_FLAGS' with no type in QT5.5
#include <QDate>

class ProductBarcode
{
public:
    enum E_ProductFeature {
        PF_None = 0x00,
        PF_WorkingCurrent = 0x01,
        PF_StandbyCurrent = 0x02,
        PF_OpenShort = 0x04,
        PF_All = PF_WorkingCurrent | PF_StandbyCurrent | PF_OpenShort,
    };
    Q_DECLARE_FLAGS(ProductFeatures, E_ProductFeature)
    Q_FLAGS(ProductFeatures)

    struct T_Barcode {
        QString OriginalString;
        QString HalVersion;
        QString DeviceVersion;
        QDate ProductDate;
        int SerialNumber;
        T_Barcode() {
            OriginalString = "";
            HalVersion = "";
            SerialNumber = 0;
        }
    };

    static bool GetProductBarcode(T_Barcode &barcode);
    static void GetFeatureFromBarcode(const T_Barcode &barcode, ProductFeatures &productFeatures);
    static bool GetRootfsVersion(QString &rootfsVer);
    static bool GetRootfsPatchVersion(QString &patchVer);
};

#endif // PRODUCTBARCODE_H
