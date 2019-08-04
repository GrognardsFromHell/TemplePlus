#include "stdafx.h"
#include "common.h"
#include "tab_file.h"
#include "util/fixes.h"

TabFileSystem tabSys;





#pragma region TabFileSystem Implementation

TabFileSystem::TabFileSystem()
{
	macRebase(tabFileStatusInit, 101F2C10)
	macRebase(tabFileStatusDealloc, 101F2C30)
	macRebase(tabFileParseLines, 101F2C70)
	macRebase(tabFileGetNumLines, 101F2D40)
	macRebase(tabFileStatusBasicFormatter,101F2E40)
}

#pragma endregion

// Turns out ToEE doesn't properly add the null-byte when loading a tab file
static class TabParserFixes : public TempleFix {
	
	virtual void apply() override
	{
		// formatRawString
		static void(*orgFunction)(TabFileStatus *);
		orgFunction = replaceFunction<void(TabFileStatus *)>(0x101F2DC0, [](TabFileStatus *parser) {
			*parser->endOfString = '\0';
			orgFunction(parser);
		});

	}

} tabParserFix;
