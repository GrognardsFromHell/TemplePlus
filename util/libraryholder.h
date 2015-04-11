#ifndef LIBRARYHOLDER_H
#define LIBRARYHOLDER_H

#include "stdafx.h"

class LibraryHolder
{
public:
	LibraryHolder(const wstring& path);
	~LibraryHolder();

	template <typename T>
	T getFun(const char* name)
	{
		T result = (T) GetProcAddress(mLibraryHandle, name);
		if (!result)
		{
			logger->error("Unable to find proc {} in DLL", name);
		}
		return result;
	}

	bool valid();

	const string& errorText() const
	{
		return mErrorText;
	}

private:
	HMODULE mLibraryHandle;
	string mErrorText;
};

#endif // LIBRARYHOLDER_H

