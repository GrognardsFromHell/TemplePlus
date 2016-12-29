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
			return ui.ProcessMessage(*msg) ? TRUE : FALSE;
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
			 auto wid = ui.GetWidget(widgetId);
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
}

void LgcyButton::SetDefaultSounds()
{
	sndDown = 3012;
	sndClick = 3013;
	hoverOn = 3010;
	hoverOff = 3011;
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
	auto p = ui.GetWindow(parentId);
	this->x += p->x;
	this->y += p->y;
	return false;
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

LgcyWidgetId Ui::AddWindow(LgcyWindow& widget)
{
	widget.type = LgcyWidgetType::Window;

	auto widgetId = mLegacy.AddWidget(&widget, __FILE__, __LINE__);
	mLegacy.AddWindow(widgetId);
	return widgetId;
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

LgcyWidgetId Ui::AddButton(LgcyButton& button)
{
	button.type = LgcyWidgetType::Button;
	return mLegacy.AddWidget(&button, __FILE__, __LINE__);
}

LgcyWidgetId Ui::AddButton(LgcyButton &button, LgcyWidgetId parentId)
{
	auto buttonId = AddButton(button);
	mLegacy.AddChild(parentId, buttonId);
	return buttonId;
}

LgcyWidgetId Ui::AddScrollBar(LgcyScrollBar& scrollBar)
{
	scrollBar.type = LgcyWidgetType::Scrollbar;
	return mLegacy.AddWidget(&scrollBar, __FILE__, __LINE__);
}

LgcyWidgetId Ui::AddScrollBar(LgcyScrollBar& scrollBar, LgcyWidgetId parentId)
{
	auto scrollBarId = AddScrollBar(scrollBar);
	mLegacy.AddChild(parentId, scrollBarId);
	return scrollBarId;
}

void Ui::SetButtonState(LgcyWidgetId widgetId, LgcyButtonState newState)
{
	auto button = GetButton(widgetId);
	Expects(button);
	button->buttonState = newState;
}

void Ui::WidgetRemoveRegardParent(LgcyWidgetId widIdx)
{
	mLegacy.RemoveChildWidget(widIdx);
}

void Ui::WidgetAndWindowRemove(LgcyWidgetId widId)
{
	mLegacy.RemoveWindow(widId);
	mLegacy.RemoveWidget(widId);
}

void Ui::WidgetRemove(LgcyWidgetId widId) {
	mLegacy.RemoveWidget(widId);
}

void Ui::WidgetSetHidden(LgcyWidgetId widId, BOOL hidden)
{
	mLegacy.SetHidden(widId, hidden == TRUE);
}

LgcyWindow* Ui::GetWindow(LgcyWidgetId widId){
	LgcyWidget* result = GetWidget(widId);
	if (!result)
		return nullptr;
	if (result->type == LgcyWidgetType::Window)	{
		return static_cast<LgcyWindow*>(result);
	}
	return nullptr;
}

LgcyButton* Ui::GetButton(LgcyWidgetId widId){
	auto result = GetWidget(widId);
	if (!result || result->type != LgcyWidgetType::Button) {
		return nullptr;
	}
	return static_cast<LgcyButton*>(result);
}

LgcyScrollBar * Ui::GetScrollBar(LgcyWidgetId widId) {
	auto result = mLegacy.GetWidget(widId);
	if (!result || result->type != LgcyWidgetType::Scrollbar)
		return nullptr;
	return static_cast<LgcyScrollBar*>(result);
}

LgcyWidget* Ui::GetWidget(LgcyWidgetId widId)
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

LgcyButtonState Ui::GetButtonState(LgcyWidgetId widId) {
	auto button = GetButton(widId);
	Expects(button);
	return button->buttonState;
}

void Ui::WidgetBringToFront(LgcyWidgetId widId)
{
	mLegacy.BringToFront(widId);
}

int Ui::WidgetlistIndexof(LgcyWidgetId widgetId, int* widgetlist, int size)
{
	for (int i = 0; i < size; i++) {
		if (widgetlist[i] == widgetId)
			return i;
	}

	return -1;
}

bool Ui::WidgetContainsPoint(int widgetId, int x, int y)
{
	auto widg = GetWidget(widgetId);
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
	auto wndWidget = GetWindow(windowId);
	if (wndWidget == nullptr)
		return windowId;
	if (wndWidget->IsHidden())
		return windowId;

	for (int i = wndWidget->childrenCount - 1; i >= 0; i-- )
	{
		auto childId = wndWidget->children[i];
		if (childId != -1)
		{
			auto childWid = GetWidget(childId);
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
			auto globalWid = GetWidget(globalWidId);
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
				if (!GetWidget(globalWid->parentId)->IsHidden())
				{
					enqueue4 = true;
				}
			}
			// scrollbar
			else if (globalWid->IsScrollBar())
			{
				if (globalWid->IsHidden() || GetWidget(globalWid->parentId)->IsHidden())
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
			auto widAtCursor = GetWidget(widIdAtCursor);
			if (widAtCursor->type == LgcyWidgetType::Window)
			{
				auto widAtCursorWindow = GetWindow(widIdAtCursor);
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

bool Ui::ProcessMessage(TigMsg &msg)
{

	switch (msg.type) {
	case TigMsgType::MOUSE:
		return ProcessMouseMessage(msg);
	case TigMsgType::WIDGET:
		return ProcessWidgetMessage(msg);
	default:
		for (auto &windowId : mLegacy.GetActiveWindows()) {
			auto window = mLegacy.GetWidget(windowId);

			if (!window->IsHidden() && window->CanHandleMessage()) {
				if (window->HandleMessage(msg)) {
					return true;
				}
			}
		}
		return false;
	}

}

bool Ui::ScrollbarGetY(int widId, int * scrollbarY) {
	auto widget = (LgcyScrollBar*) GetWidget(widId);
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

void Ui::ScrollbarSetYmax(LgcyWidgetId widId, int yMax)
{
	LgcyScrollBar *widg = GetScrollBar(widId);
	Expects(widg);
	widg->yMax = yMax;
}

void Ui::ScrollbarSetY(LgcyWidgetId widId, int value) {
	auto scrollbar = GetScrollBar(widId);

	scrollbar->scrollbarY = value;

	TigMsg msg;
	msg.createdMs = temple::GetRef<int>(0x11E74578);
	msg.type = TigMsgType::WIDGET;
	msg.arg2 = 5;
	msg.arg1 = scrollbar->parentId;
	messageQueue->Enqueue(msg);
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

bool Ui::ProcessWidgetMessage(TigMsg & msg)
{
	auto widgetId = msg.arg1;
	LgcyWidget *dispatchTo;
	while (widgetId != -1 && (dispatchTo = GetWidget(widgetId)) != nullptr)
	{
		if ((dispatchTo->parentId == -1 || !IsWidgetHidden(dispatchTo->parentId)) && !dispatchTo->IsHidden())
		{
			if (dispatchTo->CanHandleMessage())
			{
				if (dispatchTo->HandleMessage(msg)) {
					return true;
				}
			}
		}
		// Bubble up the msg if the widget didn't handle it
		widgetId = dispatchTo->parentId;
	}
	return false;
}

bool Ui::ProcessMouseMessage(TigMsg & msg)
{
	// Handle if a widget requested mouse capture
	if (mMouseCaptureWidgetId != -1)
	{
		auto widget = GetWidget(mMouseCaptureWidgetId);
		if (widget && widget->CanHandleMessage()) {
			widget->HandleMessage(msg);
			return true;
		}
		return false;
	}

	auto &windowIds = mLegacy.GetActiveWindows();

	for (size_t i = 0; i < windowIds.size(); i++) {
		auto window = mLegacy.GetWindow(windowIds[i]);

		if (!window || !window->IsWindow() || window->IsHidden() || !WidgetContainsPoint(windowIds[i], msg.arg1, msg.arg2)) {
			continue;
		}

		// Try dispatching the msg to all children of the window that are also under the mouse cursor, in reverse order of their
		// own insertion into the children list
		for (auto j = 0u; j < window->childrenCount; j++) {
			auto idx = window->childrenCount - 1 - j;
			auto childId = window->children[idx];

			if (mLegacy.DoesWidgetContain(childId, msg.arg1, msg.arg2))
			{
				auto child = mLegacy.GetWidget(childId);
				if (child && child->CanHandleMessage() && !child->IsHidden()) {
					if (child->HandleMessage(msg)) {
						return true;
					}
				}
			}
		}

		// After checking with all children, dispatch the msg to the window itself
		if (window->CanHandleMessage() && !window->IsHidden() && window->HandleMessage(msg)) {
			return true;
		}

	}

	return false;
}
