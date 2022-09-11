#include "stdafx.h"

#include <temple/dll.h>

#include "combat.h"
#include "tig/tig_mouse.h"
#include "messages/messagequeue.h"
#include "ui_ingame.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"
#include "ui_dialog.h"
#include "gameview.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "gamesystems/mapobjrender.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamerenderer.h"
#include "radialmenu.h"
#include "ai.h"
#include "party.h"
#include "dungeon_master.h"
#include "tig/tig_loadingscreen.h"
#include "tig/tig_console.h"
#include "ui_char.h"
#include "ui_townmap.h"
#include "raycast.h"
#include "hotkeys.h"
#include "infrastructure/keyboard.h"
#include "location.h"
#include "action_sequence.h"
#include "graphics/device.h"

UiInGame::UiInGame(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10112e70);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Intgame");
	}
}
UiInGame::~UiInGame() {
	auto shutdown = temple::GetPointer<void()>(0x10112eb0);
	shutdown();
}
void UiInGame::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10112ec0);
	resize(&resizeArg);
}
void UiInGame::Reset() {
	auto reset = temple::GetPointer<void()>(0x101140b0);
	reset();
}
bool UiInGame::LoadGame(const UiSaveFile &save) {
	auto load = temple::GetPointer<int(const UiSaveFile*)>(0x101140c0);
	return load(&save) == 1;
}
const std::string &UiInGame::GetName() const {
	static std::string name("Intgame");
	return name;
}

void UiInGame::ClearGroupArray()
{
	static auto ClearIntgameGroupArray = temple::GetPointer<int()>(0x10138c70);
	ClearIntgameGroupArray();
}

void UiInGame::AddToGroupArray(objHndl handle)
{
	static auto add_to_intgame_group_arr = temple::GetPointer<void(objHndl)>(0x10138c90);
	add_to_intgame_group_arr(handle);
}

void UiInGame::ResetFocusObject()
{
	static auto ResetIntgameFocusObj = temple::GetPointer<int()>(0x10138c80);
	ResetIntgameFocusObj();
}

void UiInGame::SetFocusObject(objHndl handle)
{
	static auto SetIntgameFocus = temple::GetPointer<void(objHndl)>(0x10138cb0);
	SetIntgameFocus(handle);
}

void UiInGame::ProcessMessage(const TigMsg & msg) {
	DoKeyboardScrolling();


	if (dmSys.IsActive()){

		auto actionWasActive = dmSys.IsActionActive();

		if (dmSys.HandleMsg(msg)){
			return;
		}
		dmSys.SetIsHandlingMsg(false);
			

		if (dmSys.IsMoused()){
			return;
		}

		if (msg.type == TigMsgType::MOUSE){
			auto mouseMsg = *(TigMsgMouse*)&msg;
			if ( (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK)
				 || (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) ){

				if (dmSys.IsEditorActive()) // to release edited obj
					return;
				if (dmSys.GetHoveredCritter()) // to acquire edited obj
					return;
			}
		}
		
		if (dmSys.IsActionActive())
			return;

		if (actionWasActive) {
			auto asd = 1;
		}
	}
	
	

	if (HandleRadialMenuMessage(msg)) {
		return;
	}

	if (HandlePickerMessage(msg)) {
		return;
	}

	if (radialMenus.GetActiveRadialMenuNode() != -1) {
		return;
	}

	if (uiSystems->GetDlg().IsActive())
	{
		if (msg.type == TigMsgType::KEYSTATECHANGE)
		{
			InGameKeyEvent kbEvent(msg);
			uiSystems->GetManager().SetState(0);
			uiSystems->GetManager().HandleKeyEvent(kbEvent);
		}
	}
	else
	{
		if (uiSystems->GetChar().GetInventoryObject() && msg.type == TigMsgType::MOUSE && msg.arg4 & 0x44)
		{
			uiSystems->GetChar().SetInventoryObject(objHndl::null);
			mouseFuncs.SetDraggedIcon(0, 0, 0);
		}

		static int* viewportIds = temple::GetPointer<int>(0x10BD3AF8);
		static uint32_t &viewportIdx = temple::GetRef<uint32_t>(0x10BD3B44);

		
		if (!viewportIds[viewportIdx+1])
		{
			static bool combatModeMsg = false;
			if (combatSys.isCombatActive())
			{
				// First message in combat
				if (!mPrevMsgWasInCombat) {
					mPartyMembersMoving = false;
				}
				mPrevMsgWasInCombat = true;
				HandleCombatMessage(msg);
			}
			else
			{
				mPrevMsgWasInCombat = false;
				HandleNonCombatMessage(msg);
			}
		}
	}

}

bool UiInGame::HandleRadialMenuMessage(const TigMsg &msg) {
	static auto UiRadialMenuMsg = temple::GetPointer<BOOL(const TigMsg &msg)>(0x1013dc90);
	return UiRadialMenuMsg(msg) != 0;
}

bool UiInGame::HandlePickerMessage(const TigMsg &msg) {
	static auto PickerMsgHandler = temple::GetPointer<BOOL(const TigMsg &tigMsg)>(0x101375e0);
	return PickerMsgHandler(msg) != 0;
}

void UiInGame::HandleCombatMessage(const TigMsg & msg)
{
	if (msg.type == TigMsgType::KEYSTATECHANGE)
	{
		HandleCombatKeyStateChange(msg);
	}
	else if (msg.type == TigMsgType::MOUSE)
	{
		if (msg.arg4 & MSF_POS_CHANGE || msg.arg4 & MSF_POS_CHANGE_SLOW) {
			HandleMouseMoveEvent(msg);
		}
	}
}

void UiInGame::HandleNonCombatMessage(const TigMsg & msg)
{
	if (uiSystems->GetChar().IsVisible() && msg.type == TigMsgType::MOUSE)
		return;

	if (msg.type == TigMsgType::KEYSTATECHANGE) {
		return HandleNonCombatKeyStateChange(msg);
	}

	if (msg.type == TigMsgType::MOUSE) {

		auto tigMsgMouse = (TigMsgMouse&)msg;
		auto mouseFlags = tigMsgMouse.buttonStateFlags;
		if (mouseFlags & MSF_LMB_RELEASED){
			return temple::GetRef<void(__cdecl)(const TigMsg&)>(0x10114AF0)(msg); // normal LMB released handler
		}
		else if ( (mouseFlags & MSF_LMB_CLICK) || ( (mouseFlags & (MSF_POS_CHANGE | MSF_LMB_DOWN)) == (MSF_POS_CHANGE | MSF_LMB_DOWN))){
			return temple::GetRef<void(__cdecl)(const TigMsg&)>(0x101148B0)(msg); // LMB drag handler
		}
		else if (mouseFlags & MSF_RMB_CLICK){
			return temple::GetRef<void(__cdecl)(const TigMsg&)>(0x10114D90)(msg); // RMB handler
		}
		else if ( (mouseFlags & (MSF_POS_CHANGE)) || (mouseFlags & (MSF_POS_CHANGE_SLOW) ) ){
			//static auto NormalMsgHandler = temple::GetPointer<void(const TigMsg &msg)>(0x10114e30);
			//NormalMsgHandler(msg);
			return HandleMouseMoveEvent(msg);
		}
	}

}

void UiInGame::HandleNonCombatKeyStateChange(const TigMsg& msg){
	//return temple::GetRef<void(__cdecl)(const TigMsg&)>(0x101130B0)(msg); // normal keystate change handler
	InGameKeyEvent kmsg(msg);
	auto msga = reinterpret_cast<const TigKeyStateChangeMsg&>(msg);
	if (msga.down){
		return;
	}

	auto leader = party.GetConsciousPartyLeader();
	if (leader){
		if (hotkeys.IsReservedHotkey(msga.key)){
			if (hotkeys.IsKeyPressed(VK_LCONTROL) || hotkeys.IsKeyPressed(VK_RCONTROL)){ // trying to assign hotkey to reserved hotkey
				hotkeys.HotkeyReservedPopup(msga.key);
				return;
			}
		}
		else if (hotkeys.IsNormalNonreservedHotkey(msga.key)){
			if (hotkeys.IsKeyPressed(VK_LCONTROL) || hotkeys.IsKeyPressed(VK_RCONTROL)) { // assign hotkey
				auto leaderLoc = objects.GetLocationFull(leader);
				auto screenPos = gameView->WorldToScreenUi(leaderLoc.ToInches3D());

				radialMenus.SpawnMenu(int(screenPos.x), int(screenPos.y));
				radialMenus.MsgHandler(&msg);
				return;
			}
			
			actSeqSys.TurnBasedStatusInit(leader);
			leader = party.GetConsciousPartyLeader();// in case the leader changes somehow...
			logger->info("Intgame: Resetting sequence.");
			actSeqSys.curSeqReset(leader);
			
			d20Sys.GlobD20ActnInit();
			auto radmenuHotkeySthg = temple::GetRef<RadialMenu*(__cdecl)(objHndl, int)>(0x100F3D60);
			if (radmenuHotkeySthg(party.GetConsciousPartyLeader(), msga.key))
			{
				actSeqSys.ActionAddToSeq();
				actSeqSys.sequencePerform();
				auto fellowPc = party.GetFellowPc(leader);

				char voicelineText[1000];
				voicelineText[0] = 0;
				int soundId = 0;
				critterSys.GetOkayVoiceLine(leader, fellowPc, voicelineText, &soundId);
				critterSys.PlayCritterVoiceLine(leader, fellowPc, voicelineText, soundId);
				radialMenus.ClearActiveRadialMenu();
				return;
			}
			
		}
	}

	uiSystems->GetManager().SetState(0);
	uiSystems->GetManager().HandleKeyEvent(kmsg);
}

void UiInGame::HandleCombatKeyStateChange(const TigMsg & msg)
{
	static auto CombatKeystateHandler = temple::GetPointer<void(const TigMsg &msg)>(0x101132b0);
	CombatKeystateHandler(msg);
}

// was 0x10114690
void UiInGame::HandleMouseMoveEvent(const TigMsg & msg)
{
	uiSystems->GetParty().SetHoveredObj(objHndl::null);
	uiSystems->GetParty().SetPressedObj(objHndl::null);

	uiSystems->GetInGame().ResetFocusObject();
	uiSystems->GetInGame().ClearGroupArray();

	if (gameSystems->GetMap().IsClearingMap()) {
		return;
	}

	auto mouseTarget = GetMouseTarget(msg.arg1, msg.arg2);

	if (!mouseTarget) {
		return;
	}

	if (uiSystems->GetChar().IsVisible()
		|| uiSystems->GetTownmap().IsVisible()
		|| uiSystems->GetLogbook().IsVisible()) {
		return;
	}

	if (uiSystems->GetChar().IsLevelingUp()) {
		return;
	}

	auto normalLmbClicked = temple::GetRef<BOOL>(0x10BD3B5C);	
	if (normalLmbClicked == 1)
	{
		auto dragViewport = temple::GetRef<int>(0x10BD3AC8);
		auto mouseDragTarget = temple::GetRef<objHndl>(0x10BD3AD8);
		if (!dragViewport && mouseTarget == mouseDragTarget)
		{
			uiSystems->GetInGame().AddToGroupArray(mouseTarget);
			uiSystems->GetParty().SetPressedObj(mouseTarget);
		}		
	} else if (objects.IsEquipment(mouseTarget) 
		|| objects.IsContainer(mouseTarget)
		|| objects.IsPortal(mouseTarget)
		|| objSystem->GetProtoId(mouseTarget) == 2064
		) {
		// 2064 is the guestbook
		uiSystems->GetInGame().SetFocusObject(mouseTarget);
	} else if (objects.IsCritter(mouseTarget)) {
		uiSystems->GetInGame().SetFocusObject(mouseTarget);
		if (party.IsInParty(mouseTarget)) {
			uiSystems->GetParty().SetHoveredObj(mouseTarget);
		}
	} else if (objects.GetType(mouseTarget) == obj_t_scenery) {
		if (objects.getInt32(mouseTarget, obj_f_scenery_teleport_to)) {
			uiSystems->GetInGame().SetFocusObject(mouseTarget);
		}
	}

}

objHndl UiInGame::GetMouseTarget(int x, int y)
{
	// Pick the object using the screen coordinates first
	auto mousedOver = PickObject(x, y);
	if (!mousedOver) {		
		return objHndl::null;
	}

	if (objects.IsUntargetable(mousedOver)) {
		return objHndl::null;
	}

	if (objects.IsCritter(mousedOver) && critterSys.IsConcealed(mousedOver)) {
		return objHndl::null;
	}

	return mousedOver;
}

void UiInGame::DoKeyboardScrolling(){
	
	static auto scrollRefTime = timeGetTime();
	auto now = timeGetTime();
	if (now < scrollRefTime + 16){
		auto scrollButter = temple::GetRef<int>(0x102AC238);
		if (!scrollButter)
			return;
	}
	scrollRefTime = now;
	auto &console =tig->GetConsole();
	if (console.IsOpen() && console.InputIsActive()){
		return;
	}
	// don't scroll if game is out of focus (e.g. debugging...)
	if (tig->GetMainWindow().GetHwnd() != GetFocus()) {
		return;
	}
	static auto doKeyboardScrolling = temple::GetPointer<void()>(0x10113fb0);
	doKeyboardScrolling();

}

objHndl UiInGame::PickObject(int x, int y, GameRaycastFlags flags)
{
	objHndl result = objHndl::null;
	if (PickObjectOnScreen(x, y, &result, flags) == 0) {
		return objHndl::null;
	} else {
		return result;
	}
}

void UiInGame::ResetInput()
{
	static auto ui_intgame_reset_input = temple::GetPointer<void()>(0x10112f10);
	ui_intgame_reset_input();
}
