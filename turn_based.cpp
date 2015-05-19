#include "stdafx.h"
#include "common.h"
#include "turn_based.h"


class TurnBasedReplacements : public TempleFix
{
public: 
	const char* name() override { return "Turn Based" "Function Replacements";} 
	void apply() override 
	{
		logger->info("Replacing Turn Based System functions");

		replaceFunction(0x100DEE10, _turnBasedSetCurrentActor); 
		replaceFunction(0x100DEE40, _turnBasedGetCurrentActor); 
	}
} tbReplacements;

#pragma region Turn Based System Implementation
struct TurnBasedSys tbSys;

TurnBasedSys::TurnBasedSys()
{
	rebase(turnBasedCurrentActor,0x10BCAD88); 
}

objHndl TurnBasedSys::turnBasedGetCurrentActor()
{
	return *turnBasedCurrentActor;
}

void TurnBasedSys::turnBasedSetCurrentActor(objHndl objHnd)
{
	*turnBasedCurrentActor = objHnd;
}

void _turnBasedSetCurrentActor(objHndl objHnd)
{
	tbSys.turnBasedSetCurrentActor(objHnd);
}

objHndl _turnBasedGetCurrentActor()
{
	return tbSys.turnBasedGetCurrentActor();
}
#pragma endregion

#pragma region Turn Based Function Replacements

#pragma endregion 