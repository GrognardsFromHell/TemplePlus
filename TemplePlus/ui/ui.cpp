#include "stdafx.h"
#include "ui.h"
#include "tig/tig.h"
#include <util/fixes.h>
#include <tig/tig_msg.h>
#include <sound.h>
#include <tig/tig_font.h>
#include <tig/tig_mouse.h>
#include "messages/messagequeue.h"

Ui ui;

/*
	Native ToEE functions we use.
*/
typedef void (__cdecl *SaveCallback)();

static struct UiFuncs : temple::AddressTable {

	ImgFile*(__cdecl *LoadImg)(const char* filename);
	int (__cdecl *GetAsset)(UiAssetType assetType, uint32_t assetIndex, int& textureIdOut, int offset);
	void (__cdecl *UpdateCombatUi)();
	void (__cdecl *UpdatePartyUi)();
	void (__cdecl *ShowWorldMap)(int unk);
	void (__cdecl *WorldMapTravelByDialog)(int destination);
	void (__cdecl *ShowPartyPool)(bool ingame);
	void (__cdecl *ShowCharUi)(int page);
	bool (__cdecl *ShowWrittenUi)(objHndl handle);
	
	BOOL(__cdecl*BindButton)(int parentId, int buttonId);
	BOOL(__cdecl*AddWindow)(LgcyWidget* widget, unsigned size, int* widgetId, const char* codeFileName, int lineNumber);
	BOOL(__cdecl*AddButton)(LgcyButton* button, unsigned size, int* widgId, const char* codeFileName, int lineNumber);
	BOOL(__cdecl*ButtonSetButtonState)(int widgetId, LgcyButtonState newState);
	BOOL(__cdecl*WidgetAndWindowRemove)(int widId);
	BOOL(__cdecl*WidgetRemove)(int widId);
	BOOL(__cdecl*WidgetSetHidden)(int widId, int hiddenState);
	BOOL(__cdecl*GetButtonState)(int widId, int* state);
	void(__cdecl*WidgetBringToFront)(int widId);
	void(*UiMouseMsgHandlerRenderTooltipCallback)(int x, int y, void* data); //

	int* uiWidgetMouseHandlerWidgetId;
	int* uiMouseButtonId;

	UiFuncs() {
		rebase(LoadImg, 0x101E8320);
		rebase(GetAsset, 0x1004A360);
		rebase(UpdateCombatUi, 0x1009A730);
		rebase(UpdatePartyUi, 0x1009A740);
		rebase(ShowWorldMap, 0x1015F140);
		rebase(WorldMapTravelByDialog, 0x10160450);
		rebase(ShowPartyPool, 0x10165E60);
		rebase(ShowCharUi, 0x10148E20);
		rebase(ShowWrittenUi, 0x10160F50);

		rebase(uiWidgetMouseHandlerWidgetId, 0x10301324);
		rebase(uiMouseButtonId, 0x10301328);

		rebase(WidgetBringToFront, 0x101F8E40);
		rebase(BindButton, 0x101F8950);
		rebase(AddWindow,  0x101F8FD0);
		rebase(AddButton,  0x101F9460);
		rebase(WidgetAndWindowRemove, 0x101F9010);
		rebase(WidgetRemove,          0x101F9420);
		rebase(WidgetSetHidden, 0x101F9100);
		rebase(ButtonSetButtonState, 0x101F9510);
		rebase(GetButtonState,  0x101F9740);
		rebase(UiMouseMsgHandlerRenderTooltipCallback, 0x101F9870);
		
		
	}

	
} uiFuncs;


class UiReplacement : TempleFix
{
public:

	void apply() override
	{
		replaceFunction<BOOL(TigMouseMsg*)>(0x101F9970, [](TigMouseMsg *msg) {
			return ui.TranslateMouseMessage(msg);
		});

		replaceFunction<BOOL(TigMsg*)>(0x101F8A80, [](TigMsg *msg) {
			return ui.ProcessMessage(msg);
		});

		 /*
		 Hook for missing portraits (annoying logspam!)
		 */
		 static int(__cdecl*orgGetAsset)(int, int, void*, int) = replaceFunction<int(__cdecl)(int, int, void*, int)>(0x1004A360, [](int assetType, int id, void* out, int subId)
		 {
			 // portraits
			 if (assetType == 0) {
				 MesLine line;
				 line.key = id + subId;
				 auto uiArtMgrMesFiles = temple::GetRef<MesHandle*>(0x10AA31C8);

				 if (!mesFuncs.GetLine(uiArtMgrMesFiles[assetType], &line)){
					 return orgGetAsset(assetType, 0, out, subId);
				 }
			 }
			 return orgGetAsset(assetType, id, out, subId);
		 });

		 // DrawTextInWidget hook
		 replaceFunction<bool(__cdecl)(int, char*, TigRect&, TigTextStyle&)>(0x101F87C0, [](int widgetId ,char* text, TigRect& rect, TigTextStyle& style)->bool
		 {
			 auto wid = ui.WidgetGet(widgetId);
			 if (!wid)
				 return 1;
			 if (*text == 0)
				 return 1;

			

			 TigRect extents(rect.x + wid->x, rect.y + wid->y, rect.width, rect.height);
			 if (rect.x < 0 || rect.x > 10000 || rect.width > 10000 || rect.width < 0)
			 {
				 extents.x = wid->x;
				 extents.width = wid->width;

				 //auto tigRects = temple::GetPointer<TigRect>(0x10C7C638);
				 //TigRect rects[20];
				 //memcpy(rects, tigRects, sizeof(rects));
				 //
				 //auto uiCharEditorSkillsWnd = temple::GetPointer<LgcyWindow>(0x10C7B628);
				 //int dummy = 1;


			 }
			 
			 

			 return tigFont.Draw(text, extents, style) != 0;

		 });


	}
} uiReplacement;

bool LgcyWindow::Add(int* widIdOut){
	return ui.AddWindow(this, sizeof(LgcyWindow), widIdOut, "ui.cpp", 325) != 0;
}

LgcyButton::LgcyButton(char* ButtonName, int ParentId, int X, int Y, int Width, int Height){
	if (ButtonName){
		auto pos = name;
		while( *ButtonName && (pos - name < 63) ){
			*pos = *ButtonName;
			pos++; ButtonName++;
		}
		*pos = 0;
	}

	x = X;
	y = Y;
	width = Width;
	height = Height;
	parentId = ParentId;
	yrelated = Y;
	xrelated = X;
	flags = 0;
	renderTooltip = nullptr;
	render = nullptr;
	handleMessage = nullptr;
	buttonState = LgcyButtonState::Normal;
	field98 = 0;
	type = LgcyWidgetType::Button;
	widgetId = -1;
	field8C = -1;
	field90 = -1;
	field84 = -1;
	field88 = -1;
	sndDown = -1;
	sndClick = -1;
	hoverOn = -1;
	hoverOff = -1;
}

LgcyButton::LgcyButton(char* ButtonName, int ParentId, TigRect& rect){
	if (ButtonName) {
		auto pos = name;
		while (*ButtonName && (pos - name < 63)) {
			*pos = *ButtonName;
			pos++; ButtonName++;
		}
		*pos = 0;
	}

	x = rect.x;
	y = rect.y;
	width = rect.width;
	height = rect.height;
	parentId = ParentId;
	yrelated = rect.y;
	xrelated = rect.x;
	flags = 0;
	renderTooltip = nullptr;
	render = nullptr;
	handleMessage = nullptr;
	buttonState = LgcyButtonState::Normal;
	field98 = 0;
	type = LgcyWidgetType::Button;
	widgetId = -1;
	field8C = -1;
	field90 = -1;
	field84 = -1;
	field88 = -1;
	sndDown = -1;
	sndClick = -1;
	hoverOn = -1;
	hoverOff = -1;
	return;
}

bool LgcyButton::Add(int* widIdOut){
	return ui.AddButton(this, sizeof(LgcyButton), widIdOut, "ui.cpp", 367) != 0;
}

int LgcyScrollBar::GetY()
{
	auto getY = temple::GetRef<int(__cdecl)(int id, int& yOut)>(0x101FA150);
	int y = 0;
	if (getY(this->widgetId, y)){
		logger->warn("scrollbar error!");
		y = -1;
	}
	return y;
}

bool LgcyScrollBar::Init(int X, int Y, int Height){

	x = X;
	y = Y;
	type = LgcyWidgetType::Scrollbar;
	parentId = -1;
	widgetId = -1;
	flags = 0;
	size = 176;
	width = 13;
	height = Height;
	field90 = 0;
	render = temple::GetRef<void(__cdecl)(int)>(0x101FA1B0);
	handleMessage = temple::GetRef<BOOL(__cdecl)(int, TigMsg*)>(0x101FA410);
	renderTooltip = nullptr;
	yMin = 0;
	yMax = 100;
	scrollQuantum = 1;
	field8C = 5;
	scrollbarY = 0;
	field98= 0;
	field9C = 0;
	fieldAC = 0;
	fieldA8 = 0;
	fieldA4 = 0;
	fieldA0 = 0;
	field94 = 0;
	return false;
}

bool LgcyScrollBar::Init(int x, int y, int height, int parentId)
{
	Init(x, y, height);
	this->parentId = parentId;
	auto p = ui.WidgetGetType1(parentId);
	this->x += p->x;
	this->y += p->y;
	return false;
}

bool LgcyScrollBar::Add(int* widIdOut){
	return temple::GetRef<bool(__cdecl)(LgcyWidget*, size_t, int*, const char*, int)>(0x101F93D0)(this, sizeof(LgcyScrollBar), widIdOut, "ui.cpp", 366);
}

bool Ui::GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int& textureIdOut) {
	return uiFuncs.GetAsset(assetType, static_cast<uint32_t>(assetIndex), textureIdOut, 0) == 0;
}

ImgFile* Ui::LoadImg(const char* filename) {
	return uiFuncs.LoadImg(filename);
}

void Ui::UpdateCombatUi() {
	uiFuncs.UpdateCombatUi();
}

void Ui::UpdatePartyUi() {
	uiFuncs.UpdatePartyUi();
}

void Ui::ShowWorldMap(int unk) {
	uiFuncs.ShowWorldMap(unk);
}

void Ui::WorldMapTravelByDialog(int destination) {
	uiFuncs.WorldMapTravelByDialog(destination);
}

void Ui::ShowPartyPool(bool ingame) {
	uiFuncs.ShowPartyPool(ingame);
}

void Ui::ShowCharUi(int page) {
	uiFuncs.ShowCharUi(page);
}

bool Ui::ShowWrittenUi(objHndl handle) {
	return uiFuncs.ShowWrittenUi(handle);
}

bool Ui::CharEditorIsActive()
{
	auto charEditorWndId = temple::GetRef<int>(0x10BE8E50);
	bool charEditorHidden = ui.IsWidgetHidden(charEditorWndId);
	if (!charEditorHidden)
	{
		return true;
	}
		
	return false;
}

bool Ui::CharLootingIsActive()
{
	auto result = temple::GetRef<int>(0x10BE6EE8);
	return result != 0;
}

bool Ui::IsWidgetHidden(int widId)
{
	auto widget = mLegacy.GetWidget(widId);
	return !widget || widget->IsHidden();
}

BOOL Ui::AddWindow(LgcyWidget* widget, unsigned size, int* widgetId, const char* codeFileName, int lineNumber)
{
	return uiFuncs.AddWindow(widget, size, widgetId, codeFileName, lineNumber);
}

BOOL Ui::ButtonInit(LgcyButton* widg, char* buttonName, int parentId, int x, int y, int width, int height)
{
	if (buttonName)
	{
		char * c = buttonName;
		memcpy(widg->name, buttonName, min(sizeof(widg->name), strlen(buttonName)));
	}
	widg->x = x;
	widg->xrelated = x;
	widg->y = y;
	widg->yrelated = y;
	widg->parentId = parentId;
	widg->width = width;
	widg->height = height;
	widg->flags = 0;
	widg->buttonState = LgcyButtonState::Normal;
	widg->renderTooltip = 0;
	widg->field98 = 0;
	widg->type = LgcyWidgetType::Button;
	widg->widgetId = -1;
	widg->field8C = -1;
	widg->field90 = -1;
	widg->field84 = -1;
	widg->field88 = -1;
	widg->sndDown = -1;
	widg->sndClick = -1;
	widg->hoverOn = -1;
	widg->hoverOff = -1;
	return 0;
}

BOOL Ui::AddButton(LgcyButton* button, unsigned size, int* widgId, const char* codeFileName, int lineNumber)
{
	return uiFuncs.AddButton(button, size, widgId, codeFileName, lineNumber);
}

BOOL Ui::BindToParent(int parentId, int buttonId){
	return uiFuncs.BindButton(parentId, buttonId);
}

BOOL Ui::SetDefaultSounds(int widId){
	LgcyButton widg;
	if (WidgetCopy(widId, &widg))
		return true;
	widg.sndDown = 3012;
	widg.sndClick = 3013;
	widg.hoverOn = 3010;
	widg.hoverOff = 3011;
	return WidgetSet(widId, &widg) != 0;
}

BOOL Ui::ButtonSetButtonState(int widgetId, LgcyButtonState newState)
{
	return uiFuncs.ButtonSetButtonState(widgetId, newState);
}

void Ui::WidgetRemoveRegardParent(int widIdx)
{
	mLegacy.RemoveChildWidget(widIdx);
}

BOOL Ui::WidgetAndWindowRemove(int widId)
{
	return uiFuncs.WidgetAndWindowRemove(widId);
}

BOOL Ui::WidgetRemove(int widId){
	return uiFuncs.WidgetRemove(widId);
}

BOOL Ui::WidgetSetHidden(int widId, int hiddenState)
{
	return uiFuncs.WidgetSetHidden(widId, hiddenState);
}


LgcyWindow* Ui::WidgetGetType1(int widId){
	LgcyWidget* result = WidgetGet(widId);
	if (!result)
		return nullptr;
	if (result->type == LgcyWidgetType::Window)	{
		return static_cast<LgcyWindow*>(result);
	}
	return nullptr;
}

LgcyButton* Ui::GetButton(int widId){
	auto result = WidgetGet(widId);
	if (!result || result->type != LgcyWidgetType::Button) {
		return nullptr;
	}
	return static_cast<LgcyButton*>(result);
}

LgcyScrollBar * Ui::ScrollbarGet(int widId){
	auto result = WidgetGet(widId);
	if (!result || result->type != LgcyWidgetType::Scrollbar)
		return nullptr;
	return static_cast<LgcyScrollBar*>(result);
}

LgcyWidget* Ui::WidgetGet(int widId)
{
	return mLegacy.GetWidget(widId);
}

int Ui::GetWindowContainingPoint(int x, int y)
{
	auto window = mLegacy.GetWindowAt(x, y);
	if (window) {
		return window->widgetId;
	} else {
		return -1;
	}
}

BOOL Ui::GetButtonState(int widId, int* state){
	return uiFuncs.GetButtonState(widId, state);
}

bool Ui::GetButtonState(int widId, LgcyButtonState& state){
	auto widg = ui.GetButton(widId);
	if (!widg)
		return true;
	state = widg->buttonState;
	return false;
}

void Ui::WidgetBringToFront(int widId)
{
	return uiFuncs.WidgetBringToFront(widId);
}

int Ui::WidgetlistIndexof(int widgetId, int* widgetlist, int size)
{
	for (int i = 0; i < size; i++) {
		if (widgetlist[i] == widgetId)
			return i;
	}

	return -1;
}

BOOL Ui::WidgetContainsPoint(int widgetId, int x, int y)
{
	auto widg = WidgetGet(widgetId);
	if (!widg)
		return false;
	if (x < widg->x || x > (widg->x + (int)widg->width) )
		return false;
	if (y < widg->y || y >(widg->y + (int)widg->height))
		return false;
	return true;
}

int Ui::GetAtInclChildren(int x, int y)
{
	int windowId = GetWindowContainingPoint(x, y);
	if (windowId == -1)
		return windowId;
	auto wndWidget = WidgetGetType1(windowId);
	if (wndWidget == nullptr)
		return windowId;
	if (wndWidget->IsHidden())
		return windowId;

	for (int i = wndWidget->childrenCount - 1; i >= 0; i-- )
	{
		auto childId = wndWidget->children[i];
		if (childId != -1)
		{
			auto childWid = WidgetGet(childId);
			if (childWid != nullptr
				&& !childWid->IsHidden()
				&& WidgetContainsPoint(childId, x,y))
			{
				return childId;
			}
		}
		
	}
	return windowId;
}

int Ui::TranslateMouseMessage(TigMouseMsg* mouseMsg)
{
	int flags = mouseMsg->flags;
	int x = mouseMsg->x, y = mouseMsg->y;
	TigMsgWidget newTigMsg;
	newTigMsg.createdMs = temple::GetRef<int>(0x11E74578); // system time as recorded by the Msg Process function
	newTigMsg.type = TigMsgType::WIDGET;
	newTigMsg.x = x;
	newTigMsg.y = y;

	int widIdAtCursor = GetAtInclChildren(x, y);

	int globalWidId = *uiFuncs.uiWidgetMouseHandlerWidgetId;

	// moused widget changed
	if ((flags & 0x1000) && widIdAtCursor != globalWidId)
	{
		if (widIdAtCursor != -1 && mouseFuncs.GetCursorDrawCallbackId() == (uint32_t) uiFuncs.UiMouseMsgHandlerRenderTooltipCallback)
		{
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
		}

		if (globalWidId != -1)
		{
			bool enqueue4 = false;
			auto globalWid = WidgetGet(globalWidId);
			// if window
			if (globalWid->IsWindow())
			{
				auto prevHoveredWindow = (LgcyWindow*)globalWid;
				if (prevHoveredWindow->mouseState == LgcyWindowMouseState::Pressed) {
					prevHoveredWindow->mouseState = LgcyWindowMouseState::PressedOutside;
				} else if (prevHoveredWindow->mouseState != LgcyWindowMouseState::PressedOutside) {
					prevHoveredWindow->mouseState = LgcyWindowMouseState::Outside;
				}
				enqueue4 = true;
			} 
			// button
			else if (globalWid->IsButton() && !globalWid->IsHidden())
			{
				auto buttonWid = GetButton(globalWidId);
				switch (buttonWid->buttonState) {
				case 1:
					// Unhover
					buttonWid->buttonState = LgcyButtonState::Normal;
					sound.MssPlaySound(buttonWid->hoverOff);
					break;
				case 2:
					// Down -> Released without click event
					buttonWid->buttonState = LgcyButtonState::Released;
					break;
				}
				if (!WidgetGet(globalWid->parentId)->IsHidden())
				{
					enqueue4 = true;
				}
			}
			// scrollbar
			else if (globalWid->IsScrollBar())
			{
				if (globalWid->IsHidden() || WidgetGet(globalWid->parentId)->IsHidden())
					enqueue4 = false;
				else
					enqueue4 = true;
			}

			if (enqueue4)
			{
				newTigMsg.widgetId = globalWidId;
				newTigMsg.widgetEventType = TigMsgWidgetEvent::Exited;
				messageQueue->Enqueue(newTigMsg);
			}
		}

		if (widIdAtCursor != -1)
		{
			auto widAtCursor = WidgetGet(widIdAtCursor);
			if (widAtCursor->type == LgcyWidgetType::Window)
			{
				auto widAtCursorWindow = WidgetGetType1(widIdAtCursor);
				if (widAtCursorWindow->mouseState == LgcyWindowMouseState::PressedOutside) {
					widAtCursorWindow->mouseState = LgcyWindowMouseState::Pressed;
				} else if (widAtCursorWindow->mouseState != LgcyWindowMouseState::Pressed) {
					widAtCursorWindow->mouseState = LgcyWindowMouseState::Hovered;
				}
			} else if (widAtCursor->type == LgcyWidgetType::Button) {
				auto buttonWid = GetButton(widIdAtCursor);
				if (buttonWid->buttonState)
				{
					if (buttonWid->buttonState == LgcyButtonState::Released)
					{
						buttonWid->buttonState = LgcyButtonState::Down;
					}
				} 
				else
				{
					buttonWid->buttonState = LgcyButtonState::Hovered;
					sound.MssPlaySound(buttonWid->hoverOn);
				}
			}
			newTigMsg.widgetId = widIdAtCursor;
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Entered;
			messageQueue->Enqueue(newTigMsg);
		}
		globalWidId = *uiFuncs.uiWidgetMouseHandlerWidgetId = widIdAtCursor;
	}
	
	if (mouseMsg->flags & MouseStateFlags::MSF_POS_CHANGE2 && globalWidId != -1 && !mouseFuncs.GetCursorDrawCallbackId())
	{
		mouseFuncs.SetCursorDrawCallback([](int x, int y) {
			(*uiFuncs.UiMouseMsgHandlerRenderTooltipCallback)(x, y, uiFuncs.uiWidgetMouseHandlerWidgetId);
		}, (uint32_t)*uiFuncs.UiMouseMsgHandlerRenderTooltipCallback);
	}

	if (mouseMsg->flags & MouseStateFlags::MSF_LMB_CLICK)
	{
		auto widIdAtCursor2 = GetAtInclChildren(mouseMsg->x, mouseMsg->y); // probably redundant to do again, but just to be safe...
		if (widIdAtCursor2 != -1)
		{
			auto button = GetButton(widIdAtCursor2);
			if (button)
			{
				int buttonState = button->buttonState;
				switch (button->buttonState) {
				case LgcyButtonState::Hovered:
					button->buttonState = LgcyButtonState::Down;
					sound.MssPlaySound(button->sndDown);
					break;
				case LgcyButtonState::Disabled:
					return 0;
				}
			}
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Clicked;
			newTigMsg.widgetId = widIdAtCursor2;
			*uiFuncs.uiMouseButtonId = widIdAtCursor2;
			messageQueue->Enqueue(newTigMsg);
		}
	}

	if ( (mouseMsg->flags & MouseStateFlags::MSF_LMB_RELEASED) && *uiFuncs.uiMouseButtonId != -1)
	{
		auto button = GetButton(*uiFuncs.uiMouseButtonId);
		if (button)
		{
			switch (button->buttonState)
			{
			case LgcyButtonState::Down:
				button->buttonState = LgcyButtonState::Hovered;
				sound.MssPlaySound(button->sndClick);
				break;
			case LgcyButtonState::Released:
				button->buttonState = LgcyButtonState::Normal;
				sound.MssPlaySound(button->sndClick);
				break;
			case LgcyButtonState::Disabled:
				return 0;
			}
		}
		auto widIdAtCursor2 = GetAtInclChildren(mouseMsg->x, mouseMsg->y); // probably redundant to do again, but just to be safe...
		newTigMsg.widgetId = *uiFuncs.uiMouseButtonId;
		newTigMsg.widgetEventType = (widIdAtCursor2 != *uiFuncs.uiMouseButtonId)?TigMsgWidgetEvent::MouseReleasedAtDifferentButton : TigMsgWidgetEvent::MouseReleased;
		messageQueue->Enqueue(newTigMsg);
		*uiFuncs.uiMouseButtonId = -1;
	}

	return 0;
}

int Ui::WidgetSet(int widId, const LgcyWidget* widg)
{
	auto dst = WidgetGet(widId);
	memcpy(dst, widg, dst->size);
	return 0;
}

int Ui::WidgetCopy(int widId, LgcyWidget* widg)
{
	auto src = WidgetGet(widId);
	memcpy(widg, src, src->size);
	return 0;
}

int Ui::ProcessMessage(TigMsg * msg)
{
	TigMsg *v1; // ebx@1
	TigMsgType eventType; // eax@1
	LgcyWidgetHandleMsgFn v5;
	int v7; // edi@10
	LgcyWidget *v8; // eax@11
	LgcyWidget *v9; // esi@11
	LgcyWidgetHandleMsgFn v10;
	LgcyWidget *v12; // eax@20
	LgcyWidgetHandleMsgFn v13;
	int v14; // esi@24
	LgcyWindow *v15; // eax@25
	LgcyWindow *v16; // ebp@25
	signed __int32 v17; // edi@29
	int *v18; // esi@30
	LgcyWidget *v19; // eax@32
	LgcyWidgetHandleMsgFn v20;
	LgcyWidgetHandleMsgFn v21;
	int a1a; // [sp+14h] [bp+4h]@24

	v1 = msg;
	eventType = msg->type;
	if (eventType != TigMsgType::MOUSE)
	{
		// TMT_UNK1
		if (eventType == TigMsgType::WIDGET)
		{
			v7 = msg->arg1;
			if (v7 != -1)
			{
				while (1)
				{
					v8 = WidgetGet(v7);
					v9 = v8;
					if (!v8)
						break;
					if ((v8->parentId == -1 || !IsWidgetHidden(v8->parentId)) && !v9->IsHidden())
					{
						v10 = v9->handleMessage;
						if (v10)
						{
							if (v10(v7, msg))
								return 1;
						}
					}
					v7 = v9->parentId;
					if (v7 == -1)
						return 0;
				}
			}
		}
		else
		{
			for (auto &windowId : mLegacy.GetActiveWindows()) {
				auto window = mLegacy.GetWidget(windowId);

				if (!window->IsHidden()) {
					v5 = window->handleMessage;
					if (v5)
					{
						if (v5(windowId, msg))
							return 1;
					}
				}
			}
			return 0;
		}
		return 0;
	}

	static int& widgetMouseCaptureId = temple::GetRef<int>(0x11E74384);

	if (widgetMouseCaptureId != -1)
	{
		v12 = WidgetGet(widgetMouseCaptureId);
		if (v12)
		{
			v13 = v12->handleMessage;
			if (v13)
			{
				v13(widgetMouseCaptureId, msg);
				return 1;
			}
		}
		return 0;
	}
	v14 = 0;
	a1a = 0;

	auto &windowIds = mLegacy.GetActiveWindows();

	if (windowIds.size() <= 0)
		return 0;
	while (1)
	{
		v15 = mLegacy.GetWindow(windowIds[v14]);
		v16 = v15;
		if (!v15 || !v15->IsWindow() || v15->IsHidden() || !WidgetContainsPoint(windowIds[v14], v1->arg1, v1->arg2))
			goto LABEL_41;
		v17 = v16->childrenCount - 1;
		if (v17 >= 0)
			break;
	LABEL_38:
		v21 = v16->handleMessage;
		if (v21 && !v16->IsHidden() && v21(windowIds[v14], v1))
			return 1;
	LABEL_41:
		a1a = ++v14;
		if (v14 >= windowIds.size())
			return 0;
	}
	v18 = &v16->children[v17];
	while (1)
	{
		if (mLegacy.DoesWidgetContain(*v18, v1->arg1, v1->arg2))
		{
			v19 = mLegacy.GetWidget(*v18);
			if (v19)
			{
				v20 = v19->handleMessage;
				if (v20)
				{
					if (!v19->IsHidden() && v20(*v18, v1))
						return 1;
				}
			}
		}
		--v17;
		--v18;
		if (v17 < 0)
		{
			v14 = a1a;
			goto LABEL_38;
		}
	}
}

bool Ui::ScrollbarGetY(int widId, int * scrollbarY) {
	auto widget = (LgcyScrollBar*) WidgetGet(widId);
	if (!widget || widget->type != LgcyWidgetType::Scrollbar)
		return true;

	if (!scrollbarY)
		return false;
	auto y = 0;
	auto ymax = widget->yMax;
	auto ymin = widget->yMin;

	if (ymax > ymin) {
		auto field90 = widget->field90;
		if (!field90) {
			y = widget->scrollbarY;
		}
		else {
			y = (ymax + widget->field8C - ymin)
				* ( ((widget->height - 44) * widget->scrollbarY) / (ymax + widget->field8C - ymin) - field90)
				/ (widget->height - 44);
		}
	}
	

	if (y > ymax) {
		*scrollbarY = ymax;
		return false;
	}
	
	if (y < ymin) {
		*scrollbarY = ymin;
		return false;
	}

	*scrollbarY = y;
	return false;
}

void Ui::ScrollbarSetYmax(int widId, int yMax)
{
	LgcyScrollBar widg;
	if (!ui.WidgetCopy(widId, &widg))
	{
		widg.yMax = yMax;
		WidgetSet(widId, &widg);
	}
}

BOOL Ui::ScrollbarSetY(int widId, int value){
	LgcyScrollBar scrollbarWid;
	auto result = WidgetCopy(widId, &scrollbarWid);
	if (!result)
	{
		scrollbarWid.scrollbarY = value;
		TigMsg msg;
		msg.createdMs = temple::GetRef<int>(0x11E74578);
		msg.type = TigMsgType::WIDGET;
		msg.arg2 = 5;
		msg.arg1 = scrollbarWid.parentId;
		messageQueue->Enqueue(msg);
		result = ui.WidgetSet(widId, &scrollbarWid);
	}
	return result;
}

const char* Ui::GetTooltipString(int line) const
{
	auto getTooltipString = temple::GetRef<const char*(__cdecl)(int)>(0x10122DA0);
	return getTooltipString(line);
}

const char* Ui::GetStatShortName(Stat stat) const
{
	return temple::GetRef<const char*(__cdecl)(Stat)>(0x10074980)(stat);
}
const char * Ui::GetStatMesLine(int lineNumber) const
{
	auto mesHandle = temple::GetRef<MesHandle>(0x10AAF1F4);
	MesLine line(lineNumber);
	mesFuncs.GetLine_Safe(mesHandle, &line);
	return line.value;
}
