#pragma once

#include "common.h"
#include <temple/dll.h>

struct TurnBasedSys : temple::AddressTable
{
	objHndl * turnBasedCurrentActor;
	void(__cdecl* _CloneInitiativeFromObj)(objHndl obj, objHndl sourceObj);


	TurnBasedSys();
	objHndl turnBasedGetCurrentActor();
	void turnBasedSetCurrentActor(objHndl objHnd);
	void CloneInitiativeFromObj(objHndl obj, objHndl sourceObj);
	void TbSysNextSthg_100DF5A0(objHndl obj, int idx);
};

extern TurnBasedSys tbSys;


void _turnBasedSetCurrentActor(objHndl objHnd);
objHndl __cdecl _turnBasedGetCurrentActor();