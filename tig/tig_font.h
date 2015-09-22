
#pragma once

#include <temple/dll.h>
#include "tig.h"

enum TigTextStyleFlag {
	TTSF_DROP_SHADOW = 0x8,
	TTSF_CENTER = 0x10
};

/**
 * Describes how text is rendered on the screen (style, font, etc.)
 */
#pragma pack(push, 1)
struct TigTextStyle
{
	int field0;
	int tracking = 0;
	int kerning = 1; // Anything less than this doesn't render properly
	int leading = 0;
	int field10;
	int field14;
	int field18;
	int field1c;
	int field20;
	int field24;
	/*
		8 seems to be drop shadow
		0x10 centers the text
		0x400 Draws a filled rect behind the text (see bgColor)
		0x800 Draws a border box around the text, always black
		0xC00 is used for tooltips
		Not seen in the wild:
		0x2000
		0x4000 truncates text if too long for rect and appends "..."
	*/
	int flags = 0;
	int field2c;
	int field30;
	ColorRect *textColor = nullptr;
	ColorRect *colors2 = nullptr;
	ColorRect *shadowColor = nullptr; // Use with flags |= 0x8
	ColorRect *colors4 = nullptr;
	ColorRect *bgColor = nullptr; // Use with flags |= 0x400
	int field48;
	int field4c;
};
#pragma pack(pop)

/**
 * Plug in "text" and pass to Measure to get the on-screen measurements a blob of text
 * will have.
 */
struct TigFontMetrics
{
	const char *text = nullptr;
	int width = 0;
	int height = 0;
	int lines = 0;
	int lineheight = 0;
};

struct TigFontGlyph {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t field10;
	uint32_t width_line;
	uint32_t width_line_x_offset;
	uint32_t base_line_y_offset;
};

struct TigFont {
	uint32_t field0;
	uint32_t largestHeight;
	uint32_t fontsize;
	uint32_t fieldc;
	uint32_t glyphcount;
	uint32_t field14;
	TigFontGlyph* glyphs;
	const char* name;
	uint32_t artIds[4];
};

struct TigFontData {
	uint16_t indices[4800];
	TigFont fonts[128];
	int fontStack[128];
	uint32_t dword_10EF2E48;
	uint32_t dword_10EF2E4C;
	uint32_t fontStackSize;
	uint32_t dword_10EF2E54;
	uint32_t dword_10EF2E58; // Possibly never read or written to
	uint32_t dword_10EF2E5C;
};

struct TigFontFuncs : temple::AddressTable
{
	int(__cdecl *Measure)(const TigTextStyle &style, TigFontMetrics &metrics);
	int(__cdecl *Draw)(const char *text, const TigRect& extents, const TigTextStyle& style);
	int(__cdecl *LoadAll)(const char *pattern);
	int(__cdecl *PushFont)(const char *name, int size, bool antialias);
	int(__cdecl *PopFont)();

	TigFontFuncs() {
		rebase(Measure, 0x101EA4E0);
		rebase(Draw, 0x101EAF30);
		rebase(LoadAll, 0x101EA1F0);
		rebase(PushFont, 0x101E89D0);
		rebase(PopFont, 0x101E8AC0);
	}
};

extern temple::GlobalStruct<TigFontData, 0x10EEEEC8> fontData;
extern TigFontFuncs tigFont;
