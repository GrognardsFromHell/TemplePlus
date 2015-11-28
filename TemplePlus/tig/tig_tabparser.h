#pragma once
#include <temple/dll.h>

/*
	The following structures and functions are used by ToEE to parse all
	tab separated files (.tab) i.e. proto.tab, trap.tab, etc.
*/

struct TigTabParser;
typedef int (__cdecl TigTabLineParser)(const TigTabParser* parser, int lineIdx, char** columns);

#pragma pack(push, 1)
struct TigTabParser {
	char* filename;
	int lineCount;
	int maxColumns;
	int curLineIdx;
	char* fileContent;
	int fileContentLen;
	TigTabLineParser * lineParser;
};
#pragma pack(pop)

struct TigTabParserFuncs : temple::AddressTable {

	void (__cdecl *Init)(TigTabParser* parser, TigTabLineParser* tabParserFunc);
	int (__cdecl *Open)(TigTabParser* parser, const char* filename);
	int (__cdecl *GetLineCount)(TigTabParser* parser);
	int (__cdecl *Process)(TigTabParser* parser);
	void (__cdecl *Close)(TigTabParser* parser);

	TigTabParserFuncs();
};

extern TigTabParserFuncs tigTabParserFuncs;
