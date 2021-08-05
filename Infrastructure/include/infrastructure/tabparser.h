
#pragma once

#include <functional>
#include <vector>
#include <string_view>
#include <map>

class TabFileColumn {
	friend class TabFileRecord;
public:

	bool IsEmpty() const {
		return mValue.size() == 0;
	}

	operator bool() const {
		return !IsEmpty();
	}

	operator std::string_view() const {
		return mValue;
	}

	std::string AsString() const {
		return std::string(mValue.begin(), mValue.end());
	}

	bool EqualsIgnoreCase(const char *text) const {
		return !_strnicmp(mValue.data(), text, mValue.size());
	}

	bool TryGetFloat(float &value) const {
		if (mValue.size() == 0) {
			return false;
		}
		return _snscanf_s(mValue.data(), mValue.size(), "%f", &value) == 1;
	}

	template<typename T>
	bool TryGetEnum(const std::map<std::string, T> &mapping, T& value) {
		for (auto it = mapping.begin(); it != mapping.end(); ++it) {
			if (!_strnicmp(it->first.c_str(), mValue.data(), mValue.size())) {
				value = it->second;
				return true;
			}
		}
		return false;
	}

private:
	explicit TabFileColumn(std::string_view value) : mValue(value) {
	}
	std::string_view mValue;
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
	std::vector<std::string_view> mColumns;
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
