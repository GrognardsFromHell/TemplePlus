#pragma once

// Thanks Stackoverflow...

// trim from start
inline string &ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
inline string &rtrim(string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

// trim from both ends
inline string &trim(string &s) {
	return ltrim(rtrim(s));
}

inline vector<string>& split(const string& s, char delim, vector<string>& elems, bool trimItems = false, bool keepEmpty = false) {
	stringstream ss(s);
	string item;
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


inline vector<string> split(const string& s, char delim, bool trim = false, bool keepEmpty = false) {
	vector<string> elems;
	split(s, delim, elems, trim, keepEmpty);
	return elems;
}
