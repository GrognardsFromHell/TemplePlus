#include "libraryholder.h"

LibraryHolder::LibraryHolder(const QString &path) : mLibraryHandle(0)
{
    qDebug() << "Loading" << path;
    const wchar_t *dllPath = (LPCWSTR)path.utf16();
    mLibraryHandle = LoadLibraryW(dllPath);

    if (!mLibraryHandle) {
        LPWSTR bufPtr = NULL;
        DWORD err = GetLastError();
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
        mErrorText = (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                       QString("Unknown Error %1").arg(err);
        LocalFree(bufPtr);
    }
}

LibraryHolder::~LibraryHolder()
{
    if (mLibraryHandle) {
        FreeLibrary(mLibraryHandle);
    }
}

bool LibraryHolder::valid()
{
    return !!mLibraryHandle;
}

