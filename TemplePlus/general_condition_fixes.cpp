#include "stdafx.h"
#include "util/fixes.h"
#include "dispatcher.h"
#include "condition.h"

#include "gamesystems/objects/objsystem.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "ui/ui_char.h"
#include <critter.h>
#include <combat.h>
#include <history.h>
#include <d20_level.h>
#include <damage.h>

#define CONDFIX(fname) static int fname ## (DispatcherCallbackArgs args);
#define HOOK_ORG(fname) static int (__cdecl* org ##fname)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>

class GeneralConditionFixes : public TempleFix {
public:

	static int WeaponKeenQuery(DispatcherCallbackArgs args);
	static int TempNegativeLevelOnAdd(DispatcherCallbackArgs args); // fixes critters dying due to neg HP
	static int TempNegativeLevelNewday(DispatcherCallbackArgs args);
	static int PermanentNegativeLevelOnAdd(DispatcherCallbackArgs args); // fixes critters dying due to neg HP
	
	static int WeaponKeenCritHitRange(DispatcherCallbackArgs args);
	static int ImprovedCriticalGetCritThreatRange(DispatcherCallbackArgs args);
	
	static int ArmorCheckSkillPenalty(DispatcherCallbackArgs args);
	static int EncumbranceSkillPenalty(DispatcherCallbackArgs args);
	static int RacialAbilityAdjustment(DispatcherCallbackArgs args);

	void apply() override {

		{ // Fix for duplicate Blackguard tooltip when fallen paladin
			SubDispDefNew sdd;
			sdd.dispKey = DK_NONE;
			sdd.dispType = dispTypeEffectTooltip;
			sdd.dispCallback = [](DispatcherCallbackArgs args) {
				
				GET_DISPIO(dispIOTypeEffectTooltip, DispIoEffectTooltip);

				if (objects.StatLevelGet(args.objHndCaller, stat_level_blackguard))
					return 0;
				dispIo->Append(args.GetData1(), -1, nullptr);
				return 0;
			};
			sdd.data1.sVal = 0xAF;
			write(0x102E7400, &sdd, sizeof(sdd));
		}
		{ // Fix for Negative Level (Temp/Negative) being removed on death
			SubDispDefNew sdd;
			sdd.dispKey = DK_SIG_Killed;
			sdd.dispType = dispTypeD20Signal;
			sdd.dispCallback = [](DispatcherCallbackArgs args) {
				// originally this removed the condition
				return 0;
			};
			write(0x102E7298, &sdd, sizeof(sdd)); // Temp Negative Level
			write(0x102E736C, &sdd, sizeof(sdd)); // Perm Negative Level
		}
		
		{ // Fix Shocking Burst, Icy Burst damage type
			int buff = 9;
			write(0x102F0D94, &buff, sizeof(buff));
			buff = 8;
			write(0x102F0CFC, &buff, sizeof(buff));
		}

		replaceFunction(0x100FF670, WeaponKeenQuery);
		replaceFunction(0x100EF540, TempNegativeLevelOnAdd); // fixes NPCs with no class levels instantly dying due to temp negative level
		replaceFunction(0x100EB620, TempNegativeLevelNewday); // fixes NPCs with no class levels instantly dying due to temp negative level
		replaceFunction(0x100EB6A0, PermanentNegativeLevelOnAdd); // fixes NPCs with no class levels instantly dying due to temp negative level
		replaceFunction(0x100FFD20, WeaponKeenCritHitRange); // fixes Weapon Keen stacking (Keen Edge spell / Keen enchantment)
		replaceFunction(0x100F8320, ImprovedCriticalGetCritThreatRange); // fixes stacking with Keen Edge spell / Keen enchantment
		replaceFunction(0x101005B0, ArmorCheckSkillPenalty);
		replaceFunction(0x100EBA70, EncumbranceSkillPenalty);
		


		replaceFunction(0x100FD850, RacialAbilityAdjustment);
		
		// Allow filtering out certain types of negative levels from the GetLevel query.
		static int (*origNegLvl)(DispatcherCallbackArgs) = replaceFunction<int(DispatcherCallbackArgs)>(0x100EF8B0, [](DispatcherCallbackArgs args)->int {
			GET_DISPIO(dispIoTypeObjBonus, DispIoObjBonus);
			auto condName = args.subDispNode->condNode->condStruct->condName;

			auto omit = static_cast<LevelDrainType>(dispIo->flags);

			auto mask = LevelDrainType::NegativeLevel;

			// Temp Negative Level and the various aligned equipment penalties count
			// as `NegativeLevel`. The only built-in condition that is not of this
			// type is Perm Negative Level.
			if (!_stricmp(condName, "Perm Negative Level")) {
				mask = LevelDrainType::DrainedLevel;
			}

			// flags indicate whether we should _skip_ a particular condition, so that the
			// existing default of 0 includes all adjustments.
			if ((omit & mask) == mask) return 0;

			return origNegLvl(args);
			});

		// Amulet of Natural Armor - bonus type changed to 10 so it doesn't stack with other enhancement bonuses (Barkskin, Righteous Might)
		replaceFunction<int(DispatcherCallbackArgs)>(0x10104AB0, [](DispatcherCallbackArgs args)->int {
			GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
			auto bonVal = args.GetCondArg(0);
			auto invIdx = args.GetCondArg(2);
			auto item = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
			
			auto itemName = description.getDisplayName(item, args.objHndCaller);
			if (itemName)
				dispIo->bonlist.AddBonusWithDesc(bonVal, 10, 112, itemName); // bonus type changed to 10
			else {
				dispIo->bonlist.AddBonus(bonVal, 10, 112); // bonus type changed to 10
			}
			
			return 0;
			});



	}
} genCondFixes;

int GeneralConditionFixes::WeaponKeenQuery(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
	dispIo->return_val = 2; // default
	
	auto itemHndl = uiSystems->GetChar().GetTooltipItem();
	if (!itemHndl || !objSystem->IsValidHandle(itemHndl))
		return 0;

	// meh, it doesn't have a handle
	/*auto invIdx = args.GetCondArg(2);
	auto itemHndl = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
	if (!itemHndl || !objSystem->IsValidHandle(itemHndl))
		return 0;
		*/
	auto item = objSystem->GetObject(itemHndl);
	
	auto critRange = item->GetInt32(obj_f_weapon_crit_range);
	dispIo->return_val = critRange;

	return 0;
}

int GeneralConditionFixes::TempNegativeLevelOnAdd(DispatcherCallbackArgs args)
{
	auto highestLvl = 0;
	auto highestClass = 0;

	if (args.GetData1() == 273) { // aligned weapon enchantment (Holy/Unholy/Axiomatic/Chaotic)
		auto alignmentMask = args.GetData2();
		auto critterAlignment = objects.StatLevelGetBase(args.objHndCaller, Stat::stat_alignment);
		if ((alignmentMask & critterAlignment) != alignmentMask)
			return 0;
	}

	critterSys.CritterHpChanged(args.objHndCaller, objHndl::null, -5);

	// fix for negative levels killing critters without class levels - 
	// instead of class levels, get the hit dice num (taking into account previous negative levels etc)
	//auto lvl = dispatch.Dispatch61GetLevel(args.objHndCaller, Stat::stat_level, nullptr, objHndl::null);
	auto hd = objects.GetHitDiceNum(args.objHndCaller, /*getBase=*/ false); 
	if (hd  <= 0) {
		critterSys.Kill(args.objHndCaller); // buuug
	}

	// extend this to new classes as well
	for (auto classId : d20ClassSys.classEnums) {
		auto classLvl = dispatch.Dispatch61GetLevel(args.objHndCaller, (Stat)classId);
		if (classLvl > highestLvl) {
			highestLvl = classLvl;
			highestClass = classId;
		}
	}

	args.SetCondArg(0, highestClass);
	critterSys.CritterHpChanged(args.objHndCaller, objHndl::null, 0); // hmmm?

	return 0;
}

int GeneralConditionFixes::TempNegativeLevelNewday(DispatcherCallbackArgs args)
{
	auto classCode = args.GetCondArg(0);
	auto saveThrowDc = args.GetCondArg(1);
	// New: if save throw DC is set to negative value, the save throw/perm level is skipped
	// Used for Enervation spell effect
	auto noPermEffect = saveThrowDc < 0; 

	auto isDead = critterSys.IsDeadNullDestroyed(args.objHndCaller);
	
	if (saveThrowDc == 0) {
		saveThrowDc = 14; // default value when DC not specified
	}

	if (noPermEffect || damage.SavingThrow(args.objHndCaller, objHndl::null, saveThrowDc, SavingThrowType::Fortitude, 0) ) {
		// The condition is going to get removed, so max HP is going up by 5
		// To prevent resurrection when critter is already dead, apply 5 HP damage
		// If critter is not dead, then it's fine
		if (isDead) {
			auto dmgNew = critterSys.GetHpDamage(args.objHndCaller) + 5;
			critterSys.SetHpDamage(args.objHndCaller, dmgNew);
		}
		args.RemoveCondition(); // fixed: not being removed on successful save
		if (!isDead) {
			critterSys.CritterHpChanged(args.objHndCaller, objHndl::null, 5); // 
		}
	}
	else {
		args.RemoveCondition();
		conds.AddTo(args.objHndCaller, "Perm Negative Level", { classCode, 0, 0 });
	}
	return 0;
}

int GeneralConditionFixes::PermanentNegativeLevelOnAdd(DispatcherCallbackArgs args)
{
	auto highestLvl = 0;
	auto highestClass = 0;
	auto effLv = critterSys.GetEffectiveDrainedLevel(args.objHndCaller);
	
	critterSys.CritterHpChanged(args.objHndCaller, objHndl::null, -5);

	// fix for negative levels killing critters without class levels - 
	// instead of class levels, get the hit dice num (taking into account previous negative levels etc)
	//auto lvl = dispatch.Dispatch61GetLevel(args.objHndCaller, Stat::stat_level, nullptr, objHndl::null);
	auto hd = objects.GetHitDiceNum(args.objHndCaller, /*getBase=*/ false);
	if (hd <= 0) {
		critterSys.Kill(args.objHndCaller);
	}
	else {
		auto newXp = d20LevelSys.GetPenaltyXPForDrainedLevel(effLv);

		// set negative XP
		objects.setInt32(args.objHndCaller, obj_f_critter_experience, newXp);
	}

	histSys.CreateRollHistoryLineFromMesfile(22, args.objHndCaller, objHndl::null); // [ACTOR] ~loses a level permanently~[TAG_LEVEL_LOSS]!
	combatSys.FloatCombatLine(args.objHndCaller, 126); //"Permanant Level Loss"

	args.SetCondArg(1, effLv+1);
	
	return 0;
}

int GeneralConditionFixes::WeaponKeenCritHitRange(DispatcherCallbackArgs args)
{
	auto bonValue = 1;
	auto invIdx = args.GetCondArg(2);

	auto itemHndl = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
	
	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	
	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	auto weapObj = objSystem->GetObject(weapUsed);
	if (!weapObj) {
		return 0;
	}
	
	auto weapFlags = weapObj->GetInt32(obj_f_weapon_flags);
	
	if ( itemHndl == weapUsed
		|| ( (weapFlags& WeaponFlags::OWF_RANGED_WEAPON) && inventory.ItemWornAt(args.objHndCaller, EquipSlot::Ammo) == weapUsed)) // keen arrows? shuriken?
	{
		bonValue = weapObj->GetInt32(obj_f_weapon_crit_range);
	}
	auto weaponName = description.getDisplayName(weapUsed, args.objHndCaller);
	dispIo->bonlist.AddBonusWithDesc(bonValue, 12, 246, weaponName); // fix: was untyped bonus, allowing stacking
	return 0;
}

int GeneralConditionFixes::ImprovedCriticalGetCritThreatRange(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);

	auto threatRangeSize = 1;
	auto featEnum = (feat_enums)args.GetCondArg(0);
	auto impCriticalWeapType = (WeaponTypes)args.GetCondArg(1);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	
	auto weapType = WeaponTypes::wt_unarmed_strike_medium_sized_being;
	if (weapUsed) {
		auto weapObj = objSystem->GetObject(weapUsed);
		if (!weapObj) {
			return 0;
		}
		weapType = (WeaponTypes)weapObj->GetInt32(obj_f_weapon_type);
		threatRangeSize = weapObj->GetInt32(obj_f_weapon_crit_range);
	}
	
	if (weapType == impCriticalWeapType) {
		auto featName = feats.GetFeatName(featEnum);
		
		dispIo->bonlist.AddBonusWithDesc(threatRangeSize, 12, 114, featName); // fix: was untyped bonus in vanilla, leading to stacking with Keen weapons
	}
	return 0;
}

// Replacement armor check skill penalty function to use an overlapping bonus
// type with the below encumbrance penalty. Must test for shields because those
// are supposed to stack.
int GeneralConditionFixes::ArmorCheckSkillPenalty(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIoTypeObjBonus, DispIoObjBonus);

	auto inv_idx = args.GetCondArg(2);
	auto armor = inventory.GetItemAtInvIdx(args.objHndCaller, inv_idx);
	auto penalty = GetArmorCheckPenalty(armor);
	auto name = description.getDisplayName(armor, args.objHndCaller);
	auto type = inventory.GetArmorType(armor) == ARMOR_TYPE_SHIELD ? 0 : 28;

	if (penalty < 0) {
		dispIo->bonOut->AddBonusWithDesc(penalty, type, 112, name);
	}

	return 0;
}

int GeneralConditionFixes::EncumbranceSkillPenalty(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIoTypeObjBonus, DispIoObjBonus);

	dispIo->bonOut->AddBonus(- args.GetData1(), 28, args.GetData2());

	return 0;
}

int GeneralConditionFixes::RacialAbilityAdjustment(DispatcherCallbackArgs args)
{
	auto key = args.dispKey;
	bool polymorphed = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
	bool physical = DK_STAT_STRENGTH <= key && key <= DK_STAT_CONSTITUTION;
	bool invalid = key < DK_STAT_STRENGTH || DK_STAT_CHARISMA < key;

	// polymorph overrides this adjustment
	if (polymorphed && physical || invalid) return 0;

	GET_DISPIO(dispIOTypeBonusList, DispIoBonusList);

	auto critter = objSystem->GetObject(args.objHndCaller);
	int amount = args.GetData1();

	// get the base value
	//
	// note: testing the base int32 makes this insensitive to order of applied
	// conditions, so it no longer matters if the racial condition is earlier than
	// any other penalties.
	int base = critter->GetInt32(obj_f_critter_abilities_idx, key-1);

	// racial penalties cannot take someone below 3 int, since that is the minimum
	// necessary for speech and such.
	if (base + amount < 3 && amount < 0 && key == DK_STAT_INTELLIGENCE) {
		// if the base is below 3 for some reason, don't give a bonus
		amount = std::min(0, 3 - base);
	}

	if (amount > 0) {
		dispIo->bonlist.AddBonus(amount, 0, 139);
	} else if (amount < 0) {
		// a negative isn't really a bonus, is it?
		std::string desc = "Racial Penalty";
		dispIo->bonlist.AddBonus(amount, 0, desc);
	}

	return 0;
}
