#include "stdafx.h"
#include "ui_popup.h"
#include "ui.h"
#include "tig/tig_keyboard.h"

UiPopupHandler uiPopupHandler;

class UiPopupReplacement : public TempleFix
{
	void apply() override {
		
		//UiPopupMsg
		replaceFunction<BOOL(int, TigMsg*)>(0x10171B50, [](int widId, TigMsg* msg){
			return uiPopupHandler.UiPopupMsg(widId, msg);
		});

		// Wnd Msg
		replaceFunction<BOOL(int, TigMsg*)>(0x10171050, [](int widId, TigMsg* msg){
			return uiPopupHandler.UiPopupWndMsg(widId, msg);
		});

	}
} uiPopupFixes;

BOOL UiPopupHandler::UiPopupMsg(int widId, TigMsg * msg){

	uiManager->GetButton(widId);

	if (msg->type == TigMsgType::MOUSE)
		return TRUE;

	if (msg->type == TigMsgType::WIDGET){
		auto _msg = (TigMsgWidget*)msg;

		if (_msg->widgetEventType == TigMsgWidgetEvent::MouseReleased) {

			auto btnIdx = FindPopupBtnIdx(widId);
			ExecuteCallback(GetCurrentPopupIdx(), btnIdx);

			return TRUE;
		}

	}
		
	
	return FALSE;
}

BOOL UiPopupHandler::UiPopupWndMsg(int widId, TigMsg * msg)
{
	if (msg->type == TigMsgType::MOUSE)
		return TRUE;

	if (msg->type == TigMsgType::CHAR) {
		auto key = msg->arg1;
		auto curPopup = GetCurrentPopupIdx();
		if (curPopup < 0)
			return FALSE;
		
		if (key == VK_RETURN) {
			ExecuteCallback(curPopup, 0);
			return TRUE;
		}	 
		else if (key == VK_ESCAPE)	{
			ExecuteCallback(curPopup, 1);
			return TRUE;
		}
	}
	return FALSE;
}

UiPromptListEntry& UiPopupHandler::GetPopupByType(int popupType){
	Expects(popupType < 5 && popupType >= 0);
	return temple::GetRef<UiPromptListEntry[]>(0x10C03BD8)[popupType];
}

int UiPopupHandler::VanillaPopupShow(const char * bodyText, const char * title, int buttonTextType, int(*callback)(int), int(*callback2)(int)){
	return temple::GetRef<int(__cdecl)(const char * , const char * , int(*)(int), int(*)(int))>(0x1017CF20)(bodyText, title, callback, callback2);
}

int UiPopupHandler::PopupsAllInactive()
{
	return temple::GetRef<int(__cdecl)()>(0x10171A70)();
}

int UiPopupHandler::FindPopupBtnIdx(int widId){

	auto result = 0;
	auto &popup = GetCurPopup();
	for (auto i = 0; i < 3; i++){
		if (popup.btns[i]->widgetId == widId){
			return i;
		}
	}

	return 3;
}

void UiPopupHandler::ExecuteCallback(int popupIdx, int btnIdx){
	// temple::GetRef<void(__cdecl)(int, int)>(0x101719D0)(popupIdx, btnIdx);

	auto &popup = GetPopupByType(popupIdx);
	if (!(popup.flags & 1)){
		if (popup.prompt.callback){
			popup.prompt.callback(btnIdx);
		}
	}

	auto wnd = popup.wnd;
	popup.isActive = 0;
	SetCurrentPopupIdx(-1);
	uiManager->SetHidden(wnd->widgetId, true);
	if (popup.prompt.renderFuncMaybe){
		popup.prompt.renderFuncMaybe();
	}

	popup.ResetWnd();
	popup.prompt.Reset();

	if ((popup.flags & 1)) {
		if (popup.prompt.callback) {
			popup.prompt.callback(btnIdx);
		}
	}
}

UiPromptListEntry & UiPopupHandler::GetCurPopup(){
	return temple::GetRef<UiPromptListEntry[]>(0x10C03BD8)[GetCurrentPopupIdx()];
}

int UiPopupHandler::GetCurrentPopupIdx(){
	return temple::GetRef<int>(0x102FC218);
}

void UiPopupHandler::SetCurrentPopupIdx(int popupIdx){
	temple::GetRef<int>(0x102FC218) = popupIdx;
}

void UiPromptPacket::Reset(){
	memset(this, 0, sizeof(UiPromptPacket));
	this->idx = -1;
}

void UiPromptListEntry::ResetWnd(){
	this->flags = 0;
	this->isActive = 0;
	this->wnd->x = 0;
	this->wnd->y = 0;
	this->wnd->xrelated = 0;
	this->wnd->yrelated = 0;
	this->wnd->width = 0;
	this->wnd->height = 0;

	for (auto i  = 0; i < 3; i++ ){
		auto btn = this->btns[i];
		btn->x = btn->y = btn->xrelated = btn->yrelated = 0;
		btn->width = btn->height = 0;
	}
	this->prompt.texture0 = 0;
	this->prompt.texture1 = 0;
	this->prompt.texture2 = 0;
}
