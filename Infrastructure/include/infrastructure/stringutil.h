#pragma once

#include <gsl/string_span>

#include <fmt/format.h>

#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <sstream>
#include <cctype>
#include <iosfwd>

// Thanks Stackoverflow...

std::string ucs2_to_local(const std::wstring &);
std::wstring local_to_ucs2(const std::string &);
std::string ucs2_to_utf8(const std::wstring &);
std::wstring utf8_to_ucs2(const std::string &);

// trim from start
inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](auto c) { return !std::isspace(c); }));
	return s;
}

// trim from end
inline std::string &rtrim(std::string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), [](auto c) { return !std::isspace(c); }).base(), s.end());
	return s;
}

// trim from both ends
inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

inline std::vector<gsl::cstring_span<>>& split(gsl::cstring_span<> s, 
	char delim, 
	std::vector<gsl::cstring_span<>>& elems, 
	bool trimItems = false, 
	bool keepEmpty = false) {

	int pos = 0;
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
			elems.push_back(s.subspan(start, count));
		} else if (keepEmpty) {
			elems.push_back({});
		}

	}

	return elems;
}

inline std::vector<gsl::cstring_span<>> split(gsl::cstring_span<> s, char delim, bool trim = false, bool keepEmpty = false) {
	std::vector<gsl::cstring_span<>> elems;
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
		std::transform(result.begin(), result.end(), result.begin(), [](char ch) { return std::tolower((int)ch); });
		return result;
	} else {
		return s;
	}
}

inline std::string tounderscore(const std::string &s) {
	auto needsConversion = std::any_of(s.begin(), s.end(), [](char a) {
		return a == ' ';
	});
	if (needsConversion) {
		std::string result = s;
		std::transform(result.begin(), result.end(), result.begin(), [](char ch){
			if (ch == ' '){
				return '_';
			}
			return ch;
		});
		return result;
	}
	else {
		return s;
	}
}

inline std::string toupper(const std::string &s) {
	auto needsConversion = std::any_of(s.begin(), s.end(), [](char a) {
		return std::toupper(a) != a;
	});
	if (needsConversion) {
		std::string result = s;
		std::transform(result.begin(), result.end(), result.begin(), [](char ch) { return std::toupper((int)ch); });
		return result;
	}
	else {
		return s;
	}
}

// Nice to have operator for serializing vectors in the logger
namespace std {
	template<typename T>
	void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const std::vector<T> &v) {
		using namespace std;

		f.writer().write("[");
		for (size_t i = 0; i < v.size(); ++i) {
			f.writer().write("{}", v[i]);
			if (i != v.size() - 1) {
				f.writer().write(", ");
			}
		}
		f.writer().write("]");
	}
}

inline bool endsWith(const std::string &str, const std::string &suffix) {
	if (suffix.length() > str.length()) {
		return false; // Short-circuit
	}

	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}
