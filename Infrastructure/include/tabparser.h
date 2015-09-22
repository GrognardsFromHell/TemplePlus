
#pragma once

#include <string>
#include <functional>
#include <vector>
#include <map>

class TabFileColumn {
	friend class TabFileRecord;
public:

	bool IsEmpty() const {
		return mValue.empty();
	}

	operator bool() const {
		return !IsEmpty();
	}

	operator const std::string &() const {
		return mValue;
	}

	const std::string &AsString() {
		return mValue;
	}

	bool EqualsIgnoreCase(const char *text) {
		return !_stricmp(mValue.c_str(), text);
	}

	bool TryGetFloat(float &value) {
		return sscanf_s(mValue.c_str(), "%f", &value) == 1;
	}

	template<typename T>
	bool TryGetEnum(const std::map<std::string, T> &mapping, T& value) {
		for (auto it = mapping.begin(); it != mapping.end(); ++it) {
			if (!_stricmp(it->first.c_str(), mValue.c_str())) {
				value = it->second;
				return true;
			}
		}
		return false;
	}

private:
	TabFileColumn(const std::string &value) : mValue(value) {
	}
	const std::string &mValue;	
};

class TabFileRecord {
friend class TabFile;
public:
	int GetLineNumber() const {
		return mLineNumber;
	}

	size_t GetColumnCount() const {
		return mColumns.size();
	}

	TabFileColumn operator[](size_t i) const {
		if (i >= mColumns.size()) {
			return TabFileColumn(mMissingColumn);
		}
		return TabFileColumn(mColumns[i]);
	}

private:
	static std::string mMissingColumn;

	int mLineNumber = 0;
	std::vector<std::string> mColumns;
};

class TabFile {
public:
	
	typedef std::function<void(const TabFileRecord&)> Callback;

	static void ParseFile(
		const std::string &filename,
		const Callback &callback
	);

	static void ParseString(
		const std::string &content,
		const Callback &callback
	);

};
