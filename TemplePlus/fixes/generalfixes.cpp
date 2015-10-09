
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "obj.h"
#include "turn_based.h"
#include "critter.h"
#include "condition.h"
#include "bonus.h"


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
	if (critterSys.GetRace(objHnd) == race_dwarf)
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



class DwarfEncumbranceFix: public TempleFix
{
public:
	const char* name() override {
		return "Prevents medium / heavy encumbrance from affecting dwarves";
	}
	static int EncumberedMoveSpeedCallback(DispatcherCallbackArgs args);
	void apply() override {
		replaceFunction(0x100EBAA0, EncumberedMoveSpeedCallback);
	}
} dwarfEncumbranceFix;

int DwarfEncumbranceFix::EncumberedMoveSpeedCallback(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIOCheckIoType13(args.dispIO);
	if ( dispIo->bonlist->bonFlags  == 3) //  in case the cap has already been set (e.g. by web/entangle) - recreating the spellslinger fix
		return 0;
	if (args.subDispNode->subDispDef->data2 == 324) // overburdened
	{
		bonusSys.bonusSetOverallCap(5, dispIo->bonlist, 5, 0, 324, 0);
		bonusSys.bonusSetOverallCap(6, dispIo->bonlist, 5, 0, 324, 0);
		return 0;
	} 
	
	if (critterSys.GetRace(args.objHndCaller) == Race::race_dwarf) // dwarves do not suffer movement penalty for meidum/heavy encumbrance
		return 0;

	if (dispIo->bonlist->bonusEntries[0].bonValue <= 20) // this is probably the explicit form for base speed...
	{
		bonusSys.bonusAddToBonusList(dispIo->bonlist, -5, 0, args.subDispNode->subDispDef->data2);
	} else
	{
		bonusSys.bonusAddToBonusList(dispIo->bonlist, -10, 0, args.subDispNode->subDispDef->data2);
	}
	
	return 0;
}

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
		logger->info("Prevented Scather AoO bug! TB Actor is {}, Attachee is {},  target is {}", 
			description.getDisplayName(curActor), description.getDisplayName(attachee), description.getDisplayName(tgtObj) );
		return 0;
	}


	/*
	disable AoO effect for other identical conditions (so you don't get the 2 AoO Hang) // TODO: Makes this work like Great Cleave for maximum munchkinism
	*/
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0) == 1)
	{
		auto dispatcher = objects.GetDispatcher(args.objHndCaller);
		auto cond = &dispatcher->itemConds;
		while (*cond)
		{
			if ((*cond)->condStruct->condName == args.subDispNode->condNode->condStruct->condName
				&& (*cond) != args.subDispNode->condNode)
			{
				(*cond)->args[0] = 0;
			}
			cond = &(*cond)->nextCondNode;
		}
		auto result = OrgFragarachAnswering(args); // Call original method
		return result;
	}
	
	return 0;
	
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



uint32_t GetCourageBonus(objHndl objHnd)
{
	auto bardLvl = (int32_t)objects.StatLevelGet(objHnd, stat_level_bard);
	if (bardLvl < 8) return 1;
	if (bardLvl < 14) return 2;
	if (bardLvl < 20) return 3;
	return 4;
};

uint32_t __cdecl BardicInspiredCourageInitArgs(DispatcherCallbackArgs args)
{
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, 5);
	conds.CondNodeSetArg(args.subDispNode->condNode, 1, 0);
	auto courageBonus = 1;
	if ( objects.IsCritter(args.objHndCaller) )
	{
		courageBonus = GetCourageBonus(args.objHndCaller);
	}
	else
	{
		logger->info("Bardic Inspired Courage dispatched from non-critter! Mon seigneur {}", 
			description.getDisplayName(args.objHndCaller));
	}
	conds.CondNodeSetArg(args.subDispNode->condNode, 3, courageBonus);
	return 0;
};

class BardicInspireCourageFix : public TempleFix
{
	public: const char* name() override {
		return "Bardic Inspire Courage Function Replacements";
	};
	void apply() override
	{
		replaceFunction(0x100EA5C0, BardicInspiredCourageInitArgs);
	}
} bardicInspireCourageFix;


class SorcererFailureDoubleChargeFix : public TempleFix
{
public: const char* name() override {
	return "Sorcerer Spell Failure Double Debit Fix";
};
		void apply() override
		{
			writeHex(0x1008D80E, "90 90 90 90 90");
		}
} sorcererSpellFailureFix;


class CripplingStrikeFix : public TempleFix
{
	// fixes Str damage on crippling strike (should be 2 instead of 1)
public: const char* name() override {
	return "Crippling Strike Fix";
};
		void apply() override
		{
			writeHex(0x100F9B70, "6A 02");
		}
} cripplingStrikeFix;



class TestImprovedTWF : public TempleFix
{
public: const char* name() override {
	return "TWF TEst";
};
		void apply() override
		{
		//	replaceFunction(0x100FD1C0, sub_100FD1C0);
		}
} testImprovedTWF;