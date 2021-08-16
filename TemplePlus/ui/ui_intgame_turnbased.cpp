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
#include "gameview.h"
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
#include "ui_tooltip.h"
#include "ui_render.h"
#include "animgoals/anim.h"
#include <tig\tig_mouse.h>
#include <dungeon_master.h>
#include "gamesystems/objects/objsystem.h"
#include "raycast.h"
#include "hotkeys.h"

UiIntgameTurnbased uiIntgameTb;

struct UiIntgameTurnbasedAddresses : temple::AddressTable {
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
	LgcyWindow** uiIntgameMainWnd;
	int* uiIntgameWaypointMode;
	objHndl* intgameActor;
	objHndl* intgameFocusObj; // hovered char, more or less
	objHndl* uiIntgameTargetObjFromPortraits;
	LocAndOffsets* locFromScreenLoc;
	objHndl* uiIntgameObjFromRaycast;
	int* activePickerIdx;
	ActnSeq* uiIntgameCurSeqBackup;
	ActnSeq* uiIntgameCurSeqBackup_GenerateSequence;
	LocAndOffsets* uiIntgameWaypointLoc; // the last fixed waypoint in waypoint mode
	int64_t* screenXfromMouseEvent;
	int64_t* screenYfromMouseEvent;

	UiIntgameTurnbasedAddresses() {
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
		rebase(intgameFocusObj, 0x10BE60D8);

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
		rebase(uiIntgameCurSeqBackup_GenerateSequence, 0x10B3BF50);
	}


} intgameAddresses;


// UI Intgame Turnbased Function Replacements
class UiIntegameTurnbasedRepl : public TempleFix {
public:
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
		static void CurSeqBackup(); // backs up current sequence for UiIntgameGenerateSequence
		static void RestoreSeqBackup();
		static bool ShouldRollbackSequence();
		static BOOL UiIntgameLocIsFarFromDesignatedLoc(LocAndOffsets loc);
	
	static int UiIntgameMsgHandler(int widId, TigMsg* msg);
		static bool ToggleAcquisition(TigMsg* msg);
		static bool ResetViaRmb(TigMsg* msg);
	static int (__cdecl*orgUiIntgameMsgHandler)(int widId, TigMsg* msg);

	static BOOL UiIntgameRaycast(objHndl* obj, int x, int y, GameRaycastFlags flags);
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

		replaceFunction<void(__cdecl)()>(0x10097060, [](){
			uiIntgameTb.CursorRenderUpdate();
		});
		replaceFunction<BOOL(__cdecl)(objHndl)>(0x10173B30, [](objHndl handle){
			return uiIntgameTb.AooPossible(handle) ? TRUE : FALSE;
		});
	}
} uiIntgameTurnbasedReplacements;

void UiIntegameTurnbasedRepl::IntgameTurnbasedRender(int widId) {

	auto widEntered = *intgameAddresses.uiIntgameWidgetEnteredForRender;
	if (!widEntered){
		*intgameAddresses.intgameFocusObj = 0i64;
	}


	if (uiPicker.PickerActiveCheck()){

		auto showPreview = 0;
		if (infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU))
			showPreview = UITB_ShowPathPreview;
		int flags = (*intgameAddresses.uiIntgameWaypointMode) || showPreview;
		HourglassUpdate(*intgameAddresses.uiIntgameAcquireByRaycastOn, *intgameAddresses.uiIntgameSelectionConfirmed, flags);
		return;
	}


	if (widEntered){
		auto showPreview = 0;
		if (infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU))
			showPreview = UITB_ShowPathPreview;
		int flags = (*intgameAddresses.uiIntgameWaypointMode) || showPreview;
		HourglassUpdate(*intgameAddresses.uiIntgameAcquireByRaycastOn, *intgameAddresses.uiIntgameSelectionConfirmed, flags);

		if (combatSys.isCombatActive()){

			// display hovered character tooltip
			std::string tooltipText;
			tooltipText.resize(3000);
			auto getTooltipTextFromIntgameUpdateOutput = temple::GetRef<bool(__cdecl)(char*)>(0x1008B6B0);

			if (getTooltipTextFromIntgameUpdateOutput(&tooltipText[0])){

				auto ttLen = tooltipText.size();

				// trim last \n
				if (ttLen > 0 && tooltipText[ttLen - 1] == '\n')
					tooltipText[ttLen - 1] = 0;

				int x = (int)(*intgameAddresses.screenXfromMouseEvent);
				int y = (int)(*intgameAddresses.screenYfromMouseEvent);

				auto ttStyle = tooltips.GetStyle(0);
				UiRenderer::PushFont(ttStyle.fontName, ttStyle.fontSize);
				TigTextStyle ttTextStyle;


				ColorRect textColor(XMCOLOR(-1));
				ColorRect shadowColor(XMCOLOR(0xFF000000));
				ColorRect bgColor(XMCOLOR(0x99111111));
				ttTextStyle.kerning = 2;
				ttTextStyle.tracking = 2;
				ttTextStyle.flags = 0xC08;
				ttTextStyle.field2c = -1;
				ttTextStyle.colors4 = nullptr;
				ttTextStyle.colors2 = nullptr;
				ttTextStyle.field0 = 0;
				ttTextStyle.leading = 0;
				ttTextStyle.textColor = &textColor;
				ttTextStyle.shadowColor = &shadowColor;
				ttTextStyle.bgColor = &bgColor;

				TigRect extents = UiRenderer::MeasureTextSize(tooltipText, ttTextStyle);
				extents.x = x;
				if (y > extents.height)
					extents.y = y - extents.height;
				else
					extents.y = y;
				UiRenderer::RenderText(tooltipText, extents, ttTextStyle);
				UiRenderer::PopFont();
			}


			auto drawThreatRanges = temple::GetRef<void(__cdecl)()>(0x10173C00);
			drawThreatRanges();

			// draw circle at waypoint / destination location
			if (*intgameAddresses.uiIntgameWaypointMode){
				auto actor = tbSys.turnBasedGetCurrentActor();
				auto actorRadius = objects.GetRadius(actor);
				LocAndOffsets loc;
				loc = *intgameAddresses.uiIntgameWaypointLoc;
				intgameAddresses.RenderCircle(loc, 1.0, 0x80008000, 0xFF00FF00, actorRadius);
			}

		}
		return;
	}

	// else
	*intgameAddresses.cursorState = 0;
	intgameAddresses.CursorRenderUpdate();

	// orgIntgameTurnbasedRender(widId);
}

void UiIntegameTurnbasedRepl::UiIntgameBackupCurSeq() {
	*intgameAddresses.uiIntgameCurSeqBackup = *(*actSeqSys.actSeqCur);
}

void UiIntegameTurnbasedRepl::SeqPickerTargetingTypeReset() {
	*actSeqSys.seqPickerTargetingType = D20TC_Invalid;
	*actSeqSys.seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
	*actSeqSys.seqPickerD20ActnData1 = 0;
}

int UiIntegameTurnbasedRepl::UiIntgamePathSequenceHandler(TigMsgMouse* msg) {
	if (uiPicker.PickerActiveCheck()) {
		return 0;
	}

	bool performSeq = true;
	objHndl actor = tbSys.turnBasedGetCurrentActor();


	auto &isWaypointMode = *intgameAddresses.uiIntgameWaypointMode;

	auto &tgtFromPortraits   = *intgameAddresses.uiIntgameTargetObjFromPortraits;
	auto &objFromRaycast     = *intgameAddresses.uiIntgameObjFromRaycast;
	auto &acquireByRaycastOn = *intgameAddresses.uiIntgameAcquireByRaycastOn;

	auto &waypointLoc        = *intgameAddresses.uiIntgameWaypointLoc;
	auto &locFromScreenLoc   = *intgameAddresses.locFromScreenLoc;

	if (acquireByRaycastOn) {

		if (*intgameAddresses.uiIntgameSelectionConfirmed && !actSeqSys.isPerforming(actor)) {

			auto altIsPressed = infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU);

			if ( (altIsPressed || isWaypointMode) && !objFromRaycast) {
				LocAndOffsets curd20aTgtLoc;
				actSeqSys.GetPathTargetLocFromCurD20Action(&curd20aTgtLoc);
				if (locSys.Distance3d(curd20aTgtLoc, locFromScreenLoc) >= 24.0) {
					performSeq = false;
				} else if (isWaypointMode == 0
					|| locFromScreenLoc.location != waypointLoc.location) {
					// this initiates waypoint mode
					isWaypointMode = 1;
					waypointLoc = locFromScreenLoc;
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
		logger->info("UiIntgame: \t Issuing Sequence for current actor {}, cur seq: {}", actor, (void*)(*actSeqSys.actSeqCur));
		if (*actSeqSys.actSeqCur) {
			logger->debug("\t\t num actions: {}, action[0] type: {}", (*actSeqSys.actSeqCur)->d20ActArrayNum, (*actSeqSys.actSeqCur)->d20ActArray[0].d20ActType);
		}
		actSeqSys.sequencePerform();
		tgtFromPortraits = 0i64;
		isWaypointMode = 0;
		SeqPickerTargetingTypeReset();
		auto comrade = party.GetFellowPc(actor);
		char text[1000];
		int soundId;
		critterSys.GetOkayVoiceLine(actor, comrade, text, &soundId);
		critterSys.PlayCritterVoiceLine(actor, comrade, text, soundId);
	}
	acquireByRaycastOn = 0;
	objFromRaycast = 0i64;
	IntgameValidateMouseSelection(msg);
	return 1;
	// return orgUiIntgamePathPreviewHandler(msg);
}

void UiIntegameTurnbasedRepl::UiIntgameGenerateSequence(int isUnnecessary) {
	auto curSeq = *actSeqSys.actSeqCur;
	// replacing this just for debug purposes really

	auto actor = tbSys.turnBasedGetCurrentActor();

	if (actSeqSys.isPerforming(actor)){
		return;
	}

	CurSeqBackup();

	auto altIsPressed = infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU);
	
	auto &isWaypointMode = *intgameAddresses.uiIntgameWaypointMode;

	auto &tgtFromPortraits = *intgameAddresses.uiIntgameTargetObjFromPortraits;
	auto &objFromRaycast = *intgameAddresses.uiIntgameObjFromRaycast;

	auto &x = *intgameAddresses.screenXfromMouseEvent;
	auto &y = *intgameAddresses.screenYfromMouseEvent;

	auto &locFromScreenLoc = *intgameAddresses.locFromScreenLoc;

	auto actionLoc = locFromScreenLoc; // 

	if (isWaypointMode || altIsPressed){

		if (tgtFromPortraits){
			objFromRaycast = tgtFromPortraits;
		}
		else {
			auto raycastFlags = GameRaycastFlags::GRF_ExcludeUnconscious | GameRaycastFlags::GRF_ExcludePortals |
				GameRaycastFlags::GRF_ExcludeItems | GameRaycastFlags::GRF_HITTEST_SEL_CIRCLE;

			if (!PickObjectOnScreen(x, y, &objFromRaycast, raycastFlags)){
				objFromRaycast = objHndl::null;
				locSys.GetLocFromScreenLocPrecise(x, y, locFromScreenLoc);
				actionLoc = locFromScreenLoc;

				if (UiIntgameLocIsFarFromDesignatedLoc(locFromScreenLoc)){
					RestoreSeqBackup();
					locFromScreenLoc = *intgameAddresses.uiIntgameWaypointLoc;
					return;
				}
			}
		}
	}

	else if (tgtFromPortraits){
		objFromRaycast = tgtFromPortraits;
	}

	else{
		auto raycastFlags = GameRaycastFlags::GRF_HITTEST_3D | GameRaycastFlags::GRF_ExcludePortals;

		if (!PickObjectOnScreen(x, y, &objFromRaycast, raycastFlags)) {
			objFromRaycast = objHndl::null;
			locSys.GetLocFromScreenLocPrecise(x, y, locFromScreenLoc);
			actionLoc = *intgameAddresses.locFromScreenLoc;
		}
	}
	

	auto mouseLoc = LocAndOffsets::null;
	locSys.GetLocFromScreenLocPrecise(x, y, mouseLoc);
	auto subTile = locSys.subtileFromLoc(&mouseLoc);

	auto canGenerate = objFromRaycast != objHndl::null;
	if (!canGenerate){
		auto sub_100AC5C0 = temple::GetRef<BOOL(__cdecl)(int64_t, int)>(0x100AC5C0);
		if (!sub_100AC5C0(subTile, 1)){
			auto fogFlags = temple::GetRef<uint8_t(__cdecl)(LocAndOffsets)>(0x1002ECB0)(mouseLoc);
			if (fogFlags & 4) // explored
				canGenerate = true;
		}
	}

	if (!canGenerate){
		actor = tbSys.turnBasedGetCurrentActor();
		if (objects.IsPlayerControlled(actor)){
			if (isWaypointMode) {
				RestoreSeqBackup();
			}
			else{
				actSeqSys.curSeqReset(actor);
			}
				
			actSeqSys.TurnBasedStatusInit(actor);
			d20Sys.GlobD20ActnInit();
			objFromRaycast = objHndl::null;
		}
		return;
	}


	if (!actSeqSys.isPerforming(actor)){
		if (objFromRaycast) {
			if (!critterSys.IsCombatModeActive(actor)) {
				combatSys.enterCombat(actor);
			}


			switch (objSystem->GetObject(objFromRaycast)->type) {
			case obj_t_container:
				if (isWaypointMode)
					RestoreSeqBackup();
				else
					actSeqSys.curSeqReset(actor);
				if (isUnnecessary)
					d20Sys.GlobD20ActnSetD20CAF(D20CAF_UNNECESSARY);
				actSeqSys.TurnBasedStatusInit(actor);
				d20Sys.GlobD20ActnInit();
				actSeqSys.ActionTypeAutomatedSelection(objFromRaycast);
				if (!d20Sys.GlobD20ActnSetTarget(objFromRaycast, nullptr)) {
					actSeqSys.ActionAddToSeq();
				}
				break;
			case obj_t_weapon:
			case obj_t_ammo:
			case obj_t_armor:
			case obj_t_money:
			case obj_t_food:
			case obj_t_scroll:
			case obj_t_key:
			case obj_t_written:
			case obj_t_generic:
				if (actSeqSys.SeqPickerHasTargetingType())
					return;
				if (isWaypointMode) {
					RestoreSeqBackup();
				}
				else {
					logger->debug("Generate Sequence: Reseting sequence");
					actSeqSys.curSeqReset(actor);
				}
				if (isUnnecessary)
					d20Sys.GlobD20ActnSetD20CAF(D20CAF_UNNECESSARY);
				actSeqSys.TurnBasedStatusInit(actor);
				d20Sys.GlobD20ActnInit();
				actSeqSys.ActionTypeAutomatedSelection(objFromRaycast);
				if (!d20Sys.GlobD20ActnSetTarget(objFromRaycast, nullptr)) {
					actSeqSys.ActionAddToSeq();
				}
				break;
			case obj_t_pc:
			case obj_t_npc:
				if (isWaypointMode) {
					RestoreSeqBackup();
				}
				else {
					actSeqSys.curSeqReset(actor);
				}
				if (isUnnecessary)
					d20Sys.GlobD20ActnSetD20CAF(D20CAF_UNNECESSARY);
				actSeqSys.TurnBasedStatusInit(actor);
				d20Sys.GlobD20ActnInit();
				actSeqSys.ActionTypeAutomatedSelection(objFromRaycast);
				if (!d20Sys.GlobD20ActnSetTarget(objFromRaycast, nullptr)) {
					actSeqSys.ActionAddToSeq();
				}
				break;
			default:
				break;
			}
		}
		else if (!critterSys.IsDeadOrUnconscious(actor)){
			if (isWaypointMode) {
				RestoreSeqBackup();
			}
			else {
				actSeqSys.curSeqReset(actor);
			}
			actSeqSys.TurnBasedStatusInit(actor);
			d20Sys.GlobD20ActnInit();
			if (isUnnecessary)
				d20Sys.GlobD20ActnSetD20CAF(D20CAF_UNNECESSARY);
			
			actSeqSys.ActionTypeAutomatedSelection(objHndl::null);
			if (!d20Sys.GlobD20ActnSetTarget(objHndl::null, &actionLoc)) {
				actSeqSys.ActionAddToSeq();
			}
		}
	}

	if (ShouldRollbackSequence()){
		if (*actSeqSys.actSeqCur){
			**actSeqSys.actSeqCur = *intgameAddresses.uiIntgameCurSeqBackup_GenerateSequence;
		}
	}

	// orgUiIntgameGenerateSequence(isUnnecessary);
	if (*actSeqSys.actSeqCur != curSeq) {
		logger->info("Sequence switch from Generate Sequence to {}", (void*)*actSeqSys.actSeqCur);
		int dummy = 1;
	};
}

void UiIntegameTurnbasedRepl::CurSeqBackup(){
	if (*actSeqSys.actSeqCur){
		*intgameAddresses.uiIntgameCurSeqBackup_GenerateSequence = **actSeqSys.actSeqCur;
	}
}

void UiIntegameTurnbasedRepl::RestoreSeqBackup()
{
	auto restoreSeqBackup = temple::GetRef<void(__cdecl)()>(0x10093110);
	restoreSeqBackup();
}

bool UiIntegameTurnbasedRepl::ShouldRollbackSequence(){
	return (*pathfindingSys.rollbackSequenceFlag) != 0;
}

BOOL UiIntegameTurnbasedRepl::UiIntgameLocIsFarFromDesignatedLoc(LocAndOffsets loc)
{
	// returns actorR^2 >= dx^2 + dy^2 for waypoint mode
	return temple::GetRef<BOOL(__cdecl)(LocAndOffsets)>(0x10173CF0)(loc);
	return 0;
}

int UiIntegameTurnbasedRepl::UiIntgameMsgHandler(int widId, TigMsg* msg) {

	if (dmSys.IsActive() && !dmSys.IsMinimized()){
		if (msg->type == TigMsgType::MOUSE)
			
			if (dmSys.IsMoused()){
				return FALSE;
			}
			
			auto mouseMsg =*(TigMsgMouse*)msg;
			if ((mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK)
				|| (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED)){

				if (dmSys.IsEditorActive())
					return FALSE;
				if (dmSys.GetHoveredCritter())
					return FALSE;
				
		}

		if (dmSys.IsActionActive())
			return FALSE;

		if (dmSys.IsHandlingMsg()){
			return FALSE;
		}
			
	}
	

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

				if (msg->arg4 & MSF_LMB_CLICK) {
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
				if (msg->arg4 & MSF_RMB_RELEASED) {
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
					static std::vector<int> panic;
					if (actSeqSys.isPerforming(leader))
					{
						if (msg->arg1 == 0x14)
							panic.push_back(1);
						if (panic.size() >= 4)
						{
							gameSystems->GetAnim().Interrupt(leader, AnimGoalPriority::AGP_HIGHEST, true);
							gameSystems->GetAnim().Interrupt(leader, AnimGoalPriority::AGP_1, true);
							(*actSeqSys.actSeqCur)->seqOccupied = 0;
						}
						return 1;
					} else
					{
						if (panic.size())
							panic.clear();
					}
						

					// bind hotkey
					if (hotkeys.IsNormalNonreservedHotkey(msg->arg1)
						&& (hotkeys.IsKeyPressed(VK_LCONTROL) || infrastructure::gKeyboard.IsKeyPressed(VK_RCONTROL)))					{
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
						critterSys.GetOkayVoiceLine(leader, fellowPc, voicelineText, &soundId);
						critterSys.PlayCritterVoiceLine(leader, fellowPc, voicelineText, soundId);
						result = 1;
					}
				} 
				else if (tigMsgType == TigMsgType::WIDGET){
					auto _msg = (TigMsgWidget*)msg;
					if (_msg->widgetEventType == TigMsgWidgetEvent::Exited) {
						*intgameAddresses.uiIntgameAcquireByRaycastOn = 0;
						*intgameAddresses.uiIntgameSelectionConfirmed = 0;
						*intgameAddresses.uiIntgameObjFromRaycast = 0i64;
						*intgameAddresses.uiIntgameWidgetEnteredForGameplay = 0;
					} 
					else if (_msg->widgetEventType == TigMsgWidgetEvent::Entered){
						*intgameAddresses.uiIntgameWidgetEnteredForGameplay = 1;
					}
					
				} 
			}
		}
	}


	if (msg->type == TigMsgType::WIDGET){
		auto _msg = (TigMsgWidget*)msg;
		auto subtype = msg->arg2;
		if (_msg->widgetEventType == TigMsgWidgetEvent::Exited){ 
			*intgameAddresses.uiIntgameWidgetEnteredForRender = 0;
			return result;
		}
		if (_msg->widgetEventType == TigMsgWidgetEvent::Entered){ 
			*intgameAddresses.uiIntgameWidgetEnteredForRender = 1;
		}
	}

	if (*actSeqSys.actSeqCur != initialSeq) {
		logger->info("Sequence switch from Ui Intgame Msg Handler to {}", (void*)*actSeqSys.actSeqCur);
	}

	return result;
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

	if (*actSeqSys.seqPickerTargetingType == D20TC_Invalid)
		return 0;
	
	*actSeqSys.seqPickerTargetingType = D20TC_Invalid;
	*actSeqSys.seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
	*actSeqSys.seqPickerD20ActnData1 = 0;
	return 1;

}

BOOL UiIntegameTurnbasedRepl::UiIntgameRaycast(objHndl* obj, int x, int y, GameRaycastFlags flags) {
	if (*intgameAddresses.uiIntgameTargetObjFromPortraits) {
		*obj = *intgameAddresses.uiIntgameTargetObjFromPortraits;
		return 1;
	} else {
		return PickObjectOnScreen(x, y, obj, flags);
	}
}

int UiIntegameTurnbasedRepl::IntgameValidateMouseSelection(TigMsgMouse* msg) {
	auto obj = objHndl::null;
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
	if (UiIntgameRaycast(&obj, msg->x, msg->y, GRF_HITTEST_3D)) {
		objFromRaycast = obj;
	} else {
		locSys.GetLocFromScreenLocPrecise(msg->x, msg->y, locFromScreen);
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

	auto screenPos = gameView->WorldToScreenUi(location.ToInches3D());
	auto x = (float)(screenPos.x - texWidth / 2);
	auto y = (float)(screenPos.y - texHeight / 2);

	tig->GetShapeRenderer2d().DrawRectangle(x, y, texWidth, texHeight, *texture);

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

bool UiIntgameTurnbased::AooPossible(objHndl handle)
{
	if (!handle) return false;
	auto obj = objSystem->GetObject(handle);
	if (!obj) return false;

	auto isFocus = handle == *intgameAddresses.intgameFocusObj;

	if (isFocus){
		auto loc = obj->GetLocationFull();
		if (!d20Sys.d20QueryWithData(handle, DK_QUE_AOOPossible, handle)){
			return false;
		}
		return combatSys.CanMeleeTargetAtLoc(handle, handle, &loc);
	}

	if (objects.IsPlayerControlled(handle)) {
		return false;
	}

	auto showPreview = hotkeys.IsKeyPressed(VK_LMENU) || hotkeys.IsKeyPressed(VK_RMENU) || (*intgameAddresses.uiIntgameWaypointMode);
	if (!showPreview && (! *intgameAddresses.uiIntgameAcquireByRaycastOn || ! *intgameAddresses.uiIntgameSelectionConfirmed) ){
		return false;
	}
	auto loc = obj->GetLocationFull();
	if (critterSys.IsConcealed(handle) || !d20Sys.d20QueryWithData(handle, DK_QUE_AOOPossible, handle)){
		return false;
	}
	return combatSys.CanMeleeTargetAtLoc(handle, handle, &loc);
}

void UiIntgameTurnbased::CursorRenderUpdate(){
	
	auto &cursorStateForIntgameFocus = temple::GetRef<int>(0x10B3D5B0);
	auto &cursorState = *intgameAddresses.cursorState;
	auto &cursorPrevState = temple::GetRef<int>(0x10B3D5A8);
	auto &widgetEnteredGameplay = *intgameAddresses.uiIntgameWidgetEnteredForGameplay;
	auto &widgetEnteredRender = *intgameAddresses.uiIntgameWidgetEnteredForRender;
	auto &intgameTarget = *intgameAddresses.uiIntgameObjFromRaycast;
	auto &actionFailing = temple::GetRef<int>(0x10B3D5B4);

	// Set special cursors for locked doors
	temple::GetRef<void(__cdecl)()>(0x100936D0)(); // CursorHandleIntgameFocusObj
	if (cursorStateForIntgameFocus){
		cursorState = cursorStateForIntgameFocus;
	}

	auto &curSeq = *actSeqSys.actSeqCur;
	if ( (widgetEnteredGameplay || widgetEnteredRender) 
		&& *actSeqSys.seqPickerTargetingType != D20TC_Invalid
		&& party.GetConsciousPartyLeader() && objects.IsPlayerControlled(curSeq->performer)){
		
		auto seqRenderer = d20Sys.d20Defs[*actSeqSys.seqPickerD20ActnType].seqRenderFunc;
		if (seqRenderer && seqRenderer != d20Sys.d20Defs[D20A_MOVE].seqRenderFunc){
			D20Actn d20a;
			d20a.d20APerformer = party.GetConsciousPartyLeader();
			seqRenderer(&d20a, 0);
		}
	}

	if (*actSeqSys.actSeqPickerActive && widgetEnteredGameplay){
		auto &d20a = temple::GetRef<D20Actn>(0x118CD400);
		auto seqRenderer = d20Sys.d20Defs[d20a.d20ActType].seqRenderFunc;
		if (seqRenderer && seqRenderer != d20Sys.d20Defs[D20A_MOVE].seqRenderFunc) {
			seqRenderer(&d20a, 0);
		}
	}

	if ((widgetEnteredGameplay || widgetEnteredRender) 
		&& curSeq && objects.IsPlayerControlled(curSeq->performer)
		&& !actSeqSys.isPerforming(curSeq->performer)){
		auto seqResultCheck = actSeqSys.ActionSequenceChecksWithPerformerLocation();
		if (seqResultCheck != AEC_OK && seqResultCheck != AEC_NO_ACTIONS || actionFailing){
			temple::GetRef<int>(0x11869298) = seqResultCheck;
			*intgameAddresses.movementFeet = 0;
			*intgameAddresses.objectHoverTooltipIdx = 0;
			if (cursorState < 16)
			{
				cursorState += 16;
				if (cursorState >= 32)
					cursorState = 31;
			}
		}
	}

	auto specialCursor = temple::GetRef<int(__cdecl)()>(0x1009AC00)(); // stuff like dragging party/initiative portraits, wiki help etc.
	if (specialCursor)
		cursorState = specialCursor;

	if (cursorState != cursorPrevState){
		logger->debug("Changing cursor from {} to {}", cursorState, cursorPrevState);
		if (cursorPrevState)
			temple::GetRef<void(__cdecl)()>(0x101DD770)();

		auto static &shaderIds = temple::GetRef<int[32]>(0x118A0900);
		if (cursorState){
			MouseFuncs::SetCursor(shaderIds[cursorState]);
		}
		cursorPrevState = cursorState;
	}

	cursorStateForIntgameFocus = 0;
	actionFailing = 0;

	if (mouseFuncs.GetCursorDrawCallbackId() == 0x1008A240){
		mouseFuncs.SetCursorDrawCallback(nullptr, 0);
	}

	if ( ( widgetEnteredGameplay|| widgetEnteredRender || intgameTarget)
		&& combatSys.isCombatActive() 
		&& !*actSeqSys.actSeqPickerActive && curSeq 
		&& objects.IsPlayerControlled(curSeq->performer)
		&& !actSeqSys.isPerforming(curSeq->performer) 
		&& (*actSeqSys.seqPickerD20ActnType == D20A_STANDARD_ATTACK
			|| *actSeqSys.seqPickerD20ActnType == D20A_TRIP)
		&& (curSeq->tbStatus.attackModeCode < curSeq->tbStatus.baseAttackNumCode + curSeq->tbStatus.numBonusAttacks)){

			mouseFuncs.SetCursorDrawCallback([](int x, int y) {temple::GetRef<void(__cdecl)(int, int, void*)>(0x1008A240)(x,y, nullptr); }, 0x1008A240);
	} 
	else if ( (*actSeqSys.seqPickerD20ActnType == D20A_UNSPECIFIED_ATTACK) 
		&& intgameTarget && cursorState
		&& curSeq 
		&& (curSeq->tbStatus.attackModeCode < curSeq->tbStatus.baseAttackNumCode + curSeq->tbStatus.numBonusAttacks))
	{
		mouseFuncs.SetCursorDrawCallback([](int x, int y) {temple::GetRef<void(__cdecl)(int, int, void*)>(0x1008A240)(x, y, nullptr); }, 0x1008A240);
	}
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


	if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.globD20Action->GetActionDefinitionFlags() & D20ADF::D20ADF_DrawPathByDefault) {
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
		uiIntgameTb.CursorRenderUpdate();
		return;
	}
	if (!combatSys.isCombatActive())
		uiIntgameTb.CursorRenderUpdate();

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
		if (d20aType != D20A_NONE && d20aType >= D20A_UNSPECIFIED_MOVE && d20Sys.globD20Action->GetActionDefinitionFlags() & D20ADF::D20ADF_DrawPathByDefault) {
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
					if (d20Sys.d20Defs[actSeq->d20ActArray[i].d20ActType].seqRenderFunc)
						d20Sys.d20Defs[actSeq->d20ActArray[i].d20ActType].seqRenderFunc(&actSeq->d20ActArray[i], v33);
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
	
	//intgameAddresses.CursorRenderUpdate();
	uiIntgameTb.CursorRenderUpdate();
}

