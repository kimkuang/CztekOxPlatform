#ifndef PINGPONGBUFFER_H
#define PINGPONGBUFFER_H
#include "library_global.h"
#include "czcmtidefs.h"
#include <mutex>
#include <cstdint>

class LIBRARY_API PingPongBuffer
{
public:
    class T_DataItem {
    public:
        enum E_DataStatus {
            CanWrite = 0,
            Writting,
            CanRead,
            Reading
        };
        uchar *PtrAllocatedBuffer;
        const uint BufferCapacity;
        uint BufferSize;
        uint64 Timestamp;
        E_DataStatus Status; // CanRead, Reading, CanWrite, Writting

        T_DataItem() = delete;
        T_DataItem(uint bufferSize);
        ~T_DataItem();
    };
    typedef T_DataItem* PDataItem;
public:
    PingPongBuffer(unsigned bufferSize);
    virtual ~PingPongBuffer();

    bool ReadBuffer(uchar *pBuffer, uint &size, uint64 &timestamp);
    bool BeginReadBuffer(int &readIdx, PDataItem &pDataItem);
    void EndReadBuffer(int readIdx);
    bool WriteBuffer(const uchar *pBuffer, uint size, uint64 timestamp);
    bool BeginWriteBuffer(int &writeIdx, PDataItem &pDataItem);
    void EndWriteBuffer(int writeIdx);

    inline unsigned GetBufferSize() const {
        return m_bufferSize;
    }

private:
    unsigned m_bufferSize;
    char *m_dataItemMem;
    T_DataItem* m_dataItemPool;
    std::mutex m_mutex;
};

#endif // PINGPONGBUFFER_H
