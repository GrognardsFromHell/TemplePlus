#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"

static struct CritterAddresses : AddressTable {

	bool(__cdecl *HasMet)(objHndl critter, objHndl otherCritter);

	CritterAddresses() {
		rebase(HasMet, 0x10053CD0);
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

#pragma region Critter Hooks

uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 