
#include "stdafx.h"
#include "libraryholder.h"

LibraryHolder::LibraryHolder(const wstring& path) : mLibraryHandle(0)
{
	// logger->info("Loading" << path;
	mLibraryHandle = LoadLibraryW(path.data());

	if (!mLibraryHandle)
	{
		LPSTR bufPtr = NULL;
		DWORD err = GetLastError();
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		                                             FORMAT_MESSAGE_FROM_SYSTEM |
		                                             FORMAT_MESSAGE_IGNORE_INSERTS,
		                                             NULL, err, 0, (LPSTR)&bufPtr, 0, NULL);
		mErrorText = bufPtr ? string(bufPtr) : format("Unknown error {}", err);
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
	return mLibraryHandle != nullptr;
}
