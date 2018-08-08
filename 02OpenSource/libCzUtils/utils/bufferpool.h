#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H
#include "library_global.h"
#include <QQueue>
#include <QMutex>

class LIBRARY_API Buffer
{
public:
    Buffer(uint size, uint idx);
    ~Buffer();
    void AssertSize(uint size);
    void IncRef(int step = 1);
    void DecRef(int step = 1);
    inline void SetOccupied(bool occupied) {
        m_occupied = occupied;
    }
    inline bool IsUsing() {
        QMutexLocker locker(&m_mutex);
        return m_occupied;
    }
    inline char* GetData() {
        return m_data;
    }
    inline uint GetIndex() {
        return m_index;
    }
private:
    int m_ref;
    char *m_data;
    uint m_size;
    uint m_index;
    bool m_occupied;
    QMutex m_mutex;
};


class LIBRARY_API BufferPool : public QQueue<Buffer *>
{
public:
    enum { DEFAULT_POOL_SIZE = 32 };
    BufferPool();
    ~BufferPool();
    void SetPoolSize(int poolSize);
    Buffer *GetBuffer(uint bufferSize);
    void DebugStatus();

private:
    QMutex m_mutex;
    int m_poolSize;
};

#endif // BUFFERPOOL_H
