#include "stdafx.h"
#include "ui_popup.h"
#include "ui.h"
#include "tig/tig_keyboard.h"

UiPopup uiPopup;

class UiPopupReplacement : public TempleFix
{
	void apply() override {
		
		//UiPopupMsg
		replaceFunction<BOOL(int, TigMsg*)>(0x10171B50, [](int widId, TigMsg* msg){
			return uiPopup.UiPopupMsg(widId, msg);
		});

		// Wnd Msg
		replaceFunction<BOOL(int, TigMsg*)>(0x10171050, [](int widId, TigMsg* msg){
			return uiPopup.UiPopupWndMsg(widId, msg);
		});

	}
} uiPopupFixes;


BOOL UiPopup::UiPopupMsg(int widId, TigMsg * msg){

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

BOOL UiPopup::UiPopupWndMsg(int widId, TigMsg * msg)
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

int UiPopup::VanillaPopupShow(const char * bodyText, const char * title, int buttonTextType, int(*callback)(int), int(*callback2)(int)){
	return temple::GetRef<int(__cdecl)(const char * , const char * , int(*)(int), int(*)(int))>(0x1017CF20)(bodyText, title, callback, callback2);
}

int UiPopup::FindPopupBtnIdx(int widId){

	auto result = 0;

	auto btn = &( GetCurPopup().btn1);
	
	for ( ; result < 3; result++){
		if ((*btn)->widgetId == widId)
			return result;
		btn++;
	}
	return result;
}

void UiPopup::ExecuteCallback(int popupIdx, int btnIdx){
	temple::GetRef<void(__cdecl)(int, int)>(0x101719D0)(popupIdx, btnIdx);
}

UiPromptListEntry & UiPopup::GetCurPopup(){
	return temple::GetRef<UiPromptListEntry[]>(0x10C03BD8)[GetCurrentPopupIdx()];
}

int UiPopup::GetCurrentPopupIdx(){
	return temple::GetRef<int>(0x102FC218);
}
