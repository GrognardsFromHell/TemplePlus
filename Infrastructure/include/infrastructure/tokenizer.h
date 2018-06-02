
#pragma once

#include <sstream>
#include <gsl/string_span>

/*
	String tokenizer that works like the ToEE vanilla one.
*/
class Tokenizer {
public:

	explicit Tokenizer(const std::string &input) : mIn(input) {
	}
	
	bool NextToken();

	bool IsQuotedString() const {
		return mTokenType == TokenType::QuotedString;
	}

	bool IsNumber() const {
		return mTokenType == TokenType::Number;
	}

	bool IsIdentifier() const {
		return mTokenType == TokenType::Identifier;
	}

	bool IsIdentifier(const char *identifier) const;

	const std::string &GetTokenText() const {
		return mTokenText;
	}

	const bool& GetEnableEscapes() const {
		return mEnableEscapes;
	}

	void SetEnableEscapes(bool enableEscapes) {
		mEnableEscapes = enableEscapes;
	}

	int GetTokenInt() const {
		return mTokenInt;
	}

	float GetTokenFloat() const {
		return (float) mTokenFloat;
	}

private:
	enum class TokenType {
		Number,
		QuotedString,
		Identifier,
		Unknown
	};

	std::istringstream mIn;
	std::string mTokenText;
	TokenType mTokenType = TokenType::Unknown;
	double mTokenFloat = 0;
	int mTokenInt = 0;

	std::string mLine; // Buffer for current line
	size_t mLinePos; // Current position within mLine
	int mLineNo = 0;
	bool mEnableEscapes = true;

	bool GetLine();
	bool LineHasMoreChars() const;

	bool ReadNumber();
	bool ReadQuotedString();
	bool ReadIdentifier();

	// Character based reading on the input source
	char PeekChar();
	void SkipChar();
	char TakeChar();
	void UngetChar();

	// Seeks past any control or space characters at current line pos
	void SkipSpaceAndControl();
	// Skips past comment at current line pos to end of line
	void SkipComment();
};


