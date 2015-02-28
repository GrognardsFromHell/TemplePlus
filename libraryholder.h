#ifndef LIBRARYHOLDER_H
#define LIBRARYHOLDER_H

#include "system.h"

class LibraryHolder
{
public:
    LibraryHolder(const QString &path);
    ~LibraryHolder();

    template<typename T>
    T getFun(const char *name) {
        T result = (T) GetProcAddress(mLibraryHandle, name);
        if (!result) {
            qFatal("Unable to find proc %s in DLL.", name);
        }
        return result;
    }

    bool valid();
    const QString &errorText() const { return mErrorText; }
private:
    HMODULE mLibraryHandle;
    QString mErrorText;
};

#endif // LIBRARYHOLDER_H
