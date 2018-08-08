#ifndef __UIDEFS_H__
#define __UIDEFS_H__
#include <QObject>

namespace UiDefs {
    const QString NC_SENSOR_NAME = "--- NC ---";
    enum E_DisplayMode {
        DM_OriginalSize,
        DM_FullScreen,
        DM_FitWindowSize,
        DM_FitImageSize,
    };

    struct T_VideoDispParam {
        float ScaleFactor;
        E_DisplayMode DisplayMode;
        float DispCapRatio;
        T_VideoDispParam() {
            ScaleFactor = 1.0;
            DispCapRatio = 1.0;
            DisplayMode = DM_FitImageSize;
        }
    };

    enum E_WorkMode {
        WorkMode_Manual = 0,
        WorkMode_Machine,
    };

    enum E_Operator {
        Operator_Operator = 0,
        Operator_Engineer,
    };
}

enum E_ReservedCategoryCode {
    ReservedCategory_NoError = 0,
    ReservedCategory_OpenCamera = 1,
    ReservedCategory_CloseCamera = 2,
    ReservedCategory_NotFinished = 3,
    ReservedCategory_VideoInterrupt = 4,
};

#endif // __UIDEFS_H__
