
#include <vector>

#include <gsl/string_span>

#include "infrastructure/tabparser.h"
#include "infrastructure/stringutil.h"
#include "infrastructure/vfs.h"

std::string TabFileRecord::mMissingColumn;

using strview = gsl::cstring_span<gsl::dynamic_extent>;

static strview PostProcessColumn(strview column);

class LineReader {
public:
	LineReader(const std::string &content) : mContent(content) {
	}

	bool NextLine() {

		// Skip all \r\n
		while (!IsAtEnd() && IsAtLineEnd()) {
			mPos++;
		}

		if (IsAtEnd()) {
			return false;
		}

		auto start = mPos;
		size_t count = 0;
		while (!IsAtEnd() && !IsAtLineEnd()) {
			mPos++;
			count++;
		}
		
		mCurLine = strview(&mContent[start], count);
		return true;
	}

	strview GetLine() const {
		return mCurLine;
	}

private:
	strview mCurLine;
	const std::string &mContent;
	size_t mPos = 0;

	bool IsAtEnd() const {
		return mPos >= mContent.size();
	}

	bool IsAtLineEnd() const {
		auto ch = mContent[mPos];
		return ch == '\n' || ch == '\r';
	}
};

void TabFile::ParseFile(const std::string & filename, const Callback & callback)
{
	auto content = vfs->ReadAsString(filename);
	ParseString(content, callback);
}

void TabFile::ParseString(const std::string &content, const TabFile::Callback &callback) {
	TabFileRecord record;
	
	LineReader reader(content);
	while (reader.NextLine()) {
		auto line = reader.GetLine();
		
		record.mColumns.clear();
		split(line, '\t', record.mColumns, false, true);

		// Post-Process the columns
		for (auto& column : record.mColumns) {
			column = PostProcessColumn(column);
		}

		callback(record);
		record.mLineNumber++;
	}

}

static strview PostProcessColumn(strview column) {
	// ToEE will remove trailing vertical tabs and spaces
	int junkAtEnd = 0;
	for (int i = column.size() - 1; i >= 0; i--) {
		auto ch = column[i];
		// Vertical tabs and spaces will be trimmed
		if (ch != ' ' && ch != '\x0b') {
			break;
		}
		junkAtEnd++;
	}
	if (junkAtEnd > 0) {
		return column.subspan(0, column.size() - junkAtEnd);
	} else {
		return column;
	}
}
