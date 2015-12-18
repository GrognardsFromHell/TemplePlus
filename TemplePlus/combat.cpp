#include "stdafx.h"
#include "common.h"
#include "combat.h"
#include "tig/tig_mes.h"
#include "obj.h"
#include "critter.h"
#include "temple_functions.h"
#include "party.h"
#include "location.h"
#include "bonus.h"
#include "history.h"
#include "ai.h"
#include "util/fixes.h"
#include "weapon.h"


class CombatSystemReplacements : public TempleFix
{
public:
	const char* name() override {
		return "Combat System Replacements";
	}

	static objHndl CheckRangedWeaponAmmo(objHndl obj);
	static objHndl(__cdecl *orgCheckRangedWeaponAmmo)(objHndl obj);
	void apply() override{
		
		replaceFunction(0x100628D0, _isCombatActive);
		replaceFunction(0x100629B0, _IsCloseToParty);
		orgCheckRangedWeaponAmmo = replaceFunction(0x100654E0, CheckRangedWeaponAmmo);
		replaceFunction(0x100B4B30, _GetCombatMesLine);
	}
} combatSysReplacements;

objHndl(__cdecl* CombatSystemReplacements::orgCheckRangedWeaponAmmo)(objHndl obj);

objHndl CombatSystemReplacements::CheckRangedWeaponAmmo(objHndl obj)
{
	objHndl result = orgCheckRangedWeaponAmmo(obj);
	/*if (result)
	{
		auto secondaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
		auto primaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
		auto ammoItem = critterSys.GetWornItem(obj, EquipSlot::Ammo);
		int dummy = 1;
	}*/
	return result;
}

struct CombatSystemAddresses : temple::AddressTable
{
	int(__cdecl* GetEnemiesCanMelee)(objHndl obj, objHndl* canMeleeList);
	CombatSystemAddresses()
	{
		rebase(GetEnemiesCanMelee, 0x100B90C0);
	}
} addresses;

#pragma region Combat System Implementation
CombatSystem combatSys;

char * CombatSystem::GetCombatMesLine(int line)
{
	MesLine mesLine;
	MesHandle combatMes = *combatSys.combatMesfileIdx;
	if (line > 197 && line < 300
		|| line > 5104 && line < 6000
		|| line > 10104)
		combatMes = combatSys.combatMesNew;

	mesLine.key = line;
	if (!mesFuncs.GetLine(combatMes, &mesLine))
	{
		mesFuncs.GetLine_Safe(combatSys.combatMesNew, &mesLine);
		return (char*)mesLine.value;
	}
	mesFuncs.GetLine_Safe(combatMes, &mesLine);
	return (char*)mesLine.value;
}

int CombatSystem::IsWithinReach(objHndl attacker, objHndl target)
{
	float reach = critterSys.GetReach(attacker, D20A_UNSPECIFIED_ATTACK);
	float distTo = locSys.DistanceToObj(attacker, target);
	return distTo < reach;
}

BOOL CombatSystem::CanMeleeTargetAtLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* loc)
{
	if (weapon)
	{
		if (objects.GetType(weapon) != obj_t_weapon)
			return 0;
		if (inventory.IsRangedWeapon(weapon))
			return 0;
	} else
	{
		CritterFlag critterFlags = critterSys.GetCritterFlags(obj);
		if ((critterFlags & OCF_MONSTER) == 0 && !feats.HasFeatCountByClass(obj, FEAT_IMPROVED_UNARMED_STRIKE))
			return 0;
	}
	float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
	auto distToLoc = max((float)0.0,locSys.DistanceToLocFeet(obj, loc));
	if (locSys.InchesToFeet(objects.GetRadius(target)) + objReach < distToLoc)
		return 0;
	return 1;
}

BOOL CombatSystem::CanMeleeTargetAtLoc(objHndl obj, objHndl target, LocAndOffsets* loc)
{
	objHndl weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!combatSys.CanMeleeTargetAtLocRegardItem(obj, weapon, target, loc))
	{
		objHndl secondaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
		if (!combatSys.CanMeleeTargetAtLocRegardItem(obj, secondaryWeapon, target, loc))
		{
			if (!objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, 0))
				return 0;
			float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
			float tgtRadius = objects.GetRadius(target) / 12.0;
			if (max(static_cast<float>(0.0),
				locSys.DistanceToLocFeet(obj, loc)) - tgtRadius > objReach)
				return 0;
		}
	}
	return 1;
}

BOOL CombatSystem::CanMeleeTarget(objHndl obj, objHndl target)
{
	if (objects.GetFlags(obj) & (OF_OFF | OF_DESTROYED))
		return 0;
	auto targetObjFlags = objects.GetFlags(target);
	if (targetObjFlags & (OF_OFF | OF_DESTROYED | OF_INVULNERABLE))
		return 0;
	if (objects.IsUnconscious(obj))
		return 0;
	if (d20Sys.d20QueryWithData(target, DK_QUE_Critter_Has_Spell_Active, 407, 0)) // spell_sanctuary
		return 0;
	if (d20Sys.d20QueryWithData(obj, DK_QUE_Critter_Has_Spell_Active, 407, 0)) // presumably so the AI doesn't break its sanctuary protection?
		return 0;
	auto weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (CanMeleeTargetRegardWeapon(obj, weapon, target))
		return 1;
	auto offhandWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	if (CanMeleeTargetRegardWeapon(obj, offhandWeapon, target))
		return 1;
	if (!objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, 0))
		return 0;
	auto objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
	if (objReach > max(static_cast<float>(0.0), locSys.DistanceToObj(obj, target)))
		return 1;
	return 0;
}

BOOL CombatSystem::CanMeleeTargetRegardWeapon(objHndl obj, objHndl weapon, objHndl target)
{
	if (weapon)
	{
		if (objects.GetType(obj) != obj_t_weapon)
			return 0;
		if (inventory.IsRangedWeapon(weapon))
			return 0;
	}
	else
	{
		CritterFlag critterFlags = critterSys.GetCritterFlags(obj);
		if ((critterFlags & OCF_MONSTER) == 0 && !feats.HasFeatCountByClass(obj, FEAT_IMPROVED_UNARMED_STRIKE))
			return 0;
	}
	float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
	auto distToTgt = max((float)0.0, locSys.DistanceToObj(obj, target));
	if ( objReach <= distToTgt)
		return 0;
	return 1;
}

BOOL CombatSystem::AffiliationSame(objHndl obj, objHndl obj2)
{
	int objInParty = party.IsInParty(obj);
	return party.IsInParty(obj2) == objInParty;
}

int CombatSystem::GetThreateningCrittersAtLoc(objHndl obj, LocAndOffsets* loc, objHndl threateners[40])
{
	int n = 0;
	int initListLength = GetInitiativeListLength();
	for (int i = 0; i < initListLength; i++)
	{
		objHndl combatant = GetInitiativeListMember(i);
		if (combatant != obj && !AffiliationSame(obj, combatant))
		{
			if (combatSys.CanMeleeTargetAtLoc(combatant, obj, loc))
			{
				threateners[n++] = combatant;
				if (n == 40)
					return n;
			}
		}
	}
	return n;
}

objHndl CombatSystem::CheckRangedWeaponAmmo(objHndl obj)
{
	if (d20Sys.d20Query(obj, DK_QUE_Polymorphed))
		return 0;
	auto objType = objects.GetType(obj);
	auto invenNumField = obj_f_critter_inventory_num;
	auto invenField = obj_f_critter_inventory_list_idx;
	if (objType == obj_t_container)
	{
		invenField = obj_f_container_inventory_list_idx;
		invenNumField = obj_f_container_inventory_num;
		logger->warn("Check ammo for a container???");
	}
	int numItems = objects.getInt32(obj, invenNumField);
	if (numItems <= 0)
		return 0i64;
	auto ammoItem = critterSys.GetWornItem(obj, EquipSlot::Ammo);
	auto weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!weapon || !weapons.AmmoMatchesWeapon(weapon, ammoItem))
		weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	
	if (!weapon || !weapons.AmmoMatchesWeapon(weapon, ammoItem))
		return 0;
	return ammoItem;
	
}

bool CombatSystem::AmmoMatchesItemAtSlot(objHndl obj, EquipSlot equipSlot)
{
	objHndl ammoItem = CheckRangedWeaponAmmo(obj);
	auto weapon = critterSys.GetWornItem(obj, equipSlot);
	if (!weapon)
		return 0;
	return weapons.AmmoMatchesWeapon(weapon, ammoItem);
}

objHndl * CombatSystem::GetHostileCombatantList(objHndl obj, int * count)
{
	int initListLen = GetInitiativeListLength();
	objHndl hostileTempList[100];
	int hostileCount = 0;
	for (int i = 0; i < initListLen; i++)
	{
		auto combatant = GetInitiativeListMember(i);
		if (obj != combatant && !critterSys.IsFriendly(obj,combatant))
		{
			hostileTempList[hostileCount++] = combatant;
		}
	}
	objHndl *result = new objHndl[hostileCount];
	memcpy(result, hostileTempList, hostileCount * sizeof(objHndl));
	*count = hostileCount;
	return result;
}

bool CombatSystem::isCombatActive()
{
	return *combatSys.combatModeActive != 0;
}

uint32_t CombatSystem::IsCloseToParty(objHndl objHnd)
{
	if (critterSys.IsPC(objHnd))  return 1;
	for (uint32_t i = 0; i < party.GroupListGetLen(); i++)
	{
		auto dude = party.GroupListGetMemberN(i);
		if (locSys.DistanceToObj(objHnd, dude) < COMBAT_ACTIVATION_DISTANCE)
			return 1;
	}
	return 0;
}

int CombatSystem::GetEnemiesCanMelee(objHndl obj, objHndl* canMeleeList)
{
	return addresses.GetEnemiesCanMelee(obj, canMeleeList);
}

objHndl CombatSystem::GetWeapon(AttackPacket* attackPacket)
{
	D20CAF flags = attackPacket->flags;
	objHndl result = 0i64;

	if (!(flags & D20CAF_TOUCH_ATTACK) || flags & D20CAF_THROWN_GRENADE )
		result = attackPacket->weaponUsed;
	return result;
}

bool CombatSystem::DisarmCheck(objHndl attacker, objHndl defender, D20Actn* d20a)
{
	objHndl attackerWeapon = inventory.ItemWornAt(attacker, 3);
	if (!attackerWeapon)
		attackerWeapon = inventory.ItemWornAt(attacker, 4);
	objHndl defenderWeapon = inventory.ItemWornAt(defender, 3);
	if (!defenderWeapon)
		defenderWeapon = inventory.ItemWornAt(defender, 4);
	int attackerRoll = templeFuncs.diceRoll(1, 20, 0);
	int attackerSize = dispatch.DispatchGetSizeCategory(attacker);
	BonusList atkBonlist;
	DispIoAttackBonus dispIoAtkBonus;
	if (feats.HasFeatCountByClass(attacker, FEAT_IMPROVED_DISARM))
	{
		char * featName = feats.GetFeatName(FEAT_IMPROVED_DISARM);
		bonusSys.bonusAddToBonusListWithDescr(&dispIoAtkBonus.bonlist, 4, 0, 114, featName); // Feat Improved Disarm
	}
	bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, (attackerSize - 5 )* 4,0, 316);
	if (attackerWeapon)
	{
		int attackerWieldType = inventory.GetWieldType(attacker, attackerWeapon);
		if (attackerWieldType == 0)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, -4, 0, 340); // Light Weapon
		else if (attackerWieldType == 2)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, 4, 0, 341); // Two Handed Weapon
		int weaponType = objects.getInt32(attackerWeapon, obj_f_weapon_type);
		if (weaponType == wt_spike_chain || weaponType == wt_nunchaku || weaponType == wt_light_flail || weaponType == wt_heavy_flail || weaponType == wt_dire_flail || weaponType == wt_ranseur || weaponType == wt_halfling_nunchaku)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, 2, 0, 343); // Weapon Special Bonus
	} else
	{
		if (!feats.HasFeatCountByClass(attacker, FEAT_IMPROVED_UNARMED_STRIKE))
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, -4, 0, 342); // Disarming While Unarmed
		else
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, -4, 0, 340); // Light Weapon
	}
	
		

	dispIoAtkBonus.attackPacket.weaponUsed = attackerWeapon;
	dispatch.DispatchAttackBonus(attacker, defender, &dispIoAtkBonus, dispType72, 0); // buckler penalty
	dispatch.DispatchAttackBonus(attacker, 0, &dispIoAtkBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int atkToHitBonus = dispatch.DispatchAttackBonus(defender, 0, &dispIoAtkBonus, dispTypeToHitBonusFromDefenderCondition, 0); 
	int attackerResult = attackerRoll + bonusSys.getOverallBonus(&dispIoAtkBonus.bonlist);

	int defenderRoll = templeFuncs.diceRoll(1, 20, 0);
	int defenderSize = dispatch.DispatchGetSizeCategory(defender);
	BonusList defBonlist;
	DispIoAttackBonus dispIoDefBonus;
	bonusSys.bonusAddToBonusList(&dispIoDefBonus.bonlist, (defenderSize - 5) * 4, 0, 316);
	if (defenderWeapon)
	{
		int wieldType = inventory.GetWieldType(defender, defenderWeapon);
		if (wieldType == 0)
			bonusSys.bonusAddToBonusList(&dispIoDefBonus.bonlist, -4, 0, 340); // Light Off-hand Weapon
		else if (wieldType == 2)
			bonusSys.bonusAddToBonusList(&dispIoDefBonus.bonlist, 4, 0, 341); // Two Handed Weapon
		int weaponType = objects.getInt32(defenderWeapon, obj_f_weapon_type);
		if (weaponType == wt_spike_chain || weaponType == wt_nunchaku || weaponType == wt_light_flail || weaponType == wt_heavy_flail || weaponType == wt_dire_flail || weaponType == wt_ranseur || weaponType == wt_halfling_nunchaku)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, 2, 0, 343); // Weapon Special Bonus
	}
	
	dispIoDefBonus.attackPacket.weaponUsed = attackerWeapon;
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispType72, 0); // buckler penalty
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int defToHitBonus = dispatch.DispatchAttackBonus(attacker, 0, &dispIoDefBonus, dispTypeToHitBonusFromDefenderCondition, 0);
	int defenderResult = defenderRoll + bonusSys.getOverallBonus(&dispIoDefBonus.bonlist);

	bool attackerSucceeded = attackerResult > defenderResult;
	int rollHistId = histSys.RollHistoryAddType6OpposedCheck(attacker, defender, attackerRoll, defenderRoll, &dispIoAtkBonus.bonlist, &dispIoDefBonus.bonlist, 5109, 144 - attackerSucceeded, 1);
	histSys.CreateRollHistoryString(rollHistId);
	
	return attackerSucceeded;
}

bool CombatSystem::SunderCheck(objHndl attacker, objHndl defender, D20Actn* d20a)
{
	objHndl attackerWeapon = inventory.ItemWornAt(attacker, 3);
	if (!attackerWeapon)
		attackerWeapon = inventory.ItemWornAt(attacker, 4);
	objHndl defenderWeapon = inventory.ItemWornAt(defender, 3);
	if (!defenderWeapon)
		defenderWeapon = inventory.ItemWornAt(defender, 4);
	int attackerRoll = templeFuncs.diceRoll(1, 20, 0);
	int attackerSize = dispatch.DispatchGetSizeCategory(attacker);
	BonusList atkBonlist;
	DispIoAttackBonus dispIoAtkBonus;
	if (feats.HasFeatCountByClass(attacker, FEAT_IMPROVED_SUNDER))
	{
		char * featName = feats.GetFeatName(FEAT_IMPROVED_SUNDER);
		bonusSys.bonusAddToBonusListWithDescr(&dispIoAtkBonus.bonlist, 4, 0, 114, featName); // Feat Improved Sunder
	}
	bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, (attackerSize - 5) * 4, 0, 316);
	if (attackerWeapon)
	{
		int attackerWieldType = inventory.GetWieldType(attacker, attackerWeapon);
		if (attackerWieldType == 0)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, -4, 0, 340); // Light Weapon
		else if (attackerWieldType == 2)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, 4, 0, 341); // Two Handed Weapon
	}
	else
	{
		bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, -4, 0, 342); // Disarming While Unarmed
	}



	dispIoAtkBonus.attackPacket.weaponUsed = attackerWeapon;
	dispatch.DispatchAttackBonus(attacker, defender, &dispIoAtkBonus, dispType72, 0); // buckler penalty
	dispatch.DispatchAttackBonus(attacker, 0, &dispIoAtkBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int atkToHitBonus = dispatch.DispatchAttackBonus(defender, 0, &dispIoAtkBonus, dispTypeToHitBonusFromDefenderCondition, 0);
	int attackerResult = attackerRoll + bonusSys.getOverallBonus(&dispIoAtkBonus.bonlist);

	int defenderRoll = templeFuncs.diceRoll(1, 20, 0);
	int defenderSize = dispatch.DispatchGetSizeCategory(defender);
	BonusList defBonlist;
	DispIoAttackBonus dispIoDefBonus;
	bonusSys.bonusAddToBonusList(&dispIoDefBonus.bonlist, (defenderSize - 5) * 4, 0, 316);
	if (defenderWeapon)
	{
		int wieldType = inventory.GetWieldType(defender, defenderWeapon);
		if (wieldType == 0)
			bonusSys.bonusAddToBonusList(&dispIoDefBonus.bonlist, -4, 0, 340); // Light Off-hand Weapon
		else if (wieldType == 2)
			bonusSys.bonusAddToBonusList(&dispIoDefBonus.bonlist, 4, 0, 341); // Two Handed Weapon
	}

	dispIoDefBonus.attackPacket.weaponUsed = attackerWeapon;
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispType72, 0); // buckler penalty
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int defToHitBonus = dispatch.DispatchAttackBonus(attacker, 0, &dispIoDefBonus, dispTypeToHitBonusFromDefenderCondition, 0);
	int defenderResult = defenderRoll + bonusSys.getOverallBonus(&dispIoDefBonus.bonlist);

	bool attackerSucceeded = attackerResult > defenderResult;
	int rollHistId = histSys.RollHistoryAddType6OpposedCheck(attacker, defender, attackerRoll, defenderRoll, &dispIoAtkBonus.bonlist, &dispIoDefBonus.bonlist, 5109, 144 - attackerSucceeded, 1);
	histSys.CreateRollHistoryString(rollHistId);

	return attackerSucceeded;
}

int CombatSystem::GetClosestEnemy(objHndl obj, LocAndOffsets* locOut, objHndl* objOut, float* distOut, int flags)
{
	return _GetClosestEnemy(obj, locOut, objOut, distOut, flags);
}

int CombatSystem::GetInitiativeListLength()
{
	return _GetInitiativeListLength();
}

objHndl CombatSystem::GetInitiativeListMember(int n)
{
	return _GetInitiativeListMember(n);
}

int CombatSystem::GetClosestEnemy(AiTactic* aiTac, int selectionType)
{
	objHndl performer = aiTac->performer; 
	objHndl combatant; 
	float weightedDist; 
	float dist; 
	float closestDist; 
	int customCondition;

	closestDist = 10000.0;
	int numComatants = GetInitiativeListLength();
	
	for (int i = 0; i < numComatants; i++)
	{
		combatant = GetInitiativeListMember(i);

		switch (selectionType)
		{
		case 1: // Coup De Grace
			customCondition = d20Sys.d20Query(combatant, DK_QUE_CoupDeGrace);
			break;
		default:
			customCondition = !objects.IsUnconscious(combatant);
		}

		if (combatant != performer
			&& !critterSys.IsFriendly(performer, combatant)
			&& customCondition)
		{
			dist = locSys.DistanceToObj(performer, combatant);
			if (!d20Sys.d20Query(combatant, DK_QUE_Critter_Is_Invisible)
				|| d20Sys.d20Query(performer, DK_QUE_Critter_Can_See_Invisible))
				weightedDist = dist;
			else
				weightedDist = (dist + (float)5.0) * (float)2.5;
			if (weightedDist < closestDist)
			{
				closestDist = weightedDist;
				aiTac->target = combatant;
			}
		}
	}
	
	return 0;
}

void CombatSystem::enterCombat(objHndl objHnd)
{
	_enterCombat(objHnd);
}
#pragma endregion


uint32_t _isCombatActive()
{
	return *combatSys.combatModeActive;
}

uint32_t _IsCloseToParty(objHndl objHnd)
{
	return combatSys.IsCloseToParty(objHnd);
}

char* _GetCombatMesLine(int line)
{
	return combatSys.GetCombatMesLine(line);
}

uint32_t Combat_GetMesfileIdx_CombatMes()
{
	return *combatSys.combatMesfileIdx;
}