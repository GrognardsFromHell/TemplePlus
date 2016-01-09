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
#include "util/fixes.h"
#include "tig/tig_msg.h"
#include "ui/ui.h"
#include "ui/ui_picker.h"
#include "radialmenu.h"
#include "timeevents.h"
#include "raycast.h"
#include "location.h"
#include <infrastructure/keyboard.h>


struct UiIntgameTurnbasedAddresses : temple::AddressTable
{
	BOOL (__cdecl*GameRayCast)(int x, int y, objHndl* obj, int flags);
	void (__cdecl *CursorRenderUpdate)();
	int(__cdecl *AooIndicatorDraw)(AooShaderPacket*, int shaderId);
	int(__cdecl *GetHourglassDepletionState)();
	int * cursorState;
	int(__cdecl * UiActiveRadialMenuHasActiveNode)();
	void(__cdecl* UiActionBarGetValuesFromMovement)();

	AooShaderPacket * aooShaderLocations;
	int * aooShaderLocationsNum;
	int * uiIntgamePathpreviewState;
	float * uiIntgameGreenMoveLength;
	float * uiIntgameTotalMoveLength;
	float * uiIntgamePathdrawCumulativeDist;
	float * uiIntgamePathpreviewFromToDist;

	int * objectHoverTooltipIdx;
	int * uiIntgameActionErrorCode;
	float * movementFeet;
	int* uiWidgetMouseHandlerWidgetId;
	int *uiIntgameAcquireByRaycastOn;
	int *uiIntgameSelectionConfirmed;
	int *uiIntgameWidgetEnteredForRender;
	int *uiIntgameWidgetEnteredForGameplay;
	WidgetType1 ** uiIntgameMainWnd;
	int* uiIntgameWaypointMode;
	objHndl * intgameActor;
	objHndl * uiIntgameTargetObjFromPortraits;
	LocAndOffsets * locFromScreenLoc;
	objHndl * uiIntgameObjFromRaycast;
	int * activePickerIdx;
	ActnSeq* uiIntgameCurSeqBackup;
	LocAndOffsets * uiIntgameWaypointLoc; // the last fixed waypoint in waypoint mode
	

	UiIntgameTurnbasedAddresses()
	{
		rebase(GameRayCast, 0x10022360);
		rebase(CursorRenderUpdate, 0x10097060);
		
		
		
		rebase(UiActiveRadialMenuHasActiveNode, 0x1009AB40);


		rebase(AooIndicatorDraw, 0x10106F30);
		
		rebase(GetHourglassDepletionState, 0x10109D10);

		rebase(UiActionBarGetValuesFromMovement, 0x10173440);


		rebase(activePickerIdx, 0x102F920C);
		rebase(uiIntgameWidgetEnteredForRender, 0x102FC640);
		rebase(uiIntgameWidgetEnteredForGameplay,0x102FC644);

		rebase(uiWidgetMouseHandlerWidgetId, 0x10301324);

		rebase(aooShaderLocations, 0x10B3B948);
		rebase(aooShaderLocationsNum, 0x10B3D598);
		rebase(cursorState, 0x10B3D5AC);

		rebase(locFromScreenLoc, 0x10C040D0);
		rebase(uiIntgameObjFromRaycast, 0x10C040E8);
		rebase(uiIntgameWaypointLoc, 0x10C040F8);
		rebase(uiIntgameMainWnd, 0x10C04108);
		rebase(uiIntgameAcquireByRaycastOn, 0x10C0410C);
		rebase(uiIntgameSelectionConfirmed, 0x10C04110);
		rebase(uiIntgameWaypointMode, 0x10C04114);
		rebase(intgameActor, 0x10C04118);
		rebase(uiIntgameTargetObjFromPortraits, 0x10C04120);
		


		rebase(uiIntgamePathpreviewState, 0x115B1E40);
		rebase(uiIntgameGreenMoveLength, 0x115B1E44);
		rebase(uiIntgameTotalMoveLength, 0x115B1E48);
		rebase(uiIntgamePathdrawCumulativeDist, 0x115B1E60);
		rebase(uiIntgamePathpreviewFromToDist, 0x115B1E78);

		rebase(objectHoverTooltipIdx, 0x11869294);
		rebase(uiIntgameActionErrorCode, 0x11869298);
		rebase(movementFeet, 0x11869240);
		rebase(uiIntgameCurSeqBackup, 0x118692A0);
	}

	
} addresses;

 

class UiIntegameTurnbased : public TempleFix
{
public: 
	const char* name() override { return "UI Intgame Turnbased" "Function Replacements";} 

	static void IntgameTurnbasedRender(int widId);
	static void (__cdecl* orgIntgameTurnbasedRender)(int widId);
	static void UiIntgameBackupCurSeq();
	static void SeqPickerTargetingTypeReset();
	
	/*
	this function is in charge of either executing the action sequence
	*/
	static int UiIntgamePathSequenceHandler(TigMsgMouse* msg);
	static int(__cdecl* orgUiIntgamePathPreviewHandler)(TigMsgMouse*msg);
	static void UiIntgameGenerateSequence(int isUnnecessary);
	static void(__cdecl*orgUiIntgameGenerateSequence)(int isUnnecessary);
	static BOOL UiIntgameRaycast(objHndl* obj, int x, int y, int flags);
	static int IntgameValidateMouseSelection(TigMsgMouse*msg);

	void apply() override 
	{
		replaceFunction(0x10097320, HourglassUpdate);
		orgIntgameTurnbasedRender = replaceFunction(0x10173F70, IntgameTurnbasedRender);
		orgUiIntgamePathPreviewHandler = replaceFunction(0x10174790, UiIntgamePathSequenceHandler);
		orgUiIntgameGenerateSequence = replaceFunction(0x10174100, UiIntgameGenerateSequence);
	}
} uiIntgameTurnbasedReplacements;

void UiIntegameTurnbased::IntgameTurnbasedRender(int widId)
{
	auto shit1 = *addresses.uiIntgameAcquireByRaycastOn;
	if (shit1)
	{
		int dummy = 1;
	}
	auto shit4 = *addresses.uiIntgameSelectionConfirmed;
	if (shit4)
	{
		int dummy = 1;
	}
	auto shit2 = *addresses.uiIntgameWidgetEnteredForRender;
	if (shit2)
	{
		int dummy = 1;
	}

	auto shit3 = *addresses.uiIntgameWidgetEnteredForGameplay;
	if (shit3)
	{
		int dummy = 1;
	}
	auto shit5 = *addresses.activePickerIdx;
	if (shit5 >= 0)
	{
		int dummy = 1;
	}

	orgIntgameTurnbasedRender(widId);
}

void UiIntegameTurnbased::UiIntgameBackupCurSeq()
{
	*addresses.uiIntgameCurSeqBackup = *(*actSeqSys.actSeqCur);
}

void UiIntegameTurnbased::SeqPickerTargetingTypeReset()
{
	*actSeqSys.seqPickerTargetingType = -1;
	*actSeqSys.seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
	*actSeqSys.seqPickerD20ActnData1 = 0;
}

int UiIntegameTurnbased::UiIntgamePathSequenceHandler(TigMsgMouse* msg)
{
	if (uiPicker.PickerActiveCheck())
	{
		return 0;
	}

	bool performSeq = true;
	objHndl actor = tbSys.turnBasedGetCurrentActor();
	if (*addresses.uiIntgameAcquireByRaycastOn)
	{
		
		if (*addresses.uiIntgameSelectionConfirmed && !actSeqSys.isPerforming(actor))
		{
			if ( (infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU) || *addresses.uiIntgameWaypointMode)
				&& !*addresses.uiIntgameObjFromRaycast)
			{
				LocAndOffsets curd20aTgtLoc;
				actSeqSys.GetPathTargetLocFromCurD20Action(&curd20aTgtLoc);
				if (locSys.Distance3d(curd20aTgtLoc, *addresses.locFromScreenLoc) >= 24.0)
				{
					performSeq = false;
				} 
				else if ( *addresses.uiIntgameWaypointMode == 0
					|| addresses.locFromScreenLoc->location != addresses.uiIntgameWaypointLoc->location)
				{
					// this initiates waypoint mode
					*addresses.uiIntgameWaypointMode = 1;
					*addresses.uiIntgameWaypointLoc = *addresses.locFromScreenLoc;
					UiIntgameBackupCurSeq();
					performSeq = false;
				}
			}
		} else
		{
			performSeq = false;
		}
	}

	if (performSeq)
	{
		addresses.UiActionBarGetValuesFromMovement();
		actSeqSys.sequencePerform();
		*addresses.uiIntgameTargetObjFromPortraits = 0i64;
		*addresses.uiIntgameWaypointMode = 0;
		SeqPickerTargetingTypeReset();
		auto comrade = party.GetFellowPc(actor);
		char text[1000];
		int soundId;
		critterSys.GetCritterVoiceLine(actor, comrade, text, &soundId);
		critterSys.PlayCritterVoiceLine(actor, comrade, text, soundId);
	}
	*addresses.uiIntgameAcquireByRaycastOn = 0;
	*addresses.uiIntgameObjFromRaycast = 0i64;
	IntgameValidateMouseSelection(msg);
	return 1;
	// return orgUiIntgamePathPreviewHandler(msg);
}

void UiIntegameTurnbased::UiIntgameGenerateSequence(int isUnnecessary)
{
	orgUiIntgameGenerateSequence(isUnnecessary);
}

BOOL UiIntegameTurnbased::UiIntgameRaycast(objHndl* obj, int x, int y, int flags)
{
	if (*addresses.uiIntgameTargetObjFromPortraits)
	{
		*obj = *addresses.uiIntgameTargetObjFromPortraits;
		return 1;
	} else
	{
		return addresses.GameRayCast(x, y, obj, flags);
	}
}

int UiIntegameTurnbased::IntgameValidateMouseSelection(TigMsgMouse* msg)
{
	objHndl obj = 0i64;
	if (uiPicker.PickerActiveCheck() || radialMenus.ActiveRadialMenuHasActiveNode())
	{
		return 0;
	}
	auto actor = tbSys.turnBasedGetCurrentActor();
	auto actorRadiusSqr = objects.GetRadius(actor);
	actorRadiusSqr *= actorRadiusSqr;
	int a = timeEvents.TimeEventExpireAll(TimeEventSystem::IntgameTurnbased);
	if (!(*addresses.uiIntgameAcquireByRaycastOn))
	{
		UiIntgameGenerateSequence(1);
		return 0;
	}
	objHndl objFromRaycast;
	LocAndOffsets locFromScreen;
	PointNode prevPntNode, pntNode;
	float distSqr;
	if (UiIntgameRaycast(&obj, msg->x, msg->y, 6))
	{
		objFromRaycast = obj;
	} else
	{
		locSys.GetLocFromScreenLocPrecise(msg->x, msg->y, &locFromScreen.location, &locFromScreen.off_x, &locFromScreen.off_y);
		locSys.PointNodeInit(addresses.locFromScreenLoc,  &prevPntNode);
		locSys.PointNodeInit(&locFromScreen, &pntNode);
		objFromRaycast = 0i64;
		distSqr = (prevPntNode.absY - pntNode.absY) * (prevPntNode.absY - pntNode.absY)
			+ (prevPntNode.absX - pntNode.absX) * (prevPntNode.absX - pntNode.absX);
	}

	if (*addresses.uiIntgameSelectionConfirmed)
	{
		if (objFromRaycast != *addresses.uiIntgameObjFromRaycast
			|| !*addresses.uiIntgameObjFromRaycast && distSqr > actorRadiusSqr)
		{
			*addresses.uiIntgameSelectionConfirmed = 0;
			return 0;
		}
	} else if ( objFromRaycast == *addresses.uiIntgameObjFromRaycast
		&& (*addresses.uiIntgameObjFromRaycast || distSqr < actorRadiusSqr))
	{
		*addresses.uiIntgameSelectionConfirmed = 1;
		return 0;
	}

	return 0;
}

void(__cdecl*UiIntegameTurnbased::orgIntgameTurnbasedRender)(int widId);
int(__cdecl* UiIntegameTurnbased::orgUiIntgamePathPreviewHandler)(TigMsgMouse*msg);
void(__cdecl*UiIntegameTurnbased::orgUiIntgameGenerateSequence)(int isUnnecessary);

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


	auto shit = temple::GetRef<int>(0x102FC644);
	if (shit)
	{
		int dummy = 1;
	} else
	{
		int dummy = 1;
	}

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
	*addresses.cursorState = 0;
	if (addresses.UiActiveRadialMenuHasActiveNode())
	{
		*addresses.cursorState = 0;
		addresses.CursorRenderUpdate();
		return;
	}
	if (!combatSys.isCombatActive())
		addresses.CursorRenderUpdate();

	if (v3 || v4)
	{
		v33 = 1;
		if (v4 && a4)
			v33 = 5;
	}
	*addresses.aooShaderLocationsNum = 0;
	*addresses.movementFeet = 0;
	*addresses.objectHoverTooltipIdx = 0;
	*addresses.uiIntgameActionErrorCode = 0;

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
			if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].turnBasedStatusCheck)
			{
				if (d20Sys.d20Defs[d20aType].turnBasedStatusCheck(d20Sys.globD20Action, &tbStat1))
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

	*addresses.uiIntgamePathpreviewState = v34;
	*addresses.uiIntgameGreenMoveLength = moveDistance;
	*addresses.uiIntgameTotalMoveLength = v32;
	*addresses.uiIntgamePathdrawCumulativeDist = 0;
	*addresses.uiIntgamePathpreviewFromToDist = 0;

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

	for (int i = 0; i < *addresses.aooShaderLocationsNum ; i++)
	{
		addresses.AooIndicatorDraw(
			&addresses.aooShaderLocations[i],
			addresses.aooShaderLocations[i].shaderId  );
	}

	if (*addresses.cursorState == 3 
		&& addresses.GetHourglassDepletionState() == 1)
		*addresses.cursorState = 4;
	addresses.CursorRenderUpdate();
}
