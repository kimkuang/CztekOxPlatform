#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QElapsedTimer>
#include <atomic>
#include "hal/IHalOx.h"
#include "czcmtidefs.h"
#include "CzUtilsDefs.h"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(uint chnIdx, QObject *parent = 0);
    ~CaptureThread();

    void BindHal(IHalOx *hal);
    void BindCapTransBufferPool(PingPongBuffer *capTransPingPongBuffer);

    void StartRunning();
    void StopRunning();
    void Pause();
    void Resume();

signals:
    void fpsChanged(float fps);
    void errorOccured(int ec);

protected:
    void run();

private:
    uint m_chnIdx;
    QMutex m_mutex;
    IHalOx *m_hal;
    PingPongBuffer *m_capTransPingPongBuffer;
    FpsCalculator m_fpsCalculator;
    std::atomic<bool> m_bExitFlag;
    std::atomic<bool> m_bPauseFlag;
};

#endif // CAPTURETHREAD_H
