#pragma once

#include "common.h"
#include <temple/dll.h>

struct TurnBasedSys : temple::AddressTable
{
	objHndl * turnBasedCurrentActor;
	void(__cdecl* _CloneInitiativeFromObj)(objHndl obj, objHndl sourceObj);
	GroupArray * groupInitiativeList;

	TurnBasedSys();
	objHndl turnBasedGetCurrentActor();
	void turnBasedSetCurrentActor(objHndl objHnd);
	void CloneInitiativeFromObj(objHndl obj, objHndl sourceObj);
	void TbSysNextSthg_100DF5A0(objHndl obj, int idx);
	void InitiativeListSort();
	int GetInitiativeListIdx();
};

extern TurnBasedSys tbSys;


void _turnBasedSetCurrentActor(objHndl objHnd);
objHndl __cdecl _turnBasedGetCurrentActor();