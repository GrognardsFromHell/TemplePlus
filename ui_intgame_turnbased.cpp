#include "stdafx.h"
#include "common.h"
#include "ui_intgame_turnbased.h"
#include "d20.h"
#include "combat.h"
#include "action_sequence.h"
#include "turn_based.h"
#include "pathfinding.h"
#include "critter.h"
#include "party.h"
#include "obj.h"

struct UiIntgameTurnbasedAddresses : temple::AddressTable
{
	void (__cdecl *sub_10097060)();
	int(__cdecl *sub_10106F30)(void*, int);
	int(__cdecl *sub_10109D10)();
	int * dword_10B3D5AC;
	int(__cdecl * uiRadMenuSthgCheck_115B204C)();
	

	void * stru_10B3B948;
	int * idx_10B3D598;
	int * dword_115B1E40;
	float * flt_115B1E44;
	float * flt_115B1E48;
	int * dword_115B1E60;
	float * flt_115B1E78;

	int * dword_11869294;
	int * dword_11869298;
	float * flt_11869240;

	
	UiIntgameTurnbasedAddresses()
	{
		rebase(sub_10097060, 0x10097060);
		rebase(sub_10109D10, 0x10109D10);
		rebase(idx_10B3D598, 0x10B3D598);
		rebase(dword_10B3D5AC, 0x10B3D5AC);
		rebase(uiRadMenuSthgCheck_115B204C, 0x1009AB40);

		rebase(dword_115B1E40, 0x115B1E40);
		rebase(flt_115B1E44, 0x115B1E44);
		rebase(flt_115B1E48, 0x115B1E48);
		rebase(dword_115B1E60, 0x115B1E60);
		rebase(flt_115B1E78, 0x115B1E78);
		rebase(sub_10106F30, 0x10106F30);
		rebase(stru_10B3B948, 0x10B3B948);

		
		rebase(dword_11869294, 0x11869294);
		rebase(dword_11869298, 0x11869298);
		rebase(flt_11869240, 0x11869240);
	}
} addresses;

 

class UiIntegameTurnbased : public TempleFix
{
public: 
	const char* name() override { return "UI Intgame Turnbased" "Function Replacements";} 
	void apply() override 
	{
		replaceFunction(0x10097320, HourglassUpdate);
	}
} uiIntgameTurnbasedReplacements;

void HourglassUpdate(int a3, int a4, int flags)
{
	int v3 = flags; int v4 = a3;
	int v33 = 0; int v34 = 0;
	float moveDistance = 0.0; float v32 = 0.0;
	ActnSeq * actSeq = *actSeqSys.actSeqCur;
	TurnBasedStatus * tbStat = nullptr;
	int hourglassState = -1;
	float moveSpeed = 0.0;
	D20ActionType d20aType = d20Sys.globD20Action->d20ActType;
	TurnBasedStatus tbStat1 ;
	if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].flags & D20ADF::D20ADF_DrawPathByDefault)
	{
		v3 = 1;
		v4 = 1; 
		a3 = 1;
		a4 = 1;
	} else
	{
		v3 = flags;
		v4 = a3;
	}
	*addresses.dword_10B3D5AC = 0;
	if (addresses.uiRadMenuSthgCheck_115B204C())
	{
		*addresses.dword_10B3D5AC = 0;
		addresses.sub_10097060();
		return;
	}
	if (!combatSys.isCombatActive())
		addresses.sub_10097060();

	if (v3 || v4)
	{
		v33 = 1;
		if (v4 && a4)
			v33 = 5;
	}
	*addresses.idx_10B3D598 = 0;
	*addresses.flt_11869240 = 0;
	*addresses.dword_11869294 = 0;
	*addresses.dword_11869298 = 0;

	if (!actSeq)
		return;
	objHndl actor = tbSys.turnBasedGetCurrentActor();
	if (actSeqSys.isPerforming(actor))
	{
		v34 = 3;
	} else if (v4)
	{
		v34 = (a4 == 0) + 1;
	} else
	{
		v34 = 0;
	}

	if (combatSys.isCombatActive())
	{
		if (actSeq)
			tbStat = &actSeq->tbStatus;
		hourglassState = tbStat->hourglassState;

		if (tbStat->surplusMoveDistance >= 0.01)
		{
			if (hourglassState == -1 || actSeqSys.turnBasedStatusTransitionMatrix[hourglassState][1] == -1)
			{
				moveDistance = -1.0;
				v32 = tbStat->surplusMoveDistance;
			} else
			{
				moveDistance = tbStat->surplusMoveDistance;
				moveSpeed = dispatch.Dispatch29hGetMoveSpeed(actSeq->performer, 0) + moveDistance;
				v32 = moveSpeed;
			}
		} else
		{
			if (hourglassState == -1 || actSeqSys.turnBasedStatusTransitionMatrix[hourglassState][1] == -1)
			{
				v32 = -1.0;
				if (!(tbStat->tbsFlags & 3))
				{
					moveDistance = 5.0;
				} else
				{
					moveDistance = -1.0;
				}
			} else
			{
				moveDistance = dispatch.Dispatch29hGetMoveSpeed(actSeq->performer, 0);
				if (tbStat->tbsFlags & 3)
				{
					moveDistance = -1.0;
				}
				else if (actSeqSys.GetHourglassTransition(tbStat->hourglassState, 4) == -1)
				{
					v32 = moveDistance;
					moveDistance = -1.0;
				}
				else
				{
					moveSpeed = moveDistance + moveDistance;
					v32 = moveSpeed;
				}
					
			}
		}

		d20aType = d20Sys.globD20Action->d20ActType;
		if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].flags & D20ADF::D20ADF_DrawPathByDefault)
		{
			tbStat1.tbsFlags = tbStat->tbsFlags;
			tbStat1.surplusMoveDistance = tbStat->surplusMoveDistance;
			if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].aiCheckMaybe)
			{
				if (d20Sys.d20Defs[d20aType].aiCheckMaybe(d20Sys.globD20Action, &tbStat1))
				{
					v32 = 0.0;
					moveDistance = 0.0;
				}
				else
				{
					v32 = tbStat1.surplusMoveDistance;
					moveDistance = tbStat1.surplusMoveDistance;
				}
			}
			if (actSeq->d20ActArray[0].d20Caf & D20CAF_ALTERNATE)
			{
				v32 = 0.0;
				moveDistance = 0.0;
			}

		}
		if ((d20aType == D20A_RUN || d20aType == D20A_CHARGE) && actSeq->d20ActArrayNum > 0)
		{
			for (int i = 0; i < actSeq->d20ActArrayNum; i++)
			{
				PathQueryResult * pathQueryResult = actSeq->d20ActArray[i].path;
				if (actSeq->d20ActArray[i].d20ActType == D20A_RUN && pathQueryResult && pathQueryResult->nodeCount != 1)
				{
					v32 = 0.0;
					moveDistance = 0.0;
				}
			}
		}
		v4 = a3;

	}

	*addresses.dword_115B1E40 = v34;
	*addresses.flt_115B1E44 = moveDistance;
	*addresses.flt_115B1E48 = v32;
	*addresses.dword_115B1E60 = 0;
	*addresses.flt_115B1E78 = 0;

	if (combatSys.isCombatActive() && (!v4 || a4))
	{
		if (!actSeqSys.isPerforming(actor))
		{
			int lastActionWithPath = 0;
			if (!objects.IsPlayerControlled(actor))
				return;
			for (int i = 0; i < actSeq->d20ActArrayNum; i++)
			{
				if (actSeq->d20ActArray[i].path)
					lastActionWithPath = i;
			}
			for (int i = 0; i < actSeq->d20ActArrayNum; i++)
			{
				if (i == lastActionWithPath)
					v33 |= 2;
				if (actSeq->d20ActArray[i].d20ActType != D20A_NONE)
				{
					if (d20Sys.d20Defs[actSeq->d20ActArray[i].d20ActType].pickerFuncMaybe)
						d20Sys.d20Defs[actSeq->d20ActArray[i].d20ActType].pickerFuncMaybe(&actSeq->d20ActArray[i], v33);
				}
			}
		}
	} else if (v4 && !a4 && (!actSeq || !objects.IsPlayerControlled(actor) || actSeq->targetObj))
	{
		return;
	}

	for (int i = 0; i < *addresses.idx_10B3D598 ; i++)
	{
		addresses.sub_10106F30(((int*)addresses.stru_10B3B948) + 6 * i, * ((int*)addresses.stru_10B3B948 + 6 * i + 4 ) );
	}

	if (*addresses.dword_10B3D5AC == 3 && addresses.sub_10109D10() == 1)
		*addresses.dword_10B3D5AC = 4;
	addresses.sub_10097060();
}