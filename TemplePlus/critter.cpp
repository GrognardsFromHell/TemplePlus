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

	void (__cdecl *GetStandpoint)(objHndl, StandPointType, StandPoint *);
	void (__cdecl *SetStandpoint)(objHndl, StandPointType, const StandPoint *);
	void (__cdecl *SetSubdualDamage)(objHndl critter, int damage);

	void (__cdecl *BalorDeath)(objHndl critter);
	void (__cdecl *SetConcealed)(objHndl critter, int concealed);

	uint32_t(__cdecl *Resurrect)(objHndl critter, ResurrectType type, int unk);

	uint32_t(__cdecl *IsDeadOrUnconscious)(objHndl critter);

	objHndl (__cdecl *GiveItem)(objHndl critter, int protoId);

	void(__cdecl *TakeMoney)(objHndl critter, int platinum, int gold, int silver, int copper);
	void(__cdecl *GiveMoney)(objHndl critter, int platinum, int gold, int silver, int copper);

	int (*GetWeaponAnim)(objHndl critter, objHndl primaryWeapon, objHndl secondary, gfx::WeaponAnim animId);

	void(__cdecl*GetCritterVoiceLine)(objHndl obj, objHndl fellow, char* str, int* soundId);
	int (__cdecl*PlayCritterVoiceLine)(objHndl obj, objHndl fellow, char* str, int soundId);
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
		rebase(Resurrect, 0x100809C0);
		rebase(IsDeadOrUnconscious, 0x100803E0);
		rebase(GiveItem, 0x1006CC30);

		rebase(GiveMoney, 0x1007F960);
		rebase(TakeMoney, 0x1007FA40);
		rebase(GetWeaponAnim, 0x10020B60);
	}

	
} addresses;

class CritterReplacements : public TempleFix
{
	void ShowExactHpForNPCs();

	void apply() override 
	{
		logger->info("Replacing Critter System functions");
		replaceFunction(0x10062720, _isCritterCombatModeActive); 
		replaceFunction<float(objHndl, D20ActionType)>(0x100B52D0, [](objHndl handle, D20ActionType d20aType){
			return critterSys.GetReach(handle, d20aType);
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
	}
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

uint32_t LegacyCritterSystem::AddFollower(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower) {
	return addresses.AddFollower(npc, pc, unkFlag, asAiFollower);
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

uint32_t LegacyCritterSystem::IsFriendly(objHndl pc, objHndl npc) {

	if (d20Sys.d20Query(pc, DK_QUE_Critter_Is_AIControlled) && d20Sys.d20Query(npc, DK_QUE_Critter_Is_AIControlled))
		return true;

	return addresses.IsFriendly(pc, npc);
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

int LegacyCritterSystem::GetReaction(objHndl of, objHndl towards){
	return temple::GetRef<int(objHndl, objHndl)>(0x10054180)(of, towards);
}

int LegacyCritterSystem::SoundmapCritter(objHndl critter, int id) {
	return addresses.SoundmapCritter(critter, id);
}

void LegacyCritterSystem::Kill(objHndl critter, objHndl killer) {
	return addresses.Kill(critter, killer);
}

void LegacyCritterSystem::KillByEffect(objHndl critter, objHndl killer) {
	return addresses.KillByEffect(critter, killer);
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

void LegacyCritterSystem::GenerateHp(objHndl critter){
	temple::GetRef<void(__cdecl)(objHndl)>(0x1007F720)(critter);
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

uint32_t LegacyCritterSystem::Resurrect(objHndl critter, ResurrectType type, int unk) {
	return addresses.Resurrect(critter, type, unk);
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

bool LegacyCritterSystem::CanSense(objHndl critter, objHndl tgt){
	return temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1007FFF0)(critter, tgt) != FALSE;
}

int LegacyCritterSystem::GetLevel(objHndl critter) {
	return objects.StatLevelGet(critter, stat_level);
}

int LegacyCritterSystem::SkillLevel(objHndl critter, SkillEnum skill){
	return dispatch.dispatch1ESkillLevel(critter, skill, nullptr, critter, 1);
}

Race LegacyCritterSystem::GetRace(objHndl critter) {
	return (Race)objects.StatLevelGet(critter, stat_race);
}

Gender LegacyCritterSystem::GetGender(objHndl critter) {
	return (Gender)objects.StatLevelGet(critter, stat_gender);
}

std::string LegacyCritterSystem::GetHairStylePreviewTexture(HairStyle style)
{
	return GetHairStyleFile(style, "tga");
}

std::string LegacyCritterSystem::GetHairStyleModel(HairStyle style)
{
	return GetHairStyleFile(style, "skm");
}

gfx::EncodedAnimId LegacyCritterSystem::GetAnimId(objHndl critter, gfx::WeaponAnim anim)
{
	auto weaponPrim = GetWornItem(critter, EquipSlot::WeaponPrimary);
	auto weaponSec = GetWornItem(critter, EquipSlot::WeaponSecondary);
	if (!weaponSec) {
		weaponSec = GetWornItem(critter, EquipSlot::Shield);
	}

	int rawAnimId = addresses.GetWeaponAnim(critter, weaponPrim, weaponSec, anim);
	return gfx::EncodedAnimId(rawAnimId);
}

int LegacyCritterSystem::GetModelRaceOffset(objHndl obj)
{
	// Meshes above 1000 are monsters, they dont get a creature type
	auto meshId = objects.getInt32(obj, obj_f_base_mesh);
	if (meshId >= 1000) {
		return -1;
	}

	auto race = GetRace(obj);
	auto gender = GetGender(obj);
	bool isMale = (gender == Gender::Male);

	/*
		The following table comes from materials.mes, where
		the offsets into the materials and addmesh table are listed.
	*/
	int result;
	switch (race)
	{
	case race_human:
		result = (isMale ? 0 : 1);
		break;
	case race_elf:
		result = (isMale ? 2 : 3);
		break;
	case race_halforc:
		result = (isMale ? 4 : 5);
		break;
	case race_dwarf:
		result = (isMale ? 6 : 7);
		break;
	case race_gnome:
		result = (isMale ? 8 : 9);
		break;
	case race_halfelf:
		result = (isMale ? 10 : 11);
		break;
	case race_halfling:
		result = (isMale ? 12 : 13);
		break;
	default:
		result = 0;
		break;
	}

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
}

const std::vector<std::string>& LegacyCritterSystem::GetAddMeshes(int matIdx, int raceOffset) {

	static std::vector<std::string> sEmptyAddMeshes;
	
	// Lazily load the addmesh rules
	if (mAddMeshes.empty()) {
		auto mapping(MesFile::ParseFile("rules\\addmesh.mes"));
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

void LegacyCritterSystem::ApplyReplacementMaterial(gfx::AnimatedModelPtr model, int mesId)
{
	auto& replacementSet = tig->GetMdfFactory().GetReplacementSet(mesId);
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

void LegacyCritterSystem::UpdateModelEquipment(objHndl obj)
{

	UpdateAddMeshes(obj);
	auto raceOffset = GetModelRaceOffset(obj);
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
	gameSystems->GetAAS().InvalidateBuffers(model->GetHandle());

	// Apply the naked replacement materials for
	// equipment slots that support them
	ApplyReplacementMaterial(model, 0 + raceOffset); // Helmet
	ApplyReplacementMaterial(model, 100 + raceOffset); // Cloak
	ApplyReplacementMaterial(model, 200 + raceOffset); // Gloves
	ApplyReplacementMaterial(model, 500 + raceOffset); // Armor
	ApplyReplacementMaterial(model, 800 + raceOffset); // Boots
	
	// Now apply it for the actual equipment
	for (uint32_t slotId = 0; slotId < (uint32_t)EquipSlot::Count; ++slotId) {
		auto slot = (EquipSlot) slotId;
		auto item = GetWornItem(obj, slot);
		if (item) {
			auto materialSlot = objects.getInt32(item, obj_f_item_material_slot);
			if (materialSlot != -1) {
				ApplyReplacementMaterial(model, materialSlot + raceOffset);
			}
		}
	}
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
		if (invLocation >= 200 && invLocation <= 216) {
			continue; // Currently equipped on the corpse
		}

		auto itemFlags = objects.GetItemFlags(item);
		if (itemFlags & OIF_NO_LOOT) {
			continue; // Flagged as unlootable
		}

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

float LegacyCritterSystem::GetReach(objHndl obj, D20ActionType actType) {

	float naturalReach = (float)objects.getInt32(obj, obj_f_critter_reach);
	auto protoId = d20Sys.d20Query(obj, DK_QUE_Polymorphed);
	if (protoId) {
		auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoId);
		if (protoHandle) {
			naturalReach = (float)gameSystems->GetObj().GetObject(protoHandle)->GetInt32(obj_f_critter_reach);
		}
	}

	if (naturalReach < 0.01) {
		naturalReach = 5.0;
	}

	if (actType != D20A_TOUCH_ATTACK)
	{
		objHndl weapon = inventory.GetItemAtInvIdx(obj, 203);
		// todo: handle cases where enlarged creatures dual wield polearms ><
		if (weapon){
			auto weapType = objects.GetWeaponType(weapon);
			if (weapons.IsReachWeaponType(weapType)){
				return naturalReach + 3.0f; // +5.0 - 2.0
			}
			
			return naturalReach - 2.0f;
		}
	}
	return naturalReach - 2.0f;
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
	return (curHp - subdualDam) / maxHp;
}

int LegacyCritterSystem::GetCritterAttackType(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
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
	auto obj = gameSystems->GetObj().GetObject(handle);
	if (obj->type == obj_t_npc){
		auto npcHd = obj->GetInt32(obj_f_npc_hitdice_idx, 0);
		auto moncat = critterSys.GetCategory(handle);
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
			return bab + (3 * npcHd / 4);


		case mc_type_dragon: 
		case mc_type_magical_beast:
		case mc_type_monstrous_humanoid:
		case mc_type_outsider:
			return bab + npcHd;

		case mc_type_fey: 
		case mc_type_undead:
			return bab + npcHd / 2;

		default: break;
		}
	}
	return bab;
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
	return dispatch.DispatchGetBaseCasterLevel(handle, classCode);
}

int LegacyCritterSystem::GetSpellListLevelExtension(objHndl handle, Stat classCode)
{
	return dispatch.DispatchSpellListLevelExtension(handle, classCode);
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

	return false;
}

int LegacyCritterSystem::GetArmorClass(objHndl obj, DispIoAttackBonus* dispIo){
	return dispatch.DispatchAttackBonus(obj, objHndl::null, dispIo, dispTypeGetAC, DK_NONE);
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
#pragma region Critter Hooks
uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 
