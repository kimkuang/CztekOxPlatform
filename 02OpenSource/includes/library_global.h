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
            #define LIBRARY_API __attribute__((visibility("default")))
        #else
            #define LIBRARY_API __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define LIBRARY_API __attribute__((visibility("default")))
        #else
            #define LIBRARY_API __declspec(dllimport)
        #endif
    #endif

#endif

#endif // LIBRARY_GLOBAL_H
