#include "CaptureThread.h"
#include <QDebug>
#include <QMutexLocker>
#include "utils/utils.h"
#include <QUuid>
#include <chrono>
#include <cstdint>

CaptureThread::CaptureThread(uint chnIdx, QObject *parent) :
    QThread(parent),
    m_fpsCalculator(FpsCalculator::CC_ByElapsedTime, 2000)
{
    m_chnIdx = chnIdx;
    m_hal = nullptr;
    m_capTransPingPongBuffer = nullptr;

    m_bExitFlag = false;
    m_bPauseFlag = false;
}

CaptureThread::~CaptureThread()
{
}

void CaptureThread::BindHal(IHalOx *hal)
{
    m_hal = hal;
}

void CaptureThread::BindCapTransBufferPool(PingPongBuffer *capTransPingPongBuffer)
{
    m_capTransPingPongBuffer = capTransPingPongBuffer;
}

void CaptureThread::StartRunning()
{
    QMutexLocker locker(&m_mutex);
    m_bExitFlag = false;
    m_bPauseFlag = false;
    start();
}

void CaptureThread::StopRunning()
{
    QMutexLocker locker(&m_mutex);
    m_bExitFlag = true;
    m_bPauseFlag = false;
    wait(3000);
}

void CaptureThread::Pause()
{
    m_bPauseFlag = true;
}

void CaptureThread::Resume()
{
    m_bPauseFlag = false;
}

void CaptureThread::run()
{
    qInfo("============%s[%p]============", __FUNCTION__, QThread::currentThread());
    float fps;
    m_fpsCalculator.Reset();
    int bufIndex;
    int retCode = 0;
    uchar *pDrvBuffer;
    uint size = m_capTransPingPongBuffer->GetBufferSize();
    struct timeval capTime;
    while (!m_bExitFlag)
    {
        while (m_bPauseFlag) {
            msleep(20);
        }
        if ((retCode = m_hal->DequeueBuffer(bufIndex, &pDrvBuffer, capTime)) != 0) {
            if (ERR_VideoDequeuTimeout == retCode) { // 可恢复
                continue;
            }
            else if (ERR_VideoNoDevice == retCode) {
                qCritical("error occured! No Device!!!");
                emit errorOccured((int)ERR_VideoNoDevice);
                break;
            }
            else { // 不可恢复
                qCritical("error occured! Failed!!!");
                emit errorOccured((int)ERR_Failed);
                break;
            }
        }
        if (m_bExitFlag) {
            qDebug("User canncelled!");
            break;
        }

        uint64 timestamp = (uint64)capTime.tv_sec * 1000000 + capTime.tv_usec;
//std::int64_t currTimestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
//qInfo("%llu, %lld,%llu", timestamp, currTimestamp, currTimestamp - timestamp);
        m_capTransPingPongBuffer->WriteBuffer(pDrvBuffer, size, timestamp);
        if ((retCode = m_hal->EnqueueBuffer(bufIndex)) != 0) {
            qCritical("Calling EnqueueBuffer() failed.");
            emit errorOccured(ERR_VideoNoDevice == retCode ? (int)ERR_VideoNoDevice : (int)ERR_Failed);
            break;
        }

        if (m_fpsCalculator.Update(fps)) {
//            qDebug("%s[%p: %d] %.3f fps", __FUNCTION__, QThread::currentThread(), m_chnIdx, fps);
            emit fpsChanged(fps);
        }
    }
    m_bExitFlag = true;
    qInfo()<<__FUNCTION__<<"exited.";
}
