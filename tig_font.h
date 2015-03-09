
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

struct TigFontFuncs : AddressTable
{
	int(__cdecl *Measure)(tig_text_style *style, tig_font_metrics *metrics);
	int(__cdecl *Draw)(const char *text, tig_font_extents* extents, tig_text_style *style);

	void rebase(Rebaser rebase) override {
		rebase(Measure, 0x101EA4E0);
		rebase(Draw, 0x101EAF30);
	}
};

extern TigFontFuncs tigFont;
