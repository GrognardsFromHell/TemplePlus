#pragma once
#include "stdafx.h"
#include "common.h"


struct TurnBasedSys : AddressTable
{
	objHndl * turnBasedCurrentActor;

	TurnBasedSys();
	objHndl turnBasedGetCurrentActor();
	void turnBasedSetCurrentActor(objHndl objHnd);
};

extern TurnBasedSys tbSys;


void _turnBasedSetCurrentActor(objHndl objHnd);
objHndl __cdecl _turnBasedGetCurrentActor();