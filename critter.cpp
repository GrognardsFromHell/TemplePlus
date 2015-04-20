#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"
#include "inventory.h"

static struct CritterAddresses : AddressTable {

	bool (__cdecl *HasMet)(objHndl critter, objHndl otherCritter);
	bool (__cdecl *AddFollower)(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);
	bool (__cdecl *RemoveFollower)(objHndl npc, int unkFlag);
	objHndl (__cdecl *GetLeader)(objHndl critter);
	int (__cdecl *HasLineOfSight)(objHndl critter, objHndl target);
	void (__cdecl *Attack)(objHndl target, objHndl attacker, int n1, int n2);

	CritterAddresses() {
		rebase(HasMet, 0x10053CD0);
		rebase(AddFollower, 0x100812F0);
		rebase(RemoveFollower, 0x10080FD0);
		rebase(GetLeader, 0x1007EA70);
		rebase(HasLineOfSight, 0x10059470);
		rebase(Attack, 0x1005E8D0);
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
}
#pragma region Critter Hooks

uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 