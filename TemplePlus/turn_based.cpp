
#include "stdafx.h"
#include "common.h"
#include "turn_based.h"
#include "util/fixes.h"
#include "description.h"
#include "party.h"
#include "combat.h"
#include "obj.h"
#include "temple_functions.h"
#include "d20_obj_registry.h"

class TurnBasedReplacements : public TempleFix
{
public: 
	const char* name() override { return "Turn Based Function Replacements";} 

	static void PortraitDragChangeInitiative(objHndl obj, int newInitiativeListIdx)
	{
		logger->debug("Changing {}'s combat initiative slot to {}", description.getDisplayName(obj), newInitiativeListIdx);
		orgPortraitDragChangeInitiative(obj, newInitiativeListIdx);
	};
	static void (__cdecl*orgPortraitDragChangeInitiative)(objHndl obj, int idx);

	static void InitiativeNextActor();

	void apply() override 
	{
		logger->debug("Replacing Turn Based System functions");

		replaceFunction(0x100DEE10, _turnBasedSetCurrentActor); 
		replaceFunction(0x100DEE40, _turnBasedGetCurrentActor); 
		replaceFunction(0x100DF310, InitiativeNextActor);
		orgPortraitDragChangeInitiative = replaceFunction(0x100DF5A0, PortraitDragChangeInitiative);
		
	}
} tbReplacements;

void TurnBasedReplacements::InitiativeNextActor()
{
	tbSys.InitiativeListNextActor();
}

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

unsigned TurnBasedSys::GetInitiativeListLength()
{
	return groupInitiativeList->GroupSize;
}

int TurnBasedSys::InitiativeRefresh(int initiative, int initiativeNext)
{
	auto func = temple::GetRef< int(__cdecl)(int, int)>(0x100DFE40);
	return func(initiative, initiativeNext);
}

void TurnBasedSys::InitiativeListNextActor()
{
	auto actorInitiative = objects.getInt32(*turnBasedCurrentActor, obj_f_initiative);
	auto actorInitiativeIdx = party.ObjFindInGroupArray(groupInitiativeList, *turnBasedCurrentActor);
	auto nextInitiativeIdx = actorInitiativeIdx + 1;
	int initiativeListLen = GetInitiativeListLength();

	if (nextInitiativeIdx >= initiativeListLen)
	{
		temple::GetRef<int>(0x10BCAD90) = 0; // surprise round
		for (int i = 0; i < initiativeListLen; i++)
		{
			auto combatant = groupInitiativeList->GroupMembers[i];
			auto dispatcher = objects.GetDispatcher(combatant);
			if (dispatch.dispatcherValid(dispatcher))
			{
				dispatch.DispatcherProcessor(dispatcher, dispTypeInitiative, 0, 0);
			}	
		}
		actorInitiative = nextInitiativeIdx = 0;
		InitiativeRefresh(actorInitiative, 0);
	}
	
	objHndl actorNext = 0i64;
	if (nextInitiativeIdx < groupInitiativeList->GroupSize)
		actorNext = groupInitiativeList->GroupMembers[nextInitiativeIdx];
	*turnBasedCurrentActor = actorNext;
	if (actorNext)
	{
		logger->debug("Turn Based actor changed to {} ({})", description.getDisplayName(actorNext), actorNext);
		auto nextActorInitiative = objects.getInt32(actorNext, obj_f_initiative);
		if (actorInitiative != nextActorInitiative)
		{
			InitiativeRefresh(actorInitiative, nextActorInitiative);
		}
	} else
	{
		logger->debug("Turn Based actor changed to NULL");
	}

}

int TurnBasedSys::GetInitiativeListIdx()
{
	return party.ObjFindInGroupArray(groupInitiativeList, *turnBasedCurrentActor );
}

void _turnBasedSetCurrentActor(objHndl objHnd)
{
	if (objHnd)
		logger->debug("Turn Based Actor changed to {}({})", description.getDisplayName(objHnd), objHnd);
	tbSys.turnBasedSetCurrentActor(objHnd);
}

objHndl _turnBasedGetCurrentActor()
{
	return tbSys.turnBasedGetCurrentActor();
}
#pragma endregion

#pragma region Turn Based Function Replacements

#pragma endregion 