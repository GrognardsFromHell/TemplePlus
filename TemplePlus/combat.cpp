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
#include "float_line.h"
#include "pathfinding.h"
#include "action_sequence.h"
#include "turn_based.h"
#include "gametime.h"
#include "ui/ui_combat.h"
#include "raycast.h"
#include "gamesystems/objects/objsystem.h"
#include "python/python_integration_obj.h"
#include "gamesystems/gamesystems.h"
#include "maps.h"
#include "tutorial.h"
#include "objlist.h"
#include "ui/ui_dialog.h"
#include "condition.h"
#include "legacyscriptsystem.h"
#include "config/config.h"
#include "d20_obj_registry.h"
#include "animgoals/anim.h"


struct CombatSystemAddresses : temple::AddressTable
{
	void(__cdecl *AddToInitiative)(objHndl critter);

	int(__cdecl* GetEnemiesCanMelee)(objHndl obj, objHndl* canMeleeList);
	void(__cdecl*CombatTurnProcessAi)(objHndl obj);
	void(__cdecl*CombatTurnAdvance)(objHndl obj);
	BOOL (__cdecl*CheckFleeCombatMap)();
	int(__cdecl*GetFleecombatMap)(void*); // MapPacket
	int (*GetFleeStatus)();
	int* combatTimeEventIndicator;
	int * combatTimeEventSthg; 
		objHndl * combatActor; 
		int * combatRoundCount;
	ActionBar** barPkt;

	CombatSystemAddresses()
	{
		rebase(GetEnemiesCanMelee, 0x100B90C0);
		rebase(EndTurn, 0x100632B0);
		rebase(CombatPerformFleeCombat, 0x100633C0);
		rebase(CombatTurnAdvance, 0x100634E0);
		rebase(CombatTurnProcessAi, 0x100635E0);
		rebase(Subturn, 0x10063760);
		rebase(GetFleecombatMap, 0x1006F970);
		rebase(CheckFleeCombatMap, 0x1006F990);
		rebase(GetFleeStatus, 0x1006F9B0);

		rebase(barPkt, 0x10AA8404);
		rebase(combatRoundCount,0x10AA841C);
		rebase(combatActor, 0x10AA8438);
		rebase(combatTimeEventSthg, 0x10AA8440);
		rebase(combatTimeEventIndicator, 0x10AA8444);

		rebase(AddToInitiative, 0x100DF1E0);
	}

	void (*Subturn)();
	void (*EndTurn)();
	void (*CombatPerformFleeCombat)(objHndl obj);
} addresses;

class CombatSystemReplacements : public TempleFix
{
public:
	static objHndl CheckRangedWeaponAmmo(objHndl obj);
	static objHndl(__cdecl *orgCheckRangedWeaponAmmo)(objHndl obj);

	static void CombatTurnAdvance(objHndl obj);
	static void (__cdecl*orgCombatTurnAdvance)(objHndl obj);

	static void ActionBarResetCallback(int resetArg)
	{
		if (resetArg != 0)
		{
			int dumm = 1;
			// seems to be 0 always... might've been some debug thing
		}
		
		auto actor = tbSys.turnBasedGetCurrentActor();
		if (actor )
			if (objSystem->IsValidHandle(actor))
				logger->debug("Greybar Reset! Current actor: {} ({})", description.getDisplayName(actor), actor);
			else
			{
				logger->debug("Greybar Reset! Current actor is invalid handle.");
			}
				
		else
			logger->debug("Greybar Reset! Actor is null.");

		if (actor && objSystem->IsValidHandle(actor) && !objects.IsPlayerControlled(actor))
		{
			actSeqSys.GreybarReset();
		}
		// orgActionBarResetCallback();
	};
	static void (__cdecl*orgActionBarResetCallback)(int);


	static void TurnStart2(int idx)
	{
		combatSys.TurnStart2(idx);
	};
	static void(__cdecl*orgTurnStart2)(int);

	void apply() override{
		
		replaceFunction(0x100628D0, _isCombatActive);

		// TurnProcessAi
		replaceFunction<void(__cdecl)(objHndl)>(0x100635E0, [](objHndl obj){
			combatSys.TurnProcessAi(obj);
		});


		replaceFunction(0x100629B0, _IsCloseToParty);
		orgActionBarResetCallback = replaceFunction(0x10062E60, ActionBarResetCallback);
		orgTurnStart2 = replaceFunction(0x100638F0, TurnStart2);
		orgCombatTurnAdvance = replaceFunction(0x100634E0, CombatTurnAdvance);
		replaceFunction<BOOL(__cdecl)()>(0x10062A30, [](){ // Combat End
			return combatSys.CombatEnd()?TRUE:FALSE;
		});

		orgCheckRangedWeaponAmmo = replaceFunction(0x100654E0, CheckRangedWeaponAmmo);
		
		replaceFunction(0x100B4B30, _GetCombatMesLine);

		// replace the ToHitProcessing function
		replaceFunction<void(__cdecl)(D20Actn&)>(0x100B7160, [](D20Actn & d20a){
			combatSys.ToHitProcessing(d20a);
		});

		replaceFunction<BOOL(__cdecl)()>(0x100EBB90, []()->BOOL
		{
			return combatSys.IsBrawlInProgress();
		});

		// AddToInitiativeWithinRect
		replaceFunction<void(__cdecl)(objHndl)>(0x10062AC0, [](objHndl handle){
			combatSys.AddToInitiativeWithinRect(handle);
		});
			
	}
} combatSysReplacements;

objHndl(__cdecl* CombatSystemReplacements::orgCheckRangedWeaponAmmo)(objHndl obj);
void(__cdecl*CombatSystemReplacements::orgCombatTurnAdvance)(objHndl obj);
void(__cdecl*CombatSystemReplacements::orgActionBarResetCallback)(int);
void(__cdecl*CombatSystemReplacements::orgTurnStart2)(int);

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

void CombatSystemReplacements::CombatTurnAdvance(objHndl obj)
{
	combatSys.CombatAdvanceTurn(obj);

	//auto curSeq = *actSeqSys.actSeqCur;
	//orgCombatTurnAdvance(obj);
	//if (curSeq != *actSeqSys.actSeqCur)
	//{
	//	logger->debug("Combat Turn Advance changed sequence to {}", (void*)*actSeqSys.actSeqCur);
	//}
}



#pragma region Combat System Implementation
LegacyCombatSystem combatSys;

char * LegacyCombatSystem::GetCombatMesLine(int line) const
{
	MesLine mesLine;
	MesHandle combatMes = *combatSys.combatMesfileIdx;
	if (line > 197 && line < 300
		|| line > 5104 && line < 6000
		|| line > 6016 && line < 7000
		|| line > 10104)
		combatMes = combatMesNew;

	mesLine.key = line;
	if (!mesFuncs.GetLine(combatMes, &mesLine))
	{
		mesFuncs.GetLine_Safe(combatSys.combatMesNew, &mesLine);
		return (char*)mesLine.value;
	}
	mesFuncs.GetLine_Safe(combatMes, &mesLine);
	return (char*)mesLine.value;
}

void LegacyCombatSystem::FloatCombatLine(objHndl obj, int line)
{
	auto objType = objects.GetType(obj);
	FloatLineColor floatColor = FloatLineColor::White;
	if (objType == obj_t_npc )
	{
		auto npcLeader = critterSys.GetLeaderForNpc(obj);
		if (!party.IsInParty(npcLeader))
			floatColor = FloatLineColor::Red;
		else
			floatColor = FloatLineColor::Yellow;
	}

	auto combatLineText = GetCombatMesLine(line);
	if (combatLineText)
		floatSys.floatMesLine(obj, 1, floatColor, combatLineText);
}

void LegacyCombatSystem::FloatCombatLine(objHndl obj, int line, FloatLineColor floatColor)
{

	auto combatLineText = GetCombatMesLine(line);
	if (combatLineText)
		floatSys.floatMesLine(obj, 1, static_cast<FloatLineColor>(floatColor), combatLineText);
}

void LegacyCombatSystem::FloatTextBubble(objHndl handle, int combatMesLine)
{
	auto combatLineText = GetCombatMesLine(combatMesLine);
	uiDialog.ShowTextBubble(handle, handle, combatLineText);
}

int LegacyCombatSystem::IsWithinReach(objHndl attacker, objHndl target)
{
	float reach = critterSys.GetReach(attacker, D20A_UNSPECIFIED_ATTACK);
	float distTo = locSys.DistanceToObj(attacker, target);
	return distTo < reach;
}


BOOL LegacyCombatSystem::CanMeleeTargetAtLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* loc)
{
	if (!obj || critterSys.IsDeadOrUnconscious(obj))
		return FALSE;

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
	float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK),
		tgtRadius = locSys.InchesToFeet(objects.GetRadius(target));
	auto distToLoc = max((float)0.0,locSys.DistanceToLocFeet(obj, loc));
	if (tgtRadius + objReach < distToLoc)
		return 0;
	return TRUE;
}

BOOL LegacyCombatSystem::CanMeleeTargetAtLoc(objHndl obj, objHndl target, LocAndOffsets* loc){

	if (!obj || critterSys.IsDeadOrUnconscious(obj))
		return FALSE;

	objHndl weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!combatSys.CanMeleeTargetAtLocRegardItem(obj, weapon, target, loc))
	{
		objHndl secondaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
		if (!combatSys.CanMeleeTargetAtLocRegardItem(obj, secondaryWeapon, target, loc))
		{
			if (!objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, 0))
				return 0;
			float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
			float tgtRadius = objects.GetRadius(target) / 12.0f;
			auto distToLoc = locSys.DistanceToLocFeet(obj, loc);
			if (distToLoc < 0)
				distToLoc = 0;

			if (distToLoc - tgtRadius > objReach){
				return 0;
			}
			
		}
	}
	return 1;
}

BOOL LegacyCombatSystem::CanMeleeTargetFromLoc(objHndl obj, objHndl target, LocAndOffsets* objLoc)
{
	objHndl weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!combatSys.CanMeleeTargetFromLocRegardItem(obj, weapon, target, objLoc))
	{
		objHndl secondaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
		if (!combatSys.CanMeleeTargetFromLocRegardItem(obj, secondaryWeapon, target, objLoc))
		{
			if (!objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, 0))
				return 0;
			float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
			float tgtRadius = objects.GetRadius(target) / 12.0f;
			if (max(static_cast<float>(0.0),
				locSys.DistanceToLocFeet(target, objLoc)) - tgtRadius > objReach)
				return 0;
		}
	}
	return 1;
}

bool LegacyCombatSystem::CanMeleeTargetFromLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* objLoc)
{
	if (weapon)
	{
		if (objects.GetType(weapon) != obj_t_weapon)
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
	float objReach  = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK),
		  tgtRadius = locSys.InchesToFeet(objects.GetRadius(target));
	auto distToLoc = max((float)0.0, locSys.DistanceToLocFeet(target, objLoc));

	if (tgtRadius + objReach < distToLoc)
		return 0;
	return 1;
}

BOOL LegacyCombatSystem::CanMeleeTarget(objHndl obj, objHndl target){
	if (!target)
		return 0;
	if (objects.GetFlags(obj) & (OF_OFF | OF_DESTROYED))
		return 0;
	auto targetObjFlags = objects.GetFlags(target);
	if (targetObjFlags & (OF_OFF | OF_DESTROYED | OF_INVULNERABLE))
		return 0;
	if (critterSys.IsDeadOrUnconscious(obj))
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

BOOL LegacyCombatSystem::CanMeleeTargetRegardWeapon(objHndl obj, objHndl weapon, objHndl target)
{
	if (weapon)
	{
		if (objects.GetType(weapon) != obj_t_weapon)
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

BOOL LegacyCombatSystem::AffiliationSame(objHndl obj, objHndl obj2)
{
	auto objInParty = party.IsInParty(obj);
	return party.IsInParty(obj2) == objInParty;
}

int LegacyCombatSystem::GetThreateningCrittersAtLoc(objHndl obj, LocAndOffsets* loc, objHndl threateners[40])
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

objHndl LegacyCombatSystem::CheckRangedWeaponAmmo(objHndl obj)
{
	if (d20Sys.d20Query(obj, DK_QUE_Polymorphed))
		return objHndl::null;
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
		return objHndl::null;
	auto ammoItem = critterSys.GetWornItem(obj, EquipSlot::Ammo);
	auto weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!weapon || !weapons.AmmoMatchesWeapon(weapon, ammoItem))
		weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	
	if (!weapon || !weapons.AmmoMatchesWeapon(weapon, ammoItem))
		return objHndl::null;
	return ammoItem;
	
}

bool LegacyCombatSystem::AmmoMatchesItemAtSlot(objHndl obj, EquipSlot equipSlot)
{
	objHndl ammoItem = CheckRangedWeaponAmmo(obj);
	auto weapon = critterSys.GetWornItem(obj, equipSlot);
	if (!weapon)
		return 0;
	return weapons.AmmoMatchesWeapon(weapon, ammoItem);
}

objHndl * LegacyCombatSystem::GetHostileCombatantList(objHndl obj, int * count)
{
	int initListLen = GetInitiativeListLength();
	objHndl hostileTempList[100];
	int hostileCount = 0;
	for (int i = 0; i < initListLen; i++)
	{
		auto combatant = GetInitiativeListMember(i);
		if (combatant && obj != combatant && !critterSys.IsFriendly(obj,combatant)){
			hostileTempList[hostileCount++] = combatant;
		}
	}
	objHndl *result = new objHndl[hostileCount];
	memcpy(result, hostileTempList, hostileCount * sizeof(objHndl));
	*count = hostileCount;
	return result;
}

std::vector<objHndl> LegacyCombatSystem::GetHostileCombatantList(objHndl handle){
	int initListLen = GetInitiativeListLength();
	std::vector<objHndl> result;
	for (int i = 0; i < initListLen; i++){
		auto combatant = GetInitiativeListMember(i);
		if (combatant && handle != combatant && !critterSys.IsFriendly(handle, combatant)) {
			result.push_back(combatant);
		}
	}
	return result;
}

void LegacyCombatSystem::GetEnemyListInRange(objHndl performer, float rangeFeet, std::vector<objHndl>& enemiesOut){

	auto perfLoc = objSystem->GetObject(performer)->GetLocationFull();

	ObjList enemies;
	enemies.ListRadius(perfLoc, rangeFeet* INCH_PER_TILE, OLC_CRITTERS);
	for (int i = 0; i < enemies.size(); i++) {
		auto resHandle = enemies[i];
		if (!resHandle)
			break;

		if (critterSys.IsDeadNullDestroyed(resHandle))
			continue;

		if (resHandle != performer && !critterSys.NpcAllegianceShared(resHandle, performer) && !critterSys.IsFriendly(performer, resHandle)) {
			enemiesOut.push_back(resHandle);
		}
	}

}

bool LegacyCombatSystem::HasLineOfAttack(objHndl obj, objHndl target)
{
	BOOL result = 1;
	RaycastPacket objIt;
	objIt.origin = objects.GetLocationFull(obj);
	LocAndOffsets tgtLoc = objects.GetLocationFull(target);
	objIt.targetLoc = tgtLoc;
	objIt.flags = static_cast<RaycastFlags>(RaycastFlags::StopAfterFirstBlockerFound | RaycastFlags::ExcludeItemObjects | RaycastFlags::HasTargetObj | RaycastFlags::HasSourceObj | RaycastFlags::HasRadius);
	objIt.radius = static_cast<float>(0.1);
	bool blockerFound = false;
	if (objIt.Raycast())
	{
		auto results = objIt.results;
		for (auto i = 0; i < objIt.resultCount; i++)
		{
			objHndl resultObj = results[i].obj;
			if (!resultObj)
			{
				if (results[i].flags & RaycastResultFlags::BlockerSubtile)
				{
					blockerFound = true;
				}
				continue;
			}

			auto objType = objects.GetType(resultObj);
			if (objType == obj_t_portal)
			{
				if (!objects.IsPortalOpen(resultObj))
				{
					blockerFound = 1;
				}
				continue;
			}
			if (objType == obj_t_pc || objType == obj_t_npc)
			{
				if (critterSys.IsDeadOrUnconscious(resultObj)
					|| d20Sys.d20Query(resultObj, DK_QUE_Prone))
				{
					continue;
				}
				// TODO: flag for Cover 
			}
		}
	}
	objIt.RaycastPacketFree();
	if (!blockerFound)
	{
		return 1;
	}

	return 0;
}

bool LegacyCombatSystem::HasLineOfAttackFromPosition(LocAndOffsets fromPosition, objHndl target){
	RaycastPacket objIt;
	objIt.origin = fromPosition;
	LocAndOffsets tgtLoc = objects.GetLocationFull(target);
	objIt.targetLoc = tgtLoc;
	objIt.flags = static_cast<RaycastFlags>(RaycastFlags::StopAfterFirstBlockerFound | RaycastFlags::ExcludeItemObjects | RaycastFlags::HasTargetObj | RaycastFlags::HasSourceObj | RaycastFlags::HasRadius);
	objIt.radius = static_cast<float>(0.1);
	bool blockerFound = false;
	if (objIt.Raycast())
	{
		auto results = objIt.results;
		for (auto i = 0; i < objIt.resultCount; i++)
		{
			objHndl resultObj = results[i].obj;
			if (!resultObj)
			{
				if (results[i].flags & RaycastResultFlags::BlockerSubtile)
				{
					blockerFound = true;
				}
				continue;
			}

			auto objType = objects.GetType(resultObj);
			if (objType == obj_t_portal)
			{
				if (!objects.IsPortalOpen(resultObj))
				{
					blockerFound = 1;
				}
				continue;
			}
			if (objType == obj_t_pc || objType == obj_t_npc)
			{
				if (critterSys.IsDeadOrUnconscious(resultObj)
					|| d20Sys.d20Query(resultObj, DK_QUE_Prone))
				{
					continue;
				}
				// TODO: flag for Cover 
			}
		}
	}
	objIt.RaycastPacketFree();
	if (!blockerFound)
	{
		return true;
	}

	return false;
}

void LegacyCombatSystem::AddToInitiativeWithinRect(objHndl handle) const
{

	auto obj = gameSystems->GetObj().GetObject(handle);
	auto loc = obj->GetLocation();
	ObjList objlist;
	TileRect trect;
	trect.x1 = loc.locx - 18;
	trect.x2 = loc.locx + 18;
	trect.y1 = loc.locy - 18;
	trect.y2 = loc.locy + 18;

	auto& unk = temple::GetRef<int>(0x102BE130);
	unk = 18; // no idea what this is, looks like part of a 3x6 matrix

	objlist.ListRect(trect, OLC_CRITTERS );

	for (int i = 0; i < objlist.size() ; i++){
		auto resHandle = objlist[i];
		if (!resHandle)
			break;

		// check if the object is ok to act (not dead, OF_OFF, OF_DONTDRAW (to prevent the naughty Co8 critters from getting into combat), destroyed , unconscious)
		auto resObj = gameSystems->GetObj().GetObject(resHandle);
		if (resObj->GetFlags() & ( OF_OFF | OF_DESTROYED | OF_DONTDRAW ))
			continue;

		if (critterSys.IsDeadOrUnconscious(resHandle)){
			continue;
		}
			

		if (!tbSys.IsInInitiativeList(resHandle)){
			auto critFlags = critterSys.GetCritterFlags(resHandle);
			if (!(critFlags & OCF_COMBAT_MODE_ACTIVE) && !party.IsInParty(resHandle)) {
				aiSys.AiProcess(resHandle); // todo: originally there was a dangling IsPerforming check in the function, should it be added to the conditions??
			}
		}

		auto critFlags = critterSys.GetCritterFlags(resHandle);
		if (critFlags & OCF_COMBAT_MODE_ACTIVE){
			tbSys.AddToInitiative(resHandle);
		}
	}

}

void LegacyCombatSystem::TurnProcessAi(objHndl obj)
{
	//return addresses.TurnProcessing_100635E0(obj);
	auto actor = tbSys.turnBasedGetCurrentActor();
	if (obj != actor && obj != actSeqSys.getNextSimulsPerformer())
	{
		logger->warn("Not AI processing {} (wrong turn...)", description.getDisplayName(obj));
		return;
	}

	if (objects.IsPlayerControlled(obj)){
		if (critterSys.IsDeadOrUnconscious(obj)){
			logger->info("Combat for {} ending turn (unconscious)", description.getDisplayName(obj));
			CombatAdvanceTurn(obj);
		}
		// TODO: bug? they probably meant to do an OR

		return;
	}

	auto isLoadingGame = temple::GetRef<int>(0x103072D4);
	if (isLoadingGame)
		return;

	auto isPcUnderAiControl = temple::GetRef<int(__cdecl)(objHndl)>(0x1005AD20);
	auto aiProcessPc = temple::GetRef<int(__cdecl)(objHndl)>(0x1005AE10);
	if (isPcUnderAiControl(obj)){

		// tutorial shite
		if (maps.GetCurrentMapId() == 5118 && scriptSys.GetGlobalFlag(7))	{
			if (!tutorial.IsTutorialActive()){
				tutorial.Toggle();
			}
			tutorial.ShowTopic(31);
			scriptSys.SetGlobalFlag(7, 0);
		}
		if (!aiProcessPc(obj)){
			logger->info("Combat for {} ending turn (ai fail).", description.getDisplayName(obj));
		}
		// TODO: possibly bugged if there's no "Advance Turn"?
		return;
	}

	if (!pythonObjIntegration.ExecuteObjectScript(obj, obj, ObjScriptEvent::Heartbeat))	{
		logger->info("Combat for {} ending turn (script).", description.getDisplayName(obj));
		CombatAdvanceTurn(obj);
		return;
	}

	if (gameSystems->GetObj().GetObject(obj)->GetFlags() & OF_OFF) {
		logger->info("Combat for {} ending turn (OF_OFF).", description.getDisplayName(obj));
		CombatAdvanceTurn(obj);
		return;
	}
	aiSys.AiProcess(obj);
	if (gameSystems->GetObj().GetObject(obj)->GetFlags() & OF_OFF) {
		CombatAdvanceTurn(obj);
	}

}

BOOL LegacyCombatSystem::StartCombat(objHndl combatInitiator, int setToFirstInitiativeFlag){

	if (!*combatModeActive){

		if (AllPcsUnconscious())
			return FALSE;

		return temple::GetRef<BOOL(__cdecl)(objHndl, int)>(0x100639A0)(combatInitiator, setToFirstInitiativeFlag);
	}
	return TRUE;
}

void LegacyCombatSystem::EndTurn()
{
	auto actor = tbSys.turnBasedGetCurrentActor();
	
	if (party.IsInParty(actor)){
		static auto uiCombatInitiativePortraitsReset = temple::GetRef<int(__cdecl*)(int)>(0x10AA83FC);
		uiCombatInitiativePortraitsReset(0);
	} 
	else
	{
		pythonObjIntegration.ExecuteObjectScript(actor, actor, ObjScriptEvent::EndCombat);
	}

	tbSys.InitiativeListNextActor();

	if (party.IsInParty(actor) && !actSeqSys.isPerforming(actor)){
		//static auto addToInitiativeWithinRect = temple::GetRef<void(__cdecl)(objHndl)>(0x10062AC0);
		AddToInitiativeWithinRect(actor);
	}

	// remove dead and OF_OFF from initiative
	for (uint32_t i = 0; i < tbSys.GetInitiativeListLength(); ) {
		auto combatant = tbSys.groupInitiativeList->GroupMembers[i];
		auto combatantObj = gameSystems->GetObj().GetObject(combatant);
		
		auto shouldRemove = critterSys.IsDeadNullDestroyed(combatant) || (combatantObj->GetFlags() & OF_OFF);
		if (!shouldRemove && combatantObj->IsNPC() && !party.IsInParty(combatant)){
			auto aifs = AIFS_NONE;
			aiSys.GetAiFightStatus(combatant, &aifs, nullptr);
			if (aifs == AIFS_NONE){
				shouldRemove = true;
			}
		}
		if (shouldRemove) {
			static auto removeFromInitiative = temple::GetRef<int(__cdecl)(objHndl)>(0x100DF530);
			removeFromInitiative(combatant);
		}
		else
			i++;
	}
	
	if (tbSys.turnBasedGetCurrentActor()){
		actSeqSys.TurnStart(tbSys.turnBasedGetCurrentActor());
	}

	if (AllCombatantsFarFromParty()){
		logger->info("Ending combat (enemies far from party)");
		auto leader = party.GetConsciousPartyLeader();
		combatSys.CritterExitCombatMode(leader);
	} else if (AllPcsUnconscious())
	{
		auto leader = party.GetConsciousPartyLeader();
		combatSys.CritterExitCombatMode(leader);
	}
	//addresses.EndTurn();
}

void LegacyCombatSystem::CombatSubturnEnd()
{
	GameTime timeDelta (0,1000);
	gameTimeSys.GameTimeAdd(&timeDelta);
	if (party.GetConsciousPartyLeader())
	{
		++(*addresses.combatRoundCount);
		*addresses.combatActor = 0i64;
		*addresses.combatTimeEventSthg = 0;
		*addresses.combatTimeEventIndicator = 0;
	}
}

void LegacyCombatSystem::Subturn()
{
	
	auto actor = tbSys.turnBasedGetCurrentActor();
	auto partyLeader = party.GetConsciousPartyLeader();

	if (!actSeqSys.isPerforming(actor) ){
		if (party.IsInParty(actor))
			combatSys.AddToInitiativeWithinRect(actor);
		else if (!critterSys.IsFriendly(actor, partyLeader)){
	
			ObjList objList;
			objList.ListRangeTiles(actor, 24, OLC_CRITTERS);
			for (auto i=0; i< objList.size(); i++){
				auto resHandle = objList[i];
				if (!resHandle)
					break;
				if (resHandle == actor)
					continue;

				
				auto resObj = gameSystems->GetObj().GetObject(resHandle);
				if (resObj->GetFlags() & (OF_OFF | OF_DESTROYED | OF_DONTDRAW))
					continue;
				if (critterSys.IsDeadOrUnconscious(resHandle)) {
					continue;
				}

				if (party.IsInParty(resHandle))
					continue;

				if (tbSys.IsInInitiativeList(resHandle) || critterSys.IsCombatModeActive(resHandle))
					continue;

				auto objDesc = description.getDisplayName(resHandle);

				if (!combatSys.HasLineOfAttack(resHandle, actor)){


					if (locSys.DistanceToObj(actor, resHandle) > 40){
						continue;
					}
					// check pathfinding short distances
					auto pathFlags = PathQueryFlags::PQF_HAS_CRITTER | PQF_IGNORE_CRITTERS 
						|PathQueryFlags::PQF_800 | PathQueryFlags::PQF_TARGET_OBJ
						 | PathQueryFlags::PQF_ADJUST_RADIUS | PathQueryFlags::PQF_ADJ_RADIUS_REQUIRE_LOS
						| PathQueryFlags::PQF_DONT_USE_PATHNODES | PathQueryFlags::PQF_A_STAR_TIME_CAPPED;

					if (!config.alertAiThroughDoors){
						pathFlags |= PathQueryFlags::PQF_DOORS_ARE_BLOCKING;
					}

					if (!pathfindingSys.CanPathTo(actor, resHandle, (PathQueryFlags)pathFlags, 40)){
						//logger->debug("Failed to alert {} because of PF distance", resHandle);
						continue;
					}	
				}
				
				if (aiSys.GetAllegianceStrength(resHandle, actor)){ // check that they have a faction in common
					aiSys.ProvokeHostility(partyLeader, resHandle, 3, 0);
					continue;
				}

				if (factions.HasNullFaction(resHandle) && factions.HasNullFaction(actor)){
					if (aiSys.WillKos(resHandle, partyLeader)) {
						aiSys.ProvokeHostility(partyLeader, resHandle, 3, 0);
					}
				}

			}
			 
		}
	}

	if (!actor){
		logger->error("Combat Subturn: Coudn't start TB combat Turn due to no Active Critters!");
		CombatEnd();
		return;
	}

	auto & combatSubturnTimeEvent = temple::GetRef<int>(0x10AA8420);
	combatSubturnTimeEvent = 10;
	static auto uiCombatInitiativePortraitsReset = temple::GetRef<int(__cdecl*)(int)>(0x10AA83FC);
	party.CurrentlySelectedClear();

	if (objects.IsPlayerControlled(actor)){
		
		uiCombatInitiativePortraitsReset(combatSubturnTimeEvent);

		party.AddToCurrentlySelected(actor);
		temple::GetRef<objHndl>(0x10AA8430) = actor; // looks like a write-only debug thing?

		// there was a call to some nullsub here

		auto combatSubturnCallback = temple::GetRef<void(__cdecl*)(objHndl)>(0x10AA8400);
		if (combatSubturnCallback){
			combatSubturnCallback(actor);
		}

		if (maps.GetCurrentMapId() == 5118 && scriptSys.GetGlobalFlag(7)
			&& objSystem->GetObject(actor)->IsPC()) {
			if (!tutorial.IsTutorialActive()) {
				tutorial.Toggle();
			}
			tutorial.ShowTopic(31);
			scriptSys.SetGlobalFlag(7, 0);
		}
		return;
	}

	// non-player controlled

	uiCombatInitiativePortraitsReset(0);
	temple::GetRef<objHndl>(0x10AA8430) = actor; // looks like a write-only debug thing?
	auto combatSubturnCallback = temple::GetRef<void(__cdecl*)(objHndl)>(0x10AA8400);
	if (combatSubturnCallback) {
		combatSubturnCallback(actor);
	}

	if (actSeqSys.isPerforming(actor))
		return;

	if (!pythonObjIntegration.ExecuteObjectScript(actor, actor, ObjScriptEvent::StartCombat)){
		logger->info("Skipping AI Process for {} (script)", actor);
		combatSys.CombatAdvanceTurn(actor);
		return;
	}
	
	logger->info("Calling AI Process for {}", actor);
	combatSys.TurnProcessAi(actor);

	// addresses.Subturn();
}

void LegacyCombatSystem::TurnStart2(int prevInitiativeIdx)
{
	auto actor = tbSys.turnBasedGetCurrentActor();
	int curActorInitIdx = tbSys.GetInitiativeListIdx();
	if (prevInitiativeIdx > curActorInitIdx)
	{
		logger->debug("TurnStart2: \t End Subturn. Cur Actor: {} ({}), Initiative Idx: {}; Prev Initiative Idx: {} ", description.getDisplayName(actor), actor, curActorInitIdx, prevInitiativeIdx);
		CombatSubturnEnd();
	}

	// start new turn for current actor
	actor = tbSys.turnBasedGetCurrentActor();
	curActorInitIdx = tbSys.GetInitiativeListIdx();
	logger->debug("TurnStart2: \t Starting new turn for {} ({}). InitiativeIdx: {}", description.getDisplayName(actor), actor, curActorInitIdx);
	Subturn();

	// set action bar values
	uiCombat.ActionBarUnsetFlag1(*addresses.barPkt);
	if (!objects.IsPlayerControlled(actor))
	{
		uiCombat.ActionBarSetMovementValues(*addresses.barPkt, 0.0, 20.0, 1.0);
	}
	
	// handle simuls
	if (actSeqSys.SimulsAdvance())
	{
		actor = tbSys.turnBasedGetCurrentActor();
		logger->debug("TurnStart2: \t Actor {} ({}) starting turn...(simul)", description.getDisplayName(actor), actor);
		CombatAdvanceTurn(actor);
	}
}

void LegacyCombatSystem::CombatAdvanceTurn(objHndl obj)
{
	if (addresses.CheckFleeCombatMap() && addresses.GetFleeStatus())
	{
		addresses.CombatPerformFleeCombat(obj);
	}
	if (!isCombatActive())
		return;
	tbSys.InitiativeListSort();
	if ( tbSys.turnBasedGetCurrentActor() != obj && !(actSeqSys.isSimultPerformer(obj) || actSeqSys.IsSimulsCompleted()))	{
		logger->warn("Combat Advance Turn: Not {}'s turn...", description.getDisplayName(obj));
		return;
	}
	if ( actSeqSys.IsLastSimulsPerformer(obj)){
		logger->warn("Combat Advance Turn: Next turn waiting on simuls actions...");
		return;
	}
	static int combatInitiative = 0;
	combatInitiative++;
	auto curSeq = *actSeqSys.actSeqCur;
	logger->debug("Combat Advance Turn: Actor {} ({}) ending his turn. CurSeq: {}", description.getDisplayName(obj), obj, (void*)curSeq);
	if (combatInitiative <= GetInitiativeListLength())
	{
		int initListIdx = tbSys.GetInitiativeListIdx();
		EndTurn();
		if (isCombatActive())
			TurnStart2(initListIdx);
	}
	combatInitiative--;

	// return addresses.CombatTurnAdvance(obj);
}

BOOL LegacyCombatSystem::IsBrawlInProgress()
{
	auto brawlInProgress= temple::GetRef<int>(0x10BD01C0);
	if (!brawlInProgress)
		return false;
	auto brawlOpponent = temple::GetRef<objHndl>(0x10BD01D0);
	if (brawlOpponent)
		return true;
	return false;
}

void LegacyCombatSystem::CritterExitCombatMode(objHndl handle) {

	if (!objects.IsPlayerControlled(handle)) {
		auto critterFlags = critterSys.GetCritterFlags(handle);
		if (critterFlags & OCF_COMBAT_MODE_ACTIVE) {
			objSystem->GetObject(handle)->SetInt32(obj_f_critter_flags, critterFlags & ~OCF_COMBAT_MODE_ACTIVE);
		}
		return;
	}

	if (!isCombatActive())
		return;

	if (party.GetConsciousPartyLeader() != handle){
		return;
	}

	if (critterSys.IsDeadNullDestroyed(handle)){
		if (party.GetLivingPartyMemberCount() >= 1)
			return;
	}

	if (!CombatEnd())
		return;

	static auto uiCombatResetCallback = temple::GetRef<int(__cdecl*)()>(0x10AA83F8);
	uiCombatResetCallback();

	static auto combatMusicEnd = temple::GetRef<void(__cdecl)(objHndl)>(0x1003C8B0);
	combatMusicEnd(handle);

	auto N = party.GroupListGetLen();
	for (auto i=0; i < N; i++){
		auto partyMem = party.GroupListGetMemberN(i);
		auto critterFlags = critterSys.GetCritterFlags(partyMem);
		if (critterFlags & OCF_COMBAT_MODE_ACTIVE){
			objSystem->GetObject(partyMem)->SetInt32(obj_f_critter_flags, critterFlags & ~OCF_COMBAT_MODE_ACTIVE);
		}
	}

	// temple::GetRef<void(__cdecl)(objHndl)>(0x100630F0)(handle);
}

bool LegacyCombatSystem::CombatEnd(){
	//static auto combatEnd = temple::GetRef<int(__cdecl)()>(0x10062A30);
	if (!isCombatActive() )
		return true;

	d20ObjRegistrySys.D20ObjRegistrySendSignalAll(DK_SIG_Combat_End, 0, 0);
	*combatSys.combatModeActive = 0;
	animationGoals.SetRuninfoDeallocCallback(nullptr);
	if (!animationGoals.InterruptAllForTbCombat()){
		logger->debug("CombatEnd: Anim goal interrupt FAILED!");
	}
	static auto actSeqResetOnCombatEnd = temple::GetRef<void(__cdecl)()>(0x10097BE0);
	actSeqResetOnCombatEnd();
	auto &mResettingCombatSystem = temple::GetRef<BOOL>(0x10AA8448);
	if (!mResettingCombatSystem)
		tbSys.ExecuteExitCombatScriptForInitiativeList();
	tbSys.TbCombatEnd();
	if (!mResettingCombatSystem){
		auto N = party.GroupListGetLen();
		for (auto i=0; i < N; i++){
			auto partyMember = party.GroupListGetMemberN(i);
			temple::GetRef<void(__cdecl)(objHndl)>(0x100B70A0)(partyMember);
		}
		auto combatGiveXp = temple::GetRef<void(__cdecl)()>(0x100B88C0);
		combatGiveXp();
	}
	return true;
}

bool LegacyCombatSystem::isCombatActive()
{
	auto isActive = *combatSys.combatModeActive;
	return isActive != 0;
}

bool LegacyCombatSystem::IsAutoAttack(){
	if (isCombatActive())
		return false;
	return config.GetVanillaInt("auto attack") != 0;
}

bool LegacyCombatSystem::AllCombatantsFarFromParty()
{
	const double PEACEOUT_DISTANCE = 125.0;

	if (!isCombatActive())
		return true;

	auto N = GetInitiativeListLength();

	for (auto i=0; i < N; i++){
		auto combatant = GetInitiativeListMember(i);
		if (party.IsInParty(combatant))
			continue;
		if (critterSys.IsDeadOrUnconscious(combatant))
			continue;
		if (critterSys.IsConcealed(combatant)){
			continue; // TODO maybe revamp this condition?
		}

		auto Nparty = party.GroupListGetLen();
		for (auto j=0; j < Nparty; j++){
			auto partyMem = party.GroupListGetMemberN(j);
			if (locSys.DistanceToObj(combatant, partyMem) < PEACEOUT_DISTANCE){
				return false;
			}
				
		}
	}

	return true;

//	static auto combatantsFarFromParty = temple::GetRef<int(__cdecl)()>(0x10062CB0);
	//return combatantsFarFromParty();

}

bool LegacyCombatSystem::AllPcsUnconscious(){

	auto N = party.GroupPCsLen();
	for (auto i=0; i < N; i++){
		auto pc = party.GroupPCsGetMemberN(i);
		if (!pc)
			continue;
		if (!critterSys.IsDeadOrUnconscious(pc))
			return false;
	}

	return true;
}

uint32_t LegacyCombatSystem::IsCloseToParty(objHndl objHnd)
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

int LegacyCombatSystem::GetEnemiesCanMelee(objHndl obj, objHndl* canMeleeList)
{
	return addresses.GetEnemiesCanMelee(obj, canMeleeList);
}

std::vector<objHndl> LegacyCombatSystem::GetEnemiesCanMelee(objHndl handle){
	std::vector<objHndl> result;

	auto N = combatSys.GetInitiativeListLength();
	for (auto i = 0; i < N; i++) {
		auto combatant = combatSys.GetInitiativeListMember(i);
		if (!combatant || combatant == handle || critterSys.IsFriendly(handle, combatant))
			continue;
		if (!combatSys.CanMeleeTarget(combatant, handle))
			continue;
		result.push_back(combatant);
	}

	return result;
}

objHndl LegacyCombatSystem::GetWeapon(AttackPacket* attackPacket)
{
	D20CAF flags = attackPacket->flags;
	objHndl result = objHndl::null;

	if (!(flags & D20CAF_TOUCH_ATTACK) || flags & D20CAF_THROWN_GRENADE )
		result = attackPacket->weaponUsed;
	return result;
}

bool LegacyCombatSystem::IsUnarmed(objHndl handle){
	if (inventory.ItemWornAt(handle, EquipSlot::WeaponPrimary))
		return false;
	if (inventory.ItemWornAt(handle, EquipSlot::WeaponSecondary))
		return false;
	return true;
}

bool LegacyCombatSystem::DisarmCheck(objHndl attacker, objHndl defender, D20Actn* d20a)
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
		auto weaponType = objects.GetWeaponType(attackerWeapon);
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
	dispatch.DispatchAttackBonus(attacker, defender, &dispIoAtkBonus, dispTypeBucklerAcPenalty, 0); // buckler penalty
	dispatch.DispatchAttackBonus(attacker, objHndl::null, &dispIoAtkBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int atkToHitBonus = dispatch.DispatchAttackBonus(defender, objHndl::null, &dispIoAtkBonus, dispTypeToHitBonusFromDefenderCondition, 0);
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
		auto weaponType = objects.GetWeaponType(defenderWeapon);
		if (weaponType == wt_spike_chain || weaponType == wt_nunchaku || weaponType == wt_light_flail || weaponType == wt_heavy_flail || weaponType == wt_dire_flail || weaponType == wt_ranseur || weaponType == wt_halfling_nunchaku)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, 2, 0, 343); // Weapon Special Bonus
	}
	
	dispIoDefBonus.attackPacket.weaponUsed = attackerWeapon;
	dispatch.DispatchAttackBonus(defender, objHndl::null, &dispIoDefBonus, dispTypeBucklerAcPenalty, 0); // buckler penalty
	dispatch.DispatchAttackBonus(defender, objHndl::null, &dispIoDefBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int defToHitBonus = dispatch.DispatchAttackBonus(attacker, objHndl::null, &dispIoDefBonus, dispTypeToHitBonusFromDefenderCondition, 0);
	int defenderResult = defenderRoll + bonusSys.getOverallBonus(&dispIoDefBonus.bonlist);

	bool attackerSucceeded = attackerResult > defenderResult;
	int rollHistId = histSys.RollHistoryAddType6OpposedCheck(attacker, defender, attackerRoll, defenderRoll, &dispIoAtkBonus.bonlist, &dispIoDefBonus.bonlist, 5109, 144 - attackerSucceeded, 1);
	histSys.CreateRollHistoryString(rollHistId);
	
	return attackerSucceeded;
}

bool LegacyCombatSystem::SunderCheck(objHndl attacker, objHndl defender, D20Actn* d20a)
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
	dispatch.DispatchAttackBonus(attacker, defender, &dispIoAtkBonus, dispTypeBucklerAcPenalty, 0); // buckler penalty
	dispatch.DispatchAttackBonus(attacker, objHndl::null, &dispIoAtkBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int atkToHitBonus = dispatch.DispatchAttackBonus(defender, objHndl::null, &dispIoAtkBonus, dispTypeToHitBonusFromDefenderCondition, 0);
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
	dispatch.DispatchAttackBonus(defender, objHndl::null, &dispIoDefBonus, dispTypeBucklerAcPenalty, 0); // buckler penalty
	dispatch.DispatchAttackBonus(defender, objHndl::null, &dispIoDefBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int defToHitBonus = dispatch.DispatchAttackBonus(attacker, objHndl::null, &dispIoDefBonus, dispTypeToHitBonusFromDefenderCondition, 0);
	int defenderResult = defenderRoll + bonusSys.getOverallBonus(&dispIoDefBonus.bonlist);

	bool attackerSucceeded = attackerResult > defenderResult;
	int rollHistId = histSys.RollHistoryAddType6OpposedCheck(attacker, defender, attackerRoll, defenderRoll, &dispIoAtkBonus.bonlist, &dispIoDefBonus.bonlist, 5109, 144 - attackerSucceeded, 1);
	histSys.CreateRollHistoryString(rollHistId);

	return attackerSucceeded;
}

int LegacyCombatSystem::GetClosestEnemy(objHndl obj, LocAndOffsets* locOut, objHndl* objOut, float* distOut, int flags)
{
	return _GetClosestEnemy(obj, locOut, objOut, distOut, flags);
}

int LegacyCombatSystem::GetInitiativeListLength()
{
	return _GetInitiativeListLength();
}

objHndl LegacyCombatSystem::GetInitiativeListMember(int n)
{
	return _GetInitiativeListMember(n);
}

int LegacyCombatSystem::GetClosestEnemy(AiTactic* aiTac, int selectionType)
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
			customCondition = !critterSys.IsDeadOrUnconscious(combatant);
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

void LegacyCombatSystem::Brawl(objHndl a, objHndl b){

	temple::GetRef<int>(0x102E7F38) = -1; // reset brawl state (fixes weird issues... also allows brawling to be reused)
	_Brawl(a, b);
}

void LegacyCombatSystem::enterCombat(objHndl objHnd){

	if (!d20Sys.d20Query(objHnd, DK_QUE_EnterCombat))
		return;

	if (!combatSys.IsCloseToParty(objHnd))
		return;


	if (objects.IsPlayerControlled(objHnd)){
		
		auto partyCount = party.GroupListGetLen();
		for (auto i=0; i < partyCount; i++){
			auto partyMember = party.GroupListGetMemberN(i);
			if (!critterSys.IsCombatModeActive(partyMember)){
				temple::GetRef<void(__cdecl)(objHndl)>(0x10062740)(partyMember); // set combat mode active and force spreadout
			}
		}

	}
	else if (!critterSys.IsCombatModeActive(objHnd)){
		auto partyMember0 = party.GroupListGetMemberN(0);
		inventory.WieldBestAll(objHnd, partyMember0);
		temple::GetRef<void(__cdecl)(objHndl)>(0x10062740)(objHnd); // set combat mode active and force spreadout
		temple::GetRef<void(__cdecl)(objHndl)>(0x1003C770)(objHnd); // turn combat music on
	}

	//_enterCombat(objHnd);
}

void LegacyCombatSystem::AddToInitiative(objHndl critter){
	tbSys.AddToInitiative(critter);
}

int LegacyCombatSystem::DispelRoll(objHndl obj, BonusList* bonlist, int modifier, int dc, const char* text, int *rollHistId){
	Dice dice(1, 20, modifier);
	auto d20RollRes = dice.Roll();
	int rollResId = histSys.RollHistoryType4Add(obj, dc, text, dice.ToPacked(), d20RollRes, bonlist);
	if (rollHistId)
		*rollHistId = rollResId;
	else
		histSys.CreateRollHistoryString(rollResId);
	return d20RollRes - dc + bonlist->GetEffectiveBonusSum();
}

void LegacyCombatSystem::ToHitProcessing(D20Actn& d20a){
	auto performer = d20a.d20APerformer;
	auto d20Data = d20a.data1;
	auto caflags = d20a.d20Caf;
	auto tgt = d20a.d20ATarget;
	if (!tgt)
		return;

	// mirror image processing
	auto mirrorImageCond = conds.GetByName("sp-Mirror Image");
	if (d20Sys.d20QueryWithData(tgt, DK_QUE_Critter_Has_Condition, mirrorImageCond, 0)){
		auto spellId = (uint32_t) d20Sys.d20QueryReturnData(tgt, DK_QUE_Critter_Has_Condition, mirrorImageCond, 0);
		SpellPacketBody spellPkt(spellId);
		Dice dice(1,spellPkt.targetCount);
		if (dice.Roll() != 1 ){ // mirror image nominally struck
			DispIoAttackBonus mirrorImAc;
			mirrorImAc.attackPacket.flags = (D20CAF)(d20a.d20Caf | D20CAF_TOUCH_ATTACK);
			mirrorImAc.attackPacket.d20ActnType = d20a.d20ActType;
			mirrorImAc.attackPacket.attacker = performer;
			mirrorImAc.attackPacket.victim = tgt;
			auto tgtAc = critterSys.GetArmorClass(tgt, &mirrorImAc);
			DispIoAttackBonus mirrorImToHit;
			mirrorImToHit.Dispatch(performer, objHndl::null, dispTypeToHitBonus2, DK_NONE);
			auto spName = spellPkt.GetName();
			auto dispelRes = DispelRoll(performer, &mirrorImToHit.bonlist, 0, tgtAc, spName, &d20a.rollHistId0  );
			if (dispelRes >= 0)	{
				d20Sys.d20SendSignal(tgt, DK_SIG_Spell_Mirror_Image_Struck, spellPkt.spellId, 0);
				floatSys.FloatCombatLine(tgt, 109);
				histSys.CreateRollHistoryLineFromMesfile(10, performer, tgt);
				return;
			}
		}
	}



	// miss chances handling
	static auto getDefenderConcealmentMissChance = [](objHndl attacker, objHndl victim, D20Actn & d20a) {

		auto cond = conds.GetByName("sp-True Strike");
		if (d20Sys.d20QueryWithData(attacker, DK_QUE_Critter_Has_Condition, cond, 0))
			return 0;
		cond = conds.GetByName("Weapon Seeking");
		if (d20Sys.d20QueryWithData(attacker, DK_QUE_Critter_Has_Condition, cond, 0))
			return 0;

		DispIoAttackBonus dispIo;
		dispIo.attackPacket.flags = (D20CAF)d20a.d20Caf;
		dispIo.attackPacket.victim = victim;
		dispIo.attackPacket.attacker = attacker;
		dispIo.attackPacket.dispKey = 0;
		return dispIo.Dispatch(victim, attacker, dispTypeGetDefenderConcealmentMissChance, DK_NONE);
	};

	static auto getAttackerConcealmentMissChance = [](objHndl attacker) {

		auto dispatcher = gameSystems->GetObj().GetObject(attacker)->GetDispatcher();
		DispIoObjBonus dispIo;
		dispatcher->Process(dispTypeGetAttackerConcealmentMissChance, DK_NONE, &dispIo);
		return temple::GetRef<int(__cdecl)(BonusList&)>(0x100E6680)(dispIo.bonlist); // special bonus handler for blindness miss chance
	};

	auto defenderMissChance = getDefenderConcealmentMissChance(performer, tgt, d20a);
	auto attackerMissChance = getAttackerConcealmentMissChance(performer);
	if (defenderMissChance > 0 || attackerMissChance > 0){

		if (attackerMissChance > defenderMissChance)
			defenderMissChance = attackerMissChance;
		
		// roll 1d100
		auto missChanceRoll = Dice::Roll(1, 100, 0);
		if (missChanceRoll > defenderMissChance){ // success
			d20a.rollHistId1 = histSys.RollHistoryType5Add(performer, tgt, defenderMissChance, 60, missChanceRoll, 194, 193);
		} 
		else { // failure
			d20a.rollHistId1 = histSys.RollHistoryType5Add(performer, tgt, defenderMissChance, 60, missChanceRoll, 195, 193);


			// Blind Fight handling (second chance)
			if (!feats.HasFeatCountByClass(performer, FEAT_BLIND_FIGHT))
				return;
			missChanceRoll = Dice::Roll(1, 100, 0);
			if (missChanceRoll <= defenderMissChance) {
				histSys.RollHistoryType5Add(performer, tgt, defenderMissChance, 61, missChanceRoll, 195, 193);
				return;
			}
			d20a.rollHistId2 = histSys.RollHistoryType5Add(performer, tgt, defenderMissChance, 61, missChanceRoll, 194, 193);
		}
	}

	// get the To Hit bonus
	DispIoAttackBonus dispIoToHitBon, dispIoAtkBon, dispIoTgtAc;
	dispIoToHitBon.attackPacket.flags = (D20CAF)d20a.d20Caf;
	dispIoToHitBon.attackPacket.victim = tgt;
	dispIoToHitBon.attackPacket.d20ActnType = d20a.d20ActType;
	dispIoToHitBon.attackPacket.attacker = performer;
	dispIoToHitBon.attackPacket.dispKey = d20Data;
	if (d20a.d20Caf & D20CAF_TOUCH_ATTACK){
		dispIoToHitBon.attackPacket.weaponUsed = 0i64;
	} 
	else{
		if (d20a.d20Caf & D20CAF_SECONDARY_WEAPON)
			dispIoToHitBon.attackPacket.weaponUsed = inventory.ItemWornAt(performer, EquipSlot::WeaponSecondary);
		else
			dispIoToHitBon.attackPacket.weaponUsed = inventory.ItemWornAt(performer, EquipSlot::WeaponPrimary);
	}
	dispIoToHitBon.Dispatch(performer, objHndl::null, dispTypeBucklerAcPenalty, DK_NONE); // adds buckler penalty to the bonus list
	
	
	if (dispIoToHitBon.attackPacket.weaponUsed
		&& gameSystems->GetObj().GetObject(dispIoToHitBon.attackPacket.weaponUsed)->type != obj_t_weapon){
		dispIoToHitBon.attackPacket.weaponUsed = 0i64;
	}
	dispIoToHitBon.attackPacket.ammoItem = CheckRangedWeaponAmmo(performer);
	dispIoToHitBon.attackPacket.flags = (D20CAF) (dispIoToHitBon.attackPacket.flags | D20CAF_FINAL_ATTACK_ROLL);
	dispIoToHitBon.Dispatch(performer, objHndl::null, dispTypeToHitBonus2, DK_NONE); // note: the "Global" condition has ToHitBonus2 hook that dispatches the ToHitBonusBase
	auto toHitBonFinal = dispIoToHitBon.Dispatch(tgt, objHndl::null, dispTypeToHitBonusFromDefenderCondition, DK_NONE);

	dispIoTgtAc.attackPacket = dispIoToHitBon.attackPacket;
	dispIoTgtAc.field_4 = dispIoToHitBon.field_4;
	critterSys.GetArmorClass(tgt, &dispIoTgtAc);
	auto tgtAcFinal = dispIoTgtAc.Dispatch(performer, objHndl::null, dispTypeAcModifyByAttacker, DK_NONE);

	auto toHitRoll = Dice::Roll(1, 20);

	if (critterSys.IsPC(performer)) {
		auto ff = tio_fopen("pc_rolls.txt", "a");
		auto text = fmt::format("{}\n", toHitRoll);
		tio_fwrite(&text[0], text.size(), 1, ff);
		tio_fclose(ff);
	}
	else {
		auto ff = tio_fopen("npc_rolls.txt", "a");
		auto text = fmt::format("{}\n", toHitRoll);
		tio_fwrite(&text[0], text.size(), 1, ff);
		tio_fclose(ff);
	}

	if (party.IsInParty(performer)){
		auto ff = tio_fopen("party_rolls.txt", "a");
		auto text = fmt::format("{}\n", toHitRoll);
		tio_fwrite(&text[0], text.size(), 1, ff);
		tio_fclose(ff);
	}
	else {
		auto ff = tio_fopen("enemy_rolls.txt", "a");
		auto text = fmt::format("{}\n", toHitRoll);
		tio_fwrite(&text[0], text.size(), 1, ff);
		tio_fclose(ff);
	}

	auto ff = tio_fopen("overall_rolls.txt", "a");
	auto text = fmt::format("{}\n", toHitRoll);
	tio_fwrite(&text[0], text.size(), 1, ff);
	tio_fclose(ff);
	

	auto critAlwaysCheat = temple::GetRef<int>(0x10BCA8B0);

	auto isMiss = [critAlwaysCheat](int roll, int toHitBon, int tgtAc) {
		if (critAlwaysCheat)
			return false;
		return roll == 1 || roll != 20 && roll + toHitBon < tgtAc;
	};
	#define IS_MISS isMiss(toHitRoll, toHitBonFinal, tgtAcFinal)

	if (!(dispIoToHitBon.attackPacket.flags & D20CAF_ALWAYS_HIT) && !critAlwaysCheat){
		// check miss
		if (IS_MISS){
			if (d20Sys.d20Query(performer, DK_QUE_RerollAttack)){
				histSys.RollHistoryType0Add(toHitRoll, -1, performer, tgt, &dispIoToHitBon.bonlist, &dispIoTgtAc.bonlist, dispIoToHitBon.attackPacket.flags);
				toHitRoll = Dice::Roll(1, 20);
				dispIoToHitBon.attackPacket.flags = (D20CAF)(dispIoToHitBon.attackPacket.flags | D20CAF_REROLL);
			}
		}
		// still a miss
		if (IS_MISS){
			temple::GetRef<void(__cdecl)(objHndl)>(0x1009A9D0)(performer); // handling for consecutive misses in the logbook
		}
	}

	auto critHitRoll = -1;
	if (!IS_MISS || (dispIoToHitBon.attackPacket.flags & D20CAF_ALWAYS_HIT) ){
		// register a hit
		dispIoToHitBon.attackPacket.flags = (D20CAF)(dispIoToHitBon.attackPacket.flags | D20CAF_HIT); 
		temple::GetRef<void(__cdecl)(objHndl)>(0x1009A9B0)(performer); // logbook consecutive hits handling

		// do Critical Hit roll
		dispIoAtkBon.attackPacket = dispIoToHitBon.attackPacket;
		dispIoAtkBon.field_4 = dispIoToHitBon.field_4;
		auto critThreatRange = 21 - dispIoAtkBon.Dispatch(performer, objHndl::null, dispTypeGetCriticalHitRange, DK_NONE);
		if (!d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Immune_Critical_Hits)){
			if (toHitRoll >= critThreatRange || critAlwaysCheat) {
				critHitRoll = Dice::Roll(1, 20);
				
				// RerollCritical handling (e.g. from Luck domain)
				if (isMiss(critHitRoll, toHitBonFinal, tgtAcFinal) && d20Sys.d20Query(performer, DK_QUE_RerollCritical)){
					histSys.RollHistoryType0Add(toHitRoll, critHitRoll, performer, tgt, &dispIoToHitBon.bonlist, &dispIoTgtAc.bonlist, dispIoToHitBon.attackPacket.flags);
					critHitRoll = Dice::Roll(1, 20);
				}

				if (!isMiss(critHitRoll, toHitBonFinal, tgtAcFinal))
					dispIoToHitBon.attackPacket.flags = (D20CAF)(dispIoToHitBon.attackPacket.flags | D20CAF_CRITICAL);
			}
		}

		// do Deflect Arrow dispatch
		dispIoToHitBon.Dispatch(dispIoToHitBon.attackPacket.victim, objHndl::null, dispTypeDeflectArrows, DK_NONE);
	}


	

	// sphagetti handling for Sanctuary spell
	if (d20a.d20ATarget && gameSystems->GetObj().GetObject(d20a.d20ATarget)->IsCritter()
		&& !d20Sys.D20QueryWithDataDefaultTrue(d20a.d20ATarget, DK_QUE_CanBeAffected_PerformAction, &d20a,0)){
		
		if (dispIoToHitBon.attackPacket.flags & D20CAF_CRITICAL){
			dispIoToHitBon.attackPacket.flags = (D20CAF)(dispIoToHitBon.attackPacket.flags ^ D20CAF_CRITICAL);
		}
		if (dispIoToHitBon.attackPacket.flags & D20CAF_HIT) {
			dispIoToHitBon.attackPacket.flags = (D20CAF)(dispIoToHitBon.attackPacket.flags ^ D20CAF_HIT);
			dispIoToHitBon.bonlist.ZeroBonusSetMeslineNum(262);
		}
	}

	// all this hard work just to set a couple of flags :D
	d20a.d20Caf = dispIoToHitBon.attackPacket.flags;
	d20a.rollHistId0 = histSys.RollHistoryType0Add(toHitRoll, critHitRoll, performer, tgt, &dispIoToHitBon.bonlist, &dispIoTgtAc.bonlist, dispIoToHitBon.attackPacket.flags);
	
	// there were some additional debug stubs here (nullsubs)
}

bool LegacyCombatSystem::TripCheck(objHndl handle, objHndl target){
	if (d20Sys.d20Query(target, DK_QUE_Untripable))	{
		histSys.CreateFromFreeText( combatSys.GetCombatMesLine(171)); 
		return false;
	}

	static auto abilityScoreCheckModDispatch = [](objHndl obj, objHndl opponent, Stat statUsed, BonusList * bonlist, int flags) {
		auto dispatcher = gameSystems->GetObj().GetObject(obj)->GetDispatcher();
		DispIoObjBonus dispIo;
		if (!dispatcher->IsValid())
			return 0;
		dispIo.bonOut = bonlist;
		dispIo.flags = flags;
		dispIo.obj = opponent;
		dispatch.DispatcherProcessor(dispatcher, dispTypeAbilityCheckModifier, DK_STAT_STRENGTH + statUsed, &dispIo);
		return dispIo.bonOut->GetEffectiveBonusSum();
	};


	auto attackerRoll = Dice(1, 20).Roll();
	BonusList attackerBon;
	auto attackerStr = objects.StatLevelGet(handle, Stat::stat_strength);
	auto attackerStrMod = objects.GetModFromStatLevel(attackerStr);
	attackerBon.AddBonus(attackerStrMod, 0, 103);
	abilityScoreCheckModDispatch(handle, target, stat_strength, &attackerBon, 1);
	auto attackerSize = dispatch.DispatchGetSizeCategory(handle);
	if (attackerSize != 5){
		attackerBon.AddBonus(4 * (attackerSize - 5), 0, 316);
	}

	auto attackerResult = attackerRoll + attackerBon.GetEffectiveBonusSum();

	auto defenderRoll = Dice(1, 20).Roll();
	BonusList defenderBon;
	auto defenderStr = objects.StatLevelGet(target, Stat::stat_strength);
	auto defenderDex = objects.StatLevelGet(target, Stat::stat_dexterity);
	Stat defenderStat = stat_strength;
	if (defenderDex > defenderStr){
		defenderStat = stat_dexterity;
		auto defenderMod = objects.GetModFromStatLevel(defenderDex);
		defenderBon.AddBonus(defenderMod, 0, 104);
	} else
	{
		auto defenderMod = objects.GetModFromStatLevel(defenderStr);
		defenderBon.AddBonus(defenderMod, 0, 103);
	}
	abilityScoreCheckModDispatch(target, handle, defenderStat, &defenderBon, 3);
	auto defenderSize = dispatch.DispatchGetSizeCategory(target);
	if (defenderSize != 5) {
		defenderBon.AddBonus(4 * (defenderSize - 5), 0, 316);
	}
	auto defenderResult = defenderRoll + defenderBon.GetEffectiveBonusSum();



	auto succeeded = attackerResult > defenderResult;
	auto rollId = histSys.RollHistoryAddType6OpposedCheck(handle, target, attackerRoll, defenderRoll, &attackerBon, &defenderBon, 5062, 144 - (succeeded != false), 1 );
	histSys.CreateRollHistoryString(rollId);

	return succeeded;
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