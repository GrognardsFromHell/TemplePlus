#pragma once

#include "gsl/string_view.h"

#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <sstream>
#include <cctype>
#include <iosfwd>

// Thanks Stackoverflow...

std::string ucs2_to_local(const std::wstring &);
std::string ucs2_to_utf8(const std::wstring &);
std::wstring utf8_to_ucs2(const std::string &);

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

inline std::vector<gsl::cstring_view<>>& split(gsl::cstring_view<> s, 
	char delim, 
	std::vector<gsl::cstring_view<>>& elems, 
	bool trimItems = false, 
	bool keepEmpty = false) {

	size_t pos = 0;
	while (pos < s.size()) {
		auto ch = s[pos++];

		// Empty item
		if (ch == delim) {
			if (keepEmpty) {
				elems.push_back({});
			}
			continue;
		}

		if (trimItems && isspace(ch)) {
			// Skip all chars that are considered whitespace
			continue;
		}

		auto start = pos - 1; // Start of item
		size_t count = 1; // Size of item

		// Seek past all of the item's content
		while (pos < s.size()) {
			ch = s[pos++];
			if (ch == delim) {
				break; // reached the end of the item
			}
			count++;
		}

		// Trim whitespace at the end of the string
		if (trimItems) {
			while (count > 0 && isspace(s[start + count - 1])) {
				count--;
			}
		}

		if (count != 0) {
			elems.push_back(s.sub(start, count));
		} else if (keepEmpty) {
			elems.push_back({});
		}

	}

	return elems;
}

inline std::vector<gsl::cstring_view<>> split(gsl::cstring_view<> s, char delim, bool trim = false, bool keepEmpty = false) {
	std::vector<gsl::cstring_view<>> elems;
	split(s, delim, elems, trim, keepEmpty);
	return elems;
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

// Nice to have operator for serializing vectors in the logger
template<typename T>
std::ostream &operator <<(std::ostream &os, const std::vector<T> &v) {
	using namespace std;

	os << "[";
	for (size_t i = 0; i < v.size(); ++i) {
		os << v[i];
		if (i != v.size() - 1) {
			os << ", ";
		}
	}
	os << "]";

	return os;
}
