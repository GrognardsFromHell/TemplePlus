
#include <vector>
#include <sstream>

#include "infrastructure/tabparser.h"
#include "infrastructure/stringutil.h"
#include "infrastructure/vfs.h"

std::string TabFileRecord::mMissingColumn;

static void PostProcessColumn(std::string &column);

void TabFile::ParseFile(const std::string & filename, const Callback & callback)
{
	auto content = vfs->ReadAsString(filename);
	ParseString(content, callback);
}

void TabFile::ParseString(const std::string &content, const TabFile::Callback &callback) {
	std::string line;
	TabFileRecord record;

	std::istringstream in(content);
	while (std::getline(in, line, '\n')) {
		record.mColumns.clear();
		split(line, '\t', record.mColumns, false, true);

		// Post-Process the columns
		for (std::string &column : record.mColumns) {
			PostProcessColumn(column);
		}

		callback(record);
		record.mLineNumber++;
	}

}

static void PostProcessColumn(std::string &column) {
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
		column.erase(column.size() - junkAtEnd, junkAtEnd);
	}
}
