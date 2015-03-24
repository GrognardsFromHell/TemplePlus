
#include "stdafx.h"
#include "fixes.h"
/*
	Gives an example for a memory modifying fix.
*/

/*
static void __cdecl sanitizeSpellSlots(int arg1, int arg2) {
	logger->info("HAHAHAHAHA");
}

class BonusSpellFix : public TempleFix {
public:
	const char* name() override {
		return "bonus spell fix";
	}
	
	void apply() override {
		writeHex(0x100F4C65, "E8 869BF5FF");
		writeHex(0x1010EFE8, "E8 0973DA01");
		redirectCall(0x1010EFE8, sanitizeSpellSlots);
	}
} bonusSpellFix;
*/
