#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"
#include <objlist.h>
#include "inventory.h"
#include "float_line.h"
#include "condition.h"
#include "d20.h"
#include "particles.h"
#include "config/config.h"
#include "python/python_integration_obj.h"
#include "python/python_object.h"
#include "tig/tig_startup.h"
#include "util/fixes.h"
#include "gamesystems/particlesystems.h"
#include "animgoals/anim.h"
#include <graphics/mdfmaterials.h>
#include <infrastructure/meshes.h>
#include <infrastructure/vfs.h>
#include <temple/meshes.h>
#include <infrastructure/mesparser.h>
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "ai.h"
#include "party.h"
#include "ui\ui.h"
#include "temple_functions.h"
#include "combat.h"
#include "history.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "rng.h"
#include "weapon.h"
#include "d20_race.h"
#include "d20_level.h"
#include "location.h"


static struct CritterAddresses : temple::AddressTable {

	uint32_t (__cdecl *HasMet)(objHndl critter, objHndl otherCritter);
	uint32_t (__cdecl *AddFollower)(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);
	uint32_t (__cdecl *RemoveFollower)(objHndl npc, int unkFlag);
	objHndl (__cdecl *GetLeader)(objHndl critter);
	objHndl(__cdecl *GetLeaderRecursive)(objHndl critter);
	int (__cdecl *HasLineOfSight)(objHndl critter, objHndl target);
	void (__cdecl *Attack)(objHndl target, objHndl attacker, int n1, int n2);
	uint32_t (__cdecl *IsFriendly)(objHndl pc, objHndl npc);
	int (__cdecl *SoundmapCritter)(objHndl critter, int id);
	void (__cdecl *KillByEffect)(objHndl critter, objHndl killer);
	void (__cdecl *Kill)(objHndl critter, objHndl killer);
	void(__cdecl *CritterHpChanged)(objHndl obj, objHndl assailant, int damAmt);

	void (__cdecl *GetStandpoint)(objHndl, StandPointType, StandPoint *);
	void (__cdecl *SetStandpoint)(objHndl, StandPointType, const StandPoint *);
	void (__cdecl *SetSubdualDamage)(objHndl critter, int damage);

	void (__cdecl *BalorDeath)(objHndl critter);
	void (__cdecl *SetConcealed)(objHndl critter, int concealed);

	uint32_t(__cdecl *IsDeadOrUnconscious)(objHndl critter);

	objHndl (__cdecl *GiveItem)(objHndl critter, int protoId);

	void(__cdecl *TakeMoney)(objHndl critter, int platinum, int gold, int silver, int copper);
	void(__cdecl *GiveMoney)(objHndl critter, int platinum, int gold, int silver, int copper);

	void(__cdecl*GetCritterVoiceLine)(objHndl obj, objHndl fellow, char* str, int* soundId);
	int (__cdecl*PlayCritterVoiceLine)(objHndl obj, objHndl fellow, char* str, int soundId);

	int(__cdecl* GetCritterMap)(objHndl critter);
	CritterAddresses() {
		rebase(PlayCritterVoiceLine, 0x10036120);
		rebase(GetCritterVoiceLine, 0x100373C0);
		rebase(HasMet, 0x10053CD0);
		rebase(AddFollower, 0x100812F0);
		rebase(RemoveFollower, 0x10080FD0);
		rebase(GetLeader, 0x1007EA70);
		rebase(GetLeaderRecursive, 0x10080430);
		rebase(HasLineOfSight, 0x10059470);
		rebase(Attack, 0x1005E8D0);
		rebase(IsFriendly, 0x10080E00);
		rebase(SoundmapCritter, 0x1006DEF0);
		rebase(Kill, 0x100B8A00);
		rebase(KillByEffect, 0x100B9460);
		rebase(GetStandpoint, 0x100BA890);
		rebase(SetStandpoint, 0x100BA8F0);
		rebase(SetSubdualDamage, 0x1001DB10);
		rebase(BalorDeath, 0x100F66F0);
		rebase(SetConcealed, 0x10080670);
		rebase(IsDeadOrUnconscious, 0x100803E0);
		rebase(GiveItem, 0x1006CC30);

		rebase(GiveMoney, 0x1007F960);
		rebase(TakeMoney, 0x1007FA40);
		rebase(CritterHpChanged, 0x100B8AA0);
		rebase(GetCritterMap, 0x10080790);
	}

	
} addresses;

class CritterReplacements : public TempleFix
{
public:
	bool CanSense(objHndl critter, objHndl tgt);
	int InvisibleAttackBonus(DispatcherCallbackArgs args);

private:
	void ShowExactHpForNPCs();

	void apply() override 
	{
		logger->info("Replacing Critter System functions");
		replaceFunction(0x10062720, _isCritterCombatModeActive); 
		replaceFunction<float(objHndl, D20ActionType)>(0x100B52D0, [](objHndl handle, D20ActionType d20aType){
			return critterSys.GetReach(handle, d20aType);
		});

		// Invisibility Attack bonus
		oldInvisibleAttackBonus = replaceFunction<int(DispatcherCallbackArgs)>(0x100E88D0, [](DispatcherCallbackArgs args) {
			return critterReplacements.InvisibleAttackBonus(args);
		});

		// CanSense
		oldCanSense = replaceFunction<bool(objHndl, objHndl)>(0x1007FFF0, [](objHndl critter, objHndl tgt) {
			return critterReplacements.CanSense(critter, tgt);
		});

		// CritterGenerateHp
		replaceFunction<void(objHndl)>(0x1007F720, [](objHndl handle){
			critterSys.GenerateHp(handle);
		});

		// CritterGetWeaponAnimId
		replaceFunction<int(objHndl, gfx::WeaponAnim)>(0x10020c60, [](objHndl handle, gfx::WeaponAnim animId) {
			return (int) critterSys.GetAnimId(handle, animId);
		});

		// something used in the anim goals
		replaceFunction<int(objHndl, objHndl)>(0x10065C30, [](objHndl item, objHndl parent)->int{

			if (!item && !parent) {
				logger->warn("Null item & parent! func 0x10065C30");
				return 0;
			}

			if (!item) {
				return (int) critterSys.GetReach(parent, D20A_NONE);
			}

			auto itemObj = gameSystems->GetObj().GetObject(item);
			if (itemObj->type != obj_t_weapon){
				return (int) critterSys.GetReach(parent, D20A_NONE);
			}

			auto weapFlags = (WeaponFlags)itemObj->GetInt32(obj_f_weapon_flags);
			if (weapFlags & WeaponFlags::OWF_RANGED_WEAPON)	{
				return itemObj->GetInt32(obj_f_weapon_range);
			}
			return itemObj->GetInt32(obj_f_weapon_range) + (int) critterSys.GetReach(parent, D20A_NONE);
		});
		ShowExactHpForNPCs();

		//GetNumNaturalAttacks
		replaceFunction<int(__cdecl)(objHndl)>(0x100800C0, [](objHndl handle) {
			return (int)critterSys.GetNumNaturalAttacks(handle);
		});


		// GetSize
		replaceFunction<int(__cdecl)(objHndl)>(0x1004D690, [](objHndl handle) {
			return critterSys.GetSize(handle);
			});

		replaceFunction<int(__cdecl)(objHndl,objHndl,objHndl,int)>(0x10020B60, [](objHndl wielder, objHndl prim, objHndl scnd, int animId) {
			return (int)critterSys.GetWeaponAnim(wielder, prim, scnd, (gfx::WeaponAnim)animId);
			});

		replaceFunction<uint32_t(__cdecl)(objHndl,ResurrectType,int)>(0x100809C0, [](objHndl critter, ResurrectType type, int clvl) {
			return critterSys.Resurrect(critter, type, clvl);
		});

		replaceFunction<int(__cdecl)(objHndl)>(0x100B70A0, [](objHndl critter) {
			return critterSys.AutoReload(critter) ? 1 : 0;
		});
	}

private:
	int(*oldInvisibleAttackBonus)(DispatcherCallbackArgs args);
	bool(*oldCanSense)(objHndl, objHndl);

} critterReplacements;

void CritterReplacements::ShowExactHpForNPCs()
{
	if (config.showExactHPforNPCs)
	{
		char displayExactHpForNPCsChar = 0;
		char * displayExactHpForNPCsBuffer = &displayExactHpForNPCsChar;
		write(0x1012441B + 2, displayExactHpForNPCsBuffer, 1);
	}
}

int CritterReplacements::InvisibleAttackBonus(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);

	// If the attacker can be seen with blindsight, don't add the bonus
	if (critterSys.CanSeeWithBlindsight(dispIo->attackPacket.victim, args.objHndCaller)) {
		return 0;
	}

	return oldInvisibleAttackBonus(args);
}

bool CritterReplacements::CanSense(objHndl critter, objHndl tgt)
{
	//First check if the critter can see the target with blindsense
	if (critterSys.CanSeeWithBlindsight(critter, tgt)) return true;

	// Default to let TOEE handle it
	return oldCanSense(critter, tgt);
}

#pragma region Critter System Implementation

struct LegacyCritterSystem critterSys;

uint32_t LegacyCritterSystem::isCritterCombatModeActive(objHndl objHnd)
{
	return (objects.getInt32(objHnd, obj_f_critter_flags) & OCF_COMBAT_MODE_ACTIVE) != 0;
}

LegacyCritterSystem::LegacyCritterSystem()
{
}

uint32_t LegacyCritterSystem::IsPC(objHndl objHnd)
{
	return objects.getInt32(objHnd, obj_f_type) == obj_t_pc;
}
#pragma endregion

int LegacyCritterSystem::GetLootBehaviour(objHndl npc) {
	return objects.getInt32(npc, obj_f_npc_pad_i_3) & 0xF;
}

void LegacyCritterSystem::SetLootBehaviour(objHndl npc, int behaviour) {
	objects.setInt32(npc, obj_f_npc_pad_i_3, behaviour & 0xF);
}

uint32_t LegacyCritterSystem::HasMet(objHndl critter, objHndl otherCritter) {
	return addresses.HasMet(critter, otherCritter);
}

uint32_t LegacyCritterSystem::AddFollower(objHndl npc, objHndl pc, int forcedFollower, bool asAiFollower) {
	return addresses.AddFollower(npc, pc, forcedFollower, asAiFollower);
}

bool LegacyCritterSystem::FollowerAtMax(){
	auto followers = party.GroupNPCFollowersLen();
	auto pcs = party.GroupPCsLen();
	auto res = false;
	if (config.maxPCsFlexible)
	{
		res = (followers + pcs >= PARTY_SIZE_MAX) || followers >= PARTY_NPC_SIZE_MAX;
	}
	else {

		res = (followers >= PARTY_SIZE_MAX - (uint32_t)config.maxPCs) || followers >= PARTY_NPC_SIZE_MAX;
	}
	return res;
}

uint32_t LegacyCritterSystem::RemoveFollower(objHndl npc, int forceFollower) {
	return addresses.RemoveFollower(npc, forceFollower);
}

objHndl LegacyCritterSystem::GetLeader(objHndl critter) {
	return addresses.GetLeader(critter);
}

objHndl LegacyCritterSystem::GetLeaderForNpc(objHndl critter){

	auto obj = objSystem->GetObject(critter);
	if (!obj->IsNPC())
		return objHndl::null;

	auto leader = critter;
	while (leader && !objSystem->GetObject(leader)->IsPC()){
		leader = GetLeader(leader);
	}
	return leader;
}

bool LegacyCritterSystem::CanSeeWithBlindsight(objHndl critter, objHndl target) {
	if (!critter || !target){
		return false;
	}
	auto blindsightDistance = d20Sys.D20QueryPython(critter, "Blindsight Range");

	if (blindsightDistance > 0) {
		auto distance = locSys.DistanceToObj(critter, target);

		if (distance < blindsightDistance) {
			bool bEthereal = d20Sys.d20Query(target, DK_QUE_Is_Ethereal);

			// Rules are not 100% clear but I dont think blindsense should work on ethereal creatures
			return !bEthereal;
		}
	}

	return false;
}

int LegacyCritterSystem::HasLineOfSight(objHndl critter, objHndl target) {
	return addresses.HasLineOfSight(critter, target);
}

objHndl LegacyCritterSystem::GetWornItem(objHndl handle, EquipSlot slot) {
	return inventory.ItemWornAt(handle, (uint32_t) slot);
}

void LegacyCritterSystem::Attack(objHndl provoked, objHndl agitator, int rangeType, int flags) {
	// flags:
	// 0x1 - if 1, will adjust reaction by -10 regardless of the hostility threshold
	// 0x2 - 
	// 0x4
	aiSys.ProvokeHostility(provoked, agitator, rangeType, flags);
}

bool LegacyCritterSystem::AutoReload(objHndl critter)
{
	if (!critter || IsDeadOrUnconscious(critter)) return false;
	// TODO: more conditions blocking?

	if (!combatSys.NeedsToReload(critter)) return false;

	if (!combatSys.AmmoMatchesItemAtSlot(critter, WeaponPrimary)) {
		// out of ammo
		histSys.CreateRollHistoryLineFromMesfile(0, critter, objHndl::null);
		objects.floats->FloatCombatLine(critter, 44);
		return false;
	}

	if (combatSys.isCombatActive()) return false;

	auto weapon = inventory.ItemWornAt(critter, WeaponPrimary);
	weapons.SetLoaded(weapon);

	objects.floats->FloatCombatLine(critter, 42);
	histSys.CreateRollHistoryLineFromMesfile(2, critter, objHndl::null);

	return true;
}

void LegacyCritterSystem::Pickpocket(objHndl handle, objHndl tgt, int & gotCaught){
	if (!handle || !tgt)
		return;
	auto obj = objSystem->GetObject(handle);
	if (obj->GetFlags() & (OF_OFF | OF_DESTROYED))
		return;
	if (critterSys.IsDeadOrUnconscious(handle))
		return;

	auto tgtObj = objSystem->GetObject(tgt);

	auto pickpocketFailed = true;
	int deltaFromDc = 0;

	auto tgtMoney = critterSys.MoneyAmount(tgt) / 100;
	auto isStealingMoney = Dice::Roll(1, 2) == 1;
	auto dc = 20;
	
	if (isStealingMoney){
		if (tgtMoney < 1) {  // less than 1 GP
			isStealingMoney = false;
		}
	}
	
	if (skillSys.SkillRoll(handle, SkillEnum::skill_pick_pocket, dc, &deltaFromDc, 1 )){

		if (isStealingMoney){
			auto moneyAmt = Dice::Roll(1, 1900, 99) / 100;
			if (moneyAmt > tgtMoney)
				moneyAmt = tgtMoney;
			critterSys.TakeMoney(tgt, 0, moneyAmt, 0, 0);
			critterSys.GiveMoney(handle, 0, moneyAmt , 0, 0);
			histSys.CreateFromFreeText(fmt::format("Stole {} GP.\n\n", moneyAmt).c_str());
			pickpocketFailed = false;
		}
		else{
			auto invenCount = tgtObj->GetInt32(obj_f_critter_inventory_num);
			std::vector<objHndl> stealableItems;
			for (auto i=0; i < invenCount; i++){
				auto item = tgtObj->GetObjHndl(obj_f_critter_inventory_list_idx, i);
				if (inventory.ItemCanBePickpocketed(item))
					stealableItems.push_back(item);
			}

			if (stealableItems.size()){
				auto itemStolen = stealableItems[Dice::Roll(1, stealableItems.size()) - 1];
				histSys.CreateFromFreeText(fmt::format("Stole {}.\n\n", description.getDisplayName(itemStolen, handle)).c_str());
				inventory.SetItemParent(itemStolen, handle, ItemInsertFlags::IIF_None);
				pickpocketFailed = false;
			}
			else if (tgtMoney > 0 ) // steal coins instead
			{
				auto moneyAmt = Dice::Roll(1, 1900, 99) / 100;
				if (moneyAmt > tgtMoney)
					moneyAmt = tgtMoney;
				critterSys.TakeMoney(tgt, 0, moneyAmt, 0, 0);
				critterSys.GiveMoney(handle, 0, moneyAmt , 0, 0);
				histSys.CreateFromFreeText(fmt::format("Stole {} GP.\n\n", moneyAmt).c_str());
				pickpocketFailed = false;
			}
			else{
				histSys.CreateFromFreeText("Nothing to steal...\n\n");
			}
			
		}

	}


	if (skillSys.SkillRoll(tgt, SkillEnum::skill_spot, 20 + deltaFromDc, nullptr, 1)) {
		pythonObjIntegration.ExecuteObjectScript(tgt, handle,  0, ObjScriptEvent::CaughtThief); // e.g. when Dala is stealing from you
		gotCaught = 1;
		aiSys.ProvokeHostility(handle, tgt, 1, 2);
	}
	
	if ( objects.IsPlayerControlled(handle) || gotCaught){
		MesLine line(1100);
		if (pickpocketFailed)
			line.key++;
		auto skillUiMes = temple::GetRef<MesHandle>(0x103074C8);
		mesFuncs.GetLine_Safe(skillUiMes, &line);
		floatSys.floatMesLine(handle, 1, FloatLineColor::Blue, line.value);

		if (gotCaught){
			line.key = 1102;
			mesFuncs.GetLine_Safe(skillUiMes, &line);
			floatSys.floatMesLine(handle, 1, FloatLineColor::Red, line.value);
		}
	}


}

int LegacyCritterSystem::MoneyAmount(objHndl handle){
	return temple::GetRef<int(__cdecl)(objHndl)>(0x1007F880)(handle);
}

uint32_t LegacyCritterSystem::IsFriendly(objHndl critter1, objHndl critter2) {

	if (!critter1 || !critter2)
		return FALSE;

	if (critter1 == critter2)
		return TRUE;

	// added to account for both being AI controlled (assumed friendly - TODO overhaul in the future!)
	if (d20Sys.d20Query(critter1, DK_QUE_Critter_Is_AIControlled) && d20Sys.d20Query(critter2, DK_QUE_Critter_Is_AIControlled))
		return TRUE;

	static auto checkNotCharmedPartyMember = [](objHndl handle) { // returns false if is in party, is charmed, and charmed is out of party
		auto isInParty = party.IsInParty(handle);
		if (!isInParty)
			return true;
		if (!d20Sys.d20Query(handle, DK_QUE_Critter_Is_Charmed))
			return true;

		objHndl charmer;
		charmer.handle = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Charmed);
		if (!charmer)
			return true;
		if (party.IsInParty(charmer))
			return true;

		return false;
	};

	auto critter1_in_party = party.IsInParty(critter1);
	auto critter2_in_party = party.IsInParty(critter2);
	auto critter1_leader = critterSys.GetLeader(critter1);
	auto critter2_leader = critterSys.GetLeader(critter2);

	// if both are in party, or critter2's leader is in party
	if ((critter1_in_party && critter2_in_party)
		|| (party.IsInParty(critter2_leader) && critter1_in_party)
		|| (party.IsInParty(critter1_leader) && critter2_in_party)) { // added the flip condition too () - was missing in vanilla, looked like a bug

		if (checkNotCharmedPartyMember(critter2) && checkNotCharmedPartyMember(critter1))
			return TRUE;
	}
	//else{
	//	// bug? was in vanilla code...
	//	if (critter1_in_party && party.IsInParty(critter2_leader)){
	//		if (checkNotCharmedPartyMember(critter2) && checkNotCharmedPartyMember(critter1))
	//			return TRUE;
	//	}
	//}

	// Here, they are not both party members (up to "charm person" situations)

	auto obj1 = objSystem->GetObject(critter1);
	auto obj2 = objSystem->GetObject(critter2);
	
	// if both are NPCs:
	if (obj1->IsNPC() && obj2->IsNPC()) {

		if (d20Sys.d20Query(critter1, DK_QUE_Critter_Is_Charmed)){
			return FALSE;
		}

		if (critter1_leader == critter2 || critter2_leader == critter1 || critterSys.NpcAllegianceShared(critter1, critter2)) {
			return TRUE;
		}

		// Faction 0 critters
		if (critterSys.HasNoFaction(critter1) && critterSys.HasNoFaction(critter2)) {
			if (!critter1_in_party && !critter2_in_party){ // added so your animal companions attack faction 0 critters...
				return TRUE;
			}
		}

		return FALSE;
	}


	// in this section, at least one of the critters is a PC
	auto pc = critter1;
	auto npc = critter2;

	if (!objSystem->GetObject(pc)->IsPC()){
		pc = critter2;
		npc = critter1;
		// they can't be both NPCs at this point - if they were, it'd have returned in the previous section.
	}

	if (!objSystem->GetObject(pc)->IsPC()) {
		return FALSE; // just in case something that's not even a critter somehow got here
	}
	
	if (!d20Sys.d20Query(pc, DK_QUE_Critter_Is_Charmed))
		return FALSE;
	objHndl charmer;
	charmer.handle = d20Sys.d20QueryReturnData(pc, DK_QUE_Critter_Is_Charmed);
	if (charmer != npc)
		return FALSE;
	return TRUE;



	//// check if the other one is a PC
	//if (!objSystem->GetObject(npc)->IsPC()) {
	//	return FALSE;
	//}

	//if (!objSystem->GetObject(critter1)->IsNPC()){

	//	if (objSystem->GetObject(critter1)->IsPC()){
	//		if (!d20Sys.d20Query(critter1, DK_QUE_Critter_Is_Charmed))
	//			return FALSE;
	//		objHndl charmer;
	//		charmer.handle = d20Sys.d20QueryReturnData(critter1, DK_QUE_Critter_Is_Charmed);
	//		if (charmer != critter2)
	//			return FALSE;
	//		return TRUE;

	//	}

	//	if (!objSystem->GetObject(critter2)->IsPC()) {
	//		return FALSE;
	//	}

	//	if (!d20Sys.d20Query(critter2, DK_QUE_Critter_Is_Charmed))
	//		return FALSE;
	//	objHndl charmer;
	//	charmer.handle = d20Sys.d20QueryReturnData(critter2, DK_QUE_Critter_Is_Charmed);
	//	if (charmer != critter1)
	//		return FALSE;
	//	return TRUE;
	//}

	//if (objSystem->GetObject(critter2)->IsNPC()) {
	//	
	//	if (!d20Sys.d20Query(critter1, DK_QUE_Critter_Is_Charmed)){
	//		auto critter1_leader = critterSys.GetLeader(critter1);
	//		if (critter1_leader == critter2_leader)
	//			return TRUE;
	//		if (critter2_leader == critter1
	//			|| critterSys.NpcAllegianceShared(critter1, critter2)
	//			|| critterSys.HasNoFaction(critter1) && critterSys.HasNoFaction(critter2) ){
	//			return TRUE;
	//		}
	//			
	//	}
	//	return FALSE;
	//}
	//

	//return addresses.IsFriendly(critter1, critter2);
}

BOOL LegacyCritterSystem::NpcAllegianceShared(objHndl handle, objHndl handle2){

	const int FACTION_ARRAY_MAX = 50;

	auto obj = objSystem->GetObject(handle);
	auto obj2 = objSystem->GetObject(handle2);
	auto npc = handle;
	auto pc = handle2;
	

	if (obj->IsPC()) {
		if ( obj2->IsPC() )
			return FALSE;
		pc = handle;
		npc = handle2;
	}
	// handle1 is NPC
	else if (obj2->IsNPC()){  // handle2 is also NPC

		auto objLeader = critterSys.GetLeaderForNpc(handle);
		auto obj2Leader = critterSys.GetLeaderForNpc(handle2);

		// check leaders:
		// if one is the leader of the other, or their leaders are identical (and not null) - TRUE
		if ( (objLeader && objLeader == obj2Leader)
			|| objLeader == handle2 
			|| obj2Leader == handle){
			return TRUE;
		}

		// check joint factions
		for (auto factionIdx = 0; factionIdx < FACTION_ARRAY_MAX; factionIdx++){
			auto objFaction = obj->GetInt32(obj_f_npc_faction, factionIdx);
			if (!objFaction)
				return FALSE;
			if (factions.FactionHas(handle2, objFaction))
				return TRUE;
		}

		// If no joint factions - return FALSE
		return FALSE;
	}
	else{ // handle2 is PC
		pc = handle2;
		npc = handle;
	}

	auto leader = critterSys.GetLeaderForNpc(npc);
	if (pc == leader)
		return TRUE;

	auto npcObj = objSystem->GetObject(npc);
	for (auto factionIdx = 0; factionIdx < FACTION_ARRAY_MAX; factionIdx++) {
		auto objFaction = npcObj->GetInt32(obj_f_npc_faction, factionIdx);
		if (!objFaction)
			return FALSE;
		if (factions.PCHasFactionFromReputation(pc, objFaction))
			return TRUE;
	}
	return FALSE;

	//auto allegShared = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10080A70);
	//return allegShared(handle, handle2);
}

bool LegacyCritterSystem::HasNoFaction(objHndl handle){
	if (!handle)
		return true;

	auto obj = objSystem->GetObject(handle);
	
	if (obj->IsPC())
		return true;
	
	if (party.IsInParty(handle))
		return false;

	auto result = obj->GetInt32(obj_f_npc_faction, 0) == 0;

	return result;
}

int LegacyCritterSystem::GetReaction(objHndl of, objHndl towards){
	return temple::GetRef<int(objHndl, objHndl)>(0x10054180)(of, towards);
}

int LegacyCritterSystem::SoundmapCritter(objHndl critter, int id) {
	return addresses.SoundmapCritter(critter, id);
}

/* 0x100B8A00 */
void LegacyCritterSystem::Kill(objHndl critter, objHndl killer) {
	return addresses.Kill(critter, killer);
}

void LegacyCritterSystem::KillByEffect(objHndl critter, objHndl killer) {
	return addresses.KillByEffect(critter, killer);
}

void LegacyCritterSystem::Banish(objHndl target, objHndl killer, bool xp)
{
	// if not dead yet, take care of some death stuff.
	if (!IsDeadNullDestroyed(target) && killer && xp) {
		AwardXpFor(killer, target);
	}

	gameSystems->GetParticleSys().CreateAtObj("Fizzle", target);
	auto critter = objSystem->GetObject(target);
	auto aiFlags = critter->GetInt64(obj_f_npc_ai_flags64);
	aiFlags |= AiFlag::RunningOff;
	critter->SetInt64(obj_f_npc_ai_flags64, aiFlags);
	objects.FadeTo(target, 0, 2, 5, 1);
}

// port of 0x100B88E0
void LegacyCritterSystem::AwardXpFor(objHndl killer, objHndl critter)
{
	auto LogbookDefeat = temple::GetRef<int(objHndl, objHndl)>(0x1009A910);
	LogbookDefeat(killer, critter);

	if (!party.IsInParty(killer)) return;

	if (IsPC(critter)) return;

	auto summoned = conds.GetByName("sp-Summoned");
	if (d20Sys.d20QueryWithData(critter, DK_QUE_Critter_Has_Condition, summoned, 0))
		return;

	auto flags = objects.getInt32(critter, obj_f_critter_flags);
	if (flags & OCF_EXPERIENCE_AWARDED) return;

	auto cr = objects.getInt32(critter, obj_f_npc_challenge_rating);
	cr += objects.StatLevelGet(critter, stat_level);

	auto AwardXpForCR = temple::GetRef<void(int)>(0x100B8880);
	AwardXpForCR(cr);
	flags |= OCF_EXPERIENCE_AWARDED;
	objects.setInt32(critter, obj_f_critter_flags, flags);
}

/* 0x100B8AA0 */
void LegacyCritterSystem::CritterHpChanged(objHndl obj, objHndl assailant, int damAmt)
{
	
	d20Sys.d20SendSignal(obj, DK_SIG_HP_Changed, (int64_t)damAmt);
	if (critterSys.IsDeadNullDestroyed(obj)) {
		critterSys.Kill(obj, assailant);
	}

	if (objects.abilityScoreLevelGet(obj, stat_constitution, nullptr) <= 0) {
		if (!d20Sys.d20Query(obj, DK_QUE_Critter_Has_No_Con_Score)) {
			critterSys.Kill(obj, assailant);
		}
	}

	if (ShouldParalyzeByAbilityScore(obj)) {
		conds.AddTo(obj, "Paralyzed - Ability Score", {});
	}
}

 /* 0x1004E9F0 */
bool LegacyCritterSystem::ShouldParalyzeByAbilityScore(objHndl handle)
{
	// If another condition is causing helplessness, it will likely set a
	// score to 0, but adding paralysis would just be noise.
	if (d20Sys.d20Query(handle, DK_QUE_Helpless)) return false;

	for (auto stat = (int)stat_strength; stat <= stat_charisma; ++stat) {
		if (stat == stat_constitution) {
			continue; // negative CON kills, rather than paralyzes
		}
		if (objects.abilityScoreLevelGet(handle, (Stat)stat, nullptr) <= 0) {
			return true;
		}
	}
	return false;
}

/* 0x1001DAF0 */
int LegacyCritterSystem::GetHpDamage(objHndl handle)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj)
		return 0;
	return obj->GetInt32(obj_f_hp_damage);
}

/* 0x1001DBA0 */
void LegacyCritterSystem::SetHpDamage(objHndl handle, int damage)
{
	if (damage < 0)
		damage = 0;
	auto obj = objSystem->GetObject(handle);
	if (!obj) return;
	obj->SetInt32(obj_f_hp_damage, damage);
	if (damage > 0 && obj->IsNPC()) {
		static auto scheduleHealing = temple::GetRef<void(__cdecl)(objHndl, BOOL)>(0x1007F140);
		scheduleHealing(handle, FALSE);
	}
	static auto ui_unk_1009A3F0 = temple::GetRef<void(__cdecl)(objHndl)>(0x1009A3F0);
	ui_unk_1009A3F0(handle);
}

static_assert(temple::validate_size<StandPoint, 0x20>::value, "Invalid size");

void LegacyCritterSystem::SetStandPoint(objHndl critter, StandPointType type, const StandPoint& standpoint) {
	addresses.SetStandpoint(critter, type, &standpoint);
}

StandPoint LegacyCritterSystem::GetStandPoint(objHndl critter, StandPointType type) {
	StandPoint result;
	addresses.GetStandpoint(critter, type, &result);
	return result;
}


int LegacyCritterSystem::GetWaypointsCount(objHndl critter)
{
	return (int)objects.getArrayFieldInt64(critter, obj_f_npc_waypoints_idx, 0);
}

bool LegacyCritterSystem::GetWaypoint(objHndl critter, int index, Waypoint& wp)
{
	if (index >= GetWaypointsCount(critter))
		return false;
	WaypointPacked wpp;
	auto obj = objSystem->GetObject(critter);
	int64_t* wpRaw = (int64_t*)&wpp;
	for (int row = 0; row < sizeof(WaypointPacked) / sizeof(int64_t); row++)
	{
		*wpRaw = obj->GetInt64(obj_f_npc_waypoints_idx, 2 + index * 8 + row);
		wpRaw++;
	}
	wpp.AssignToWaypoint(wp);

	return true;
}

void LegacyCritterSystem::SetWaypointsCount(objHndl critter, int count)
{
	auto obj = objSystem->GetObject(critter);
	obj->ClearArray(obj_f_npc_waypoints_idx);
	if (count > 0) {
		obj->SetInt64(obj_f_npc_waypoints_idx, 0, count);
		for (int idx = 1; idx < count * sizeof(WaypointPacked) / sizeof(int64_t) + 2; idx++)
			obj->SetInt64(obj_f_npc_waypoints_idx, idx, 0);
	}
}

void LegacyCritterSystem::SetWaypoint(objHndl critter, int index, const Waypoint& wp)
{
	WaypointPacked wpp(wp);
	auto obj = objSystem->GetObject(critter);
	int64_t* wpRaw = (int64_t*)&wpp;
	for (int row = 0; row < sizeof(WaypointPacked) / sizeof(int64_t); row++)
	{
		obj->SetInt64(obj_f_npc_waypoints_idx, 2 + index * 8 + row, *wpRaw);
		wpRaw++;
	}
}

Dice LegacyCritterSystem::GetRacialHitDice(objHndl critter)
{
	if (!critter || !objSystem->IsValidHandle(critter)) {
		return Dice();
	}
	auto obj = objSystem->GetObject(critter);
	if (obj->IsNPC()) {
		return objects.GetHitDice(critter);
	}
	else {
		return d20RaceSys.GetHitDice(critterSys.GetRace(critter, false));
	}
}

void LegacyCritterSystem::GenerateHp(objHndl handle){
	if (!handle){
		return;
	}

	auto hpPts = 0;
	auto critterLvlIdx = 0;
	auto obj = objSystem->GetObject(handle);

	auto conMod = 0;
	if (!d20Sys.d20Query(handle, DK_QUE_Critter_Has_No_Con_Score)){
		auto conScore = objects.StatLevelGetBaseWithModifiers(handle, stat_constitution);
		conMod = objects.GetModFromStatLevel(conScore);
	}

	auto numLvls = obj->GetInt32Array(obj_f_critter_level_idx).GetSize();
	for (auto i=0u; i < numLvls; i++)
	{
		auto classType = (Stat)obj->GetInt32(obj_f_critter_level_idx, i);
		auto classHd = d20ClassSys.GetClassHitDice(classType);
		if (i == 0){
			hpPts = classHd; // first class level gets full HP
		}
		else
		{
			auto hdRoll = Dice::Roll(1, classHd, 0);
			auto cfgLower(tolower(config.hpOnLevelup));
			if (!_stricmp(cfgLower.c_str(), "max")) {
				hdRoll = classHd;
			}
			else if (!_stricmp(cfgLower.c_str(), "average")) {
				hdRoll = classHd/ 2 + rngSys.GetInt(0, 1); // hit die are always even numbered so randomize the roundoff
			}
			

			if (hdRoll + conMod < 1)
				hdRoll = 1 - conMod; // note: the con mod is applied separately! This just makes sure it doesn't dip to negatives
			hpPts += hdRoll;
		}
	}

	Dice racialHd = d20RaceSys.GetHitDice(critterSys.GetRace(handle, false));
	hpPts += racialHd.Roll();

	if (obj->IsNPC()){
		auto numDice = obj->GetInt32(obj_f_npc_hitdice_idx, 0);
		auto npcHd = Dice(numDice, obj->GetInt32(obj_f_npc_hitdice_idx, 1),obj->GetInt32(obj_f_npc_hitdice_idx, 2));
		auto npcHdVal = npcHd.Roll();

		auto cfgLower(tolower(config.HpForNPCHd));
		if ((!_stricmp(cfgLower.c_str(), "max")) || config.maxHpForNpcHitdice) {
			npcHdVal = numDice * npcHd.GetSides() + npcHd.GetModifier();
		} else if (!_stricmp(cfgLower.c_str(), "min")) {
			npcHdVal = numDice + npcHd.GetModifier();
		} else if (!_stricmp(cfgLower.c_str(), "average")) {
			npcHdVal = (numDice * (npcHd.GetSides() + 1)) / 2 + npcHd.GetModifier(); //Round to match monster manual numbers
		} else if (!_stricmp(cfgLower.c_str(), "threefourth")) {
			npcHdVal = static_cast<int>((static_cast<double>(numDice * (npcHd.GetSides())) * .75) + .5) + npcHd.GetModifier(); //Round
		}

		if (npcHdVal + conMod*numDice < 1)
			npcHdVal = numDice*(1 - conMod);
		hpPts += npcHdVal;
	}

	if (hpPts < 1)
		hpPts = 1;
	obj->SetInt32(obj_f_hp_pts, hpPts);

	//temple::GetRef<void(__cdecl)(objHndl)>(0x1007F720)(handle);
}

void LegacyCritterSystem::SetSubdualDamage(objHndl critter, int damage) {
	addresses.SetSubdualDamage(critter, damage);
}

void LegacyCritterSystem::AwardXp(objHndl critter, int xpAwarded) {
	auto xp = objects.getInt32(critter, obj_f_critter_experience);
	xp += xpAwarded;
	d20Sys.d20SendSignal(critter, DK_SIG_Experience_Awarded, xp, 0);
	objects.setInt32(critter, obj_f_critter_experience, xp);
}

void LegacyCritterSystem::BalorDeath(objHndl npc) {
	addresses.BalorDeath(npc);
}

void LegacyCritterSystem::SetConcealed(objHndl critter, int concealed) {
	addresses.SetConcealed(critter, concealed);
}

int LegacyCritterSystem::GetSubdualDamage(objHndl critter)
{
	auto obj = objSystem->GetObject(critter);
	if (!obj) return 0;

	return obj->GetInt32(obj_f_critter_subdual_damage);
}

void LegacyCritterSystem::SetConcealedWithFollowers(objHndl obj, int newState){
	SetConcealed(obj, newState);
	ObjList objList;
	objList.ListFollowers(obj);
	for (auto i=0; i<objList.size(); i++){
		auto follower = objList[i];
		if (!follower)
			continue;
		SetConcealed(follower, newState);
	}
}

bool LegacyCritterSystem::ShouldResurrect(objHndl critter, ResurrectType type) {
	// I think this is the intended check in the original. Seems like null should
	// still short circuit, though. TBD.
	if (!IsDeadNullDestroyed(critter)) return false;

	auto category = GetCategory(critter);
	auto protoid = objSystem->GetProtoId(critter);
	auto deathEffect = conds.GetByName("Killed By Death Effect");
	auto hd = objects.GetHitDiceNum(critter, false);
	auto con = objects.StatLevelGetBase(critter, stat_constitution);

	// Note: cases here intentionally fall through to avoid duplicating tests.
	switch (type)
	{
	case ResurrectType::CuthbertResurrect:
		return true; // gods do what they want
	case ResurrectType::RaiseDead:
		// creatures killed by death effects can't be raised
		if (d20Sys.d20QueryWithData(critter, DK_QUE_Critter_Has_Condition, deathEffect, 0) == 1)
			return false;
	case ResurrectType::Resurrect:
		// resurrect/raise costs 1 hit dice or 2 constitution; can't do it
		// if you can't pay.
		if (hd < 1 || hd == 1 && con < 3) return false;

		switch (category)
		{
		case mc_type_outsider:
			// Native outsiders can be raised/resurrected. Others can't.
			if (IsCategorySubtype(critter, mc_subtype_native)) break;

		case mc_type_elemental:
			return false;
		default:
			break;
		}
	case ResurrectType::ResurrectTrue:
		// Even true resurrection doesn't work on constructs or undead.
		// In principle you can true resurrect an undead creature to the
		// _original_ creature, but we'd need to know what that is.
		switch (category)
		{
		case mc_type_construct:
		case mc_type_undead:
			return false;
		}

		// all checks passed
		return true;

	default:
		return false;
	}
}

uint32_t LegacyCritterSystem::Resurrect(objHndl critter, ResurrectType type, int casterLvl) {
	uint32_t result = 0;
	if (ShouldResurrect(critter, type)) {
		result = 1;
		ResurrectApplyPenalties(critter, type);
	}
	d20Sys.d20SendSignal(critter, DK_SIG_Resurrection, 0, 0);
	return result;
}

void LegacyCritterSystem::ResurrectApplyPenalties(objHndl critter, ResurrectType type) {
	auto hd = objects.GetHitDiceNum(critter, false);
	bool damaged = false;
	int con = 0;
	CondStruct *negLevel;
	vector<int> negArgs({0, 0, 0});

	// Note: intentional fallthrough for common logic.
	switch (type)
	{
	case ResurrectType::RaiseDead:
		// Raise Dead causes a 50% chance of losing each prepared spell.
		//
		// Note: this will also affect Reincarnate, because it just applies
		// the same condition. The books don't indicate that it behaves this
		// way, so possibly revisit this later (with a new enum value perhaps).
		if (config.stricterRulesEnforcement) {
			spellSys.ForgetMemorized(critter, true, 50);
			spellSys.DeductSpontaneous(critter, static_cast<Stat>(-1), 50);
		}

		// Raise dead leaves the target with 1 hp/hit die
		damaged = true;
	case ResurrectType::Resurrect:
		// Raise and Resurrect reduce level/hit dice by 1 or reduce constitution.
		if (hd > 1) {
			// The level loss wasn't in the original game, so test for strict rules.
			if (config.stricterRulesEnforcement) {
				negLevel = conds.GetByName("Perm Negative Level");
				conds.AddTo(critter, negLevel, negArgs);
			} else {
				// The original game just took away XP.
				auto effLv = GetEffectiveDrainedLevel(critter);
				auto newXp = d20LevelSys.GetPenaltyXPForDrainedLevel(effLv-1);
				objects.setInt32(critter, obj_f_critter_experience, newXp);
			}
		} else {
			// The resurrection check should have already ensured this doesn't go
			// negative.
			con = objects.StatLevelGetBase(critter, stat_constitution);
			objects.StatLevelSetBase(critter, stat_constitution, con-2);
		}
	case ResurrectType::CuthbertResurrect:
	case ResurrectType::ResurrectTrue:
		// No negative consequences
		break;
	}

	// reset damage
	if (damaged) {
		auto maxHp = objects.StatLevelGet(critter, Stat::stat_hp_max);
		// hit dice might have been reduced
		hd = objects.GetHitDiceNum(critter, false);
		SetHpDamage(critter, maxHp - hd);
	} else {
		SetHpDamage(critter, 0);
	}
	gameSystems->GetAnim().PushAnimate(critter, 67);

	// Let the game know the critter has been resurrected, for e.g. plot purposes
	pythonObjIntegration.ExecuteObjectScript(critter, critter, ObjScriptEvent::Resurrect);
}

uint32_t LegacyCritterSystem::Dominate(objHndl critter, objHndl caster) {

	
	floatSys.FloatSpellLine(critter, 20018, FloatLineColor::Red);  // Float a "charmed!" line above the critter

	vector<int> args(3);

	args[0] = gameSystems->GetParticleSys().CreateAtObj("sp-Dominate Person", critter);
	args[1] = (caster.handle >> 32) & 0xFFFFFFFF;
	args[2] = caster.handle & 0xFFFFFFFF;

	auto cond = conds.GetByName("Dominate");
	return conds.AddTo(critter, cond, args);
}

bool LegacyCritterSystem::IsDeadNullDestroyed(objHndl critter)
{
	if (!critter) {
		return true;
	}

	if (!objSystem->IsValidHandle(critter))
	{
		return true;
	}

	auto flags = objects.GetFlags(critter);
	if (flags & OF_DESTROYED) {
		return true;
	}

	return objects.GetHPCur(critter) <= -10;
}

bool LegacyCritterSystem::IsDeadOrUnconscious(objHndl critter) {
	if (IsDeadNullDestroyed(critter)) {
		return true;
	}
	return d20Sys.d20Query(critter, DK_QUE_Unconscious) != 0;
}

bool LegacyCritterSystem::IsProne(objHndl critter)
{
	return d20Sys.d20Query(critter, DK_QUE_Prone) != 0;
}

CritterFlag LegacyCritterSystem::GetCritterFlags(objHndl critter)
{
	return (CritterFlag) objects.getInt32(critter, obj_f_critter_flags);
}

bool LegacyCritterSystem::IsMovingSilently(objHndl critter)
{
	auto flags = GetCritterFlags(critter);
	return (flags & OCF_MOVING_SILENTLY) == OCF_MOVING_SILENTLY;
}

void LegacyCritterSystem::SetMovingSilently(objHndl critter, BOOL newMovingSilState){
	temple::GetRef<void(__cdecl)(objHndl, BOOL)>(0x100805C0)(critter, newMovingSilState);
}

bool LegacyCritterSystem::IsCombatModeActive(objHndl critter)
{
	auto flags = GetCritterFlags(critter);
	return (flags & OCF_COMBAT_MODE_ACTIVE) == OCF_COMBAT_MODE_ACTIVE;
}

bool LegacyCritterSystem::IsConcealed(objHndl critter)
{
	auto flags = GetCritterFlags(critter);
	return (flags & OCF_IS_CONCEALED) == OCF_IS_CONCEALED;
}

int LegacyCritterSystem::GetPortraitId(objHndl leader) {
	return objects.getInt32(leader, obj_f_critter_portrait);
}

bool LegacyCritterSystem::CanSense(objHndl critter, objHndl tgt)
{
	return critterReplacements.CanSense(critter, tgt);
}

bool LegacyCritterSystem::CanSenseForSneakAttack(objHndl critter, objHndl tgt)
{
	auto uncanny = feats.HasFeatCountByClass(critter, FEAT_UNCANNY_DODGE);
	auto blindfight = feats.HasFeatCountByClass(critter, FEAT_BLIND_FIGHT);

	// for checking melee range
	auto reach = critterSys.GetNaturalReach(critter);
	auto dist = locSys.DistanceToObj(critter,tgt);

	if (uncanny || (blindfight && dist <= reach)) return true;

	return CanSense(critter, tgt);
}

int LegacyCritterSystem::GetSize(objHndl handle)
{
	return dispatch.DispatchGetSizeCategory(handle);
}

int LegacyCritterSystem::GetLevel(objHndl critter) {
	return objects.StatLevelGet(critter, stat_level);
}

int LegacyCritterSystem::SkillLevel(objHndl critter, SkillEnum skill){
	return dispatch.dispatch1ESkillLevel(critter, skill, nullptr, critter, 1);
}

Race LegacyCritterSystem::GetRace(objHndl critter, bool getBaseRace) {
	auto race = objects.StatLevelGet(critter, stat_race);
	if (!getBaseRace){
		race += objects.StatLevelGet(critter, stat_subrace) << 5;
	}
	return (Race)race;
}

Subrace LegacyCritterSystem::GetSubrace(objHndl critter) {
	return (Subrace)objects.StatLevelGet(critter, stat_subrace);
}

Gender LegacyCritterSystem::GetGender(objHndl critter) {
	return (Gender)objects.StatLevelGet(critter, stat_gender);
}

objHndl LegacyCritterSystem::GetPolymorphedHandle(objHndl handle)
{
	if (!handle) return objHndl::null;
	auto protoId = d20Sys.d20Query(handle, DK_QUE_Polymorphed);
	if (!protoId)
		return objHndl::null;
	
	auto protoHandle = objSystem->GetProtoHandle(protoId);
	return protoHandle;
}

std::string LegacyCritterSystem::GetHairStylePreviewTexture(HairStyle style)
{
	return GetHairStyleFile(style, "tga");
}

std::string LegacyCritterSystem::GetHairStyleModel(HairStyle style)
{
	return GetHairStyleFile(style, "skm");
}

gfx::EncodedAnimId LegacyCritterSystem::GetWeaponAnim(objHndl wielder, objHndl wpn, objHndl scnd, gfx::WeaponAnim animId)
{
	gfx::WeaponAnimType pAnimId = gfx::WeaponAnimType::Unarmed;
	gfx::WeaponAnimType sAnimId = gfx::WeaponAnimType::Unarmed;

	bool mainTwoHand = false;
	bool special = gfx::WeaponAnim::Special1 <= animId && animId <= gfx::WeaponAnim::Special3;

	if (wpn) {
		auto primaryType = objects.GetType(wpn);
		mainTwoHand = inventory.IsWieldedTwoHanded(wpn, wielder, special);
		if (ObjectType::obj_t_weapon == primaryType) {
			pAnimId = (gfx::WeaponAnimType)inventory.GetWeaponAnimId(wpn, wielder, special);
		} else if (ObjectType::obj_t_armor == primaryType) {
			pAnimId = gfx::WeaponAnimType::Shield;
		}
		if (mainTwoHand) {
			sAnimId = pAnimId;
		}
	}

	if (scnd && !mainTwoHand) {
		auto secondaryType = objects.GetType(scnd);
		if (ObjectType::obj_t_weapon == secondaryType) {
			sAnimId = (gfx::WeaponAnimType)inventory.GetWeaponAnimId(scnd, wielder, special);
		} else if (ObjectType::obj_t_armor == secondaryType) {
			sAnimId = gfx::WeaponAnimType::Shield;
		}
	}

	auto isMonk = feats.HasFeatCountByClass(wielder, FEAT_IMPROVED_UNARMED_STRIKE);
	if (gfx::WeaponAnimType::Unarmed == pAnimId && isMonk) {
		pAnimId = gfx::WeaponAnimType::Monk;
	}
	if (gfx::WeaponAnimType::Unarmed == sAnimId && isMonk) {
		sAnimId = gfx::WeaponAnimType::Monk;
	}

	return gfx::EncodedAnimId(animId, pAnimId, sAnimId);
}

gfx::EncodedAnimId LegacyCritterSystem::GetAnimId(objHndl critter, gfx::WeaponAnim anim)
{
	auto weaponPrim = GetWornItem(critter, EquipSlot::WeaponPrimary);
	auto weaponSec = GetWornItem(critter, EquipSlot::WeaponSecondary);
	if (!weaponSec) {
		weaponSec = GetWornItem(critter, EquipSlot::Shield);
	}

	return GetWeaponAnim(critter, weaponPrim, weaponSec, anim);
}

int LegacyCritterSystem::GetModelRaceOffset(objHndl obj, bool useBaseRace)
{
	// Meshes above 1000 are monsters, they dont get a creature type
	auto meshId = objects.getInt32(obj, obj_f_base_mesh);
	if (meshId >= 1000) {
		return -1;
	}

	auto race = GetRace(obj, useBaseRace);
	auto gender = GetGender(obj);
	bool isMale = (gender == Gender::Male);

	/*
		The following table comes from materials.mes (or materials_ext.mes), where
		the offsets into the materials and addmesh table are listed.
	*/
	int result = d20RaceSys.GetRaceMaterialOffset(race) + (isMale? 0:1);
	
	return result;
}

void LegacyCritterSystem::UpdateAddMeshes(objHndl obj)
{

	auto raceOffset = GetModelRaceOffset(obj);

	// For monsters, normal addmeshes are not processed
	if (raceOffset == -1) {
		return;
	}

	auto model(objects.GetAnimHandle(obj));

	if (!model) {
		return;
	}

	// Reset all existing add meshes
	model->ClearAddMeshes();

	// Do not process add meshes if the user is polymorphed
	if (d20Sys.d20Query(obj, DK_QUE_Polymorphed)) {
		return;
	}

	// Adjust the hair style size based on the worn helmet
	auto helmet = GetWornItem(obj, EquipSlot::Helmet);

	// This seems to be the helmet type (0 = small, 2 = large)
	int helmetType = 0;
	if (helmet) {
		auto wearFlags = objects.GetItemWearFlags(helmet);
		if (wearFlags & OIF_WEAR_HELMET) {
			helmetType = (objects.getInt32(helmet, obj_f_armor_flags) >> 2) & 3;
		}
	}

	auto robes = GetWornItem(obj, EquipSlot::Robes);

	for (int slotId = 0; slotId < (int) EquipSlot::Count; ++slotId) {
		auto slot = (EquipSlot)slotId;
		auto item = GetWornItem(obj, slot);
		if (!item) {
			continue;
		}

		// Armor is hidden by a robe
		if (robes && slot == EquipSlot::Armor) {
			continue;
		}

		// Addmesh / Material index of the item
		auto matIdx = objects.getInt32(item, obj_f_item_material_slot);
		if (matIdx == -1) {
			continue; // No assigned addmesh or material replacement
		}
		
		auto& addMeshes(GetAddMeshes(matIdx, raceOffset));

		if (!addMeshes.empty()) {
			model->AddAddMesh(addMeshes[0]);
		}

		// Only add the helmet part if there's no real helmet
		if (helmetType == 0 && addMeshes.size() >= 2) {
			model->AddAddMesh(addMeshes[1]);
			helmetType = 2; // The addmesh counts as a large helmet
		}

	}

	// Add the hair model
	auto packedHairStyle = objects.getInt32(obj, obj_f_critter_hair_style);
	HairStyle hairStyle{ packedHairStyle };

	// Modify the hair style size based on the used helmet, 
	// since it might cover it
	if (helmetType == 2) {
		// Large helmets remove most of the hair
		hairStyle.size = HairStyleSize::None;
	} else if (helmetType == 1) {
		// Smaller helmets only part of it
		hairStyle.size = HairStyleSize::Small;
	} else {
		hairStyle.size = HairStyleSize::Big;
	}

	auto hairModel{GetHairStyleModel(hairStyle)};
	if (!hairModel.empty()) {
		model->AddAddMesh(hairModel);
	}

	EvtObjAddMesh evtAddMesh(obj);
	auto addMeshesFromMods = evtAddMesh.DispatchGetAddMeshes();
	for (auto& it : addMeshesFromMods) {
		auto matIdx = it;
		auto& addMeshes(GetAddMeshes(matIdx, raceOffset));

		if (!addMeshes.empty()) {
			model->AddAddMesh(addMeshes[0]);
		}

		// Only add the helmet part if there's no real helmet
		if (helmetType == 0 && addMeshes.size() >= 2) {
			model->AddAddMesh(addMeshes[1]);
			helmetType = 2; // The addmesh counts as a large helmet
		}
	}
}

const std::vector<std::string>& LegacyCritterSystem::GetAddMeshes(int matIdx, int raceOffset) {

	static std::vector<std::string> sEmptyAddMeshes;
	
	// Lazily load the addmesh rules
	if (mAddMeshes.empty()) {
		auto mapping(MesFile::ParseFile("rules\\addmesh.mes"));

		// Extender files for rules/addmesh.mes
		{
			TioFileList addmeshFlist;
			tio_filelist_create(&addmeshFlist, "rules\\addmeshes\\*.mes");

			for (auto i = 0; i < addmeshFlist.count; i++) {

				std::string combinedFname(fmt::format("rules\\addmeshes\\{}", addmeshFlist.files[i].name));
				auto addmeshesMappingExt = MesFile::ParseFile(combinedFname);
				mapping.insert(addmeshesMappingExt.begin(), addmeshesMappingExt.end());
			}
		}

		for (auto &entry : mapping) {
			mAddMeshes[entry.first] = split(entry.second, ';', true);
		}
	}

	auto it = mAddMeshes.find(matIdx + raceOffset);
	if (it != mAddMeshes.end()) {
		return it->second;
	}

	return sEmptyAddMeshes;
}

void LegacyCritterSystem::ApplyReplacementMaterial(gfx::AnimatedModelPtr model, int mesId, int fallbackMesId)
{
	auto& replacementSet = tig->GetMdfFactory().GetReplacementSet(mesId, fallbackMesId);
	for (auto& entry : replacementSet) {
		model->AddReplacementMaterial(entry.first, entry.second);
	}
}

static const char *GetHairStyleRaceName(HairStyleRace race) {
	switch (race) {
	case HairStyleRace::Human:
		return "hu";
	case HairStyleRace::Dwarf:
		return "dw";
	case HairStyleRace::Elf:
		return "el";
	case HairStyleRace::Gnome:
		return "gn";
	case HairStyleRace::HalfElf:
		return "he";
	case HairStyleRace::HalfOrc:
		return "ho";
	case HairStyleRace::Halfling:
		return "hl";
	default:
		throw TempleException("Unsupported hair style race: {}", (int)race);
	}
}

static const char *GetHairStyleSizeName(HairStyleSize size) {
	switch (size) {
	case HairStyleSize::Big:
		return "big";
	case HairStyleSize::Small:
		return "small";
	case HairStyleSize::None:
		return "none";
	default:
		throw TempleException("Unsupported hair style size: {}", (int)size);
	}
}

// Gets the race that should be fallen back to if there's 
// no model for the given race. If it returns race again
// there's no more fallback
static HairStyleRace GetHairStyleFallbackRace(HairStyleRace race) {
	switch (race) {
	case HairStyleRace::Human:
		return HairStyleRace::Human;
	case HairStyleRace::Dwarf:
		return HairStyleRace::Dwarf;
	case HairStyleRace::Elf:
		return HairStyleRace::Human;
	case HairStyleRace::Gnome:
		return HairStyleRace::Human;
	case HairStyleRace::HalfElf:
		return HairStyleRace::Human;
	case HairStyleRace::HalfOrc:
		return HairStyleRace::HalfOrc;
	case HairStyleRace::Halfling:
		return HairStyleRace::Human;
	default:
		throw TempleException("Unsupported hair style race: {}", (int)race);
	}
}

static HairStyleSize GetHairStyleFallbackSize(HairStyleSize size) {
	switch (size) {
	case HairStyleSize::Big:
		return HairStyleSize::Small;
	case HairStyleSize::Small:
		return HairStyleSize::None;
	case HairStyleSize::None:
		return HairStyleSize::None;
	default:
		throw TempleException("Unsupported hair style size: {}", (int)size);
	}
}

std::string LegacyCritterSystem::GetHairStyleFile(HairStyle style, const char * extension)
{
	const char *genderShortName;
	const char *genderDir;
	if (style.gender == Gender::Male) {
		genderShortName = "m";
		genderDir = "male";
	} else {
		genderShortName = "f";
		genderDir = "female";
	}
	
	// These will be modified if a fallback is needed, so copy them here
	auto race = style.race;
	auto size = style.size;
	auto styleNr = style.style;

	while (true)
	{
		while (true)
		{
			while (true)
			{
				auto filename(fmt::format("art\\meshes\\hair\\{}\\s{}\\{}_{}_s{}_c{}_{}.{}",
					genderDir,
					style.style,
					GetHairStyleRaceName(race),
					genderShortName,
					style.style,
					style.color,
					GetHairStyleSizeName(size),
					extension
					));
				
				if (vfs->FileExists(filename))
					return filename;

				// Fall back to a compatible race, if no model for this 
				// race exists
				auto fallbackRace = GetHairStyleFallbackRace(race);
				if (fallbackRace == race) {
					break;
				}
				race = fallbackRace;
			}

			// Fall back to a smaller size of the model
			auto fallbackSize = GetHairStyleFallbackSize(size);
			if (fallbackSize == size) {
				break;
			}
			size = fallbackSize;
		}

		// Fall back to style 5 if no other options exist
		if (styleNr == 5)
			break;
		styleNr = 5;
	}

	return std::string();
}

// Originally @ 1007E9D0
void LegacyCritterSystem::UpdateModelEquipment(objHndl obj)
{
	if (mSuspendModelUpdate)
		return;

	UpdateAddMeshes(obj);
	auto raceOffset = GetModelRaceOffset(obj, false);
	if (raceOffset == -1) {
		return;
	}

	auto model = objects.GetAnimHandle(obj);
	if (!model) {
		return; // No animation really present
	}

	// This is a bit shit but since AAS will just splice the
	// add meshes into the list of model parts, 
	// we have to reset the render buffers
	model->SetRenderState(nullptr);

	// Apply the naked replacement materials for
	// equipment slots that support them
	ApplyReplacementMaterial(model, 0 + raceOffset); // Helmet
	ApplyReplacementMaterial(model, 100 + raceOffset); // Cloak
	ApplyReplacementMaterial(model, 200 + raceOffset); // Gloves
	ApplyReplacementMaterial(model, 500 + raceOffset); // Armor
	ApplyReplacementMaterial(model, 800 + raceOffset); // Boots

	// Now apply it for the actual equipment
	auto baseRaceOffset = GetModelRaceOffset(obj, true); // use base race for equipment because holy crap ToEE has an entry for each race!!!
	for (uint32_t slotId = 0; slotId < (uint32_t)EquipSlot::Count; ++slotId) {
		auto slot = (EquipSlot) slotId;
		auto item = GetWornItem(obj, slot);
		if (item) {
			auto materialSlot = objects.getInt32(item, obj_f_item_material_slot);
			if (materialSlot != -1) {
				ApplyReplacementMaterial(model, materialSlot + raceOffset, materialSlot + baseRaceOffset);
			}
		}
	}
}

void LegacyCritterSystem::SuspendModelUpdate(bool state)
{
	mSuspendModelUpdate = state;
}

void LegacyCritterSystem::AddNpcAddMeshes(objHndl obj)
{

	auto id = objects.getInt32(obj, obj_f_npc_add_mesh);
	auto model = objects.GetAnimHandle(obj);

	for (auto &addMesh : GetAddMeshes(id, 0)) {
		model->AddAddMesh(addMesh);
	}

}

objHndl LegacyCritterSystem::GiveItem(objHndl critter, int protoId) {
	return addresses.GiveItem(critter, protoId);
}

void LegacyCritterSystem::TakeMoney(objHndl critter, int platinum, int gold, int silver, int copper)
{
	addresses.TakeMoney(critter, platinum, gold, silver, copper);
}

void LegacyCritterSystem::GiveMoney(objHndl critter, int platinum, int gold, int silver, int copper)
{
	addresses.GiveMoney(critter, platinum, gold, silver, copper);
}

int LegacyCritterSystem::NumOffhandExtraAttacks(objHndl critter)
{
	if (feats.HasFeatCount(critter, FEAT_GREATER_TWO_WEAPON_FIGHTING)
		|| feats.HasFeatCountByClass(critter, FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, (Stat)0, 0))
	{
		return 3;
	}

	if (feats.HasFeatCount(critter, FEAT_IMPROVED_TWO_WEAPON_FIGHTING)
		|| feats.HasFeatCountByClass(critter, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, (Stat)0,0))
		return 2;

	return 1;
}

int LegacyCritterSystem::IsWearingLightOrNoArmor(objHndl critter)
{
	objHndl armor = inventory.ItemWornAt(critter, 5);
	if (armor)
	{
		int armorFlags = objects.getInt32(armor, obj_f_armor_flags);
		if (inventory.GetArmorType(armorFlags) != ARMOR_TYPE_NONE	&& inventory.GetArmorType(armorFlags) != ARMOR_TYPE_LIGHT)
		{
			return 0;
		}
	}
	return 1;
}

bool LegacyCritterSystem::IsLootableCorpse(objHndl critter)
{	
	if (!objects.IsCritter(critter)) {
		return false;
	}

	if (!critterSys.IsDeadNullDestroyed(critter)) {
		return false; // It's still alive
	}

	// Find any item in the critters inventory that would be considered lootable
	auto invenCount = objects.getInt32(critter, obj_f_critter_inventory_num);
	for (size_t i = 0; i < invenCount; ++i) {
		auto item = objects.getArrayFieldObj(critter, obj_f_critter_inventory_list_idx, i);
		auto invLocation = objects.GetItemInventoryLocation(item);

		auto itemFlags = objects.GetItemFlags(item);
		if (itemFlags & OIF_NO_LOOT) {
			continue; // Flagged as unlootable
		}

		//if (inventory.IsInvIdxWorn(invLocation) ) {
		//	continue; // Currently equipped on the corpse
		//} // removing this condition - why should worn items be excluded??

		

		return true; // Found an item that is lootable
	}

	return false;
}

bool LegacyCritterSystem::CanBarbarianRage(objHndl obj)
{
	if (objects.StatLevelGet(obj, stat_level_barbarian) <= 0)
		return false;
	
	auto isRagedAlready = d20Sys.d20Query(obj, DK_QUE_Barbarian_Raged);
	if (isRagedAlready)
		return false;
	auto isFatigued = d20Sys.d20Query(obj, DK_QUE_Barbarian_Fatigued);
	if (isFatigued)
		return false;
	return true;
}

int LegacyCritterSystem::GetCritterMap(objHndl critter)
{
	return addresses.GetCritterMap(critter);
}

MonsterCategory LegacyCritterSystem::GetCategory(objHndl objHnd)
{
	if (objHnd && objects.IsCritter(objHnd)) {
		auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
		return (MonsterCategory)(monCat & 0xFFFFFFFF);
	}
	return mc_type_monstrous_humanoid; // default - so they have at least a weapons proficiency
}

MonsterSubcategoryFlag LegacyCritterSystem::GetSubcategoryFlags(objHndl objHnd){
	if (objHnd && objects.IsCritter(objHnd)) {
		auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
		auto moncatSubtype = static_cast<MonsterSubcategoryFlag>(monCat >> 32);

		return moncatSubtype;
	}
	return (MonsterSubcategoryFlag)0;
}

uint32_t LegacyCritterSystem::IsCategoryType(objHndl objHnd, MonsterCategory categoryType){
	if (objHnd && objects.IsCritter(objHnd)) {
		auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
		return (monCat & 0xFFFFFFFF) == categoryType;
	}
	return 0;
}

uint32_t LegacyCritterSystem::IsCategorySubtype(objHndl objHnd, MonsterSubcategoryFlag categoryType){
	if (objHnd && objects.IsCritter(objHnd)) {
		auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
		auto moncatSubtype = static_cast<MonsterSubcategoryFlag>(monCat >> 32);
		return (moncatSubtype & categoryType) == categoryType;
	}
	return 0;
}

uint32_t LegacyCritterSystem::IsUndead(objHndl objHnd){
	return IsCategoryType(objHnd, mc_type_undead);
}

uint32_t LegacyCritterSystem::IsOoze(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_ooze);
}

uint32_t LegacyCritterSystem::IsSubtypeFire(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtype_fire);
}

uint32_t LegacyCritterSystem::IsSubtypeAir(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtype_air);
}

uint32_t LegacyCritterSystem::IsSubtypeEarth(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtype_earth);
}

uint32_t LegacyCritterSystem::IsSubtypeWater(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtype_water);
}

// Standard reach for 'tall' creatures.
//
// Note: this is reach beyond the actual 'occupied' space, which is
// probably underestimated by ToEE.
int StdReachForSize(int size)
{
	if (size < 4) return 0;
	if (size < 9) return 5 * std::max(1, size - 4);
	return 20 + 10*(size - 8);
}

float ReachForSize(int adjustedSize)
{
	// so that tiny creatures still threaten the 'square' they occupy.
	if (adjustedSize <= 3) return 2.5;
	else if (adjustedSize <= 5) return 5.0;
	else if (adjustedSize == 6) return 10.0;
	else if (adjustedSize == 7) return 15.0;
	// in case you somehow have a collossal creature with bonus reach
	else return 20.0 + 10.0*(adjustedSize-8);
}

// Tries to infer an adjustment to the creature's size that makes the
// standard reach match their actual reach.
int DetermineReachOffset(int baseReach, int baseSize)
{
	auto exReach = StdReachForSize(baseSize);
	auto guess = (baseReach - exReach) / 5;
	auto diff = baseReach - StdReachForSize(baseSize + guess);

	if (diff >= 5) return guess+1;
	else if (diff <= -5) return guess-1;
	else return guess;
}

float LegacyCritterSystem::GetNaturalReach(objHndl obj)
{
	objHndl critter = obj;

	// Temple+: fixed wildshape reach
	auto protoId = d20Sys.d20Query(obj, DK_QUE_Polymorphed);
	if (protoId) {
		auto polyHandle = gameSystems->GetObj().GetProtoHandle(protoId);
		if (polyHandle) critter = polyHandle;
	}

	int ireach = objects.getInt32(critter, obj_f_critter_reach);
	int curSize = objects.GetSize(obj, false);

	if (0 == ireach) return ReachForSize(curSize);

	int baseSize = objects.GetSize(obj, true);
	float reach = static_cast<float>(ireach);

	if (baseSize == curSize) return reach;

	int extraOff = DetermineReachOffset(ireach, baseSize);

	return ReachForSize(curSize + extraOff);
}

/* 0x100B52D0 */
float LegacyCritterSystem::GetReach(objHndl obj, D20ActionType actType, /*added in Temple+:*/ float* minReach ) {

	float naturalReach = GetNaturalReach(obj);

	float weaponReach = 0.0f;
	float weaponMinReach = 0.0f;
	if (actType != D20A_TOUCH_ATTACK)
	{
		objHndl weapon = inventory.GetItemAtInvIdx(obj, 203);
		// todo: handle cases where enlarged creatures dual wield polearms ><
		// todo: enlarge effects on reach weapon
		if (weapon){
			auto weapType = objects.GetWeaponType(weapon);
			if (weapons.IsReachWeaponType(weapType)){
				weaponReach = 5.0f;
				if (weapType != wt_spike_chain) {
					weaponMinReach = weaponReach;
				}
			}
			
		}
	}

	float radius = locSys.InchesToFeet(objects.GetRadius(obj));
	float offset = std::min(2.0f, radius);

	auto maxReach = weaponReach + naturalReach - offset;
	
	// Temple+: added polearm minimum reach
	if (minReach) {
		*minReach = max(0.0f, weaponMinReach - offset);
	}
	return maxReach;
}

int LegacyCritterSystem::GetBonusFromSizeCategory(int sizeCategory)
{
	int result = sizeCategory;
	switch (sizeCategory)
	{
	case 1:
		result = 8;
		break;
	case 2:
		result = 4;
		break;
	case 3:
		result = 2;
		break;
	case 4:
		result = 1;
		break;
	case 5:
		result = 0;
		break;
	case 6:
		result = -1;
		break;
	case 7:
		result = -2;
		break;
	case 8:
		result = -4;
		break;
	case 9:
		result = -8;
		break;
	default:
		result = sizeCategory;
		break;
	}
	return result;
}

int LegacyCritterSystem::GetDamageIdx(objHndl obj, int attackIdx)
{
	int n =0;
	for (int i = 0; i < 4; i++)
	{
		n += objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, i);
		if (n > attackIdx)
			return i;
	}
	return 0;
}

int LegacyCritterSystem::GetNumNaturalAttacks(objHndl handle) {
	if (!objSystem->IsValidHandle(handle)) {
		return 0;
	}
	auto obj = objSystem->GetObject(handle);
	int result = 0;
	int attacksCount;
	for (int i = 0; i < 4; i++)
	{
		attacksCount = obj->GetInt32(obj_f_critter_attacks_idx, i);
		if (attacksCount > 0)
			result += attacksCount;
	}
	return result;
}

int LegacyCritterSystem::GetCritterDamageDice(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_critter_damage_idx, damageIdx);

}

DamageType LegacyCritterSystem::GetCritterAttackDamageType(objHndl obj, int attackIdx)
{
	DamageType damType[7];
	damType[0] = DamageType::SlashingAndBludgeoningAndPiercing;
	damType[1] = DamageType::PiercingAndSlashing;
	damType[2] = DamageType::PiercingAndSlashing;
	damType[3] = DamageType::Piercing;
	damType[4] = DamageType::Bludgeoning;
	damType[5] = DamageType::Bludgeoning;
	damType[6] = DamageType::Piercing;
	int damageIdx = GetDamageIdx(obj, attackIdx);
	int x = objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
	if (x > 6 || x < 0)
		return DamageType::Bludgeoning;
	return damType[x];
}

bool LegacyCritterSystem::IsSleeping(objHndl hndl)
{
	if (!hndl)
		return false;
	auto obj = objSystem->GetObject(hndl);
	if (!obj->IsCritter())
		return false;
	return ( obj->GetInt32(obj_f_critter_flags) & CritterFlag::OCF_SLEEPING) != 0;
	}

int LegacyCritterSystem::GetHpPercent(const objHndl& handle)
{
	auto maxHp = objects.StatLevelGet(handle, Stat::stat_hp_max);
	auto curHp = objects.GetHPCur(handle);
	auto subdualDam = objects.StatLevelGet(handle, Stat::stat_subdual_damage);
	if (maxHp <= 0)
		return 0;
	return 100 * (curHp - subdualDam) / maxHp;
}

int LegacyCritterSystem::GetEffectiveLevel(objHndl & objHnd)
{
	if (!objHnd) return 1;
	auto lvl = objects.StatLevelGet(objHnd, stat_level);
	auto lvlAdj = d20RaceSys.GetLevelAdjustment(objHnd);
	auto racialHdCount = 0;
	if (objSystem->GetObject(objHnd)->IsPC()) {
		Dice racialHd = d20RaceSys.GetHitDice(critterSys.GetRace(objHnd, false));
		racialHdCount = racialHd.GetCount();
	}
	else { // NPC
		racialHdCount = objSystem->GetObject(objHnd)->GetInt32(obj_f_npc_hitdice_idx, 0);
	}
	lvl += lvlAdj + racialHdCount;
	return lvl;
}

int LegacyCritterSystem::GetEffectiveDrainedLevel(objHndl & critter, LevelDrainType incl)
{
	if (!critter) return 0;

	LevelDrainType omit = ~incl;
	auto lvl = dispatch.Dispatch61GetLevel(critter, stat_level, nullptr, objHndl::null, omit);
	auto lvlAdj = d20RaceSys.GetLevelAdjustment(critter);
	auto racialHdCount = 0;
	auto ocritter = objSystem->GetObject(critter);

	if (ocritter->IsPC()) {
		Dice racialHd = d20RaceSys.GetHitDice(critterSys.GetRace(critter, false));
		racialHdCount = racialHd.GetCount();
	} else {
		racialHdCount = ocritter->GetInt32(obj_f_npc_hitdice_idx, 0);
	}
	return lvl + lvlAdj + racialHdCount;
}

int LegacyCritterSystem::GetCritterAttackType(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
}

int LegacyCritterSystem::GetRacialAttackBonus(objHndl critter)
{
	//if (obj->type != obj_t_npc){
	//  return 0;
	//}

	auto racialHd = critterSys.GetRacialHitDice(critter).GetCount();
	//auto racialHd  = obj->GetInt32(obj_f_npc_hitdice_idx, 0);
	
	auto moncat = critterSys.GetCategory(critter);
	switch (moncat)
	{
	case mc_type_aberration:
	case mc_type_animal:
	case mc_type_beast:
	case mc_type_construct:
	case mc_type_elemental:
	case mc_type_giant:
	case mc_type_humanoid:
	case mc_type_ooze:
	case mc_type_plant:
	case mc_type_shapechanger:
	case mc_type_vermin:
		return (3 * racialHd / 4);


	case mc_type_dragon:
	case mc_type_magical_beast:
	case mc_type_monstrous_humanoid:
	case mc_type_outsider:
		return racialHd;

	case mc_type_fey:
	case mc_type_undead:
		return racialHd / 2;

	default: break;
	}
	
	return 0;
}

int LegacyCritterSystem::GetBaseAttackBonus(const objHndl& handle, Stat classBeingLeveled){

	
	auto bab = 0;
	for (auto it: d20ClassSys.classEnums){
		auto classLvl = objects.StatLevelGet(handle, (Stat)it);
		if (classBeingLeveled == it)
			classLvl++;
		bab += d20ClassSys.GetBaseAttackBonus((Stat)it, classLvl);
	}

	// get BAB from NPC HD
	auto racialBab = GetRacialAttackBonus(handle);

	return bab + racialBab;
}

int LegacyCritterSystem::GetAttackBonus(const objHndl& handle, D20CAF flags) {
	DispIoAttackBonus dispIo;
	dispIo.attackPacket.flags = flags;
	return dispatch.DispatchAttackBonus(handle, objHndl::null, &dispIo, dispTypeToHitBonus2, 0);
}

int LegacyCritterSystem::GetSpellLvlCanCast(const objHndl& handle, SpellSourceType spellSourceType, SpellReadyingType spellReadyingType){
	
	return 0;
}

int LegacyCritterSystem::GetSpellEnumsKnownByClass(const objHndl& handle, int spellClass, int* spellEnums, int capacity){
	// todo: PrC extension
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto spKnown = obj->GetSpellArray(obj_f_critter_spells_known_idx);
	auto n = 0;
	for (auto i = 0u; i < spKnown.GetSize(); i++){
		auto &spData = spKnown[i];
		if (spData.classCode == spellClass){
			spellEnums[n++] = spData.spellEnum;
			if (n >= capacity){
				logger->warn("GetSpellEnumsKnown: Too many spells known! It's over 802!");
				return n;
			}
				
		}
	}
	return n;
}

int LegacyCritterSystem::GetCritterNumNaturalAttacks(objHndl obj){
	auto n = 0;

	auto critterAttacks = gameSystems->GetObj().GetObject(obj)->GetInt32Array(obj_f_critter_attacks_idx);
	for (auto i = 0u; i < critterAttacks.GetSize() && i < 3; i++)	{
		auto a = critterAttacks[i];
		if (a > 0)
			n += a;
	}

	return n;
}

bool LegacyCritterSystem::IsWarded(objHndl obj)
{
	auto args = PyTuple_New(1);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(obj));
	

	auto result = pythonObjIntegration.ExecuteScript("combat", "IsWarded", args);
	int isWarded = PyInt_AsLong(result);
	Py_DECREF(result);
	Py_DECREF(args);
	
	return isWarded != 0;
}

bool LegacyCritterSystem::IsSummoned(objHndl obj)
{
	// TODO
	return 0;
}

int LegacyCritterSystem::GetCasterLevel(objHndl obj){
	int result = 0;
	for (auto it: d20ClassSys.classEnums){
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsCastingClass(classEnum))	{
			auto cl = critterSys.GetCasterLevelForClass(obj, classEnum);
			if (cl > result)
				result = cl;
		}
	}
	return result;
}

int LegacyCritterSystem::GetCasterLevelForClass(objHndl handle, Stat classCode){
	auto baseCasterLevel = dispatch.DispatchGetBaseCasterLevel(handle, classCode);
	auto casterLevel = dispatch.DispatchGetCasterLevelStage2(handle, classCode, baseCasterLevel); // currently just for Practiced Spellcaster
	return casterLevel;
}

int LegacyCritterSystem::GetSpellListLevelExtension(objHndl handle, Stat classCode)
{
	return dispatch.DispatchSpellListLevelExtension(handle, classCode);
}

int LegacyCritterSystem::GetSpellListLevelForClass(objHndl handle, Stat classCode)
{
	auto base = objects.StatLevelGet(handle, classCode);
	auto extension = GetSpellListLevelExtension(handle, classCode);
	return base + extension;
}

bool LegacyCritterSystem::IsCaster(objHndl obj)
{
	return GetCasterLevel(obj) > 0;
}

bool LegacyCritterSystem::IsWieldingRangedWeapon(objHndl obj)
{
	auto weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!weapon)
	{
		weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	}
	if (!weapon)
		return 0;
	return (objects.getInt32(weapon, obj_f_weapon_flags) & OWF_RANGED_WEAPON ) != 0;
}

void LegacyCritterSystem::GetOkayVoiceLine(objHndl obj, objHndl fellow, char* text, int* soundId)
{
	addresses.GetCritterVoiceLine(obj, fellow, text, soundId);
}


int LegacyCritterSystem::PlayCritterVoiceLine(objHndl obj, objHndl fellow, char* text, int soundId)
{
	return addresses.PlayCritterVoiceLine(obj, fellow, text, soundId);
}

bool LegacyCritterSystem::HashMatchingClassForSpell(objHndl handle, uint32_t spellEnum) const
{
	//return temple::GetRef<BOOL(__cdecl)(objHndl, uint32_t)>(0x10075DA0)(handle, spellEnum) == TRUE;
	SpellEntry spEntry(spellEnum);
	for (auto i=0u; i<spEntry.spellLvlsNum; i++){
		auto &lvlSpec = spEntry.spellLvls[i];
		
		// domain spell
		if (spellSys.isDomainSpell(lvlSpec.spellClass)){
			
			auto obj = objSystem->GetObject(handle);
			
			// if is Cleric or NPC and the spell spec is Domain Special
			if (objects.StatLevelGet(handle, stat_level_cleric) > 0 
				|| (obj->IsNPC() && lvlSpec.spellClass == Domain_Special)){

				if (objects.StatLevelGet(handle, stat_domain_1) == lvlSpec.spellClass
					|| objects.StatLevelGet(handle, stat_domain_2) == lvlSpec.spellClass)
					return true;
				}
			
		} 
		// normal spell
		else{
			if (objects.StatLevelGet(handle, spellSys.GetCastingClass(lvlSpec.spellClass)) > 0)
				return true;
		}
	}

	auto spExtFind = spellSys.mSpellEntryExt.find(spellEnum);
	if (spExtFind != spellSys.mSpellEntryExt.end()) {
		for (auto lvlSpec : spExtFind->second.levelSpecs) {
			// TODO: domain extension for PrCs (Domain Wizard?)
			{
				if (objects.StatLevelGet(handle, spellSys.GetCastingClass(lvlSpec.spellClass)) > 0)
					return true;
			}
			
		}
	}

	// Check for an Advanced Learning Class (character will know the spell but it will not be on their list standard)
	bool bHasAdvancedLearning = false;
	auto advancedLearningClasses = spellSys.GetClassesWithAdvancedLearning();
	for (auto classEnum : advancedLearningClasses) {
		if (objects.StatLevelGet(handle, classEnum) > 0) {
			bHasAdvancedLearning = true;
		}
	}

	if (bHasAdvancedLearning) {
		if (spellSys.IsSpellKnown(handle, spellEnum)) {
			return true;
		}
	}

	return false;
}

int LegacyCritterSystem::GetArmorClass(objHndl obj, DispIoAttackBonus* dispIo){
	return dispatch.DispatchAttackBonus(obj, objHndl::null, dispIo, dispTypeGetAC, DK_NONE);
}

int LegacyCritterSystem::GetRacialSavingThrowBonus(objHndl handle, SavingThrowType saveType)
{
	if (!handle || !objSystem->IsValidHandle(handle))
		return 0;

	auto obj = objSystem->GetObject(handle);
	
	if (obj->IsNPC()) {
		auto npcSaveBonus = 0;
		auto validType = false;
		switch (saveType) {
		case SavingThrowType::Fortitude:
			npcSaveBonus = obj->GetInt32(obj_f_npc_save_fortitude_bonus);
			validType = true;
			break;
		case SavingThrowType::Reflex:
			npcSaveBonus = obj->GetInt32(obj_f_npc_save_reflexes_bonus);
			validType = true;
			break;
		case SavingThrowType::Will:
			npcSaveBonus = obj->GetInt32(obj_f_npc_save_willpower_bonus);
			validType = true;
			break;
		default:
			break;
		}
		if (validType) {
			return npcSaveBonus;
		}
		else {
			logger->error("GetRacialSavingThrowBonus(): Bad save type parameter");
		}
	}
	if (obj->IsPC()) {
		return d20RaceSys.GetSavingThrowBonus( critterSys.GetRace(handle, false) ,saveType);
	}
	return 0;
}

FightingStyle operator|(FightingStyle l, FightingStyle r)
{
	return (FightingStyle)((uint32_t)l | (uint32_t)r);
}

FightingStyle operator&(FightingStyle l, FightingStyle r)
{
	return (FightingStyle)((uint32_t)l & (uint32_t)r);
}

bool LegacyCritterSystem::CanTwoWeaponFight(objHndl critter)
{
	auto weapr = inventory.ItemWornAt(critter, EquipSlot::WeaponPrimary);
	auto weapl = inventory.ItemWornAt(critter, EquipSlot::WeaponSecondary);

	bool result = !!weapl;
	result |= d20Sys.d20Query(critter, DK_QUE_Can_Shield_Bash);
	result |= inventory.IsDoubleWeapon(weapr);

	return result;
}

objHndl LegacyCritterSystem::GetRightWield(objHndl critter)
{
	return inventory.ItemWornAt(critter, EquipSlot::WeaponPrimary);
}

objHndl LegacyCritterSystem::GetLeftWield(objHndl critter)
{
	auto weapl = inventory.ItemWornAt(critter, EquipSlot::WeaponSecondary);

	if (!weapl && d20Sys.d20Query(critter, DK_QUE_Can_Shield_Bash)) {
		weapl = inventory.ItemWornAt(critter, EquipSlot::Shield);
	} else {
		auto weapr = inventory.ItemWornAt(critter, EquipSlot::WeaponPrimary);
		if (inventory.IsDoubleWeapon(weapr))
			weapl = weapr;
	}

	return weapl;
}

objHndl LegacyCritterSystem::GetPrimaryWield(objHndl critter)
{
	auto weapr = GetRightWield(critter);
	auto weapl = GetLeftWield(critter);

	if (weapl && d20Sys.d20Query(critter, DK_QUE_Left_Is_Primary))
		return weapl;
	else
		return weapr;
}

objHndl LegacyCritterSystem::GetSecondaryWield(objHndl critter)
{
	auto weapr = GetRightWield(critter);
	auto weapl = GetLeftWield(critter);

	if (weapl && d20Sys.d20Query(critter, DK_QUE_Left_Is_Primary))
		return weapr;
	else
		return weapl;
}

FightingStyle LegacyCritterSystem::GetFightingStyle(objHndl critter)
{
	if (!critter || !objSystem->IsValidHandle(critter))
		return FightingStyle::Unknown;

	auto weapp = GetPrimaryWield(critter);
	auto weaps = GetSecondaryWield(critter);
	bool twfEnabled = false;

	// query behavior: 0 - no toggle, 1 - toggle off, 2 - toggle on
	switch (d20Sys.d20Query(critter, DK_QUE_Is_Two_Weapon_Fighting))
	{
	case 0:
		// if the toggle is missing we need to preserve the old behavior:
		// twf is enabled if you are wielding two different _weapons_.
		if (!weapp || !weaps) break;
		if (weapp == weaps) break;

		twfEnabled = objects.GetType(weaps) == obj_t_weapon;
		break;
	case 2:
		twfEnabled = true;
		break;
	default:
		break;
	}

	FightingStyle style = FightingStyle::OneHanded;

	if (weapp) {
		auto rangep = OWF_RANGED_WEAPON & objects.getInt32(weapp, obj_f_weapon_flags);
		if (weaps && twfEnabled) {
			// Note: I'm not sure there's actually any way to dual wield
			// with a ranged weapon. I don't think throwing daggers count,
			// and they might be the only candidate that doesn't occupy both
			// hands.
			auto ranges = OWF_RANGED_WEAPON & objects.getInt32(weaps, obj_f_weapon_flags);
			if (rangep == ranges) {
				style = FightingStyle::TwoWeapon;
			} else {
				style = FightingStyle::OneHanded;
			}
		} else if (inventory.IsWieldedTwoHanded(weapp, critter)) {
			style = FightingStyle::TwoHanded;
		} else {
			style = FightingStyle::OneHanded;
		}

		// Ranged status follows primary weapon; either the secondary
		// matches, or the above logic will forbid two weapon fighting.
		if (rangep)
			style = style | FightingStyle::Ranged;
	}
	// if no primary weapon, we're in one handed unarmed mode

	return style;
}

bool LegacyCritterSystem::OffhandIsLight(objHndl critter)
{
	if (!critter || !objSystem->IsValidHandle(critter))
		return false;

	auto weapp = GetPrimaryWield(critter);
	auto weaps = GetSecondaryWield(critter);

	if (weaps && weapp != weaps) {
		// if we're actually wielding two weapons, return whether the
		// secondary is light
		return inventory.GetWieldType(critter, weaps, true) == 0;
	}

	// Otherwise, we're wielding a double weapon, or the offhand is
	// null, either of which count as light.
	return true;
}

int LegacyCritterSystem::SkillBaseGet(objHndl handle, SkillEnum skill){
	if (!handle)
		return 0;
	return gameSystems->GetObj().GetObject(handle)->GetInt32(obj_f_critter_skill_idx, skill) / 2;
}

int LegacyCritterSystem::SpellNumByFieldAndClass(objHndl obj, obj_f field, uint32_t spellClassCode)
{
	auto objBody = gameSystems->GetObj().GetObject(obj);
	auto spellArray = objBody->GetSpellArray(field);
	int spellArrayNum = spellArray.GetSize();
	
	int numSpells = 0;
	for (int i = 0; i < spellArrayNum; i++){
		auto spArrayClassCode = spellArray[i].classCode;
		if (spArrayClassCode == spellClassCode)
			numSpells++;
	}
	return numSpells;

}

int LegacyCritterSystem::DomainSpellNumByField(objHndl obj, obj_f field)
{
	auto objBody = gameSystems->GetObj().GetObject(obj);
	auto spellArray = objBody->GetSpellArray(field);
	int spellArrayNum = spellArray.GetSize();

	int numSpells = 0;
	for (int i = 0; i < spellArrayNum; i++) {
		if (spellSys.isDomainSpell(spellArray[i].classCode ))
			numSpells++;
	}
	return numSpells;
}

bool LegacyCritterSystem::HasDomain(objHndl handle, uint32_t domainType){
	if (objects.StatLevelGet(handle, stat_domain_1) == domainType
		|| objects.StatLevelGet(handle, stat_domain_2) == domainType)
		return true;
	return false;
}

int LegacyCritterSystem::GetNumFollowers(objHndl obj, int excludeForcedFollowers)
{
	auto objBod = objSystem->GetObject(obj);
	auto followerArray = objBod->GetObjectIdArray(obj_f_critter_follower_idx);
	auto followersNum = followerArray.GetSize();
	if (excludeForcedFollowers)
	{
		auto orgNum = followersNum;
		for (size_t i = 0; i < orgNum; i++)
		{
			auto follower = followerArray[i];
			auto followerNpcFlags = objects.getInt32(follower.GetHandle(), obj_f_npc_flags);
			if (followerNpcFlags & NpcFlag::ONF_FORCED_FOLLOWER)
				followersNum--;
		}
	}
	return followersNum;
}
void LegacyCritterSystem::BuildRadialMenu(objHndl handle){
	if (!handle){
		return;
	}

	auto obj = objSystem->GetObject(handle);
	Dispatcher * dispatcher = obj->GetDispatcher();

	if (dispatch.dispatcherValid(dispatcher)) {
		if (objects.IsPlayerControlled(handle)) {
			objects.dispatch.DispatcherProcessor(dispatcher, dispTypeRadialMenuEntry, 0, nullptr);
			auto setActiveRadial = temple::GetRef<void(__cdecl)(objHndl)>(0x100F0A70);
			setActiveRadial(handle);
		}
	}
}

#pragma region Critter Hooks
uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 
