#include "TlvParser.h"
#include <string.h>
#include <QDebug>

TlvParser::TlvParser()
{
}

int TlvParser::CalcTlvBufLength(const QVector<T_TagLenVal> &inParams)
{
    int len = 0;
    for (auto it = inParams.begin(); it != inParams.end(); ++it)
    {
        len += (1 + 4 + it->Length);
    }
    return len;
}

void TlvParser::TlvToBuffer(const QVector<T_TagLenVal> &inParams, uchar *pBuf, int len)
{
    int offset = 0;
    for (auto it = inParams.begin(); it != inParams.end(); ++it)
    {
#if 1
        uint32 u32Temp = 0;
        if (it->Length <= 4) {
            ParseIntValue(*it, u32Temp);
        }
        else
            u32Temp = 0xffffffff;
        qDebug("Tag: 0x%02x, Length: %d, Value: %08x", it->Tag, it->Length, u32Temp);
#endif
        if ((int)(offset + 1 + 4 + it->Length) > len)
            break;

        // tag
        pBuf[offset] = (uchar)(it->Tag);
        offset++;

        // length
        for (int i = 0; i < 4; i++)
            pBuf[offset + i] = (it->Length >> (i*8)) & 0xff;
        offset += 4;

        // value
        if (1 == it->Length)
            pBuf[offset] = it->u8Value;
        else if (2 == it->Length) {
            pBuf[offset] = it->u16Value & 0xff;
            pBuf[offset + 1] = (it->u16Value >> 8) & 0xff;
        }
        else if (4 == it->Length) {
            pBuf[offset] = it->u32Value & 0xff;
            pBuf[offset + 1] = (it->u32Value >> 8) & 0xff;
            pBuf[offset + 2] = (it->u32Value >> 16) & 0xff;
            pBuf[offset + 3] = (it->u32Value >> 24) & 0xff;
        }
        else {
            memcpy(&pBuf[offset], it->szValue, it->Length);
        }
        offset += it->Length;
    }
}

void TlvParser::BufferToTlv(const uchar *pBuf, uint32 len, QVector<T_TagLenVal> &outParams)
{
    outParams.clear();
    uint32 offset = 0;
    while (offset < len)
    {
        T_TagLenVal tlv;
        tlv.Tag = pBuf[offset];
        offset++;

        if (offset + 4 > len)
            break;
        tlv.Length = (pBuf[offset+3] << 24) | (pBuf[offset+2] << 16) | (pBuf[offset+1] << 8) | pBuf[offset];
        offset += 4;

        if (offset + tlv.Length > len)
            break;
        uint32 u32Temp = 0xffffffff;
        if (1 == tlv.Length) {
            tlv.u8Value = pBuf[offset];
            u32Temp = tlv.u8Value;
        }
        else if (2 == tlv.Length) {
            tlv.u16Value = pBuf[offset] | (pBuf[offset + 1] << 8);
            u32Temp = tlv.u16Value;
        }
        else if (4 == tlv.Length) {
            tlv.u32Value = pBuf[offset] | (pBuf[offset + 1] << 8) | (pBuf[offset + 2] << 16) | (pBuf[offset + 3] << 24);
            u32Temp = tlv.u32Value;
        }
        else
            tlv.szValue = (uchar*)&pBuf[offset];
        offset += tlv.Length;
#if 1
        qDebug("Tag: 0x%02x, Length: %d, Value: %08x", tlv.Tag, tlv.Length, u32Temp);
#endif

        outParams.push_back(tlv);
    }
}

bool TlvParser::ParseIntValue(const T_TagLenVal &tlv, uint32 &val)
{
    val = 0;
    if (4 == tlv.Length) {
        val = tlv.u32Value;
        return true;
    }
    else if (2 == tlv.Length) {
        val = tlv.u16Value;
        return true;
    }
    else if (1 == tlv.Length) {
        val = tlv.u8Value;
        return true;
    }
    return false;
}
