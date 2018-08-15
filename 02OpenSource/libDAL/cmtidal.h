#ifndef CMTIDALDEFS_H
#define CMTIDALDEFS_H
#include "czcmtidefs.h"

#ifndef LIBRARY_GLOBAL_H
#define LIBRARY_GLOBAL_H

#ifdef QT_CORE_LIB
    #include <QtCore/qglobal.h>
    #if defined(LIBRARY_EXPORTS)
        #define LIBRARY_API Q_DECL_EXPORT
    #else
        #define LIBRARY_API Q_DECL_IMPORT
    #endif
#else
    #ifdef LIBRARY_EXPORTS
        #ifdef __GNUC__
            #define LIBRARY_API __attribute__((dllexport))
        #else
            #define LIBRARY_API __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define LIBRARY_API __attribute__((dllimport))
        #else
            #define LIBRARY_API __declspec(dllimport)
        #endif
    #endif
#endif /* QT_CORE_LIB */

#endif /* LIBRARY_GLOBAL_H */


#ifdef __cplusplus
extern "C" {
#endif

LIBRARY_API bool LoadSensorSettingFromFile(T_SensorSetting *sensorSetting, const char *fileName);
LIBRARY_API void FreeSensorSetting(T_SensorSetting *sensorSetting);

#ifdef __cplusplus
}
#endif

#endif // CMTIDALDEFS_H
