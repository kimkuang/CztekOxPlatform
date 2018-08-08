#ifndef SYNCEVENT_H
#define SYNCEVENT_H
#include "library_global.h"
#ifdef SYNC_QSEMAPHORE
#include <limits.h>
#include <QSemaphore>
#else
#include <QMutex>
#include <QWaitCondition>
#endif

// reference: .NET class AutoResetEvent/ManualResetEvent, MFC class CEvent
// reference: http://www.firstsolver.com/wordpress/?p=399
class LIBRARY_API SyncEvent
{
public:
    SyncEvent(bool initialState = false, bool manualReset = false);
    bool WaitOne(unsigned long time = ULONG_MAX);
    bool Set();
    bool Reset();
#ifdef SYNC_QSEMAPHORE
    void SetOne();
#endif

private:
#ifdef SYNC_QSEMAPHORE // QSemaphore使用有误差(仅为了与之前程序统计一致)
    QSemaphore m_semaphore;
#else
    volatile bool m_signalState; // 注意：阻止编译器对其进行优化
    volatile bool m_manualReset;
    QMutex m_mutex;   // 互斥量
    QWaitCondition m_waitCondition; // 条件等待信号量
#endif
};

#endif // SYNCEVENT_H
