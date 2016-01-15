
#include "stdafx.h"
#include "common.h"
#include "turn_based.h"
#include "util/fixes.h"

class TurnBasedReplacements : public TempleFix
{
public: 
	const char* name() override { return "Turn Based Function Replacements";} 

	static void TbSysNextSthg_100DF5A0(objHndl obj, int idx)
	{
		orgTbSysNextSthg_100DF5A0(obj, idx);
	};
	static void (__cdecl*orgTbSysNextSthg_100DF5A0)(objHndl obj, int idx);

	void apply() override 
	{
		logger->info("Replacing Turn Based System functions");

		replaceFunction(0x100DEE10, _turnBasedSetCurrentActor); 
		replaceFunction(0x100DEE40, _turnBasedGetCurrentActor); 
		orgTbSysNextSthg_100DF5A0 = replaceFunction(0x100DF5A0, TbSysNextSthg_100DF5A0);
	}
} tbReplacements;
void(__cdecl*TurnBasedReplacements::orgTbSysNextSthg_100DF5A0)(objHndl obj, int idx);

#pragma region Turn Based System Implementation
struct TurnBasedSys tbSys;

TurnBasedSys::TurnBasedSys()
{
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