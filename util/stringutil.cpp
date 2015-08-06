
#include "stdafx.h"
#include "util/stringutil.h"

#include <locale>
#include <codecvt>

wstring FromUTF8(const std::string& str)
{
	typedef std::codecvt_utf8<wchar_t> type;
	std::wstring_convert<type, wchar_t> converter;

	return converter.from_bytes(str);
}

string ToUTF8(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> type;
	std::wstring_convert<type, wchar_t> converter;

	return converter.to_bytes(wstr);
}
