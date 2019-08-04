
#pragma once

#include <functional>
#include <vector>
#include <map>

#include <gsl/string_span>

class TabFileColumn {
	friend class TabFileRecord;
public:

	bool IsEmpty() const {
		return mValue.size() == 0;
	}

	operator bool() const {
		return !IsEmpty();
	}

	operator gsl::cstring_span<>() const {
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
	explicit TabFileColumn(gsl::cstring_span<> value) : mValue(value) {
	}
	gsl::cstring_span<> mValue;	
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
	std::vector<gsl::cstring_span<>> mColumns;
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
