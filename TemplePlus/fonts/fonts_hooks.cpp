
#include "stdafx.h"
#include <temple/dll.h>
#include "tig/tig_font.h"
#include "util/fixes.h"
#include "fonts.h"
#include "tig/tig_startup.h"

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

} fix;

void FontRenderFix::apply() {
	replaceFunction(0x101EAF30, FontDraw);
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

	layouter.LayoutAndDraw(as_span( text, strlen(text) ), font, *extents, *style);

	return 0;
}
