#ifndef LIBRARYHOLDER_H
#define LIBRARYHOLDER_H

#include "stdafx.h"

class LibraryHolder
{
public:
	LibraryHolder(const path& path);
	~LibraryHolder();

	template <typename T>
	T getFun(const char* name)
	{
		T result = (T) GetProcAddress(mLibraryHandle, name);
		if (!result)
		{
			LOG(error) << "Unable to find proc " << name << " in DLL";
		}
		return result;
	}

	bool valid();

	const wstring& errorText() const
	{
		return mErrorText;
	}

private:
	HMODULE mLibraryHandle;
	wstring mErrorText;
};

#endif // LIBRARYHOLDER_H

