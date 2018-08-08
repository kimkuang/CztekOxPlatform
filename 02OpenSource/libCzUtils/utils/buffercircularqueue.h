#ifndef BUFFERCIRCULARQUEUE_H
#define BUFFERCIRCULARQUEUE_H
#include <QMutex>
#include <QtGlobal>
#include <QDebug>
#include <QSemaphore>
#include "library_global.h"

// http://www.cnblogs.com/diegodu/p/4619104.html

class LIBRARY_API BufferCircularQueue
{
public:
    class T_DataItem {
    public:
        uchar *PtrAllocatedBuffer;
        uint BufferCapacity;
        uint BufferSize;
        quint64 Timestamp;

        T_DataItem(uint bufferSize);
        ~T_DataItem();
    };
    typedef T_DataItem* PDataItem;
public:
    BufferCircularQueue(unsigned queueSize, unsigned bufferSize);
    virtual ~BufferCircularQueue();

    bool AcquireRearItem(PDataItem &pDataItem, int timeout = -1);
    void ReleaseRearItem();

    bool AcquireHeadItem(PDataItem &pDataItem, int timeout = -1);
    void ReleaseHeadItem();

    inline int GetBufferSize() const {
        return m_bufferSize;
    }

private:
    unsigned m_queueSize;
    int m_bufferSize;
    unsigned m_head;
    unsigned m_rear;
    QSemaphore *m_semaphoreFreeSpace;
    QSemaphore *m_semaphoreFilledSpace;
    char *m_dataItemMem;
    T_DataItem* m_dataItemPool;
};

#endif // BUFFERCIRCULARQUEUE_H
