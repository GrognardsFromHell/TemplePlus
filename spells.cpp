
#include "stdafx.h"
#include "fixes.h"
#include "spell.h"


static_assert(sizeof(SpellStoreData) == (32U), "SpellStoreData structure has the wrong size!");

SpontCastSpellLists::SpontCastSpellLists()
{
	uint32_t _spontCastSpellsDruid[] = { -1, 476, 477, 478, 479, 480, 481, 482, 483, 484, 4000 };
	uint32_t _spontCastSpellsEvilCleric[] = { 248, 247, 249, 250, 246, 61, 581, 582, 583, 583, 0 };
	uint32_t _spontCastSpellsGoodCleric[] = { 91, 90, 92, 93, 89, 221, 577, 578, 579, 579, 0 };
	uint32_t _spontCastSpellsDruidSummons[] = { -1, 2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 0 };
	memcpy(spontCastSpellsDruid, _spontCastSpellsDruid, 11);
	memcpy(spontCastSpellsEvilCleric, _spontCastSpellsEvilCleric, 11);
	memcpy(spontCastSpellsGoodCleric, _spontCastSpellsGoodCleric, 11);
	memcpy(spontCastSpellsDruidSummons, _spontCastSpellsDruidSummons, 11);
}

SpontCastSpellLists spontCastSpellLists_spells;
//GlobalPrimitive<uint16_t>
//1028D09C



void __declspec(naked) DruidRadialSelectSummonsHook()
{
	__asm{
		push eax;
		call _DruidRadialSelectSummons;
		mov ebp, eax;
		pop eax;
		retn;
	}
};

void __declspec(naked) DruidRadialSpontCastSpellEnumHook()
{
	__asm{
		push eax;
		call _DruidRadialSpontCastSpellEnumHook;
		mov ecx, eax;
		pop eax;
		retn;
	}
};

void __declspec(naked) GoodClericRadialSpontCastSpellEnumHook()
{
	__asm{
		push eax;
		call _GoodClericRadialSpontCastSpellEnumHook;
		mov ecx, eax;
		pop eax;
		retn;
	}
};

void __declspec(naked) EvilClericRadialSpontCastSpellEnumHook()
{
	__asm{
		push eax;
		call _EvilClericRadialSpontCastSpellEnumHook;
		mov ecx, eax;
		pop eax;
		retn;
	}
};



uint32_t _DruidRadialSelectSummons(uint32_t spellSlotLevel)
{
	return spontCastSpellLists_spells.spontCastSpellsDruidSummons[spellSlotLevel];
}

uint32_t _DruidRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel)
{
	return spontCastSpellLists_spells.spontCastSpellsDruid[spellSlotLevel];
}

uint32_t _GoodClericRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel)
{
	return spontCastSpellLists_spells.spontCastSpellsGoodCleric[spellSlotLevel];
}

uint32_t _EvilClericRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel)
{
	return spontCastSpellLists_spells.spontCastSpellsEvilCleric[spellSlotLevel];
}

class SpontaneousCastingExpansion : public TempleFix {
public:
	const char* name() override {
		return "Recreation of SpellSlinger's Spontaneous Casting for levels 6-9";
	}

	void apply() override {
		writeCall(0x100F1127, DruidRadialSelectSummonsHook); // replaces SpellSlinger's hook for Druid Summon options
		writeCall(0x100F113F, DruidRadialSpontCastSpellEnumHook);
		writeCall(0x100F109D, GoodClericRadialSpontCastSpellEnumHook);
		writeCall(0x100F10AE, EvilClericRadialSpontCastSpellEnumHook);
	}
} spellSpontCastExpansion;



class SpellHostilityFlagFix : public TempleFix {
public:
	const char* name() override {
		return "Spell Hostility bug: fix mass cure spells triggering hostile reaction. Can be expanded to other spells.";
	}

	void apply() override {
		writeHex(0x10076EF4, "02"); // Cure Light Wounds, Mass
		writeHex(0x10076F42, "02"); // Mass Heal
		writeHex(0x10077058, "02 02 02"); // Cure Moderate + Serious + Critical Wounds, Mass
	}
} spellHostilityFlagFix;


class SpellEnumExpansion : public TempleFix {
public:
	const char* name() override {
		return "Expand the range of usable spellEnums. Currently walled off at 802.";
	}

	void apply() override {
		// writeHex(0x100779DE + 2, "A0 0F"); // this prevents the crash from casting from scroll, but it fucks up normal spell casting... (can't go to radial menu to cast!)
	}
} spellEnumExpansionMod;


