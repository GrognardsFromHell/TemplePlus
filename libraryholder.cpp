
#include "stdafx.h"
#include "libraryholder.h"

LibraryHolder::LibraryHolder(const path& path) : mLibraryHandle(0)
{
	LOG(info) << "Loading" << path;
	mLibraryHandle = LoadLibraryW(path.wstring().data());

	if (!mLibraryHandle)
	{
		LPWSTR bufPtr = NULL;
		DWORD err = GetLastError();
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		                                             FORMAT_MESSAGE_FROM_SYSTEM |
		                                             FORMAT_MESSAGE_IGNORE_INSERTS,
		                                             NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
		mErrorText = bufPtr ? wstring(bufPtr) : (wformat(L"Unknown error %1%") % err).str();
		LocalFree(bufPtr);
	}
}

LibraryHolder::~LibraryHolder()
{
	if (mLibraryHandle)
	{
		FreeLibrary(mLibraryHandle);
	}
}

bool LibraryHolder::valid()
{
	return !!mLibraryHandle;
}
