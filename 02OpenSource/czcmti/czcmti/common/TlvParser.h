#ifndef TLVPARSER_H
#define TLVPARSER_H
#include <QVector>
#include "TlvDefs.h"

class TlvParser
{
public:
    TlvParser();

    int CalcTlvBufLength(const QVector<T_TagLenVal> &inParams);
    void TlvToBuffer(const QVector<T_TagLenVal> &inParams, uchar *pBuf, int len);
    void BufferToTlv(const uchar *pBuf, uint32 len, QVector<T_TagLenVal> &outParams);
    static bool ParseIntValue(const T_TagLenVal &tlv, uint32 &val);
};

#endif // TLVPARSER_H
