#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"
#include "inventory.h"
#include "float_line.h"
#include "condition.h"
#include "particles.h"
#include "temple_functions.h"
#include "util/config.h"

static struct CritterAddresses : AddressTable {

	uint32_t (__cdecl *HasMet)(objHndl critter, objHndl otherCritter);
	uint32_t (__cdecl *AddFollower)(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);
	uint32_t (__cdecl *RemoveFollower)(objHndl npc, int unkFlag);
	objHndl (__cdecl *GetLeader)(objHndl critter);
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

	CritterAddresses() {
		rebase(HasMet, 0x10053CD0);
		rebase(AddFollower, 0x100812F0);
		rebase(RemoveFollower, 0x10080FD0);
		rebase(GetLeader, 0x1007EA70);
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
	}

} addresses;

class CritterReplacements : public TempleFix
{
	public: const char* name() override { 
		return "Critter System" "Function Replacements";
	} 
	void ShowExactHpForNPCs();

	void apply() override 
	{
		logger->info("Replacing Critter System functions");
		replaceFunction(0x10062720, _isCritterCombatModeActive); 
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

struct CritterSystem critterSys;

uint32_t CritterSystem::isCritterCombatModeActive(objHndl objHnd)
{
	return (objects.getInt32(objHnd, obj_f_critter_flags) & OCF_COMBAT_MODE_ACTIVE) != 0;
}

CritterSystem::CritterSystem()
{
}

uint32_t CritterSystem::IsPC(objHndl objHnd)
{
	return objects.getInt32(objHnd, obj_f_type) == obj_t_pc;
}
#pragma endregion

int CritterSystem::GetLootBehaviour(objHndl npc) {
	return objects.getInt32(npc, obj_f_npc_pad_i_3) & 0xF;
}

void CritterSystem::SetLootBehaviour(objHndl npc, int behaviour) {
	objects.setInt32(npc, obj_f_npc_pad_i_3, behaviour & 0xF);
}

uint32_t CritterSystem::HasMet(objHndl critter, objHndl otherCritter) {
	return addresses.HasMet(critter, otherCritter);
}

uint32_t CritterSystem::AddFollower(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower) {
	return addresses.AddFollower(npc, pc, unkFlag, asAiFollower);
}

uint32_t CritterSystem::RemoveFollower(objHndl npc, int unkFlag) {
	return addresses.RemoveFollower(npc, unkFlag);
}

objHndl CritterSystem::GetLeader(objHndl critter) {
	return addresses.GetLeader(critter);
}

int CritterSystem::HasLineOfSight(objHndl critter, objHndl target) {
	return addresses.HasLineOfSight(critter, target);
}

objHndl CritterSystem::GetWornItem(objHndl handle, EquipSlot slot) {
	return inventory.ItemWornAt(handle, (uint32_t) slot);
}

void CritterSystem::Attack(objHndl target, objHndl attacker, int n1, int n2) {
	addresses.Attack(target, attacker, n1, n2);
}

uint32_t CritterSystem::IsFriendly(objHndl pc, objHndl npc) {
	return addresses.IsFriendly(pc, npc);
}

int CritterSystem::SoundmapCritter(objHndl critter, int id) {
	return addresses.SoundmapCritter(critter, id);
}

void CritterSystem::Kill(objHndl critter, objHndl killer) {
	return addresses.Kill(critter, killer);
}

void CritterSystem::KillByEffect(objHndl critter, objHndl killer) {
	return addresses.KillByEffect(critter, killer);
}

static_assert(validate_size<StandPoint, 0x20>::value, "Invalid size");

void CritterSystem::SetStandPoint(objHndl critter, StandPointType type, const StandPoint& standpoint) {
	addresses.SetStandpoint(critter, type, &standpoint);
}

StandPoint CritterSystem::GetStandPoint(objHndl critter, StandPointType type) {
	StandPoint result;
	addresses.GetStandpoint(critter, type, &result);
	return result;
}

void CritterSystem::SetSubdualDamage(objHndl critter, int damage) {
	addresses.SetSubdualDamage(critter, damage);
}

void CritterSystem::AwardXp(objHndl critter, int xpAwarded) {
	auto xp = objects.getInt32(critter, obj_f_critter_experience);
	xp += xpAwarded;
	d20Sys.d20SendSignal(critter, DK_SIG_Experience_Awarded, xp, 0);
	objects.setInt32(critter, obj_f_critter_experience, xp);
}

void CritterSystem::BalorDeath(objHndl npc) {
	addresses.BalorDeath(npc);
}

void CritterSystem::SetConcealed(objHndl critter, int concealed) {
	addresses.SetConcealed(critter, concealed);
}

uint32_t CritterSystem::Resurrect(objHndl critter, ResurrectType type, int unk) {
	return addresses.Resurrect(critter, type, unk);
}

uint32_t CritterSystem::Dominate(objHndl critter, objHndl caster) {

	// Float a "charmed!" line above the critter
	floatSys.FloatSpellLine(critter, 20018, FloatLineColor::Red);

	vector<int> args(3);

	args[0] = particles.CreateAtObj("sp-Dominate Person", critter);
	args[1] = (caster >> 32) & 0xFFFFFFFF;
	args[2] = caster & 0xFFFFFFFF;

	auto cond = conds.GetByName("Dominate");
	return conds.AddTo(critter, cond, args);
}

uint32_t CritterSystem::IsDeadOrUnconscious(objHndl critter) {
	return addresses.IsDeadOrUnconscious(critter);
}

int CritterSystem::GetPortraitId(objHndl leader) {
	return objects.getInt32(leader, obj_f_critter_portrait);
}

int CritterSystem::GetLevel(objHndl critter) {
	return objects.StatLevelGet(critter, stat_level);
}

Race CritterSystem::GetRace(objHndl critter) {
	return (Race)objects.StatLevelGet(critter, stat_race);
}

objHndl CritterSystem::GiveItem(objHndl critter, int protoId) {
	return addresses.GiveItem(critter, protoId);
}

void CritterSystem::TakeMoney(objHndl critter, int platinum, int gold, int silver, int copper)
{
	addresses.TakeMoney(critter, platinum, gold, silver, copper);
}

void CritterSystem::GiveMoney(objHndl critter, int platinum, int gold, int silver, int copper)
{
	addresses.GiveMoney(critter, platinum, gold, silver, copper);
}

int CritterSystem::NumOffhandExtraAttacks(objHndl critter)
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

int CritterSystem::IsWearingLightOrNoArmor(objHndl critter)
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

MonsterCategory CritterSystem::GetCategory(objHndl objHnd)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
			return (MonsterCategory)(monCat & 0xFFFFFFFF);
		}
	}
	return mc_type_monstrous_humanoid; // default - so they have at least a weapons proficiency
}

uint32_t CritterSystem::IsCategoryType(objHndl objHnd, MonsterCategory categoryType)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
			return (monCat & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

uint32_t CritterSystem::IsCategorySubtype(objHndl objHnd, MonsterCategory categoryType)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
			return ((monCat >> 32) & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

uint32_t CritterSystem::IsUndead(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_undead);
}

uint32_t CritterSystem::IsOoze(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_ooze);
}

uint32_t CritterSystem::IsSubtypeFire(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtye_fire);
}

float CritterSystem::GetReach(objHndl obj, D20ActionType actType) 
{
	float naturalReach = objects.getInt32(obj, obj_f_critter_reach);
	__asm{
		fild naturalReach;
		fstp naturalReach;
	}
	if (naturalReach < 0.01)
		naturalReach = 5.0;
	if (actType != D20A_TOUCH_ATTACK)
	{
		objHndl weapon = inventory.GetItemAtInvIdx(obj, 203);
		if (weapon)
		{
			WeaponTypes weapType = (WeaponTypes)objects.getInt32(weapon, obj_f_weapon_type);
			switch (weapType)
			{
			case wt_glaive:
			case wt_guisarme:
			case wt_longspear:
			case wt_ranseur:
			case wt_spike_chain:
				return naturalReach + 3.0; // +5.0 - 2.0
			default:
				return naturalReach - 2.0;
			}
				
		}
	}
	return naturalReach - 2.0;
}

int CritterSystem::GetBonusFromSizeCategory(int sizeCategory)
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

int CritterSystem::GetDamageIdx(objHndl obj, int attackIdx)
{
	int n =0;
	for (int i = 0; i < 4; i++)
	{
		n += objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, i);
		if (n > attackIdx)
			return n;
	}
	return 0;
}

int CritterSystem::GetCritterDamageDice(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_critter_damage_idx, damageIdx);

}

int CritterSystem::GetCritterAttackDamageType(objHndl obj, int attackIdx)
{
	int damType[7];
	damType[0] = 6;
	damType[1] = 4;
	damType[2] = 4;
	damType[3] = 1;
	damType[4] = 0;
	damType[5] = 0;
	damType[6] = 1;
	int damageIdx = GetDamageIdx(obj, attackIdx);
	int x = objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
	if (x > 6 || x < 0)
		return 0;
	return damType[x];
}

int CritterSystem::GetCritterAttackType(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
}
#pragma region Critter Hooks
uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 
