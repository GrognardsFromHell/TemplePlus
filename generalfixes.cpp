
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "obj.h"
#include "turn_based.h"
#include "temple_functions.h"


class SizeColossalFix : public TempleFix {
public:
	const char* name() override {
		return "fixes size_colossal (was misspelled as size_clossal)";
	}

	void apply() override {
		writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");
	}
} sizeColossalFix;

class KukriFix : public TempleFix {
public:
	const char* name() override {
		return "Makes Kukri Proficiency Martial";
	}

	void apply() override {
		writeHex(0x102BFD78 + 30*4, "00 09"); // marks Kukri as Martial in the sense that picking "Martial Weapons Proficiency" will now list Kukri
		// see rest of fix in weapon.cpp IsMartialWeapon
	}
} kukriFix;


objHndl __cdecl ItemWornAtModifiedForTumlbeCheck(objHndl objHnd, uint32_t itemWornSlot)
{
	if (objects.GetRace(objHnd) == race_dwarf)
	{
		return 0;
	}
	else
	{
		return objects.inventory.ItemWornAt(objHnd, itemWornSlot);
	}
}


class DwarfTumbleFix : public TempleFix
{
public:
	const char* name() override {
		return "Allows Dwarves to tumble in heavy armor";
	}

	void apply() override {
		redirectCall(0x1008AB49, ItemWornAtModifiedForTumlbeCheck);
	}

	//
} dwarfTumbleFix;


class SpellSlingerGeneralFixes : public TempleFix
{
public:
	const char* name() override{
		return "Putting SpellSlinger's small fixes concentrated here :)";
	}

	void apply() override{
		// breakfree_on_entanglement_fix.txt
		writeHex(0x100D4259, "90 90 90 90  90 90");
		

		// caravan_encounter_fix.txt // looks like it has been updated since then too! 13D6 instead of 13D5
		writeHex(0x1006ED1E, "04");

		writeHex(0x1006FE1B + 1, "D6 13");
		writeHex(0x1007173C + 1, "D6 13");
		writeHex(0x1012BDF2 + 1, "D6 13");
		writeHex(0x1012C1EC + 1, "D6 13");
		writeHex(0x1012C361 + 1, "D6 13");


		// D20STDF_fix.txt

		writeHex(0x100B8454, "BA 00000000");
		writeHex(0x100B8471     , "8D4A 0D");


		// heavy_armor_prof_fix.txt

		writeHex(0x1007C49F, "6A 06");

		// NPC_usePotion_AoO_fix.txt

		writeHex(0x10098DE7+6, "44000000");

		// rep fallen paladin fix

		writeHex(0x1005481A, "00");

		// rgr_fav_enemy_fix.txt

		writeHex(0x101AD9BE, "90 90");

		// sp125_discern_lies_fix.txt // looks like this was actually forgotten to be implemented in the Co8 DLL
		writeHex(0x102D5454, "1E 00 00 00  22 00 00 00  00 2B 0D 10   00 00 00 00");
	}
};



static uint32_t (__cdecl *OrgFragarachAnswering)(DispatcherCallbackArgs);

uint32_t __cdecl HookedFragarachAnswering(DispatcherCallbackArgs args) {
	// checks if the current TB actor is the same as the "attachee" (critter taking damage)
	// if so, aborts the answering (you can have an AoO on your turn!)
	auto dispIO = args.dispIO;
	auto curActor = tbSys.turnBasedGetCurrentActor();
	auto attachee = args.objHndCaller;
	auto tgtObj = *(objHndl*)(dispIO + 2);
	//hooked_print_debug_message("Prevented Scather AoO bug! TB Actor is %s , Attachee is %s,  target is %s", description.getDisplayName(curActor), description.getDisplayName(attachee), description.getDisplayName(tgtObj));
	if (!tgtObj || !objects.IsCritter(tgtObj) || curActor == attachee)
	{
		hooked_print_debug_message("Prevented Scather AoO bug! TB Actor is %s , Attachee is %s,  target is %s", description.getDisplayName(curActor), description.getDisplayName(attachee), description.getDisplayName(tgtObj) );
		return 0;
	}
	auto result = OrgFragarachAnswering(args); // Call original method
	return result;
}

class FragarachAoOFix : public TempleFix
{
public:
	const char* name() override {
		return "Fixes the Fragarach hang that is caused by attacking a fire creature (which deals damage to the caster -> triggers the answering ability - > attempts an AoO -> but there is no one to AoO!)";
	}

	void apply() override {
		OrgFragarachAnswering = (uint32_t(__cdecl*)(DispatcherCallbackArgs)) replaceFunction(0x10104330, HookedFragarachAnswering);

	}
} fragarachAoOFix;