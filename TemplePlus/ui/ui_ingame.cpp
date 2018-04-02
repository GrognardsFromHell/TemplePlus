#include "stdafx.h"

#include <temple/dll.h>

#include "combat.h"
#include "tig/tig_mouse.h"
#include "messages/messagequeue.h"
#include "ui_ingame.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"
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

		static int* objRecovery_10BD3AFC = temple::GetPointer<int>(0x10BD3AFC);
		static uint32_t &idx_10BD3B44 = temple::GetRef<uint32_t>(0x10BD3B44);

		
		if (!objRecovery_10BD3AFC[idx_10BD3B44])
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
			HandleCombatMouseEvent(msg);
		}
	}
}

void UiInGame::HandleNonCombatMessage(const TigMsg & msg)
{
	auto checkCritterWithInventoryOpen = temple::GetRef<BOOL(__cdecl)()>(0x10144030);
	if (checkCritterWithInventoryOpen() && msg.type == TigMsgType::MOUSE)
		return;

	if (msg.type == TigMsgType::KEYSTATECHANGE) {
		return temple::GetRef<void(__cdecl)(const TigMsg&)>(0x101130B0)(msg); // normal keystate change handler
	}

	if (msg.type == TigMsgType::MOUSE) {



	}

	static auto NormalMsgHandler = temple::GetPointer<void(const TigMsg &msg)>(0x10114e30);
	NormalMsgHandler(msg);
}

void UiInGame::HandleCombatKeyStateChange(const TigMsg & msg)
{
	static auto CombatKeystateHandler = temple::GetPointer<void(const TigMsg &msg)>(0x101132b0);
	CombatKeystateHandler(msg);
}

void UiInGame::HandleCombatMouseEvent(const TigMsg & msg)
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

	auto msg_10BD3B5C = temple::GetRef<BOOL>(0x10BD3B5C);	
	if (msg_10BD3B5C == 1)
	{
		auto dword_10BD3AC8 = temple::GetRef<int>(0x10BD3AC8);
		auto stru_10BD3AD8 = temple::GetRef<objHndl>(0x10BD3AD8);
		if (!dword_10BD3AC8 && mouseTarget == stru_10BD3AD8)
		{
			uiSystems->GetInGame().AddToGroupArray(mouseTarget);
			uiSystems->GetParty().SetPressedObj(mouseTarget);
		}		
	} else if (objects.IsEquipment(mouseTarget) || objSystem->GetProtoId(mouseTarget) == 2064) {
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
	auto mousedOver = PickObject(x, y, 6);
	if (!mousedOver) {		
		return objHndl::null;
	}

	if (IsUntargetable(mousedOver)) {
		return objHndl::null;
	}

	if (critterSys.IsConcealed(mousedOver)) {
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
	
	static auto doKeyboardScrolling = temple::GetPointer<void()>(0x10113fb0);
	doKeyboardScrolling();

}

objHndl UiInGame::PickObject(int x, int y, uint32_t flags)
{
	objHndl result = objHndl::null;
	static auto game_raycast = temple::GetPointer<BOOL(int x, int y, objHndl *pObjHndlOut, uint32_t flags)>(0x10022360);
	if (game_raycast(x, y, &result, flags) == 0) {
		return objHndl::null;
	} else {
		return result;
	}
}

bool UiInGame::IsUntargetable(objHndl obj)
{
	if (objects.GetFlags(obj) & (OF_OFF|OF_CLICK_THROUGH|OF_DONTDRAW)) {
		return true;
	}

	if (objects.IsUndetectedSecretDoor(obj)) {
		return true;
	}

	if (aiSys.IsRunningOff(obj)) {
		return true;
	}

	return false;
	
}

void UiInGame::ResetInput()
{
	static auto ui_intgame_reset_input = temple::GetPointer<void()>(0x10112f10);
	ui_intgame_reset_input();
}
