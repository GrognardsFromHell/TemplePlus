
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "spell.h"
#include "obj.h"
#include "tig/tig_mes.h"


static_assert(sizeof(SpellStoreData) == (32U), "SpellStoreData structure has the wrong size!");



SpontCastSpellLists spontCastSpellLists;

//GlobalPrimitive<uint16_t>
//1028D09C

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


class SpellFuncReplacements : public TempleFix {
public:
	const char* name() override {
		return "Expand the range of usable spellEnums. Currently walled off at 802.";
	}

	void apply() override {
		// writeHex(0x100779DE + 2, "A0 0F"); // this prevents the crash from casting from scroll, but it fucks up normal spell casting... (can't go to radial menu to cast!)
		replaceFunction(0x100FDEA0, _getWizSchool);
		macReplaceFun(100779A0, _getSpellEnum)
	}
} spellFuncReplacements;

#pragma region Spell System Implementation

SpellSystem spellSys;

uint32_t SpellSystem::getBaseSpellCountByClassLvl(uint32_t classCode, uint32_t classLvl, uint32_t slotLvl, uint32_t unknown1)
{
	__asm{
		// ecx - classLvl
		// eax - slotLvl
		// edx - unknown?
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, [ecx]._getSpellCountByClassLvl;
		mov eax, classCode;
		push eax;
		mov eax, slotLvl;
		mov ecx, classLvl;
		mov edx , unknown1
		call esi;
		add esp, 4;
		pop ecx;
		pop esi;
	}
}

uint32_t SpellSystem::getWizSchool(objHndl objHnd)
{
	return ( objects.getInt32(objHnd, obj_f_critter_school_specialization) & 0x000000FF );
}

uint32_t SpellSystem::getStatModBonusSpellCount(objHndl objHnd, uint32_t classCode, uint32_t slotLvl)
{
	uint32_t objHndLSB = (uint32_t)objHnd;
	uint32_t objHndMSB = (uint32_t)(objHnd >> 32);
	uint32_t result = 0;
	__asm{
		// esi - slotLvl
		// eax - classCode
		push edi;
		push edx;
		push esi;
		push ecx;
		mov ecx, this;
		mov edi, [ecx]._getStatModBonusSpellCount;
		mov eax, objHndMSB;
		mov edx, objHndLSB;
		push eax;
		push edx;
		mov eax, classCode;
		mov esi, slotLvl;
		call edi;
		add esp, 8;
		mov result, eax;
		pop edi;
		pop edx;
		pop esi;
		pop ecx;
	}
	return result;
}

void SpellSystem::spellPacketBodyReset(SpellPacketBody* spellPktBody)
{
	_spellPacketBodyReset(spellPktBody);
}

void SpellSystem::spellPacketSetCasterLevel(SpellPacketBody* spellPktBody)
{
	_spellPacketSetCasterLevel(spellPktBody);
}

uint32_t SpellSystem::getSpellEnum(const char* spellName)
{
	MesLine mesLine;
	for (auto i = 0; i < SPELL_ENUM_MAX; i++)
	{
		mesFuncs.GetLine_Safe(*spellEnumMesHandle, &mesLine);
		if (!_stricmp(spellName, mesLine.value))
			return i;
	}
	return 0;
}
#pragma endregion

#pragma region Spontaneous Summon Hooks

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
	return spontCastSpellLists.spontCastSpellsDruidSummons[spellSlotLevel];
}

uint32_t _DruidRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel)
{
	return spontCastSpellLists.spontCastSpellsDruid[spellSlotLevel];
}

uint32_t _GoodClericRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel)
{
	return spontCastSpellLists.spontCastSpellsGoodCleric[spellSlotLevel];
}

uint32_t _EvilClericRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel)
{
	return spontCastSpellLists.spontCastSpellsEvilCleric[spellSlotLevel];
}



#pragma endregion



uint32_t __cdecl _getWizSchool(objHndl objHnd)
{
	return spellSys.getWizSchool(objHnd);
}

uint32_t __cdecl _getSpellEnum(const char* spellName)
{
	return spellSys.getSpellEnum(spellName);
}




