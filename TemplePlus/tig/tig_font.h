
#pragma once

#include <temple/dll.h>
#include "tig.h"

enum TigTextStyleFlag {
	TTSF_DROP_SHADOW = 0x8,
	TTSF_CENTER = 0x10,
	TTSF_CONTINUATION_INDENT = 0x200,
	TTSF_BACKGROUND = 0x400,
	TTSF_BORDER = 0x800,
	TTSF_TRUNCATE = 0x4000,
	TTSF_ROTATE = 0x8000,
	TTSF_ROTATE_OFF_CENTER = 0x10000
};

/**
 * Describes how text is rendered on the screen (style, font, etc.)
 */
#pragma pack(push, 1)
struct TigTextStyle
{
	int field0 = 0;
	int tracking = 0;
	int kerning = 1; // Anything less than this doesn't render properly
	int leading = 0;
	int field10 = 0;
	int field14 = 0;
	float rotation = 0;
	float rotationCenterX = 0;
	float rotationCenterY = 0;
	int field24 = 0;
	/*
		8 seems to be drop shadow
		0x10 centers the text
		0x200 continuation indent
		0x400 Draws a filled rect behind the text (see bgColor)
		0x800 Draws a border box around the text, always black
		0xC00 is used for tooltips
		Not seen in the wild:
		0x1000
		0x2000
		0x4000 truncates text if too long for rect and appends "..."
		0x8000 seems to rotate
		0x10000 offset for rotation is set
	*/
	int flags = 0;
	int field2c = 0;
	int colorSlot = 0;
	ColorRect *textColor = nullptr; // array of text colors for use with the @n text color modifiers
	ColorRect *colors2 = nullptr;
	ColorRect *shadowColor = nullptr; // Use with flags |= 0x8
	ColorRect *colors4 = nullptr;
	ColorRect *bgColor = nullptr; // Use with flags |= 0x400
	int field48 = 0;
	int field4c = 0;
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
	TigRect rect;
	int fileIdx;
	int width_line;
	int width_line_x_offset;
	int base_line_y_offset;
};

struct TigFont {
	int baseline;
	int largestHeight;
	int fontsize;
	int antialiased;
	int glyphCount;
	int fileCount;
	TigFontGlyph* glyphs;
	const char* name;
	int textureIds[4];
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
