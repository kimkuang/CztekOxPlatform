#ifndef MACRODEFS_H
#define MACRODEFS_H

#define MAKE_WORD(lo, hi)      ((ushort)(((uchar)((lo) & 0xff)) | ((ushort)((uchar)((hi) & 0xff))) << 8))
#define MAKE_DWORD(lo, hi)     ((ulong)(((ushort)((lo) & 0xffff)) | ((ulong)((ushort)((hi) & 0xffff))) << 16))
#define LO_WORD(l)           ((ushort)((l) & 0xffff))
#define HI_WORD(l)           ((ushort)(((l) >> 16) & 0xffff))
#define LO_BYTE(w)           ((uchar)((w) & 0xff))
#define HI_BYTE(w)           ((uchar)(((w) >> 8) & 0xff))

#define STRINGIZE(...)  #__VA_ARGS__

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

template <typename T>
inline const T &MIN(const T &a, const T &b) { return (a < b) ? a : b; }

template <typename T>
inline const T &MAX(const T &a, const T &b) { return (a < b) ? b : a; }

template <typename T>
inline const T &BOUND(const T &min, const T &val, const T &max)
{ return MAX(min, MIN(max, val)); }

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)          { if(p) { delete (p); (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)    { if(p) { delete [] (p); (p)=nullptr; } }
#define SAFE_CLOSE_HANDLE(p)	{ if(p) { ::CloseHandle(p); (p)=nullptr; }	}
#define SAFE_DELETE_OBJECT(p)	{ if(p) { ::DeleteObject((HGDIOBJ) p); (p)=nullptr; } }
#endif	/* SAFE_DELETE */

#endif // MACRODEFS_H
