
#include "stdafx.h"
#include "common.h"
#include "turn_based.h"
#include "util/fixes.h"
#include "description.h"
#include "party.h"
#include "combat.h"

class TurnBasedReplacements : public TempleFix
{
public: 
	const char* name() override { return "Turn Based Function Replacements";} 

	static void PortraitDragChangeInitiative(objHndl obj, int newInitiativeListIdx)
	{
		logger->info("Changing {}'s combat initiative slot to {}", description.getDisplayName(obj), newInitiativeListIdx);
		orgPortraitDragChangeInitiative(obj, newInitiativeListIdx);
	};
	static void (__cdecl*orgPortraitDragChangeInitiative)(objHndl obj, int idx);

	void apply() override 
	{
		logger->info("Replacing Turn Based System functions");

		replaceFunction(0x100DEE10, _turnBasedSetCurrentActor); 
		replaceFunction(0x100DEE40, _turnBasedGetCurrentActor); 
		orgPortraitDragChangeInitiative = replaceFunction(0x100DF5A0, PortraitDragChangeInitiative);
	}
} tbReplacements;
void(__cdecl*TurnBasedReplacements::orgPortraitDragChangeInitiative)(objHndl obj, int idx);

#pragma region Turn Based System Implementation
struct TurnBasedSys tbSys;

TurnBasedSys::TurnBasedSys()
{
	rebase(groupInitiativeList, 0x10BCAC78);
	rebase(turnBasedCurrentActor,0x10BCAD88); 
	rebase(_CloneInitiativeFromObj, 0x100DF570);
}

objHndl TurnBasedSys::turnBasedGetCurrentActor()
{
	return *turnBasedCurrentActor;
}

void TurnBasedSys::turnBasedSetCurrentActor(objHndl objHnd)
{
	*turnBasedCurrentActor = objHnd;
}

void TurnBasedSys::CloneInitiativeFromObj(objHndl obj, objHndl sourceObj)
{
	_CloneInitiativeFromObj(obj, sourceObj);
}

void TurnBasedSys::TbSysNextSthg_100DF5A0(objHndl obj, int idx)
{

}

void TurnBasedSys::InitiativeListSort()
{
	party.GroupArraySort(combatSys.groupInitiativeList);
}

int TurnBasedSys::GetInitiativeListIdx()
{
	return party.ObjFindInGroupArray(groupInitiativeList, *turnBasedCurrentActor );
}

void _turnBasedSetCurrentActor(objHndl objHnd)
{
	if (objHnd)
		logger->info("Turn Based Actor changed to {}({})", description.getDisplayName(objHnd), objHnd);
	tbSys.turnBasedSetCurrentActor(objHnd);
}

objHndl _turnBasedGetCurrentActor()
{
	return tbSys.turnBasedGetCurrentActor();
}
#pragma endregion

#pragma region Turn Based Function Replacements

#pragma endregion 