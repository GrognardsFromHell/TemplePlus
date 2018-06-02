#include <cctype>

#include <gsl/gsl>
#include "infrastructure/tokenizer.h"
#include "infrastructure/exception.h"

bool Tokenizer::NextToken() {

	// This skips comments / spaces after a token has been read
	if (LineHasMoreChars()) {
		SkipSpaceAndControl();
		SkipComment();
	}

	// This loop finds the next line
	// If we ran out of line, seek to the next one	
	while (!LineHasMoreChars()) {
		if (!GetLine()) {
			return false;
		}
	}
	
	// Record the position where the current token started
	auto startOfToken = mLinePos;

	if (ReadNumber()) {
		return true;
	} else if (ReadQuotedString()) {
		return true;
	} else if (ReadIdentifier()) {
		return true;
	}

	mLinePos = startOfToken;
	auto ch = TakeChar();
		
	if (ispunct(ch)) {
		mTokenText = ch;
		mTokenType = TokenType::Unknown;
		return true;
	}

	throw TempleException("Unrecognized character: {}", ch);

}

bool Tokenizer::IsIdentifier(const char* identifier) const {
	
	// In debug mode ensure that the identifier 
	// being passed in is lowercase
	Expects(std::all_of(identifier, identifier + strlen(identifier), [](char a)
		{
			return tolower(a) == a;
		}));

	if (mTokenType != TokenType::Identifier) {
		return false;
	}

	return mTokenText == identifier;
}

bool Tokenizer::GetLine() {
	mLinePos = 0;
	if (std::getline(mIn, mLine)) {
		// Spaces at beginning and end of the line
		// are ignored. This is more lenient than
		// vanilla ToEE, but it is convenient
		// trim(mLine);
		mLineNo++;
		SkipSpaceAndControl();
		SkipComment();
		return true;
	}
	return false;
}

bool Tokenizer::LineHasMoreChars() const {
	return mLinePos < mLine.size();
}

bool Tokenizer::ReadNumber() {

	Expects(LineHasMoreChars());

	auto startOfToken = mLinePos;

	auto firstChar = PeekChar();

	// Handle digits
	if (firstChar != '+' && firstChar != '-' && !isdigit(firstChar)) {
		return false;
	}

	SkipChar(); // Consumes the character we peeked

	mTokenText.clear();
	mTokenText.push_back(firstChar);

	auto foundDecimalMarks = false;
	auto foundDigits = isdigit(firstChar);

	while (LineHasMoreChars()) {
		auto nextChar = TakeChar();

		if (isdigit(nextChar)) {
			foundDigits = true;
		} else if (nextChar == '.' && !foundDecimalMarks) {
			foundDecimalMarks = true;
		} else {
			UngetChar(); // Read something that doesn't belong to the number
			break;
		}

		mTokenText.push_back(nextChar);
	}

	// Seems to be a number...
	if (!foundDigits) {
		// While we started with + or -, we didn't actually read a number
		mLinePos = startOfToken;
		return false;
	}

	mTokenFloat = std::stod(mTokenText);
	mTokenInt = static_cast<int>(mTokenFloat);
	mTokenType = TokenType::Number;
	return true;

}

bool Tokenizer::ReadQuotedString() {

	auto startedOnLine = mLineNo;
	auto startedPos = mLine;
	auto firstChar = PeekChar();

	if (firstChar != '"' && firstChar != '\'') {
		return false; // Not a quoted string
	}

	TakeChar(); // We dont actually store the quote

	mTokenText.clear();

	auto lastLineEndingEscaped = false;

	while (true) {

		// If the quoted string is not terminated yet, it actually eats lines
		while (!LineHasMoreChars()) {
			if (!GetLine()) {
				// TODO Nice error
				throw TempleException("Unterminated string literal");
			}
			if (!lastLineEndingEscaped) {
				mTokenText.push_back('\n');
			} else {
				// The backslash is worth only a single line ending
				lastLineEndingEscaped = false;
			}
		}

		auto ch = TakeChar();
		if (ch == firstChar) {
			break;
		}

		// Handle escape sequences if enabled
		if (mEnableEscapes && ch == '\\') {
			// It's possible for a quoted string to span multiple lines by
			// escaping the end of the line
			if (!LineHasMoreChars()) {
				lastLineEndingEscaped = true;
				continue;
			}

			// Escape sequences always consume the next char
			auto nextChar = TakeChar();
			switch (tolower(nextChar)) {
			case 'n':
			case 'r':
				ch = '\n';
				break;
			case 't':
				ch = '\t';
				break;
				/*
				ToEE also supports hexadecimal escape sequences (a la \xF2), but
				the implementation seems broken...
				On top of that it doesn't seem to be used anywhere...
			*/
			default:
				// ... This kinda means any path with backslashes in MDF files gets mangled...
				ch = nextChar;
				break;
			}
		}

		mTokenText.push_back(ch);
	}

	mTokenType = TokenType::QuotedString;
	return true;

}

bool Tokenizer::ReadIdentifier() {

	auto startedPos = mLine;
	auto firstChar = PeekChar();

	// Identifiers start with an alpha char or underscore
	if (firstChar != '_' && !isalpha(firstChar)) {
		return false;
	}

	mTokenText.clear();
	while (LineHasMoreChars()) {
		auto ch = TakeChar();
		if (isdigit(ch) || isalpha(ch) || ch == '_') {
			// Note that identifiers are always lowercased automatically
			mTokenText.push_back(tolower(ch));
		} else {
			UngetChar();
			break;
		}
	}

	mTokenType = TokenType::Identifier;
	return true;

}

char Tokenizer::PeekChar() {
	Expects(LineHasMoreChars());
	return mLine[mLinePos];
}

void Tokenizer::SkipChar() {
	Expects(LineHasMoreChars());
	mLinePos++;
}

char Tokenizer::TakeChar() {
	Expects(LineHasMoreChars());
	return mLine[mLinePos++];
}

void Tokenizer::UngetChar() {
	mLinePos--;
	Expects(mLinePos >= 0);
}

void Tokenizer::SkipSpaceAndControl() {
	// Skip control characters and spaces
	while (LineHasMoreChars()) {
		auto ch = mLine[mLinePos];
		if (!isspace(ch) && !iscntrl(ch)) {
			return;
		}
		mLinePos++; // Skip space & control
	}
}

void Tokenizer::SkipComment() {

	if (mLinePos < mLine.size()
		&& mLine[mLinePos] == '#') {
		mLinePos = mLine.size();

	} else if (mLinePos + 1 < mLine.size()
		&& mLine[mLinePos] == '/'
		&& mLine[mLinePos + 1] == '/') {
		mLinePos = mLine.size();
	}

}
