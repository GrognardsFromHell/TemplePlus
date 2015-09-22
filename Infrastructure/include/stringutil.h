#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <sstream>
#include <cctype>

// Thanks Stackoverflow...

std::string ucs2_to_local(const std::wstring &);

// trim from start
inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
inline std::string &rtrim(std::string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

// trim from both ends
inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

inline std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems, bool trimItems = false, bool keepEmpty = false) {
	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim)) {
		if (trimItems) {
			item = trim(item);
		}
		if (keepEmpty || !item.empty()) {
			elems.push_back(item);
		}
	}
	return elems;
}


inline std::vector<std::string> split(const std::string& s, char delim, bool trim = false, bool keepEmpty = false) {
	std::vector<std::string> elems;
	split(s, delim, elems, trim, keepEmpty);
	return elems;
}

inline std::string tolower(const std::string &s) {
	auto needsConversion = std::any_of(s.begin(), s.end(), [](char a) {
		return std::tolower(a) != a;
	});
	if (needsConversion) {
		std::string result = s;
		std::transform(result.begin(), result.end(), result.begin(), std::tolower);
		return result;
	} else {
		return s;
	}
}
