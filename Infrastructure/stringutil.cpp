
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <locale>
#include <codecvt>

#include <platform/windows.h>

#include "infrastructure/stringutil.h"

std::string ucs2_to_local(const std::wstring&s) {
	
	auto slength = (int)s.length() + 1;
	auto len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	
	std::string result(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &result[0], len, 0, 0);
	result.resize(strlen(result.c_str()));
	return result;

}

std::wstring local_to_ucs2(const std::string &s) {

	auto slength = (int)s.length() + 1;
	auto len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);

	std::wstring result(len, '\0');
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &result[0], len);
	result.resize(result.length());
	return result;

}

std::string ucs2_to_utf8(const std::wstring &str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(str);
}

std::wstring utf8_to_ucs2(const std::string &str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}