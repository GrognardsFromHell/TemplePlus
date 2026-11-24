
#include "stdafx.h"
#include <temple/dll.h>
#include "tig/tig_font.h"
#include "util/fixes.h"
#include "fonts.h"
#include "tig/tig_startup.h"
#include "graphics/device.h"
#include "graphics/textengine.h"
#include <ui/ui.h>

static struct FontRenderAddresses : temple::AddressTable {

	int* stackSize;
	TigFont* loadedFonts;
	int* fontStack;

	FontRenderAddresses() {
		rebase(stackSize, 0x10EF2E50);
		rebase(loadedFonts, 0x10EF1448);
		rebase(fontStack, 0x10EF2C48);
	}
} addresses;

// Font Rendering Replacement
static class FontRenderFix : public TempleFix {
public:
	void apply() override;

	static int FontDraw(const char* text, TigRect* extents, TigTextStyle* style);
	
	static int FontMeasure(const TigTextStyle &style, TigFontMetrics &metrics);
} fix;

void FontRenderFix::apply() {
	replaceFunction(0x101EAF30, FontDraw);
	replaceFunction(0x101EA4E0, FontMeasure);
}

int FontRenderFix::FontDraw(const char* text, TigRect* extents, TigTextStyle* style) {
	style->colorSlot = 0;
	if (*addresses.stackSize < 1)
		return 3;

	auto font = addresses.loadedFonts[addresses.fontStack[0]];

	auto& layouter = tig->GetTextLayouter();

	if (extents->x < 0 || extents->width < 0){
		logger->warn("Negative Text extents! Aborting draw.");
		return 0;
	}

	layouter.LayoutAndDraw(span( text, strlen(text) ), font, *extents, *style);
	uiManager->RenderedTextTabulate(-1, text);
	return 0;
}

int FontRenderFix::FontMeasure(const TigTextStyle & style, TigFontMetrics & metrics)
{
	if (*addresses.stackSize < 1)
		return 3;

	auto font = addresses.loadedFonts[addresses.fontStack[0]];

	// Some places in ToEE will not correctly set the text color before calling into this ...
	TigTextStyle textStyleFixed = style;
	textStyleFixed.textColor = nullptr;

	auto& layouter = tig->GetTextLayouter();
	layouter.Measure(font, textStyleFixed, metrics);
	return 0;
}
