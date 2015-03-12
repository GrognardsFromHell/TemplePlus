
#pragma once

#include "addresses.h"

/**
 * Describes how text is rendered on the screen (style, font, etc.)
 */
struct tig_text_style
{
	int unk1;
};

/**
 * Plug in "text" and pass to Measure to get the on-screen measurements a blob of text
 * will have.
 */
struct tig_font_metrics
{
	const char *text = nullptr;
	int width = 0;
	int height = 0;
	int lines = 0;
	int lineheight = 0;
};

/**
 * Used by Draw to position and draw the text. measurements are made automatically if not passed in via w/h
 */
struct tig_font_extents
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
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

struct TigFontFuncs : AddressTable
{
	int(__cdecl *Measure)(tig_text_style *style, tig_font_metrics *metrics);
	int(__cdecl *Draw)(const char *text, tig_font_extents* extents, tig_text_style *style);

	void rebase(Rebaser rebase) override {
		rebase(Measure, 0x101EA4E0);
		rebase(Draw, 0x101EAF30);
	}
};

extern GlobalStruct<TigFontData, 0x10EEEEC8> fontData;
extern TigFontFuncs tigFont;
