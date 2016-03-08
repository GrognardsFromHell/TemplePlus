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
#include "gamesystems/gamesystems.h"
#include "tig/tig_startup.h"
#include "gamesystems/timeevents.h"
#include "raycast.h"
#include "location.h"
#include <infrastructure/keyboard.h>
#include <graphics/mdfmaterials.h>
#include <graphics/shaperenderer2d.h>

UiIntgameTurnbased uiIntgameTb;

struct UiIntgameTurnbasedAddresses : temple::AddressTable {
	BOOL (__cdecl*GameRayCast)(int x, int y, objHndl* obj, int flags);
	void (__cdecl *CursorRenderUpdate)();
	int (__cdecl *AooIndicatorDraw)(LocAndOffsets*, int shaderId);
	int (__cdecl *GetHourglassDepletionState)();
	int* cursorState;
	int (__cdecl * UiActiveRadialMenuHasActiveNode)();
	void (__cdecl* UiActionBarGetValuesFromMovement)();
	void (__cdecl*CreateMovePreview)(PathQueryResult* pqr, UiIntgameTurnbasedFlags flags);
	int (__cdecl*PathpreviewGetFromToDist)(PathQueryResult* path);
	void (__cdecl*RenderCircle)(LocAndOffsets loc, float zoffset, int color1, int color2, float radius);
	void (__cdecl*RenderPositioningBlueCircle)(LocAndOffsets loc, objHndl obj);
	int (__cdecl*PathRenderEndpointCircle)(LocAndOffsets* loc, objHndl obj, float scaleFactor);
	void (__cdecl*AooInterceptArrowDraw)(LocAndOffsets* perfLoc, LocAndOffsets* targetLoc);

	AooShaderPacket* aooShaderLocations;
	int* aooShaderLocationsNum;
	int* uiIntgamePathpreviewState;
	float* uiIntgameGreenMoveLength;
	float* uiIntgameTotalMoveLength;
	float* uiIntgamePathdrawCumulativeDist;
	float* uiIntgamePathpreviewFromToDist;

	int* objectHoverTooltipIdx;
	int* uiIntgameActionErrorCode;
	float* movementFeet;
	int* uiWidgetMouseHandlerWidgetId;
	int* uiIntgameAcquireByRaycastOn;
	int* uiIntgameSelectionConfirmed;
	int* uiIntgameWidgetEnteredForRender;
	int* uiIntgameWidgetEnteredForGameplay;
	WidgetType1** uiIntgameMainWnd;
	int* uiIntgameWaypointMode;
	objHndl* intgameActor;
	objHndl* uiIntgameTargetObjFromPortraits;
	LocAndOffsets* locFromScreenLoc;
	objHndl* uiIntgameObjFromRaycast;
	int* activePickerIdx;
	ActnSeq* uiIntgameCurSeqBackup;
	LocAndOffsets* uiIntgameWaypointLoc; // the last fixed waypoint in waypoint mode
	int64_t* screenXfromMouseEvent;
	int64_t* screenYfromMouseEvent;

	UiIntgameTurnbasedAddresses() {
		rebase(GameRayCast, 0x10022360);


		rebase(CursorRenderUpdate, 0x10097060);


		rebase(UiActiveRadialMenuHasActiveNode, 0x1009AB40);

		rebase(RenderCircle, 0x10106B70);
		rebase(AooIndicatorDraw, 0x10106F30);

		rebase(RenderPositioningBlueCircle, 0x10107580);
		rebase(AooInterceptArrowDraw, 0x101090E0);
		rebase(PathRenderEndpointCircle, 0x10109BE0);
		rebase(PathpreviewGetFromToDist, 0x10109C80);
		rebase(GetHourglassDepletionState, 0x10109D10);
		rebase(CreateMovePreview, 0x10109D50);


		rebase(UiActionBarGetValuesFromMovement, 0x10173440);


		rebase(activePickerIdx, 0x102F920C);
		rebase(uiIntgameWidgetEnteredForRender, 0x102FC640);
		rebase(uiIntgameWidgetEnteredForGameplay, 0x102FC644);

		rebase(uiWidgetMouseHandlerWidgetId, 0x10301324);

		rebase(aooShaderLocations, 0x10B3B948);
		rebase(aooShaderLocationsNum, 0x10B3D598);
		rebase(cursorState, 0x10B3D5AC);

		rebase(locFromScreenLoc, 0x10C040D0);
		rebase(screenYfromMouseEvent, 0x10C040E0);
		rebase(uiIntgameObjFromRaycast, 0x10C040E8);
		rebase(screenXfromMouseEvent, 0x10C040F0);
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


} intgameAddresses;


class UiIntegameTurnbasedRepl : public TempleFix {
public:
	const char* name() override {
		return "UI Intgame Turnbased" "Function Replacements";
	}
	static void IntgameTurnbasedRender(int widId);
	static void (__cdecl* orgIntgameTurnbasedRender)(int widId);
	static void UiIntgameBackupCurSeq();
	static void SeqPickerTargetingTypeReset();

	/*
	this function is in charge of either executing the action sequence
	*/
	static int UiIntgamePathSequenceHandler(TigMsgMouse* msg);
	static int (__cdecl* orgUiIntgamePathSequenceHandler)(TigMsgMouse* msg);

	static void UiIntgameGenerateSequence(int isUnnecessary);
	static void (__cdecl*orgUiIntgameGenerateSequence)(int isUnnecessary);


	static void RestoreSeqBackup();
	
	static int UiIntgameMsgHandler(int widId, TigMsg* msg);
		static bool ToggleAcquisition(TigMsg* msg);
		static bool ResetViaRmb(TigMsg* msg);
	static int (__cdecl*orgUiIntgameMsgHandler)(int widId, TigMsg* msg);

	static BOOL UiIntgameRaycast(objHndl* obj, int x, int y, int flags);
	static int IntgameValidateMouseSelection(TigMsgMouse* msg);

	static void RenderAooIndicator(const LocAndOffsets& location, int materialId);

	static int GetHourglassDepletionState();
	static void HourglassUpdate(int a3, int a4, int flags);

	void apply() override {
		replaceFunction(0x10097320, HourglassUpdate);
		orgIntgameTurnbasedRender = replaceFunction(0x10173F70, IntgameTurnbasedRender);
		orgUiIntgameGenerateSequence = replaceFunction(0x10174100, UiIntgameGenerateSequence);
		orgUiIntgamePathSequenceHandler = replaceFunction(0x10174790, UiIntgamePathSequenceHandler);
		orgUiIntgameMsgHandler = replaceFunction(0x10174A30, UiIntgameMsgHandler);
	}
} uiIntgameTurnbasedReplacements;

void UiIntegameTurnbasedRepl::IntgameTurnbasedRender(int widId) {
	orgIntgameTurnbasedRender(widId);
}

void UiIntegameTurnbasedRepl::UiIntgameBackupCurSeq() {
	*intgameAddresses.uiIntgameCurSeqBackup = *(*actSeqSys.actSeqCur);
}

void UiIntegameTurnbasedRepl::SeqPickerTargetingTypeReset() {
	*actSeqSys.seqPickerTargetingType = -1;
	*actSeqSys.seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
	*actSeqSys.seqPickerD20ActnData1 = 0;
}

int UiIntegameTurnbasedRepl::UiIntgamePathSequenceHandler(TigMsgMouse* msg) {
	if (uiPicker.PickerActiveCheck()) {
		return 0;
	}

	bool performSeq = true;
	objHndl actor = tbSys.turnBasedGetCurrentActor();
	if (*intgameAddresses.uiIntgameAcquireByRaycastOn) {

		if (*intgameAddresses.uiIntgameSelectionConfirmed && !actSeqSys.isPerforming(actor)) {
			if ((infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU) || *intgameAddresses.uiIntgameWaypointMode)
				&& !*intgameAddresses.uiIntgameObjFromRaycast) {
				LocAndOffsets curd20aTgtLoc;
				actSeqSys.GetPathTargetLocFromCurD20Action(&curd20aTgtLoc);
				if (locSys.Distance3d(curd20aTgtLoc, *intgameAddresses.locFromScreenLoc) >= 24.0) {
					performSeq = false;
				} else if (*intgameAddresses.uiIntgameWaypointMode == 0
					|| intgameAddresses.locFromScreenLoc->location != intgameAddresses.uiIntgameWaypointLoc->location) {
					// this initiates waypoint mode
					*intgameAddresses.uiIntgameWaypointMode = 1;
					*intgameAddresses.uiIntgameWaypointLoc = *intgameAddresses.locFromScreenLoc;
					UiIntgameBackupCurSeq();
					performSeq = false;
				}
			}
		} else {
			performSeq = false;
		}
	}

	if (performSeq) {
		intgameAddresses.UiActionBarGetValuesFromMovement();
		logger->info("UiIntgame: \t Issuing Sequence for current actor {} ({}), cur seq: {}", description.getDisplayName(actor), actor, (void*)(*actSeqSys.actSeqCur));
		actSeqSys.sequencePerform();
		*intgameAddresses.uiIntgameTargetObjFromPortraits = 0i64;
		*intgameAddresses.uiIntgameWaypointMode = 0;
		SeqPickerTargetingTypeReset();
		auto comrade = party.GetFellowPc(actor);
		char text[1000];
		int soundId;
		critterSys.GetCritterVoiceLine(actor, comrade, text, &soundId);
		critterSys.PlayCritterVoiceLine(actor, comrade, text, soundId);
	}
	*intgameAddresses.uiIntgameAcquireByRaycastOn = 0;
	*intgameAddresses.uiIntgameObjFromRaycast = 0i64;
	IntgameValidateMouseSelection(msg);
	return 1;
	// return orgUiIntgamePathPreviewHandler(msg);
}

void UiIntegameTurnbasedRepl::UiIntgameGenerateSequence(int isUnnecessary) {
	auto curSeq = *actSeqSys.actSeqCur;
	// replacing this just for debug purposes really
	orgUiIntgameGenerateSequence(isUnnecessary);
	if (*actSeqSys.actSeqCur != curSeq) {
		logger->info("Sequence switch from Generate Sequence to {}", (void*)*actSeqSys.actSeqCur);
		int dummy = 1;
	};
}

void UiIntegameTurnbasedRepl::RestoreSeqBackup()
{
	auto restoreSeqBackup = temple::GetRef<void(__cdecl)()>(0x10093110);
	restoreSeqBackup();
}

int UiIntegameTurnbasedRepl::UiIntgameMsgHandler(int widId, TigMsg* msg) {
	
	auto initialSeq = *actSeqSys.actSeqCur;
	int result = 0;

	auto& intgameActor = temple::GetRef<objHndl>(0x10C04118);

	if (msg->type == TigMsgType::MOUSE) {
		*intgameAddresses.screenXfromMouseEvent = ((TigMsgMouse*)msg)->x;
		*intgameAddresses.screenYfromMouseEvent = ((TigMsgMouse*)msg)->y;
	}

	if (!combatSys.isCombatActive() || radialMenus.ActiveRadialMenuHasActiveNode()) {
		intgameActor = 0i64;
		*intgameAddresses.uiIntgameAcquireByRaycastOn = 0;
		*intgameAddresses.uiIntgameSelectionConfirmed = 0;
	} 
	else
	{
		auto actor = tbSys.turnBasedGetCurrentActor();
		if (actor != intgameActor){
			intgameActor = actor;
			*intgameAddresses.uiIntgameWaypointMode = 0;
			*intgameAddresses.uiIntgameAcquireByRaycastOn = 0;
			*intgameAddresses.uiIntgameSelectionConfirmed = 0;
			if (objects.IsPlayerControlled(actor))	{
				UiIntgameGenerateSequence(0);
			}
		}

		if (objects.IsPlayerControlled(intgameActor)){
			auto tigMsgType = msg->type;
			if (tigMsgType == TigMsgType::MOUSE){
				if (msg->arg4 & MSF_LMB_CLICK){
					if (ToggleAcquisition(msg))
						result = 1;
				}
				if (msg->arg4 & MSF_LMB_RELEASED) {
					if (UiIntgamePathSequenceHandler(reinterpret_cast<TigMsgMouse*>(msg)))
						result = 1;
				}
				if (msg->arg4 & MSF_RMB_CLICK) {
					auto intgameRMB = temple::GetRef<int(__cdecl)(TigMsg*)>(0x10173D70);
					if (intgameRMB(msg))
						result = 1;
				}
				if (msg->arg4 & MSF_RMB_RELEASED){
					if (ResetViaRmb(msg))
						result = 1;
				}
				if (msg->arg4 & MSF_POS_CHANGE) {
					if (IntgameValidateMouseSelection(reinterpret_cast<TigMsgMouse*>(msg)))
						result = 1;
				}
			} 
			else { // widget or keyboard msg
				if (tigMsgType == TigMsgType::KEYSTATECHANGE && (msg->arg2 & 0xFF)== 0)	{
					logger->debug("UiIntgameMsgHandler (KEYSTATECHANGE): msg arg1 {}   arg2 {}", msg->arg1, msg->arg2);
					auto leader = party.GetConsciousPartyLeader();
					if (actSeqSys.isPerforming(leader))
						return 1;

					// bind hotkey
					auto IsNormalNonreservedHotkey = temple::GetRef<bool(__cdecl)(int)>(0x100F3D20);
					if (IsNormalNonreservedHotkey(msg->arg1)
						&& (infrastructure::gKeyboard.IsKeyPressed(VK_LCONTROL) || infrastructure::gKeyboard.IsKeyPressed(VK_RCONTROL)))
					{
						auto leaderLoc = objects.GetLocationFull(leader);
						
						PointNode pnt;
						locSys.PointNodeInit(&leaderLoc, &pnt);
						
						auto worldToLocalScreen = temple::GetRef<void(__cdecl)(PointNode, float*, float*)>(0x10029040);
						float screenX, screenY;
						worldToLocalScreen(pnt, &screenX, &screenY);
						radialMenus.SpawnMenu(int(screenX), int(screenY));
						return radialMenus.MsgHandler(msg);
						
					}

					actSeqSys.TurnBasedStatusInit(leader);
					if (*intgameAddresses.uiIntgameWaypointMode){
						RestoreSeqBackup();
					} else	{
						logger->info("Intgame: Resetting sequence.");
						actSeqSys.curSeqReset(leader);
					}
					d20Sys.GlobD20ActnInit();
					auto radmenuHotkeySthg = temple::GetRef<RadialMenu*(__cdecl)(objHndl, int)>(0x100F3D60);
					if (radmenuHotkeySthg(leader, msg->arg1))
					{
						actSeqSys.ActionAddToSeq();
						actSeqSys.sequencePerform();
						auto fellowPc = party.GetFellowPc(leader);

						char voicelineText[1000];
						voicelineText[0] = 0;
						int soundId = 0;
						critterSys.GetCritterVoiceLine(leader, fellowPc, voicelineText, &soundId);
						critterSys.PlayCritterVoiceLine(leader, fellowPc, voicelineText, soundId);
						result = 1;
					}
				} 
				else if (tigMsgType == TigMsgType::WIDGET){
					if (msg->arg2 == 4) { //exiting widget
						*intgameAddresses.uiIntgameAcquireByRaycastOn = 0;
						*intgameAddresses.uiIntgameSelectionConfirmed = 0;
						*intgameAddresses.uiIntgameObjFromRaycast = 0i64;
						*intgameAddresses.uiIntgameWidgetEnteredForGameplay = 0;
					} 
					else if (msg->arg2 == 3){
						*intgameAddresses.uiIntgameWidgetEnteredForGameplay = 1;
					}
					
				} 
			}
		}
	}


	if (msg->type == TigMsgType::WIDGET){
		auto subtype = msg->arg2;
		if (subtype == 4){ // widget exited
			*intgameAddresses.uiIntgameWidgetEnteredForRender = 0;
			return result;
		}
		if (subtype == 3){ // widget exited
			*intgameAddresses.uiIntgameWidgetEnteredForRender = 1;
		}
	}

	if (*actSeqSys.actSeqCur != initialSeq) {
		logger->info("Sequence switch from Ui Intgame Msg Handler to {}", (void*)*actSeqSys.actSeqCur);
	}

	return result;

	/*result = orgUiIntgameMsgHandler(widId, msg);
	return result;*/
}

bool UiIntegameTurnbasedRepl::ToggleAcquisition(TigMsg* msg)
{
	if (uiPicker.PickerActiveCheck())
		return false;

	if (*intgameAddresses.uiIntgameAcquireByRaycastOn){
		if (msg->arg4 == (MSF_POS_CHANGE | MSF_LMB_DOWN)) {
			IntgameValidateMouseSelection(reinterpret_cast<TigMsgMouse*>(msg));
		}
		return true;
	}

	*intgameAddresses.uiIntgameAcquireByRaycastOn = 1;
	*intgameAddresses.uiIntgameSelectionConfirmed = 1;

	return true;
}

bool UiIntegameTurnbasedRepl::ResetViaRmb(TigMsg* msg)
{
	if (uiPicker.PickerActiveCheck())
		return false;
	if (*intgameAddresses.uiIntgameWaypointMode)
	{
		*intgameAddresses.uiIntgameWaypointMode = 0;
		IntgameValidateMouseSelection((TigMsgMouse*)msg);
		return true;
	}

	if (*actSeqSys.seqPickerTargetingType == -1)
		return 0;
	
	*actSeqSys.seqPickerTargetingType = -1;
	*actSeqSys.seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
	*actSeqSys.seqPickerD20ActnData1 = 0;
	return 1;

}

BOOL UiIntegameTurnbasedRepl::UiIntgameRaycast(objHndl* obj, int x, int y, int flags) {
	if (*intgameAddresses.uiIntgameTargetObjFromPortraits) {
		*obj = *intgameAddresses.uiIntgameTargetObjFromPortraits;
		return 1;
	} else {
		return intgameAddresses.GameRayCast(x, y, obj, flags);
	}
}

int UiIntegameTurnbasedRepl::IntgameValidateMouseSelection(TigMsgMouse* msg) {
	objHndl obj = 0i64;
	if (uiPicker.PickerActiveCheck() || radialMenus.ActiveRadialMenuHasActiveNode()) {
		return 0;
	}
	auto actor = tbSys.turnBasedGetCurrentActor();
	auto actorRadiusSqr = objects.GetRadius(actor);
	actorRadiusSqr *= actorRadiusSqr;

	gameSystems->GetTimeEvent().RemoveAll(TimeEventType::IntgameTurnbased);
	if (!(*intgameAddresses.uiIntgameAcquireByRaycastOn)) {
		UiIntgameGenerateSequence(1);
		return 0;
	}
	objHndl objFromRaycast;
	LocAndOffsets locFromScreen;
	PointNode prevPntNode, pntNode;
	float distSqr;
	if (UiIntgameRaycast(&obj, msg->x, msg->y, 6)) {
		objFromRaycast = obj;
	} else {
		locSys.GetLocFromScreenLocPrecise(msg->x, msg->y, &locFromScreen.location, &locFromScreen.off_x, &locFromScreen.off_y);
		locSys.PointNodeInit(intgameAddresses.locFromScreenLoc, &prevPntNode);
		locSys.PointNodeInit(&locFromScreen, &pntNode);
		objFromRaycast = 0i64;
		distSqr = (prevPntNode.absY - pntNode.absY) * (prevPntNode.absY - pntNode.absY)
			+ (prevPntNode.absX - pntNode.absX) * (prevPntNode.absX - pntNode.absX);
	}

	if (*intgameAddresses.uiIntgameSelectionConfirmed) {
		if (objFromRaycast != *intgameAddresses.uiIntgameObjFromRaycast
			|| !*intgameAddresses.uiIntgameObjFromRaycast && distSqr > actorRadiusSqr) {
			*intgameAddresses.uiIntgameSelectionConfirmed = 0;
			return 0;
		}
	} else if (objFromRaycast == *intgameAddresses.uiIntgameObjFromRaycast
		&& (*intgameAddresses.uiIntgameObjFromRaycast || distSqr < actorRadiusSqr)) {
		*intgameAddresses.uiIntgameSelectionConfirmed = 1;
		return 0;
	}

	return 0;
}

void UiIntegameTurnbasedRepl::RenderAooIndicator(const LocAndOffsets& location, int materialId) {

	auto material = tig->GetMdfFactory().GetById(materialId);

	auto texture = material->GetPrimaryTexture();

	if (!texture) {
		return;
	}

	auto texWidth = (float) texture->GetContentRect().width;
	auto texHeight = (float) texture->GetContentRect().height;

	auto screenPos = tig->GetRenderingDevice().GetCamera().WorldToScreenUi(location.ToInches3D());
	auto x = (float)(screenPos.x - texWidth / 2);
	auto y = (float)(screenPos.y - texHeight / 2);

	tig->GetShapeRenderer2d().DrawRectangle(x, y, texWidth, texHeight, texture);

}

int UiIntegameTurnbasedRepl::GetHourglassDepletionState()
{
	if (*intgameAddresses.uiIntgamePathpreviewFromToDist > *intgameAddresses.uiIntgameGreenMoveLength)	{
		if (*intgameAddresses.uiIntgamePathpreviewFromToDist > *intgameAddresses.uiIntgameTotalMoveLength)
			return 2;
		return 1;
	}

	return 0;
}

void (__cdecl*UiIntegameTurnbasedRepl::orgIntgameTurnbasedRender)(int widId);
int (__cdecl* UiIntegameTurnbasedRepl::orgUiIntgamePathSequenceHandler)(TigMsgMouse* msg);
void (__cdecl*UiIntegameTurnbasedRepl::orgUiIntgameGenerateSequence)(int isUnnecessary);
int (__cdecl*UiIntegameTurnbasedRepl::orgUiIntgameMsgHandler)(int widId, TigMsg* msg);

void UiIntgameTurnbased::CreateMovePreview(PathQueryResult* pqr, UiIntgameTurnbasedFlags flags) {
	intgameAddresses.CreateMovePreview(pqr, flags);
}

int UiIntgameTurnbased::PathpreviewGetFromToDist(PathQueryResult* path) {
	return intgameAddresses.PathpreviewGetFromToDist(path);
}

void UiIntgameTurnbased::RenderCircle(LocAndOffsets loc, float zoffset, int fillColor, int outlineColor, float radius) {
	intgameAddresses.RenderCircle(loc, zoffset, fillColor, outlineColor, radius);
}

void UiIntgameTurnbased::PathRenderEndpointCircle(LocAndOffsets* loc, objHndl obj, float zoffset) {
	intgameAddresses.PathRenderEndpointCircle(loc, obj, zoffset);
}

void UiIntgameTurnbased::RenderPositioningBlueCircle(LocAndOffsets loc, objHndl obj) {
	intgameAddresses.RenderPositioningBlueCircle(loc, obj);
}

void UiIntgameTurnbased::AooInterceptArrowDraw(LocAndOffsets* perfLoc, LocAndOffsets* targetLoc) {
	intgameAddresses.AooInterceptArrowDraw(perfLoc, targetLoc);
}

void UiIntegameTurnbasedRepl::HourglassUpdate(int intgameAcquireOn, int intgameSelectionConfirmed, int showPathPreview) {
	int _showPathPreview = showPathPreview;
	int v33 = 0;
	float greenMoveLength = 0.0;
	float totalMoveLength = 0.0;
	ActnSeq* actSeq = *actSeqSys.actSeqCur;
	TurnBasedStatus* tbStat = nullptr;
	int hourglassState = -1;
	float moveSpeed = 0.0;
	D20ActionType d20aType = d20Sys.globD20Action->d20ActType;
	TurnBasedStatus tbStat1;


	if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].flags & D20ADF::D20ADF_DrawPathByDefault) {
		_showPathPreview = 1;
		intgameAcquireOn = 1;
		intgameSelectionConfirmed = 1;
	} else {
		_showPathPreview = showPathPreview;
	}
	if (d20aType != D20A_NONE){
		int dummy = 1;
	}
	*intgameAddresses.cursorState = 0;
	if (intgameAddresses.UiActiveRadialMenuHasActiveNode()) {
		*intgameAddresses.cursorState = 0;
		intgameAddresses.CursorRenderUpdate();
		return;
	}
	if (!combatSys.isCombatActive())
		intgameAddresses.CursorRenderUpdate();

	if (_showPathPreview || intgameAcquireOn) {
		v33 = 1;
		if (intgameAcquireOn && intgameSelectionConfirmed)
			v33 = 5;
	}
	*intgameAddresses.aooShaderLocationsNum = 0;
	*intgameAddresses.movementFeet = 0;
	*intgameAddresses.objectHoverTooltipIdx = 0;
	*intgameAddresses.uiIntgameActionErrorCode = 0;

	if (!actSeq)
		return;
	objHndl actor = tbSys.turnBasedGetCurrentActor();

	int pathPreviewState;
	if (actSeqSys.isPerforming(actor)) {
		pathPreviewState = 3;
	} else if (intgameAcquireOn) {
		pathPreviewState = (intgameSelectionConfirmed == 0) + 1;
	} else {
		pathPreviewState = 0;
	}

	if (combatSys.isCombatActive()) {
		if (actSeq)
			tbStat = &actSeq->tbStatus;
		hourglassState = tbStat->hourglassState;

		if (tbStat->surplusMoveDistance >= 0.01) // has surplus moves
		{
			if (hourglassState == -1 || actSeqSys.turnBasedStatusTransitionMatrix[hourglassState][1] == -1) // no move action remaining
			{
				greenMoveLength = -1.0;
				totalMoveLength = tbStat->surplusMoveDistance;
			} else // has a move function remaining, thus the surplus move distance must leftover from the "green"
			{
				greenMoveLength = tbStat->surplusMoveDistance;
				moveSpeed = dispatch.Dispatch29hGetMoveSpeed(actSeq->performer, 0) + greenMoveLength;
				totalMoveLength = moveSpeed;
			}
		} else // no surplus move dist
		{
			if (hourglassState == -1 || actSeqSys.turnBasedStatusTransitionMatrix[hourglassState][1] == -1) // no move action remaining
			{
				totalMoveLength = -1.0;
				if (!(tbStat->tbsFlags & 3)) {
					greenMoveLength = 5.0; // five foot step remaining
				} else {
					greenMoveLength = -1.0;
				}
			} else {
				greenMoveLength = dispatch.Dispatch29hGetMoveSpeed(actSeq->performer, 0);
				if (tbStat->tbsFlags & 3) {
					greenMoveLength = -1.0;
				} else if (actSeqSys.GetHourglassTransition(tbStat->hourglassState, 4) == -1) {
					totalMoveLength = greenMoveLength;
					greenMoveLength = -1.0;
				} else {
					moveSpeed = greenMoveLength + greenMoveLength;
					totalMoveLength = moveSpeed;
				}

			}
		}

		d20aType = d20Sys.globD20Action->d20ActType;
		if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].flags & D20ADF::D20ADF_DrawPathByDefault) {
			tbStat1.tbsFlags = tbStat->tbsFlags;
			tbStat1.surplusMoveDistance = tbStat->surplusMoveDistance;
			if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.d20Defs[d20aType].turnBasedStatusCheck) {
				if (d20Sys.d20Defs[d20aType].turnBasedStatusCheck(d20Sys.globD20Action, &tbStat1)) { // error in the check
					totalMoveLength = 0.0;
					greenMoveLength = 0.0;
				} else {
					totalMoveLength = tbStat1.surplusMoveDistance;
					greenMoveLength = tbStat1.surplusMoveDistance;
				}
			}
			if (actSeq->d20ActArray[0].d20Caf & D20CAF_ALTERNATE) // D20CAF_ALTERNATE indicates an invalid path (perhaps due to truncation)
			{
				totalMoveLength = 0.0;
				greenMoveLength = 0.0;
			}

		}
		if ((d20aType == D20A_RUN || d20aType == D20A_CHARGE) && actSeq->d20ActArrayNum > 0) {
			for (int i = 0; i < actSeq->d20ActArrayNum; i++) {
				auto pathQueryResult = actSeq->d20ActArray[i].path;
				if (actSeq->d20ActArray[i].d20ActType == D20A_RUN && pathQueryResult && pathQueryResult->nodeCount != 1) {
					greenMoveLength = 0.0;
					totalMoveLength = 0.0;
				}
			}
		}
	}

	*intgameAddresses.uiIntgamePathpreviewState = pathPreviewState;
	*intgameAddresses.uiIntgameGreenMoveLength = greenMoveLength;
	*intgameAddresses.uiIntgameTotalMoveLength = totalMoveLength;
	*intgameAddresses.uiIntgamePathdrawCumulativeDist = 0;
	*intgameAddresses.uiIntgamePathpreviewFromToDist = 0;

	if (combatSys.isCombatActive() && (!intgameAcquireOn || intgameSelectionConfirmed)) {
		if (!actSeqSys.isPerforming(actor)) {
			int lastActionWithPath = 0;
			if (!objects.IsPlayerControlled(actor))
				return;
			for (int i = 0; i < actSeq->d20ActArrayNum; i++) {
				if (actSeq->d20ActArray[i].path)
					lastActionWithPath = i;
			}
			for (int i = 0; i < actSeq->d20ActArrayNum; i++) {
				if (i == lastActionWithPath)
					v33 |= 2;
				if (actSeq->d20ActArray[i].d20ActType != D20A_NONE) {
					if (d20Sys.d20Defs[actSeq->d20ActArray[i].d20ActType].pickerFuncMaybe)
						d20Sys.d20Defs[actSeq->d20ActArray[i].d20ActType].pickerFuncMaybe(&actSeq->d20ActArray[i], v33);
				}
			}
		}
	} else if (intgameAcquireOn && !intgameSelectionConfirmed && (!actSeq || !objects.IsPlayerControlled(actor) || actSeq->targetObj)) {
		return;
	}

	for (auto i = 0; i < *intgameAddresses.aooShaderLocationsNum; i++) {
		RenderAooIndicator(
			intgameAddresses.aooShaderLocations[i].loc,
			intgameAddresses.aooShaderLocations[i].shaderId
		);
	}

	if (*intgameAddresses.cursorState == 3
		&& GetHourglassDepletionState() == 1){
		*intgameAddresses.cursorState = 4;
	}
	
	intgameAddresses.CursorRenderUpdate();
}

