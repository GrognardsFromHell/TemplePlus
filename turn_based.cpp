#include "stdafx.h"
#include "common.h"
#include "turn_based.h"


class TurnBasedReplacements : public TempleFix
{
	macTempleFix(Turn Based)
	{
		logger->info("Replacing Turn Based System functions");

		macReplaceFun(100DEE10, _turnBasedSetCurrentActor)
		macReplaceFun(100DEE40, _turnBasedGetCurrentActor)
	}
} tbReplacements;

#pragma region Turn Based System Implementation
struct TurnBasedSys tbSys;

TurnBasedSys::TurnBasedSys()
{
	macRebase(turnBasedCurrentActor, 10BCAD88)
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