
#pragma once

#include <temple/dll.h>

enum class StringTokenType : uint32_t {
	Invalid = 0,
	Identifier,
	Number,
	QuotedString,
	Unk4,
	Unk5,
	Plus = '+'
};

struct StringToken {
	StringTokenType type;
	uint32_t field_4;
	double numberFloat;
	int numberInt;
	const char *text;
	uint32_t field_18;
	uint32_t tokenStart;
	uint32_t tokenEnd; // Exclusive
	const char *originalStr;
};

struct StringTokenizerFuncs : temple::AddressTable {
	int(__cdecl *Create)(const char *line, int *strTokenId);
	int(__cdecl *NextToken)(int *tokenizerId, StringToken *tokenOut);
	int(__cdecl *Destroy)(int *tokenizerId);

	StringTokenizerFuncs() {
		rebase(Create, 0x101F2350);
		rebase(NextToken, 0x101F25F0);
		rebase(Destroy, 0x101F2560);
	}
};

extern StringTokenizerFuncs stringTokenizerFuncs;

// Uses the ToEE internal string tokenizer
class StringTokenizer {
public:
	explicit StringTokenizer(const string &line) : mTokenizerId(0) {
		int errorCode;
		if ((errorCode = stringTokenizerFuncs.Create(line.c_str(), &mTokenizerId)) != 0) {
			logger->error("Unable to create tokenizer for string {}: {}", line, errorCode);
		}
	}

	~StringTokenizer() {
		stringTokenizerFuncs.Destroy(&mTokenizerId);
		mTokenizerId = 0;
	}

	/*
	 note: logic is inverted relative to vanilla ToEE
	*/
	bool next() { 
		return !stringTokenizerFuncs.NextToken(&mTokenizerId, &mToken);
	}

	const StringToken &token() const {
		return mToken;
	}
private:
	int mTokenizerId;
	StringToken mToken;
};
