#pragma once

#include "common.h"
#include <temple/dll.h>

struct TurnBasedSys : temple::AddressTable
{
	objHndl * turnBasedCurrentActor;

	TurnBasedSys();
	objHndl turnBasedGetCurrentActor();
	void turnBasedSetCurrentActor(objHndl objHnd);
};

extern TurnBasedSys tbSys;


void _turnBasedSetCurrentActor(objHndl objHnd);
objHndl __cdecl _turnBasedGetCurrentActor();