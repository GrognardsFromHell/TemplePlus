
#include "stdafx.h"
#include "fixes.h"


class SpellHostilityFlagFix : public TempleFix {
public:
	const char* name() override {
		return "Spell Hostility bug: fix mass cure spells triggering hostile reaction. Can be expanded to other spells.";
	}

	void apply() override {
		LOG(info) << "Applying Spell Hostility (Cure X Wounds, Mass) Bug fix...";
		writeHex(0x10076EF4, "02"); // Cure Light Wounds, Mass
		writeHex(0x10076F42, "02"); // Mass Heal
		writeHex(0x10077058, "02 02 02"); // Cure Moderate + Serious + Critical Wounds, Mass
	}
} spellHostilityFlagFix;
