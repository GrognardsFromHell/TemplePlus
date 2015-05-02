
#include "stdafx.h"
#include "textbubbles.h"
#include "util/addresses.h"

TextBubbles textBubbles;

static struct TextBubbleAddresses : AddressTable {
	void (__cdecl *HideAll)();

	TextBubbleAddresses() {
		rebase(HideAll, 0x100A30F0);
	}
} addresses;

void TextBubbles::HideAll() {
	addresses.HideAll();
}
