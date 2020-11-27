#include "stdafx.h"
#include "ui_popup.h"
#include "ui.h"
#include "tig/tig_keyboard.h"
#include "ui/ui_legacysystems.h"

struct UiPromptListEntry {
	int flags; // 1 - execute callback after reseting the prompt (otherwise does so before)
	int isActive;
	LgcyWindow* wnd;
	LgcyButton* btns[3];
	UiPromptPacket prompt;
	void ResetWnd();
};

//*****************************************************************************
//* Popup-UI
//*****************************************************************************
UiPopup* uiPopup = nullptr;

UiPopup::UiPopup(const UiSystemConf& config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10171df0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Popup-UI");
	}
	uiPopup = this;
}
UiPopup::~UiPopup() {
	auto shutdown = temple::GetPointer<void()>(0x10171510);
	shutdown();
}
void UiPopup::Reset() {
	auto reset = temple::GetPointer<void()>(0x10171e70);
	reset();
}
const std::string& UiPopup::GetName() const {
	static std::string name("Popup-UI");
	return name;
}


class UiPopupReplacement : public TempleFix
{
	void apply() override {
		
		//UiPopupMsg
		replaceFunction<BOOL(int, TigMsg*)>(0x10171B50, [](int widId, TigMsg* msg){
			return uiPopup->UiPopupMsg(widId, msg);
		});

		// Wnd Msg
		replaceFunction<BOOL(int, TigMsg*)>(0x10171050, [](int widId, TigMsg* msg){
			return uiPopup->UiPopupWndMsg(widId, msg);
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

UiPromptListEntry& UiPopup::GetPopupByType(int popupType){
	Expects(popupType < 5 && popupType >= 0);
	return temple::GetRef<UiPromptListEntry[]>(0x10C03BD8)[popupType];
}

int UiPopup::VanillaPopupShow(const char * bodyText, const char * title, int buttonTextType, int(__cdecl*callback)(int), int flag){
	return temple::GetRef<int(__cdecl)(const char * , const char * , int(*)(int), int)>(0x1017CF20)(bodyText, title, callback, flag);
}

int UiPopup::FindPopupBtnIdx(int widId){

	auto result = 0;
	auto &popup = GetCurPopup();
	for (auto i = 0; i < 3; i++){
		if (popup.btns[i]->widgetId == widId){
			return i;
		}
	}

	return 3;
}

void UiPopup::ExecuteCallback(int popupIdx, int btnIdx){
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
	if (popup.prompt.onPopupHide){
		popup.prompt.onPopupHide();
	}

	popup.ResetWnd();
	popup.prompt.Reset();

	if ((popup.flags & 1)) {
		if (popup.prompt.callback) {
			popup.prompt.callback(btnIdx);
		}
	}
}

UiPromptListEntry & UiPopup::GetCurPopup(){
	return temple::GetRef<UiPromptListEntry[]>(0x10C03BD8)[GetCurrentPopupIdx()];
}

int UiPopup::GetCurrentPopupIdx(){
	return temple::GetRef<int>(0x102FC218);
}

void UiPopup::SetCurrentPopupIdx(int popupIdx){
	temple::GetRef<int>(0x102FC218) = popupIdx;
}

UiPromptPacket::UiPromptPacket()
{
	Reset();
}

/* Originally 0x10171580 */
int UiPromptPacket::Show(int promptIdx, int flags)
{
	return temple::GetRef<int(__cdecl)(UiPromptPacket*, int, int)>(0x10171580)(this, promptIdx, flags);
}

/* 0x101709E0 */
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
	this->prompt.btnNormalTexture  = 0;
	this->prompt.btn2NormalTexture = 0;
	this->prompt.btn3NormalTexture = 0;
}
