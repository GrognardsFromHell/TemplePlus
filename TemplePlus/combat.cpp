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


struct CombatSystemAddresses : temple::AddressTable
{
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
	}

	void (*Subturn)();
	void (*EndTurn)();
	void (*CombatPerformFleeCombat)(objHndl obj);
} addresses;

class CombatSystemReplacements : public TempleFix
{
public:
	const char* name() override {
		return "Combat System Replacements";
	}

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
		if (actor)
			logger->debug("Greybar Reset! Current actor: {} ({})", description.getDisplayName(actor), actor);
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

		orgCheckRangedWeaponAmmo = replaceFunction(0x100654E0, CheckRangedWeaponAmmo);
		
		replaceFunction(0x100B4B30, _GetCombatMesLine);

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

char * LegacyCombatSystem::GetCombatMesLine(int line)
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

void LegacyCombatSystem::FloatCombatLine(objHndl obj, int line)
{
	auto objType = objects.GetType(obj);
	FloatLineColor floatColor = FloatLineColor::White;
	if (objType == obj_t_npc )
	{
		auto npcLeader = critterSys.GetLeaderRecursive(obj);
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

int LegacyCombatSystem::IsWithinReach(objHndl attacker, objHndl target)
{
	float reach = critterSys.GetReach(attacker, D20A_UNSPECIFIED_ATTACK);
	float distTo = locSys.DistanceToObj(attacker, target);
	return distTo < reach;
}


BOOL LegacyCombatSystem::CanMeleeTargetAtLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* loc)
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
	float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK),
		tgtRadius = locSys.InchesToFeet(objects.GetRadius(target));
	auto distToLoc = max((float)0.0,locSys.DistanceToLocFeet(obj, loc));
	if (tgtRadius + objReach < distToLoc)
		return 0;
	return 1;
}

BOOL LegacyCombatSystem::CanMeleeTargetAtLoc(objHndl obj, objHndl target, LocAndOffsets* loc)
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
			float tgtRadius = objects.GetRadius(target) / 12.0;
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

BOOL LegacyCombatSystem::CanMeleeTarget(objHndl obj, objHndl target)
{
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
	int objInParty = party.IsInParty(obj);
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

		if (critterSys.IsDeadOrUnconscious(resHandle))
		{
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
	static auto getNextSimulsActor = temple::GetRef<objHndl(__cdecl)()>(0x100920E0);
	if (obj != actor && obj != getNextSimulsActor())
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
	auto getGlobalFlag = temple::GetRef<int(__cdecl)(int)>(0x10006790);
	auto setGlobalFlag = temple::GetRef<void(__cdecl)(int, int)>(0x100067C0);
	auto aiProcessPc = temple::GetRef<int(__cdecl)(objHndl)>(0x1005AE10);
	if (isPcUnderAiControl(obj)){

		// tutorial shite
		if (maps.GetCurrentMapId() == 5118 && getGlobalFlag(7))	{
			if (!tutorial.IsTutorialActive()){
				tutorial.Toggle();
			}
			tutorial.ShowTopic(31);
			setGlobalFlag(7, 0);
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
	for (int i = 0; i < tbSys.GetInitiativeListLength(); ) {
		auto combatant = tbSys.groupInitiativeList->GroupMembers[i];
		auto combatantObj = gameSystems->GetObj().GetObject(combatant);
		
		if (critterSys.IsDeadNullDestroyed(combatant) 
			|| (combatantObj->GetFlags() & OF_OFF)) {
			static auto removeFromInitiative = temple::GetRef<int(__cdecl)(objHndl)>(0x100DF530);
			removeFromInitiative(combatant);
		}
		else
			i++;
	}
	
	if (tbSys.turnBasedGetCurrentActor()){
		actSeqSys.TurnStart(tbSys.turnBasedGetCurrentActor());
	}

	static auto combatantsFarFromParty = temple::GetRef<int(__cdecl)()>(0x10062CB0);
	static auto combatEnd = temple::GetRef<int(__cdecl)(objHndl)>(0x100630F0);
	static auto allPcsUnconscious = temple::GetRef<int(__cdecl)()>(0x10062D60);
	
	if (combatantsFarFromParty()){
		logger->info("Ending combat (enemies far from party)");
		auto leader = party.GetConsciousPartyLeader();
		combatEnd(leader);
	} else if (allPcsUnconscious())
	{
		auto leader = party.GetConsciousPartyLeader();
		combatEnd(leader);
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
	addresses.Subturn();

	//auto actor = tbSys.turnBasedGetCurrentActor();


	//if (party.IsInParty(actor) && !actSeqSys.isPerforming(actor)) {
	//	static auto addToInitiativeWithinRect = temple::GetRef<void(__cdecl)(objHndl)>(0x10062AC0);
	//	addToInitiativeWithinRect(actor);
	//}

	//if (!actor){
	//	logger->error("Combat Subturn: Coudn't start TB combat Turn due to no Active Critters!");
	//	static auto combatEnd = temple::GetRef<int(__cdecl)()>(0x10062A30);
	//	combatEnd();
	//}

	//auto 
}

void LegacyCombatSystem::TurnStart2(int initiativeIdx)
{
	auto actor = tbSys.turnBasedGetCurrentActor();
	int curActorInitIdx = tbSys.GetInitiativeListIdx();
	if (initiativeIdx > curActorInitIdx)
	{
		logger->debug("TurnStart2: \t End Subturn. Cur Actor: {} ({}), Initiative Idx: {}; New Initiative Idx: {} ", description.getDisplayName(actor), actor, curActorInitIdx, initiativeIdx);
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
	if (!tbSys.turnBasedGetCurrentActor() == obj && !(actSeqSys.isSimultPerformer(obj) || actSeqSys.IsSimulsCompleted()))
	{
		logger->warn("Combat Advance Turn: Not {}'s turn...", description.getDisplayName(obj));
		return;
	}
	if ( actSeqSys.IsLastSimulsPerformer(obj))
	{
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

bool LegacyCombatSystem::isCombatActive()
{
	return *combatSys.combatModeActive != 0;
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

objHndl LegacyCombatSystem::GetWeapon(AttackPacket* attackPacket)
{
	D20CAF flags = attackPacket->flags;
	objHndl result = 0i64;

	if (!(flags & D20CAF_TOUCH_ATTACK) || flags & D20CAF_THROWN_GRENADE )
		result = attackPacket->weaponUsed;
	return result;
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
		auto weaponType = objects.GetWeaponType(defenderWeapon);
		if (weaponType == wt_spike_chain || weaponType == wt_nunchaku || weaponType == wt_light_flail || weaponType == wt_heavy_flail || weaponType == wt_dire_flail || weaponType == wt_ranseur || weaponType == wt_halfling_nunchaku)
			bonusSys.bonusAddToBonusList(&dispIoAtkBonus.bonlist, 2, 0, 343); // Weapon Special Bonus
	}
	
	dispIoDefBonus.attackPacket.weaponUsed = attackerWeapon;
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispTypeBucklerAcPenalty, 0); // buckler penalty
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int defToHitBonus = dispatch.DispatchAttackBonus(attacker, 0, &dispIoDefBonus, dispTypeToHitBonusFromDefenderCondition, 0);
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
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispTypeBucklerAcPenalty, 0); // buckler penalty
	dispatch.DispatchAttackBonus(defender, 0, &dispIoDefBonus, dispTypeToHitBonus2, 0); // to hit bonus2
	int defToHitBonus = dispatch.DispatchAttackBonus(attacker, 0, &dispIoDefBonus, dispTypeToHitBonusFromDefenderCondition, 0);
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

void LegacyCombatSystem::enterCombat(objHndl objHnd)
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