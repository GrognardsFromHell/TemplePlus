
#include "stdafx.h"
#include "textbubbles.h"
#include <temple/dll.h>

TextBubbles textBubbles;

static struct TextBubbleAddresses : temple::AddressTable {
	void (__cdecl *HideAll)();

	TextBubbleAddresses() {
		rebase(HideAll, 0x100A30F0);
	}
} addresses;

void TextBubbles::HideAll() {
	addresses.HideAll();
}
