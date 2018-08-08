#ifndef __LIBRARYLOADER_H__
#define __LIBRARYLOADER_H__
#include "library_global.h"
#include <string>

class LIBRARY_API LibraryLoader
{
public:
    LibraryLoader(const std::string &path);
    ~LibraryLoader();

    const char* GetName() const;
    bool Load(const std::string &libDispName);
    void* GetSymbolAddress(const char* symbolName) const;
    void Unload();

private:
    typedef void* HLIB;
	HLIB m_hDynLib;
    std::string m_libraryPath;
	char* m_pszLibName;
};

#endif /* __LIBRARYLOADER_H__ */
