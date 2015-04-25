#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"
#include "inventory.h"
#include "float_line.h"
#include "condition.h"
#include "particles.h"
#include "temple_functions.h"

static struct CritterAddresses : AddressTable {

	bool (__cdecl *HasMet)(objHndl critter, objHndl otherCritter);
	bool (__cdecl *AddFollower)(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);
	bool (__cdecl *RemoveFollower)(objHndl npc, int unkFlag);
	objHndl (__cdecl *GetLeader)(objHndl critter);
	int (__cdecl *HasLineOfSight)(objHndl critter, objHndl target);
	void (__cdecl *Attack)(objHndl target, objHndl attacker, int n1, int n2);
	bool (__cdecl *IsFriendly)(objHndl pc, objHndl npc);
	int (__cdecl *SoundmapCritter)(objHndl critter, int id);
	void (__cdecl *KillByEffect)(objHndl critter, objHndl killer);
	void (__cdecl *Kill)(objHndl critter, objHndl killer);

	void (__cdecl *GetStandpoint)(objHndl, StandPointType, StandPoint *);
	void (__cdecl *SetStandpoint)(objHndl, StandPointType, const StandPoint *);
	void (__cdecl *SetSubdualDamage)(objHndl critter, int damage);

	void (__cdecl *BalorDeath)(objHndl critter);
	void (__cdecl *SetConcealed)(objHndl critter, bool concealed);

	bool (__cdecl *Resurrect)(objHndl critter, ResurrectType type, int unk);

	bool (__cdecl *IsDeadOrUnconscious)(objHndl critter);

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
	}

} addresses;

class CritterReplacements : public TempleFix
{
	macTempleFix(Critter System)
	{
		logger->info("Replacing Critter System functions");
		macReplaceFun(10062720, _isCritterCombatModeActive)
	}
} critterReplacements;


#pragma region Critter System Implementation

struct CritterSystem critterSys;

uint32_t CritterSystem::isCritterCombatModeActive(objHndl objHnd)
{
	return (objects.getInt32(objHnd, obj_f_critter_flags) & OCF_COMBAT_MODE_ACTIVE) != 0;
}

CritterSystem::CritterSystem()
{
}
#pragma endregion

int CritterSystem::GetLootBehaviour(objHndl npc) {
	return objects.getInt32(npc, obj_f_npc_pad_i_3) & 0xF;
}

void CritterSystem::SetLootBehaviour(objHndl npc, int behaviour) {
	objects.setInt32(npc, obj_f_npc_pad_i_3, behaviour & 0xF);
}

bool CritterSystem::HasMet(objHndl critter, objHndl otherCritter) {
	return addresses.HasMet(critter, otherCritter);
}

bool CritterSystem::AddFollower(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower) {
	return addresses.AddFollower(npc, pc, unkFlag, asAiFollower);
}

bool CritterSystem::RemoveFollower(objHndl npc, int unkFlag) {
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

bool CritterSystem::IsFriendly(objHndl pc, objHndl npc) {
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

void CritterSystem::SetConcealed(objHndl critter, bool concealed) {
	addresses.SetConcealed(critter, concealed);
}

bool CritterSystem::Resurrect(objHndl critter, ResurrectType type, int unk) {
	return addresses.Resurrect(critter, type, unk);
}

bool CritterSystem::Dominate(objHndl critter, objHndl caster) {

	// Float a "charmed!" line above the critter
	floatSys.FloatSpellLine(critter, 20018, FloatLineColor::Red);

	vector<int> args(3);

	args[0] = particles.CreateAtObj("sp-Dominate Person", critter);
	args[1] = (caster >> 32) & 0xFFFFFFFF;
	args[2] = caster & 0xFFFFFFFF;

	auto cond = conds.GetByName("Dominate");
	return conds.AddTo(critter, cond, args);
}

bool CritterSystem::IsDeadOrUnconscious(objHndl critter) {
	return addresses.IsDeadOrUnconscious(critter);
}

MonsterCategory CritterSystem::GetCategory(objHndl objHnd)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects._GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
			return (MonsterCategory)(monCat & 0xFFFFFFFF);
		}
	}
	return mc_type_monstrous_humanoid; // default - so they have at least a weapons proficiency
}

bool CritterSystem::IsCategoryType(objHndl objHnd, MonsterCategory categoryType)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects._GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
			return (monCat & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

bool CritterSystem::IsCategorySubtype(objHndl objHnd, MonsterCategory categoryType)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects._GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
			return ((monCat >> 32) & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

bool CritterSystem::IsUndead(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_undead);
}

bool CritterSystem::IsOoze(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_ooze);
}

bool CritterSystem::IsSubtypeFire(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtye_fire);
}

#pragma region Critter Hooks
uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 
