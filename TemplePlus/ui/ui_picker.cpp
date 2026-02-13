#include "stdafx.h"
#include "ui_picker.h"
#include <maps.h>
#include <temple_functions.h>
#include <tig/tig_loadingscreen.h>
#include "ui_intgame_select.h"
#include <tig/tig_msg.h>
#include <tig/tig_keyboard.h>
#include <party.h>
#include <gamesystems/objects/objsystem.h>
#include <critter.h>
#include "ui_render.h"
#include <tutorial.h>
#include <legacyscriptsystem.h>
#include <tig/tig_mouse.h>
#include <config/config.h>
#include "raycast.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/map/sector.h"
#include "gamesystems/legacysystems.h"
#include "hotkeys.h"


const PickerSpec PickerSpec::null;

UiPicker uiPicker;

static struct PickerAddresses : temple::AddressTable {
	bool(__cdecl *ShowPicker)(const PickerArgs &args, void *callbackArgs);
	bool(__cdecl *FreeCurrentPicker)();
	uint32_t(__cdecl * sub_100BA030)(objHndl, PickerArgs*);
	int * activePickerIdx; 
	BOOL(__cdecl* PickerActiveCheck)();
	PickerSpec *pickerSpecs; // size 9 array containign specs for: None, Single, Multi, Cone, Area, Location, Personal, Inventory Item, Ray (corresponding to  UiPickerType )

	PickerAddresses() {
		rebase(pickerSpecs, 0x102F9158);

		rebase(ShowPicker, 0x101357E0);
		rebase(PickerActiveCheck, 0x10135970);
		rebase(FreeCurrentPicker, 0x10137680);
		
		rebase(activePickerIdx, 0x102F920C);
		rebase(sub_100BA030,0x100BA030); 
		rebase(sub_100BA480,0x100BA480); 
		rebase(sub_100BA540,0x100BA540);  // for area range type
		rebase(sub_100BA6A0,0x100BA6A0);  // something with conical selection
	}

	uint32_t (__cdecl *sub_100BA540)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	void (__cdecl *sub_100BA6A0)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t (__cdecl * sub_100BA480)(objHndl objHnd, PickerArgs* pickerArgs);
} addresses;



class UiPickerHooks : TempleFix
{
	static BOOL PickerMultiKeystateChange(TigMsg *msg);
	static BOOL PickerMultiLmbReleased(TigMsg *msg);
	void apply() override{

		// Picker Multi Keystate change - support pressing Enter key
		replaceFunction(0x10137DA0, PickerMultiKeystateChange);
		replaceFunction(0x10137A70, PickerMultiLmbReleased);

		replaceFunction<BOOL(__cdecl)(TigMsg*)>(0x10138170, [](TigMsg*msg){
			return uiPicker.MultiPosChange(msg);
		});

		replaceFunction<BOOL(PickerArgs&, objHndl)>(0x100BA0F0, [](PickerArgs& args, objHndl handle)->BOOL {
			return args.TargetValid(handle) ? TRUE:FALSE;
		});

		// Config Spell Targeting - added fix to allow AI to cast multiple projectiles with Magic Missile and the like on the selected target
		static BOOL(__cdecl *orgConfigSpellTargeting)(PickerArgs&, SpellPacketBody&) = replaceFunction<BOOL(__cdecl)(PickerArgs &, SpellPacketBody &)>(0x100B9690, [](PickerArgs &args, SpellPacketBody &spPkt)	{
			return (BOOL)spellSys.ConfigSpellTargetting(&args, &spPkt);
		});
	
	
		static void(__cdecl*orgRenderPickers)() = replaceFunction<void()>(0x101350F0, [](){
			uiPicker.RenderPickers();
		});

		replaceFunction<BOOL(__cdecl)(TigMsg*)>(0x101375E0, [](TigMsg*msg){
			return uiPicker.PickerMsgHandler(msg);
		});

		// Show Picker
		replaceFunction<BOOL(__cdecl)(PickerArgs&, void *)>(0x101357E0, [](PickerArgs& args, void *callbackArgs)->BOOL
		{
			return uiPicker.ShowPicker(args, callbackArgs);
		});

		// Free Current picker
		replaceFunction<BOOL(__cdecl)()>(0x10137680, [](){
			return uiPicker.FreeCurrentPicker();
		});

		// SetConeTargets
		replaceFunction<BOOL(__cdecl)(LocAndOffsets *, PickerArgs*)>(0x100BA6A0, [](LocAndOffsets * mouseLoc, PickerArgs* args){
			uiPicker.SetConeTargets(mouseLoc, args);
			return 1;
		});

	}
} uiPickerHooks;



BOOL UiPicker::PickerActiveCheck()
{
	return (*addresses.activePickerIdx) >= 0 && (*addresses.activePickerIdx) < MAX_PICKER_COUNT;
}

int UiPicker::ShowPicker(const PickerArgs& args, void* callbackArgs) {

	tutorial.CastingSpells(args.spellEnum);

	auto &pickerIdx = GetActivePickerIdx();

	if (pickerIdx >= MAX_PICKER_COUNT)
		return FALSE;

	
	uiManager->SetHidden(uiIntgameSelect.GetId(), false);
	pickerIdx++;

	if (pickerIdx >= MAX_PICKER_COUNT || pickerIdx < 0)
		return FALSE;
	
	TigMouseState mouseState;
	mouseFuncs.GetState(&mouseState);

	auto &picker = GetActivePicker();
	picker.x = mouseState.x;
	picker.y = mouseState.y;
	picker.field114 = 0;
	picker.cursorStackCount_Maybe = 0;
	picker.tgt = objHndl::null;
	picker.tgtIdx = 0;
	picker.callbackArgs = callbackArgs;
	picker.args = args;
	memset(&picker.args.result, 0, sizeof(PickerResult));

	auto modeTarget = picker.args.GetBaseModeTarget();
	Expects(modeTarget >= UiPickerType::None && modeTarget <= UiPickerType::Wall);

	auto initer = GetPickerSpec(modeTarget).init;
	if (initer)
		initer();

	auto cursorDrawer = GetPickerSpec(modeTarget).cursorTextDraw;
	if (cursorDrawer)
		mouseFuncs.SetCursorDrawCallback([cursorDrawer](int x, int y) {cursorDrawer(x, y, nullptr); }, (int)cursorDrawer);

	return TRUE;
	// return addresses.ShowPicker(args, callbackArgs);
}

void UiPicker::CancelPicker(){
	if (!PickerActiveCheck())
		return;
	auto &picker = GetActivePicker();
	picker.args.result.flags = PickerResultFlags::PRF_CANCELLED;
	if (picker.args.callback){
		picker.args.callback(picker.args.result, picker.callbackArgs);
	}

	// Set Char Portrait callbacks to null
	temple::GetRef<void(__cdecl)(void*, void*, void*)>(0x10131950)(nullptr, nullptr, nullptr);

	picker.args.result.FreeObjlist();
}

uint32_t UiPicker::ObjectNodesFromPickerResult(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA030(objHnd, pickerArgs);
}

BOOL UiPicker::FreeCurrentPicker() {

	auto &pickerIdx = GetActivePickerIdx();
	if (pickerIdx < 0 || pickerIdx >= MAX_PICKER_COUNT)
		return FALSE;


	auto &picker = GetActivePicker();
	picker.args.result.FreeObjlist();
	uiManager->SetHidden(uiIntgameSelect.GetId(), true);
	uiManager->SetHidden(uiIntgameSelect.GetCastNowWndId(), true);

	if (picker.cursorStackCount_Maybe){
		mouseFuncs.ResetCursor();
	}

	auto modeTarget = picker.args.GetBaseModeTarget();
	auto &pickerSpec = GetPickerSpec(modeTarget);

	if (pickerSpec.cursorTextDraw)
		mouseFuncs.SetCursorDrawCallback(nullptr, 0);

	pickerIdx--;
	return TRUE;
	// addresses.FreeCurrentPicker();
}

PickerCacheEntry & UiPicker::GetPicker(int pickerIdx){
	return temple::GetRef<PickerCacheEntry[32]>(0x10BE3490)[pickerIdx];
}

PickerCacheEntry & UiPicker::GetActivePicker(){
	return GetPicker(*addresses.activePickerIdx);
}

int &UiPicker::GetActivePickerIdx(){
	return temple::GetRef<int>(0x102F920C);
}

uint32_t UiPicker::SetSingleTarget(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA480(objHnd, pickerArgs);
}

void UiPicker::SetConeTargets(LocAndOffsets* mouseLoc, PickerArgs* pickerArgs)
{
	auto &pickRes = pickerArgs->result;
	if (pickerArgs){
		pickRes.FreeObjlist();
	}

	pickRes.flags |= (PickerResultFlags::PRF_HAS_LOCATION | PickerResultFlags::PRF_HAS_MULTI_OBJ);

	auto originator = pickerArgs->caster;
	LocAndOffsets originLoc;
	locSys.getLocAndOff(originator, &originLoc);

	auto dir = locSys.GetDirectionVector(originLoc, *mouseLoc);

	auto coneTgtLoc = *mouseLoc;

	auto coneOrigin = originLoc;
	auto angleSize = pickerArgs->degreesTarget * M_PI / 180.0;
	auto angleStart = -atan2(dir.y, dir.x) - angleSize*0.5 + M_PI * 5 / 4;
	auto rangeInches = pickerArgs->radiusTarget * INCH_PER_FEET;

	if (pickerArgs->IsModeTargetFlagSet(UiPickerType::PickOrigin)){
		// mouseLoc becomes the coneOrigin
		auto newTgtLoc = *mouseLoc;
		newTgtLoc.off_x += rangeInches * dir.x;
		newTgtLoc.off_y += rangeInches * dir.y;
		newTgtLoc.Regularize();
		coneOrigin = *mouseLoc;
		coneTgtLoc = newTgtLoc;
	}

	
	if (!(pickerArgs->flagsTarget & UiPickerFlagsTarget::FixedRadius)){
		rangeInches = locSys.Distance3d(coneTgtLoc, coneOrigin);
	}
	pickRes.objList.ListRadius(coneOrigin, rangeInches, angleStart, angleSize, OLC_ALL);

	if (! (pickerArgs->flagsTarget & UiPickerFlagsTarget::Unknown80h)){
		temple::GetRef<void(__cdecl)(LocAndOffsets &, PickerArgs*)>(0x100B9F60)(coneOrigin, pickerArgs); // raycasting etc.
	}

	
	pickerArgs->DoExclusions();
	


//	addresses.sub_100BA6A0(locAndOffsets, pickerArgs);
}

uint32_t UiPicker::GetListRange(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA540(locAndOffsets, pickerArgs);
}

/* 0x10137430 */
bool UiPicker::PickerLosBlocked(objHndl handle, const LocAndOffsets& loc)
{
	if (!handle)
		return false;
	auto obj = objSystem->GetObject(handle);
	if (!obj) return false;

	RaycastPacket rayPkt;
	rayPkt.sourceObj = handle;
	rayPkt.origin = obj->GetLocationFull();
	rayPkt.targetLoc = loc;
	rayPkt.flags = (RaycastFlags)(RaycastFlags::ExcludeItemObjects | RaycastFlags::StopAfterFirstBlockerFound );
	rayPkt.Raycast();
	
	if (rayPkt.resultCount) {
		for (auto i = 0; i < rayPkt.resultCount; i++) {
			auto& rayRes = rayPkt.results[i];
			auto resHandle = rayRes.obj;
			if (!resHandle) {
				if (rayRes.flags & RaycastResultFlags::BlockerSubtile)
					return true;
			}
			else if (objects.IsPortal(resHandle) && !objects.IsPortalOpen(resHandle)) {
				return true;
			}
		}
	}
	return false;
}

LocAndOffsets UiPicker::GetWallEndPoint(){
	return mWallEndPt;
}

UiPicker::UiPicker(){

	InitWallSpec();
	
}

PickerStatusFlags& UiPicker::GetPickerStatusFlags(){
	return temple::GetRef<PickerStatusFlags>(0x10BE5F2C);
}

void UiPicker::InitWallSpec(){
	mWallSpec.msg = &mWallMsgHandlers;
	mWallSpec.name = "SP_M_WALL";
	mWallSpec.init = []() { uiPicker.WallStateReset();  };
	mWallSpec.idx = (int)UiPickerType::Wall;
	mWallSpec.cursorTextDraw = [](int x, int y, void*) { uiPicker.WallCursorText(x, y); };
	mWallMsgHandlers.posChange = [](TigMsg*msg) {return uiPicker.WallPosChange(msg); };
	mWallMsgHandlers.lmbClick = [](TigMsg*msg) {return TRUE; }; // just so it eats the message; otherwise the game things you're doing drag select
	mWallMsgHandlers.lmbReleased = [](TigMsg*msg) {return uiPicker.WallLmbReleased(msg); };
	mWallMsgHandlers.rmbReleased = [](TigMsg*msg) {return uiPicker.WallRmbReleased(msg); };

}

void UiPicker::RenderPickers(){
	
	auto &activePickerIdx = GetActivePickerIdx();
	auto intgameSelTextDraw = temple::GetRef<void(__cdecl)()>(0x10108FA0);
	auto &pickerStatusFlags = GetPickerStatusFlags();

	auto drawSpellPlayerPointer = true;
	auto tgtCount = 0;

	// Get the picker and the originator
	if (activePickerIdx < 0 || activePickerIdx >= MAX_PICKER_COUNT){
		intgameSelTextDraw();
		return;
	}

	auto pick = GetPicker(activePickerIdx);

	auto originator = pick.args.caster;
	if (!originator){
		originator = party.GetConsciousPartyLeader();
		if (!originator)
			return;
	}
	
	auto tgt = pick.tgt;
	if (tgt){
		// renders the circle for the current hovered target (using an appropriate shader based on ok/not ok selection)
		if (pickerStatusFlags & PickerStatusFlags::PSF_Invalid)
			DrawCircleInvalidTarget(tgt, originator, pick.args.spellEnum);
		else
			DrawCircleValidTarget(tgt, originator, pick.args.spellEnum);
	}

	// Draw rotating circles for selected targets
	if (pick.args.result.flags & PickerResultFlags::PRF_HAS_SINGLE_OBJ){
		auto handle = pick.args.result.handle;
		if (pick.args.result.handle == originator){
			drawSpellPlayerPointer = false;
		}

		if (pick.args.IsBaseModeTarget(UiPickerType::Multi)){
			DrawCircleValidTarget(handle, originator, pick.args.spellEnum);
			temple::GetRef<void(objHndl, int)>(0x10108ED0)(handle, 1); // text append
		}
	}

	if (pick.args.result.flags & PickerResultFlags::PRF_HAS_MULTI_OBJ) {
		auto objNode = pick.args.result.objList.objects;
		while (objNode){
			
			auto handle = objNode->handle;
			if (handle == originator)
				drawSpellPlayerPointer = false;

			auto handleObj = objSystem->GetObject(handle);
			auto fogFlags = temple::GetRef<uint8_t(__cdecl)(LocAndOffsets)>(0x1002ECB0)(handleObj->GetLocationFull()) ;

			if ( (config.laxRules && config.showTargetingCirclesInFogOfWar) || !critterSys.IsConcealed(handle) && (fogFlags & 1) ){ // fixed rendering for hidden critters
				DrawCircleValidTarget(handle, originator, pick.args.spellEnum);
				temple::GetRef<void(objHndl, int)>(0x10108ED0)(handle, ++tgtCount); // text append	
			}
			
			objNode = objNode->next;
		}
	}

	// Draw the Spell Player Pointer
	if (drawSpellPlayerPointer){
		if (tgt){
			if (tgt != originator){
				auto tgtLoc = objSystem->GetObject(tgt)->GetLocationFull();
				DrawPlayerSpellPointer(originator, tgtLoc);
			}
		}
		else // draw the picker arrow from the originator to the mouse position
		{
			LocAndOffsets tgtLoc;
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, tgtLoc);

			DrawPlayerSpellPointer(originator, tgtLoc);
		}
	}

	auto origObj = objSystem->GetObject(originator);
	auto originLoc = origObj->GetLocationFull();
	auto originRadius = objects.GetRadius(originator);

	tgt = pick.tgt; //just in case it got updated
	auto tgtObj = objSystem->GetObject(tgt);

	// Area targeting
	if (pick.args.IsBaseModeTarget(UiPickerType::Area)){
		LocAndOffsets tgtLoc;
		if (tgt){
			tgtLoc = tgtObj->GetLocationFull();
		} 
		else{
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, tgtLoc);
		}

		float orgAbsX, orgAbsY, tgtAbsX, tgtAbsY;
		locSys.GetOverallOffset(originLoc, &orgAbsX, &orgAbsY);
		locSys.GetOverallOffset(tgtLoc, &tgtAbsX, &tgtAbsY);

		auto areaRadiusInch = INCH_PER_FEET * pick.args.radiusTarget;

		// Draw the big AoE circle
		DrawCircleAoE(tgtLoc, 1.0, areaRadiusInch, pick.args.spellEnum);


		// Draw Spell Effect pointer (points from AoE to caster)
		auto spellEffectPointerSize = areaRadiusInch / 80.0f * 38.885002f;
		if (spellEffectPointerSize <= 135.744f){
			if (spellEffectPointerSize < 11.312f)
				spellEffectPointerSize = 11.312f;
		} else
		{
			spellEffectPointerSize = 135.744f;
		}

		if (originRadius * 1.5f + areaRadiusInch  + spellEffectPointerSize < locSys.distBtwnLocAndOffs(tgtLoc, originLoc)){
			DrawSpellEffectPointer(tgtLoc, originLoc, areaRadiusInch);
		}	
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Personal)){
		if (tgt && (pick.args.flagsTarget &UiPickerFlagsTarget::Radius) && tgt == originator){

			DrawCircleAoE(originLoc, 1.0, INCH_PER_FEET * pick.args.radiusTarget, pick.args.spellEnum);

		}
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Cone)) {
		LocAndOffsets tgtLoc;
		auto degreesTarget = pick.args.degreesTarget;
		if (!(pick.args.flagsTarget & UiPickerFlagsTarget::Degrees)) {
			degreesTarget = 60.0f;
		}


		auto coneOrigin = originLoc;
		if (pick.args.IsModeTargetFlagSet(UiPickerType::PickOrigin)){
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, coneOrigin);
			auto dir = locSys.GetDirectionVector(originLoc, coneOrigin);

			LocAndOffsets newTgtLoc = coneOrigin;
			newTgtLoc.off_x += dir.x * 4000; newTgtLoc.off_y += dir.y * 4000;
			newTgtLoc.Regularize();
			tgtLoc = newTgtLoc;

		}
		else{ // normal cone emanating from caster
			if (tgt) {
				tgtLoc = tgtObj->GetLocationFull();
			}
			else {
				locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, tgtLoc);
			}
		}

		if (pick.args.flagsTarget & UiPickerFlagsTarget::FixedRadius) {
			tgtLoc = locSys.TrimToLength(coneOrigin, tgtLoc, pick.args.radiusTarget * INCH_PER_FEET);
		}

		DrawConeAoE(coneOrigin, tgtLoc, degreesTarget, pick.args.spellEnum);
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Ray)){
		if (pick.args.flagsTarget & UiPickerFlagsTarget::Range){
			LocAndOffsets tgtLoc;
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, tgtLoc);

			auto rayWidth = pick.args.radiusTarget * INCH_PER_FEET / 2.0f;
			auto rayLength = originRadius + pick.args.trimmedRangeInches;

			DrawRectangleAoE(originLoc, tgtLoc, rayWidth, rayLength, rayLength, pick.args.spellEnum);
		}
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Wall)) {
		if (pick.args.flagsTarget & UiPickerFlagsTarget::Range) {
			LocAndOffsets tgtLoc;
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, tgtLoc);

			if (mWallState == WallPicker_StartPoint){
				
			} 

			else if (mWallState == WallPicker_EndPoint){
				auto rayWidth = pick.args.radiusTarget * INCH_PER_FEET / 2.0f;
				auto rayLength = originRadius + pick.args.trimmedRangeInches;

				auto wallStart = pick.args.result.location;

				DrawRectangleAoE(wallStart, tgtLoc, rayWidth, rayLength, rayLength, pick.args.spellEnum);
			}
			else if (mWallState == WallPicker_CenterPoint){
				
			}
			else if (mWallState == WallPicker_Radius){
				
			}
			
		}
	}

	intgameSelTextDraw();
	return;
}

BOOL UiPicker::PickerMsgHandler(TigMsg* msg){
	if (!PickerActiveCheck())
		return FALSE;

	auto &pick = GetActivePicker();
	if (msg->type == TigMsgType::MOUSE){
		return PickerMsgMouse(msg);
	}

	if (msg->type == TigMsgType::KEYSTATECHANGE || msg->type == TigMsgType::CHAR) {
		return PickerMsgKeyboard(msg);
	}

	return FALSE;
}

BOOL UiPicker::PickerMsgMouse(TigMsg * msg){
	auto activePickerIdx = GetActivePickerIdx();

	if (activePickerIdx >= MAX_PICKER_COUNT || activePickerIdx < 0)
		return FALSE;

	auto &picker = GetPicker(activePickerIdx);
	auto msgMouse = (TigMsgMouse*)msg;
	auto &pickerSpec = GetPickerSpec(picker.args.GetBaseModeTarget());

	// update x/y position
	picker.x = msgMouse->x;
	picker.y = msgMouse->y;

	auto msf = (MouseStateFlags)msgMouse->buttonStateFlags;

	
	auto result = FALSE;

	if (msf & MSF_LMB_CLICK){
		auto handler = pickerSpec.msg->lmbClick;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_LMB_RELEASED) {
		auto handler = pickerSpec.msg->lmbReleased;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_RMB_CLICK) {
		auto handler = pickerSpec.msg->rmbClick;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_RMB_RELEASED) {
		auto handler = pickerSpec.msg->rmbReleased;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_MMB_CLICK) {
		auto handler = pickerSpec.msg->mmbClick;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_MMB_RELEASED) {
		auto handler = pickerSpec.msg->mmbReleased;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_POS_CHANGE) {
		auto handler = pickerSpec.msg->posChange;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_POS_CHANGE_SLOW) {
		auto handler = pickerSpec.msg->posChange2;
		if (handler && handler(msg))
			result = TRUE;
	}

	if (msgMouse->buttonStateFlags & MSF_SCROLLWHEEL_CHANGE) {
		auto handler = pickerSpec.msg->scrollwheel;
		if (handler && handler(msg))
			result = TRUE;
	}

	return result;
}

BOOL UiPicker::PickerMsgKeyboard(TigMsg * msg){

	auto modeTarget = GetActivePicker().args.GetBaseModeTarget();
	auto &pickerSpec = GetPickerSpec(modeTarget);
	
	if (msg->type == TigMsgType::KEYSTATECHANGE){
		auto keystateHandler = pickerSpec.msg->keystateChange;
		if (keystateHandler && keystateHandler(msg))
			return TRUE;
	}

	if (msg->type == TigMsgType::CHAR) {
		auto charFunc = pickerSpec.msg->charFunc;
		if (charFunc && charFunc(msg))
			return TRUE;
	}

	return FALSE;
}

const PickerSpec & UiPicker::GetPickerSpec(UiPickerType modeTarget){

	assert(modeTarget <= UiPickerType::Wall);

	if (modeTarget <= UiPickerType::Ray){
		return addresses.pickerSpecs[(int)modeTarget];
	} 
	else if (modeTarget == UiPickerType::Wall){
		return mWallSpec;
	}
	else{
		logger->error("GetPickerSpec: Invalid picker type!");
		return PickerSpec::null;
	}
}

BOOL UiPicker::WallPosChange(TigMsg * msg){

	auto activePickerIdx = GetActivePickerIdx();
	if (activePickerIdx < 0 || activePickerIdx >= MAX_PICKER_COUNT)
		return FALSE;

	auto msgMouse = (TigMsgMouse*)msg;

	auto &pick = GetActivePicker();
	pick.args.result.FreeObjlist();

	LocAndOffsets mouseLoc;
	locSys.GetLocFromScreenLocPrecise(msgMouse->x, msgMouse->y, mouseLoc);
	
	auto wallState = GetWallState();

	if (wallState == WallPicker_StartPoint || wallState == WallPicker_CenterPoint){
		// get startpoint location from mouse
		
		pick.args.result.location = mouseLoc;
		pick.args.result.offsetz = 0.0f;
		pick.args.result.flags = PickerResultFlags::PRF_HAS_LOCATION;

		return TRUE;
	}


	if (!(pick.args.result.flags & PRF_HAS_LOCATION)) {
		logger->error("WallPosChange: no location set!");
	}
	auto maxRange = INCH_PER_FEET * pick.args.range;
	auto dist = locSys.distBtwnLocAndOffs(mouseLoc, pick.args.result.location);
	if (maxRange > dist)
		maxRange = dist;

	if (wallState == WallPicker_EndPoint) {

		// get radius and range up to mouse (trimmed by walls and such)
		auto radiusInch = pick.args.radiusTarget * INCH_PER_FEET / 2.0f;	
		pick.args.GetTrimmedRange(pick.args.result.location, mouseLoc, radiusInch, maxRange, INCH_PER_FEET * 5.0);
		pick.args.degreesTarget = 2.3561945f - locSys.AngleBetweenPoints(pick.args.result.location, mouseLoc); // putting this in radians, unlike the usual usage

		pick.args.GetTargetsInPath(pick.args.result.location, mouseLoc, radiusInch);

		
		pick.args.DoExclusions();
		return TRUE;

		
	} 
	else if  (wallState == WallPicker_Radius){
		// todo
	}

	return FALSE;
}

BOOL UiPicker::WallLmbReleased(TigMsg * msg)
{
	auto pickerIdx = GetActivePickerIdx();
	if (pickerIdx < 0 || pickerIdx >= MAX_PICKER_COUNT)
		return FALSE;

	auto &pick = GetActivePicker();
	WallPosChange(msg);

	auto wallState = GetWallState();

	if (wallState == WallPicker_StartPoint){
		mWallState = WallPicker_EndPoint;
		pick.args.result.flags |= PickerResultFlags::PRF_HAS_LOCATION;
	}

	else if (wallState == WallPicker_EndPoint){
		auto msgMouse = (TigMsgMouse*)msg;
		LocAndOffsets mouseLoc;
		locSys.GetLocFromScreenLocPrecise(msgMouse->x, msgMouse->y, mouseLoc);
		auto mouseLocTrim = locSys.TrimToLength(pick.args.result.location, mouseLoc, pick.args.trimmedRangeInches);
		mWallEndPt = mouseLocTrim;
		
		return pick.Finalize();
	}

	else if (wallState == WallPicker_CenterPoint) {
		mWallState = WallPicker_Radius;
	}
	
	else if (wallState == WallPicker_Radius) {
		return pick.Finalize();
	}

	return TRUE;
}

BOOL UiPicker::WallRmbReleased(TigMsg * msg)
{
	auto pickerIdx = GetActivePickerIdx();
	if (pickerIdx < 0 || pickerIdx >= MAX_PICKER_COUNT)
		return FALSE;

	auto &pick = GetActivePicker();
	pick.args.result.FreeObjlist();

	auto wallState = GetWallState();
	if (wallState == WallPicker_StartPoint || wallState == WallPicker_CenterPoint){
		pick.args.result.flags = PRF_CANCELLED;
		if (pick.args.callback) {
			pick.args.callback(pick.args.result, pick.callbackArgs);
		}
		return TRUE;
	}

	if (wallState == WallPicker_EndPoint){
		mWallState = WallPicker_StartPoint;
	}
	else if (wallState == WallPicker_Radius){
		mWallState = WallPicker_CenterPoint;
	}
	return TRUE;
}

void UiPicker::WallCursorText(int x, int y){
	auto wallState = GetWallState();

	const int cursorOffset = 22;
	if (wallState == WallPicker_StartPoint){
		static std::string selStartPt("Start Point");
		TigRect destRect(x + cursorOffset, y + cursorOffset, 100, 13);
		
		UiRenderer::RenderText(selStartPt.c_str(), destRect, TigTextStyle::standardWhite);
	} 

	else if (wallState == WallPicker_EndPoint){
		static std::string selStartPt("End Point");
		TigRect destRect(x + cursorOffset, y + cursorOffset, 100, 13);

		UiRenderer::RenderText(selStartPt.c_str(), destRect, TigTextStyle::standardWhite);
	}

	else if (wallState == WallPicker_CenterPoint) {
		static std::string selStartPt("Center Point");
		TigRect destRect(x + cursorOffset, y + cursorOffset, 100, 13);

		UiRenderer::RenderText(selStartPt.c_str(), destRect, TigTextStyle::standardWhite);
	}

	else if (wallState == WallPicker_Radius) {
		static std::string selStartPt("Ring Radius");
		TigRect destRect(x + cursorOffset, y + cursorOffset, 100, 13);

		UiRenderer::RenderText(selStartPt.c_str(), destRect, TigTextStyle::standardWhite);
	}
}

BOOL UiPicker::MultiPosChange(TigMsg * msg)
{
	auto activePickerIdx = GetActivePickerIdx();
	if (activePickerIdx < 0 || activePickerIdx >= MAX_PICKER_COUNT)
		return FALSE;

	auto msgMouse = (TigMsgMouse*)msg;

	auto &pick = GetActivePicker();
	auto &pickStatus = GetPickerStatusFlags();

	LocAndOffsets mouseLoc;
	if (locSys.GetLocFromScreenLocPrecise(msgMouse->x, msgMouse->y, mouseLoc)){
		auto fogFlags = temple::GetRef<uint8_t(__cdecl)(LocAndOffsets)>(0x1002ECB0)(mouseLoc);
		if (! (fogFlags & 4)){

			(int&)pickStatus |= PickerStatusFlags::PSF_Invalid;
			return FALSE; // change from vanilla - decided not to reset the selection list. atari bug #536

			if (pick.args.result.flags & PickerResultFlags::PRF_HAS_MULTI_OBJ){
				pick.args.result.FreeObjlist();
				pick.tgtIdx = 0; // fix: added this - otherwise there was a visual issue where the count wouldn't get reset, even though the actual list did
			}
			pick.args.result.flags = 0;
			//(int&)pickStatus |= PickerStatusFlags::PSF_Invalid; // change from vanilla
			return FALSE;
		}
	}

	auto flgs = pick.GetFlagsFromExclusions();
	objHndl pickedHndl = objHndl::null;
	if (!PickObjectOnScreen(msgMouse->x, msgMouse->y, &pickedHndl, flgs)){
		pick.tgt = objHndl::null;
		(int&)pickStatus &= ~(PickerStatusFlags::PSF_Invalid| PSF_OutOfRange);
		return TRUE;
	}
	pick.tgt = pickedHndl;

	if ( (pick.args.flagsTarget & UiPickerFlagsTarget::Exclude1st )  ){
		if (!pick.args.TargetValid(pickedHndl)){
			(int&)pickStatus |= PickerStatusFlags::PSF_Invalid;
		}
	}
	else if (!pick.args.CheckTargetVsIncFlags(pick.tgt)){
		(int&)pickStatus |= PickerStatusFlags::PSF_Invalid;
	}
	else {
		if (pick.args.TargetValid(pickedHndl)) {
			(int&)pickStatus &= ~PickerStatusFlags::PSF_Invalid;
		}
		else {
			(int&)pickStatus |= PickerStatusFlags::PSF_Invalid;
		}
	}

	if ( !(pick.args.flagsTarget & UiPickerFlagsTarget::LosNotRequired) ){
		if (pick.args.LosBlocked(pickedHndl)){
			(int&)pickStatus |= PickerStatusFlags::PSF_Invalid;
		}
	}

	if (pick.args.flagsTarget & UiPickerFlagsTarget::Range){
		auto casterObj = objSystem->GetObject(pick.args.caster);
		if (!casterObj)
			return FALSE;
		auto casterLoc = casterObj->GetLocation();
		auto pickedObj = objSystem->GetObject(pickedHndl);
		if (!pickedObj) return FALSE;
		auto pickedObjLoc = pickedObj->GetLocation();
		if (locSys.GetTileDeltaMaxBtwnLocs(casterLoc, pickedObjLoc) > pick.args.range){
			(int&)pickStatus |= PickerStatusFlags::PSF_OutOfRange | PickerStatusFlags::PSF_Invalid;
			return FALSE;
		}
	}

	return TRUE;
}

void UiPicker::DrawConeAoE(LocAndOffsets originLoc, LocAndOffsets tgtLoc, float angularWidthDegrees, int spellEnum){
	temple::GetRef<void(__cdecl)(LocAndOffsets, LocAndOffsets, float, int)>(0x10107920)(originLoc, tgtLoc, angularWidthDegrees, spellEnum);
}

void UiPicker::DrawCircleAoE(LocAndOffsets originLoc, float unkFactor, float radiusInch, int spellEnum){
	temple::GetRef<void(__cdecl)(LocAndOffsets, float, float, int)>(0x10107610)(originLoc, unkFactor, radiusInch, spellEnum);
}

void UiPicker::DrawPlayerSpellPointer(objHndl originator, LocAndOffsets tgtLoc){
	temple::GetRef<void(__cdecl)(objHndl, LocAndOffsets&)>(0x10106D10)(originator, tgtLoc);
}

void UiPicker::DrawSpellEffectPointer(LocAndOffsets spellAoECenter, LocAndOffsets pointedToLoc, float aoeRadiusInch){
	temple::GetRef<void(__cdecl)(LocAndOffsets, LocAndOffsets, float)>(0x101068A0)(spellAoECenter, pointedToLoc, aoeRadiusInch);
}

void UiPicker::DrawCircleValidTarget(objHndl tgt, objHndl caster, int spellEnum){
	temple::GetRef<void(objHndl, objHndl, int)>(0x10109940)(tgt, caster, spellEnum); 
}

void UiPicker::DrawCircleInvalidTarget(objHndl tgt, objHndl caster, int spellEnum){
	temple::GetRef<void(objHndl, objHndl, int)>(0x10109980)(tgt, caster, spellEnum);
}

void UiPicker::DrawRectangleAoE(LocAndOffsets originLoc, LocAndOffsets endLoc, float width, float minLength, float maxLength, int spellEnum){
	temple::GetRef<void(__cdecl)(LocAndOffsets, LocAndOffsets, float, float, float, int)>(0x10108340)(originLoc, endLoc, width, minLength, maxLength, spellEnum);
}

bool PickerArgs::IsBaseModeTarget(UiPickerType type){
	auto _type = (uint64_t)type;
	return ( ((uint64_t)modeTarget) & 0xFF) == _type;
}

bool PickerArgs::IsModeTargetFlagSet(UiPickerType type)
{
	return (((uint64_t)modeTarget) & ((uint64_t)type)) == (uint64_t)type;
}

void PickerArgs::SetModeTargetFlag(UiPickerType type){
	*(uint64_t*)(&(this->modeTarget)) |= (uint64_t)type;
}

UiPickerType PickerArgs::GetBaseModeTarget()
{
	return (UiPickerType) (((uint64_t)this->modeTarget) & 0xFF);
}

void PickerArgs::GetTrimmedRange(LocAndOffsets &originLoc, LocAndOffsets &tgtLoc, float radiusInch, float maxRangeInch, float incrementInches){
	this->trimmedRangeInches = maxRangeInch;

	RaycastPacket rayPkt;
	rayPkt.sourceObj = objHndl::null;
	rayPkt.origin = originLoc;
	rayPkt.rayRangeInches = maxRangeInch;
	rayPkt.targetLoc = tgtLoc;
	rayPkt.radius = radiusInch;
	rayPkt.flags = (RaycastFlags)(RaycastFlags::ExcludeItemObjects | RaycastFlags::HasRadius | RaycastFlags::HasRangeLimit);
	rayPkt.Raycast();

	for (auto i = 0; i < rayPkt.resultCount; i++) {
		auto &rayRes = rayPkt.results[i];
		if (rayRes.obj == objHndl::null) {
			auto dist = locSys.distBtwnLocAndOffs(rayPkt.origin, rayRes.loc);
			if (dist < this->trimmedRangeInches)
				this->trimmedRangeInches = dist;
		}
	}

	if (incrementInches > 0){
		this->trimmedRangeInches = ceil(this->trimmedRangeInches / incrementInches) * incrementInches;
	}
}

void PickerArgs::GetTargetsInPath(LocAndOffsets & originLoc, LocAndOffsets & tgtLoc, float radiusInch){

	RaycastPacket rayPkt;
	rayPkt.sourceObj = objHndl::null;
	rayPkt.origin = originLoc;
	rayPkt.rayRangeInches = this->trimmedRangeInches;
	rayPkt.targetLoc = tgtLoc;
	rayPkt.radius = radiusInch;
	rayPkt.flags = (RaycastFlags)(RaycastFlags::ExcludeItemObjects | RaycastFlags::HasRadius | RaycastFlags::HasRangeLimit);
	rayPkt.Raycast();

	
	if (rayPkt.resultCount){
		result.objList.Init();
		for (auto i = 0; i < rayPkt.resultCount; i++) {
			auto &rayRes = rayPkt.results[i];
			if (rayRes.obj) {
				result.objList.PrependHandle(rayRes.obj);
			}
		}
		result.flags |= PickerResultFlags::PRF_HAS_MULTI_OBJ; // is this ok? what if no objects were found? (this is in the original)
	}

}

BOOL UiPickerHooks::PickerMultiKeystateChange(TigMsg * msg){
	if (msg->type != TigMsgType::KEYSTATECHANGE)
		return FALSE;

	if (msg->arg2 & 0xFF)
		return FALSE;

	if (msg->arg1 != DIK_SPACE && msg->arg1 != DIK_RETURN)
		return 0;

	return temple::GetRef<BOOL(__cdecl)(TigMsg*)>(0x10136810)(msg);
}

// Port of 0x10138370, generalized Any/Primary30Feet
BOOL UiPickerHooks::PickerMultiLmbReleased(TigMsg *msg)
{
	auto & activePickerIdx = uiPicker.GetActivePickerIdx();
	if (activePickerIdx < 0 || activePickerIdx >= 32)
		return FALSE;

	auto & picker = uiPicker.GetActivePicker();
	LocAndOffsets loc;

	if (locSys.GetLocFromScreenLocPrecise(msg->arg1, msg->arg2, loc)) {
		static auto GetFogStatus =
			temple::GetRef<uint8_t(LocAndOffsets)>(0x1002ECB0);
		if (!(GetFogStatus(loc) & 4)) {
			uiPicker.GetPickerStatusFlags() |= PSF_Invalid;
			return FALSE;
		}
	}

	auto rayFlags = picker.GetFlagsFromExclusions();
	objHndl tgt;

	if (!PickObjectOnScreen(msg->arg1, msg->arg2, &tgt, rayFlags))
		return TRUE;

	if (!!(picker.args.modeTarget & UiPickerType::OnceMulti)) {
		if (picker.args.ContainsHandle(tgt)) return 1;
	}

	float range = 30.0;
	if (picker.args.degreesTarget > 0.0)
		range = picker.args.degreesTarget;

	auto allowed = [=](objHndl other) {
		return locSys.DistanceToObj(other, tgt) > range;
	};

	if (!!(picker.args.modeTarget & UiPickerType::Any30Feet)) {
		if (picker.args.result.AnyHandle(allowed)) return 1;
	}

	if (!!(picker.args.modeTarget & UiPickerType::Primary30Feet)) {
		if (allowed(picker.args.result.FirstHandle())) return 1;
	}

	if (!picker.args.TargetValid(tgt)) return 1;

	if (!(picker.args.flagsTarget & UiPickerFlagsTarget::LosNotRequired)) {
		if (picker.args.LosBlocked(tgt)) return 1;
	}

	if (!picker.args.CheckTargetVsIncFlags(tgt)) return 1;

	picker.args.result.MultiAppend(tgt);
	++picker.tgtIdx;
	static auto uiCastSpellButtonShow =
		temple::GetRef<int(objHndl, objHndl)>(0x10135B30);
	uiCastSpellButtonShow(tgt, picker.args.caster);
	if (picker.args.result.MultiGetCount() < picker.args.maxTargets) {
		return 1;
	}

	picker.tgt = objHndl::null;
	picker.tgtIdx = 0;
	uiManager->SetHidden(uiIntgameSelect.GetId(), true);
	return picker.Finalize();
}


PickerSpec::PickerSpec() {
	this->name = "";
	this->cursorTextDraw = nullptr;
	this->idx = -1;
	this->init = nullptr;
	this->msg = nullptr;
}

PickerMsgHandlers::PickerMsgHandlers(){
	this->lmbClick = nullptr;
	this->lmbReleased = nullptr;
	this->rmbClick = nullptr;
	this->rmbReleased = nullptr;
	this->mmbClick = nullptr;
	this->mmbReleased = nullptr;

	this->posChange= nullptr;
	this->posChange2 = nullptr;
	this->keystateChange = nullptr;
	this->charFunc = nullptr;
	this->scrollwheel = nullptr;
}

void PickerResult::FreeObjlist(){
	if (this->flags & PickerResultFlags::PRF_HAS_MULTI_OBJ)
		this->objList.Free();
	this->flags &= ~PickerResultFlags::PRF_HAS_MULTI_OBJ;
}

// Port of 0x10136F50
void PickerResult::MultiAppend(objHndl tgt)
{
	static auto internal =
		temple::GetRef<void(__cdecl)(ObjListResult*, objHndl)>(0x1001E890);
	if (flags & PRF_HAS_MULTI_OBJ) {
		internal(&objList, tgt);
	} else if (flags & PRF_HAS_SINGLE_OBJ) {
		auto central = handle;
		FreeObjlist();
		flags &= ~PRF_HAS_SINGLE_OBJ;
		flags |= PRF_HAS_MULTI_OBJ;
		objList.Init();
		internal(&objList, central);
		internal(&objList, tgt);
		handle = objHndl::null;
	} else {
		FreeObjlist();
		flags = PRF_HAS_SINGLE_OBJ;
		handle = tgt;
	}
}

uint32_t PickerResult::MultiGetCount()
{
	if (flags & PRF_HAS_MULTI_OBJ) {
		return objList.CountResults();
	} else {
		flags &= PRF_HAS_SINGLE_OBJ;
		if (flags) return 1;
		else return 0;
	}
}

// Yields the first handle in the picker, if it has one.
objHndl PickerResult::FirstHandle()
{
	if (flags & PRF_HAS_SINGLE_OBJ) {
		return handle;
	}

	if (flags & PRF_HAS_MULTI_OBJ) {
		if (objList.objects)
			return objList.objects->handle;
	}

	return objHndl::null;
}

// Tests whether any handle in the picker satisfies the predicate.
bool PickerResult::AnyHandle(std::function<bool(objHndl)> pred)
{
	if (flags & PRF_HAS_SINGLE_OBJ)
		return pred(handle);

	if (flags & PRF_HAS_MULTI_OBJ)
		return objList.AnyHandle(pred);

	return false;
}

void PickerArgs::DoExclusions(){
	if (flagsTarget & UiPickerFlagsTarget::Exclude1st){
		ExcludeTargets();
		FilterResults();
	} 
	else{
		FilterResults();
		ExcludeTargets();
	}
}

bool PickerArgs::CheckTargetVsIncFlags(objHndl tgt)
{
	auto tgtObj = objSystem->GetObject(tgt);
	if (!tgtObj)
		return false;

	auto incFlags = this->incFlags;
	if ( (incFlags & UiPickerIncFlags::UIPI_Self) && (tgt == this->caster))
		return true;
	if ((incFlags & UiPickerIncFlags::UIPI_Other) && (tgt != this->caster))
		return true;
	auto isCritter = tgtObj->IsCritter();
	if ((incFlags & UiPickerIncFlags::UIPI_NonCritter) && !isCritter)
		return true;
	if (isCritter){
		if ((incFlags & UiPickerIncFlags::UIPI_Dead) && critterSys.IsDeadNullDestroyed(tgt))
			return true;
		if ((incFlags & UiPickerIncFlags::UIPI_Undead) && critterSys.IsUndead(tgt))
			return true;
		if ((incFlags & UiPickerIncFlags::UIPI_Unconscious) 
			&& critterSys.IsDeadOrUnconscious(tgt))
			return true;
		if ((incFlags & UiPickerIncFlags::UIPI_Hostile) && !critterSys.IsFriendly(caster,tgt))
			return true;
		if ((incFlags & UiPickerIncFlags::UIPI_Friendly) && critterSys.IsFriendly(caster, tgt))
			return true;
	}
	else if ((incFlags & UiPickerIncFlags::UIPI_Potion) 
		&& tgtObj->type == obj_t_food && inventory.IsMagicItem(tgt))
		return true;
	else if ((incFlags & UiPickerIncFlags::UIPI_Scroll) && tgtObj->type == obj_t_scroll )
		return true;

	return false;
}

/* 0x100BA0F0 */
bool PickerArgs::TargetValid(objHndl tgt)
{
	auto tgtObj = objSystem->GetObject(tgt);
	if (!tgtObj)
		return false;

	if (objects.IsUntargetable(tgt))
		return false;

	if ( (excFlags & UIPI_Self) && tgt == caster){
		return false;
	}
	if ((excFlags & UIPI_Other) && tgt != caster)
		return false;

	auto isCritter = tgtObj->IsCritter();

	if ( (excFlags & UIPI_NonCritter) && !isCritter){
		return false;
	}

	if (isCritter){
		if ((excFlags & UIPI_Dead) && critterSys.IsDeadNullDestroyed(tgt))
			return false;
		if ((excFlags & UIPI_Undead) && critterSys.IsUndead(tgt))
			return false;
		if ((excFlags & UIPI_Unconscious) && critterSys.IsDeadOrUnconscious(tgt))
			return false;
		if ((excFlags & UiPickerIncFlags::UIPI_Hostile) && !critterSys.IsFriendly(caster, tgt))
			return false;
		if ((excFlags & UiPickerIncFlags::UIPI_Friendly) && critterSys.IsFriendly(caster, tgt))
			return false;
	}
	else if ((excFlags & UiPickerIncFlags::UIPI_Potion)
		&& tgtObj->type == obj_t_food && inventory.IsMagicItem(tgt))
		return false;
	else if ((excFlags & UiPickerIncFlags::UIPI_Scroll) && tgtObj->type == obj_t_scroll)
		return false;

	if ( (flagsTarget & UiPickerFlagsTarget::Range) && 
		(IsBaseModeTarget(UiPickerType::Single) || IsBaseModeTarget(UiPickerType::Multi) )) // Temple+: also check Multi target for range
	{
		auto casterObj = objSystem->GetObject(caster);
		if (!casterObj) return false;
		auto casterLoc = casterObj->GetLocationFull();
		auto tgtLoc = tgtObj->GetLocationFull();
		if (INCH_PER_FEET * this->range + objects.GetRadius(tgt) + objects.GetRadius(caster)
			< locSys.Distance3d(casterLoc, tgtLoc))
			return false;
	}
	
	return true;
}

bool PickerArgs::LosBlocked(objHndl handle)
{
	static auto losBlocked = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x101370E0);
	return losBlocked(caster, handle);
}

bool PickerArgs::SetSingleTgt(objHndl tgt)
{
	auto tgtObj = objSystem->GetObject(tgt);
	if (!tgtObj) {
		return false;
	}
	if (result.flags& PickerResultFlags::PRF_HAS_MULTI_OBJ){
		result.FreeObjlist();
	}
	result.flags = PRF_HAS_LOCATION | PRF_HAS_SINGLE_OBJ;
	result.handle = tgt;
	result.location = tgtObj->GetLocationFull();
	result.offsetz = tgtObj->GetFloat(obj_f_offset_z);
	
	DoExclusions();
	
	return true;
}

bool PickerArgs::ContainsHandle(objHndl tgt)
{
	if (!tgt) return false;
	if (!result.flags) return false;

	if (result.flags & PRF_HAS_SINGLE_OBJ) {
		if (tgt == result.handle) return true;
	}

	if (result.flags & PRF_HAS_MULTI_OBJ) {
		return result.objList.ContainsHandle(tgt);
	}

	return false;
}

void PickerArgs::FreeObjlist()
{
	if (this->result.flags & PRF_HAS_MULTI_OBJ){
		result.FreeObjlist();
	}
	result.flags = 0;
}

void PickerArgs::ExcludeTargets(){
	//temple::GetRef<void(__cdecl)(PickerArgs*)>(0x100BA3C0)(this);

	if (result.flags & PRF_HAS_SINGLE_OBJ){
		if (!TargetValid(result.handle)){
			result.handle = objHndl::null;
			result.flags &= ~PRF_HAS_SINGLE_OBJ;
		}
	}
	if (result.flags & PRF_HAS_MULTI_OBJ) {
		
		// remove until valid found (if any)
		while (result.objList.objects){
			if (TargetValid(result.objList.objects->handle))
				break;
			auto objNode = result.objList.objects;
			result.objList.objects = result.objList.objects->next;
			objNode->ReturnToPool();
		}

		// here, the first one (if any exists) should be valid
		auto objNode = result.objList.objects;
		while (objNode){
			auto next = objNode->next;
			if (!next)
				break;

			if (!TargetValid(next->handle)){ // remove the "next" node
				auto removeNode = objNode->next;
				objNode->next = objNode->next->next;
				removeNode->ReturnToPool();
				continue;
			}
			
			objNode = objNode->next;
		}
	}
}

void PickerArgs::FilterResults(){
	temple::GetRef<void(__cdecl)(PickerArgs*)>(0x100BA030)(this);
}

BOOL PickerCacheEntry::Finalize(){

	if (!(args.flagsTarget & UiPickerFlagsTarget::Exclude1st))
		args.ExcludeTargets();

	auto flags = args.result.flags;
	auto modeTarget = args.GetBaseModeTarget();

	if (modeTarget != UiPickerType::Single && modeTarget != UiPickerType::Multi
		|| (flags & PRF_HAS_MULTI_OBJ || flags & PickerResultFlags::PRF_HAS_SINGLE_OBJ)
		&& (!(flags & PRF_HAS_MULTI_OBJ) || args.result.objList.CountResults())
		&& (!(flags & PRF_HAS_SINGLE_OBJ) || args.result.handle))	
	{
		SpellPacketBody *pkt = (SpellPacketBody*)callbackArgs;

		if (args.callback)
			args.callback(args.result, pkt); // ActionSequenceSystem::SpellPickerCallback
		args.result.FreeObjlist();
		return flags != 0;
	}
	else
	{
		return TRUE;
	}
}

GameRaycastFlags PickerCacheEntry::GetFlagsFromExclusions()
{
	GameRaycastFlags result = GameRaycastFlags::GRF_HITTEST_3D;
	if (hotkeys.IsKeyPressed(VK_LMENU) || hotkeys.IsKeyPressed(VK_RMENU)){
		result = GRF_HITTEST_SEL_CIRCLE;
	}

	if (args.excFlags & UiPickerIncFlags::UIPI_NonCritter){
		(int&)result |= (GRF_ExcludeContainers | GRF_ExcludePortals | GRF_ExcludeScenery);
	}
	if (args.excFlags & UiPickerIncFlags::UIPI_Dead) {
		(int&)result |= (GRF_ExcludeDead );
	}
	if (args.excFlags & UiPickerIncFlags::UIPI_Unconscious) {
		(int&)result |= (GRF_ExcludeUnconscious);
	}
	return result;
}
