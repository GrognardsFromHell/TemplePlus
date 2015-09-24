
#include "infrastructure/mesparser.h"
#include "infrastructure/vfs.h"
#include "infrastructure/logging.h"

class MesLexer {
public:
	MesLexer(const std::string &filename, const std::string &content) : mFilename(filename), mContent(content) {
	}

	bool ReadNextToken();
	
	const std::string &GetToken() const {
		return mToken;
	}

	int GetLine() const {
		return mLine;
	}

private:
	int GetCh();
	bool IsEof() const;
	std::string mFilename;
	std::string mToken;
	std::string mContent;
	size_t mContentPos = 0;
	int mLine = 0;
};

bool MesLexer::ReadNextToken() {
	mToken.clear();

	// Seek token start
	auto foundStart = false;
	for (auto ch = GetCh(); ch != -1; ch = GetCh()) {
		if (ch == '}') {
			logger->warn("Closing brace before opening brace @ {}:{}", mFilename, mLine);
		} else if (ch == '{') {
			foundStart = true;
			break;
		}
	}

	if (!foundStart) {
		return false;
	}

	for (auto ch = GetCh(); ch != -1; ch = GetCh()) {
		if (ch == '}') {
			return true;
		}

		mToken.push_back((char) ch);
		
		if (ch == '{') {
			logger->warn("Found opening brace before closing brace @ {}:{}", mFilename, mLine);
		}
		if (mToken.size() >= 1999) {
			logger->warn("Line exceeds 2000 char limit @ {}:{}", mFilename, mLine);
		}
	}

	logger->warn("Found EOF before reaching closing brace @ {}:{}", mFilename, mLine);
	return false; // Incomplete token
}

inline int MesLexer::GetCh() {
	if (mContentPos < mContent.size()) {
		char ch = mContent[mContentPos++];
		if (ch == '\n') {
			mLine++;
		}
		return ch;
	}
	return -1;
}

inline bool MesLexer::IsEof() const {
	return mContentPos >= mContent.size();
}

MesFile::Content MesFile::ParseFile(const std::string& filename) {
	auto content = vfs->ReadAsString(filename);
	return ParseString(content, filename);
}

MesFile::Content MesFile::ParseString(const std::string& content, const std::string &filename) {

	Content result;
	MesLexer lexer(filename, content);
		
	while (lexer.ReadNextToken()) {
		size_t idx;
		auto token = lexer.GetToken();
		auto key = stoi(token, &idx);

		if (idx == 0) {
			logger->warn("Invalid numeric key @ {}:{}", filename, lexer.GetLine());
		}

		if (!lexer.ReadNextToken()) {
			logger->warn("Key without value @ {}:{}", filename, lexer.GetLine());
			break;
		}

		result[key] = lexer.GetToken();
	}

	return result;

}
