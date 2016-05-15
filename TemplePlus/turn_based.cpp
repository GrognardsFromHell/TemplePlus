
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
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "ai.h"
#include "condition.h"

class TurnBasedReplacements : public TempleFix
{
public: 
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
		replaceFunction<void(__cdecl)(objHndl)>(0x100DF1E0, [](objHndl handle){
			tbSys.AddToInitiative(handle);
		});
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
	logger->debug("InitiativeListNextActor: Actor {} ({}) finishing.", description.getDisplayName(*turnBasedCurrentActor), *turnBasedCurrentActor);
	auto actorInitiative = objects.getInt32(*turnBasedCurrentActor, obj_f_initiative);
	auto actorInitiativeIdx = party.ObjFindInGroupArray(groupInitiativeList, *turnBasedCurrentActor);
	auto nextInitiativeIdx = actorInitiativeIdx + 1;
	int initiativeListLen = GetInitiativeListLength();

	if (nextInitiativeIdx >= initiativeListLen) // time for next round
	{
		temple::GetRef<int>(0x10BCAD90) = 0; // surprise round
		for (int i = 0; i < GetInitiativeListLength(); i++) // refreshing the init list length in case it changes
		{
			auto combatant = groupInitiativeList->GroupMembers[i];
			auto dispatcher = objects.GetDispatcher(combatant);
			if (dispatch.dispatcherValid(dispatcher)){
				dispatch.DispatcherProcessor(dispatcher, dispTypeInitiative, 0, 0);
			}	
		}
		actorInitiative = nextInitiativeIdx = 0;
		InitiativeRefresh(actorInitiative, 0);
	}
	
	objHndl actorNext = objHndl::null;
	if (nextInitiativeIdx < groupInitiativeList->GroupSize)
		actorNext = groupInitiativeList->GroupMembers[nextInitiativeIdx];
	*turnBasedCurrentActor = actorNext;
	if (actorNext)
	{
		logger->debug("InitiativeListNextActor: Turn Based actor changed to {} ({})", description.getDisplayName(actorNext), actorNext);
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

int TurnBasedSys::GetInitiativeListIdx() const
{
	return party.ObjFindInGroupArray(groupInitiativeList, *turnBasedCurrentActor );
}

void TurnBasedSys::AddToInitiativeGroup(objHndl handle) const
{
	party.ObjAddToGroupArray(groupInitiativeList, handle);
}

void TurnBasedSys::ArbitrateInitiativeConflicts() const
{
	auto arbConf = temple::GetRef<void(__cdecl)()>(0x100DEFA0);
	arbConf();
}

void TurnBasedSys::AddToInitiative(objHndl handle) const
{
	if (IsInInitiativeList(handle))
		return;
	
	auto obj = gameSystems->GetObj().GetObject(handle);
	if (critterSys.IsDeadNullDestroyed(handle))
		return;
	if (critterSys.IsDeadOrUnconscious(handle))
		return;

	if (!d20Sys.d20Query(handle, DK_QUE_EnterCombat))
		return;


	aiSys.AiOnInitiativeAdd(handle);

	auto initiativeRoll = templeFuncs.RNG(1, 20);
	int initiativeMod = 0;
	auto dispatcher = obj->GetDispatcher();
	if (dispatcher->IsValid()){
		DispIoObjBonus dispIo;
		dispIo.bonOut = &dispIo.bonlist;
		dispatcher->Process(dispTypeInitiativeMod, DK_NONE, &dispIo);
		initiativeMod = dispIo.bonlist.GetEffectiveBonusSum();
	}

	obj->SetInt32(obj_f_initiative, initiativeMod + initiativeRoll);
	AddToInitiativeGroup(handle);
	auto dexScore = objects.StatLevelGet(handle, stat_dexterity);
	obj->SetInt32(obj_f_subinitiative, 100 * dexScore);
	ArbitrateInitiativeConflicts();
	conds.AddTo(handle, "Flatfooted", {});
	auto isSurpriseRound = temple::GetRef<BOOL>(0x10BCAD90);
	if (isSurpriseRound){
		conds.AddTo(handle, "Surprised", {});
	}

	/*auto addToInit = temple::GetRef<void(__cdecl)(objHndl)>(0x100DF1E0);
	addToInit(handle);*/
}

bool TurnBasedSys::IsInInitiativeList(objHndl handle) const   // 0x100DEDD0
{
	return party.ObjIsInGroupArray(groupInitiativeList, handle) != 0;
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