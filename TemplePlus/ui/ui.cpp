#include "stdafx.h"
#include "ui.h"
#include "tig/tig.h"
#include <util/fixes.h>
#include <tig/tig_msg.h>
#include <sound.h>
#include <tig/tig_font.h>
#include <tig/tig_mouse.h>
#include "messages/messagequeue.h"
#include "widgets/widgets.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/legacysystems.h"
#include "gameview.h"
#include "ui_systems.h"
#include "ui_dialog.h"
#include "mod_support.h"
#include "pybind11/pybind11.h"
#include "python/python_integration_obj.h"

UiManager *uiManager;
namespace py = pybind11;


/**
 * Handles button click triggering.
 * When a button receives a mouse-down event, there are two possible behaviors:
 * - If the button is non-repeat, the button captures the mouse input
 *   - If the mouse button is released while the mouse is over the button, 
 *     an OnClick message is dispatched to that button.
 * - If the button is repeat, the button also captures the mouse input, and an OnClick
 *   msg is immediately dispatched.
 *  - The next trigger time is set to current-time + 500ms
 *  - When the mouse leaves the button's client area, the auto-repeat is suspended (next action time set to 0)
 *  - When the mouse button is released (regardless of where), the auto repeat is suspended
 *  - When the time update event is received, and the time for the next press elapsed, trigger an onclick msg
 */
class ButtonClickBehavior {
public:
		
	void UpdateTime(int newTime);
	
private:
	int mNextClick = 0;
	int mX;
	int mY;
	int mButtonDown = false;
	LgcyWidgetId mReceiver = -1;
};

static ButtonClickBehavior buttonClickBehavior;

UiManager::UiManager() {
	Expects(uiManager == nullptr);
	uiManager = this;
}

UiManager::~UiManager() {
	if (uiManager == this) {
		uiManager = nullptr;
	}
}

void UiManager::SetAdvancedWidget(LgcyWidgetId id, WidgetBase * widget)
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		it->second.advancedWidget = widget;
	}
}

WidgetBase * UiManager::GetAdvancedWidget(LgcyWidgetId id) const
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		return it->second.advancedWidget;
	}
	return nullptr;
}

class UiReplacement : TempleFix
{
public:

	void apply() override
	{
		 /*
		 Hook for missing portraits (annoying logspam!)
		 */
		 static int(__cdecl*orgGetAsset)(UiAssetType, int, int*, int) = replaceFunction<int(__cdecl)(UiAssetType, int, int*, int)>(0x1004A360,
			 [](UiAssetType assetType, int id, int* out, int subId) {
			 
			 if (assetType == UiAssetType::Portraits){

				 auto uiArtManAssets_ = temple::GetRef<IdxTable<int>*>(0x10AA3220);
				 IdxTableWrapper<int> uiPortraitTextures(&uiArtManAssets_[0]);

				 auto compositeId = id + subId;
				 if (gameSystems->GetPortrait().IsModularId( id)){
					 auto portpackKey = gameSystems->GetPortrait().GetKeyFromId(id);
					 auto dekey = id ^ portpackKey;
					 dekey += subId;
					 auto newCompositeId = dekey ^ portpackKey;
					 if (compositeId != newCompositeId){
						 auto dbgBp = 1;
					 }
					 compositeId = newCompositeId;
				 }

				 auto textId = uiPortraitTextures.get(compositeId);
				 if (textId){
					 *out = *textId;
					 return 0;
				 }
				
				 auto porFile = gameSystems->GetPortrait().GetPortraitFileFromId(id, subId);

				 if (textureFuncs.RegisterTexture(porFile.c_str(), out)){
					 return 1;
				 }

				 uiPortraitTextures.put(compositeId, *out);
			 }
			 return orgGetAsset(assetType, id, out, subId);
		 });

		 // DrawTextInWidget hook
		 replaceFunction<bool(__cdecl)(int, char*, TigRect&, TigTextStyle&)>(0x101F87C0, [](int widgetId ,char* text, TigRect& rect, TigTextStyle& style)->bool
		 {
			 auto wid = uiManager->GetWidget(widgetId);
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

		 replaceFunction<uint32_t(__cdecl)()>(0x10143560, []()->uint32_t // int signed int UiManagerQuickSave()
		 {
			 return uiManager->QuickSave();
		 });

	}
} uiReplacement;

LgcyButton::LgcyButton(const char* ButtonName, int ParentId, int X, int Y, int Width, int Height){
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
	sndHoverOn = -1;
	sndHoverOff = -1;
}

LgcyButton::LgcyButton(const char* ButtonName, int ParentId, TigRect& rect){
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
	sndHoverOn = -1;
	sndHoverOff = -1;
}

void LgcyButton::SetDefaultSounds()
{
	sndDown = 3012;
	sndClick = 3013;
	sndHoverOn = 3010;
	sndHoverOff = 3011;
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
	auto p = uiManager->GetWindow(parentId);
	this->x += p->x;
	this->y += p->y;
	return false;
}

LgcyWidgetId UiManager::AddWindow(LgcyWindow& widget)
{
	widget.type = LgcyWidgetType::Window;

	auto widgetId = AddWidget(&widget, __FILE__, __LINE__);
	AddWindow(widgetId);
	widget.widgetId = widgetId;
	return widgetId;
}

BOOL UiManager::ButtonInit(LgcyButton* widg, char* buttonName, int parentId, int x, int y, int width, int height)
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
	widg->sndHoverOn = -1;
	widg->sndHoverOff = -1;
	return 0;
}

LgcyWidgetId UiManager::AddButton(LgcyButton& button)
{
	button.type = LgcyWidgetType::Button;
	auto widgetId = AddWidget(&button, __FILE__, __LINE__);
	button.widgetId = widgetId;
	return widgetId;
}

LgcyWidgetId UiManager::AddButton(LgcyButton &button, LgcyWidgetId parentId)
{
	auto buttonId = AddButton(button);
	AddChild(parentId, buttonId);
	return buttonId;
}

LgcyWidgetId UiManager::AddScrollBar(LgcyScrollBar& scrollBar)
{
	scrollBar.type = LgcyWidgetType::Scrollbar;
	auto widgetId = AddWidget(&scrollBar, __FILE__, __LINE__);
	scrollBar.widgetId = widgetId;
	return widgetId;
}

LgcyWidgetId UiManager::AddScrollBar(LgcyScrollBar& scrollBar, LgcyWidgetId parentId)
{
	auto scrollBarId = AddScrollBar(scrollBar);
	AddChild(parentId, scrollBarId);
	return scrollBarId;
}

void UiManager::SetButtonState(LgcyWidgetId widgetId, LgcyButtonState newState)
{
	auto button = GetButton(widgetId);
	Expects(button);
	button->buttonState = newState;
}

LgcyButtonState UiManager::GetButtonState(LgcyWidgetId widId) {
	auto button = GetButton(widId);
	Expects(button);
	return button->buttonState;
}

bool UiManager::ScrollbarGetY(LgcyWidgetId widId, int * scrollbarY) {
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

void UiManager::ScrollbarSetYmax(LgcyWidgetId widId, int yMax)
{
	LgcyScrollBar *widg = GetScrollBar(widId);
	Expects(widg);
	widg->yMax = yMax;
}

void UiManager::ScrollbarSetY(LgcyWidgetId widId, int value) {
	auto scrollbar = GetScrollBar(widId);

	scrollbar->scrollbarY = value;

	TigMsg msg;
	msg.createdMs = temple::GetRef<int>(0x11E74578);
	msg.type = TigMsgType::WIDGET;
	msg.arg2 = 5;
	msg.arg1 = scrollbar->parentId;
	messageQueue->Enqueue(msg);
}


LgcyWidgetId UiManager::AddWidget(const LgcyWidget *widget, const char *sourceFile, uint32_t sourceLine)
{
	// Assign a free id
	auto assignedId = mNextWidgetId++;

	// Make a private copy of the widget
	auto &activeWidget = mActiveWidgets[assignedId];

	activeWidget.sourceFile = sourceFile;
	activeWidget.sourceLine = sourceLine;

	// Make a copy and have it managed by the unique_ptr
	activeWidget.widget.reset(reinterpret_cast<LgcyWidget*>(malloc(widget->GetSize())));
	memcpy(activeWidget.widget.get(), widget, widget->GetSize());

	// Set some basic properties on the widget
	activeWidget.widget->size = widget->GetSize();
	activeWidget.widget->widgetId = assignedId;

	return assignedId;
}

void UiManager::AddWindow(LgcyWidgetId id)
{

	if (find(mActiveWindows.begin(), mActiveWindows.end(), id) != mActiveWindows.end()) {
		// Window is already in the list
		return;
	}

	auto window = GetWindow(id);
	if (!window) {
		throw new TempleException(format("Trying to add widget {} as a window which isn't a window.", id));
	}

	// Don't add it, if it's hidden
	if (window->IsHidden()) {
		return;
	}

	mActiveWindows.push_back(id);
	BringToFront(id);
}

/* 0x101F8F60 */
void UiManager::RemoveWindow(LgcyWidgetId id)
{
	for (auto it = mActiveWindows.begin(); it != mActiveWindows.end(); ) {
		if (*it == id) {
			it = mActiveWindows.erase(it);
		}
		else {
			it++;
		}
	}
	SortWindows();
}

LgcyWidget * UiManager::GetWidget(LgcyWidgetId id) const
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		return it->second.widget.get();
	}
	return nullptr;
}

LgcyWindow* UiManager::GetWindow(LgcyWidgetId widId) const
{
	LgcyWidget* result = GetWidget(widId);
	if (!result)
		return nullptr;
	if (result->type == LgcyWidgetType::Window) {
		return static_cast<LgcyWindow*>(result);
	}
	return nullptr;
}

LgcyButton* UiManager::GetButton(LgcyWidgetId widId) const
{
	auto result = GetWidget(widId);
	if (!result || result->type != LgcyWidgetType::Button) {
		return nullptr;
	}
	return static_cast<LgcyButton*>(result);
}

LgcyScrollBar* UiManager::GetScrollBar(LgcyWidgetId widId) const
{
	auto result = GetWidget(widId);
	if (!result || result->type != LgcyWidgetType::Scrollbar)
		return nullptr;
	return static_cast<LgcyScrollBar*>(result);
}

void UiManager::BringToFront(LgcyWidgetId id)
{
	auto window = GetWindow(id);
	if (window) {
		window->zIndex = INT32_MAX;
		SortWindows();
	}
}

void UiManager::SendToBack(LgcyWidgetId id)
{
	auto window = GetWindow(id);
	if (window) {
		window->zIndex = INT32_MIN;
		SortWindows();
	}
}

void UiManager::SetHidden(LgcyWidgetId id, bool hidden)
{
	auto widget = GetWidget(id);
	if (hidden) {
		widget->flags |= 1;
	}
	else {
		widget->flags &= (~1);
	} 

	// New widgets are rendered recursively and dont need to be 
	// in the top-level window list to be rendered if they are nested
	// children
	bool isNewWidget = GetAdvancedWidget(id) != nullptr;

	// Update the top-level window list
	if (widget->IsWindow() && (!isNewWidget || widget->parentId == -1)) {
		if (hidden) {
			RemoveWindow(id);
		} else {
			AddWindow(id);
		}
	}

	RefreshMouseOverState();

}

/* 0x101F9420 */
void UiManager::RemoveWidget(LgcyWidgetId id)
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		if (it->second.widget->IsWindow()) {
			RemoveWindow(id);
		}
		mActiveWidgets.erase(it);
	}
	UnsetMouseCaptureWidgetId(id);
	if (mMouseButtonId == id) {
		mMouseButtonId = 0;
	}
	if (mWidgetMouseHandlerWidgetId == id) {
		mWidgetMouseHandlerWidgetId = 0;
	}
}

bool UiManager::AddChild(LgcyWidgetId parentId, LgcyWidgetId childId)
{
	auto parent = GetWindow(parentId);
	if (parent) {

		if (parent->childrenCount >= 127) {
			return false;
		}

		auto child = GetWidget(childId);
		if (child) {
			child->parentId = parentId;
		}
		parent->children[parent->childrenCount++] = childId;

		RefreshMouseOverState();
		return true;
	}
	else {
		return false;
	}
}

void UiManager::RemoveChildWidget(LgcyWidgetId id)
{
	auto widget = GetWidget(id);

	if (!widget || widget->parentId == -1) {
		return;
	}

	auto parent = GetWidget(widget->parentId);
	if (!parent) {
		return;
	}
	assert(parent->IsWindow());
	auto parentWindow = (LgcyWindow*)parent;

	for (uint32_t i = 0; i < parentWindow->childrenCount; ++i) {
		if (parentWindow->children[i] == widget->widgetId) {
			parentWindow->children[i] = -1;
			for (uint32_t j = i + 1; j < parentWindow->childrenCount; ++j) {
				parentWindow->children[j - 1] = parentWindow->children[j];
			}
			parentWindow->childrenCount--;
			return;
		}
	}

}

void UiManager::Render()
{
	// Make a copy here since some vanilla logic will show/hide windows in their render callbacks
	auto activeWindows(mActiveWindows);

	for (auto windowId : activeWindows) {

		// Our new widget system handles rendering itself
		auto advWidget = GetAdvancedWidget(windowId);
		if (advWidget) {
			advWidget->Render();
			continue;
		}

		auto window = GetWindow(windowId);
		if (window->IsHidden()) {
			continue;
		}

		// Render the widget itself
		auto renderFunc = window->render;
		if (renderFunc) {
			renderFunc(windowId);
		}

		// Render all child widgets
		for (uint32_t i = 0; i < window->childrenCount; ++i) {
			auto childId = window->children[i];
			auto child = GetWidget(childId);
			if (!child->IsHidden() && child->render) {
				child->render(childId);
			}
		}
	}
}

LgcyWindow * UiManager::GetWindowAt(int x, int y)
{
	// Backwards because of render order (rendered last is really on top)
	for (int i = mActiveWindows.size() - 1; i >= 0; --i) {
		auto windowId = mActiveWindows[i];
		auto window = GetWindow(windowId);
		if (!window->IsHidden() && DoesWidgetContain(windowId, x, y)) {
			return window;
		}
	}
	return nullptr;
}

LgcyWidgetId UiManager::GetWidgetAt(int x, int y)
{
	LgcyWidgetId result = -1;

	// Backwards because of render order (rendered last is really on top)
	for (int i = mActiveWindows.size() - 1; i >= 0; --i) {
		auto windowId = mActiveWindows[i];
		auto window = GetWindow(windowId);
		if (!window->IsHidden() && DoesWidgetContain(windowId, x, y)) {
			result = windowId;
			
			auto advWidget = GetAdvancedWidget(windowId);
			if (advWidget) {
				int localX = x - window->x;
				int localY = y - window->y;

				auto in = advWidget->PickWidget(localX, localY);
				if (in) {
					return in->GetWidgetId();
				}
				break;
			}

			// Also in reverse order
			for (int j = window->childrenCount - 1; j >= 0; --j) {
				auto childId = window->children[j];
				auto child = GetWidget(childId);
				if (child && !child->IsHidden() && DoesWidgetContain(childId, x, y)) {
					result = childId;
					break;
				}
			}

			break;
		}
	}
	return result;
}

bool UiManager::DoesWidgetContain(LgcyWidgetId id, int x, int y)
{
	if (id == -1) {
		return false;
	}

	auto widget = GetWidget(id);
	TigRect rect(widget->x, widget->y, widget->width, widget->height);

	auto advWidget = GetAdvancedWidget(id);
	if (advWidget != nullptr){
		rect = advWidget->GetContentArea();
	}

	return widget
		&& x >= rect.x
		&& y >= rect.y
		&& x < (int)(rect.x + rect.width)
		&& y < (int)(rect.y + rect.height);
}

void UiManager::RefreshMouseOverState()
{
	TigMouseState state;
	mouseFuncs.GetState(&state);

	TigMouseMsg msg;
	msg.x = state.x;
	msg.y = state.y;
	msg.mouseStateField24 = state.field24;
	msg.flags = MouseStateFlags::MSF_POS_CHANGE;
	TranslateMouseMessage(msg);
}

void UiManager::SortWindows()
{
	// Sort Windows by Z-Index
	sort(mActiveWindows.begin(), mActiveWindows.end(), [this](LgcyWidgetId idA, LgcyWidgetId idB) {
		auto windowA = GetWindow(idA);
		auto windowB = GetWindow(idB);
		return windowA->zIndex < windowB->zIndex;
	});

	// Reassign a zindex in monotonous order
	for (size_t i = 0; i < mActiveWindows.size(); ++i) {
		auto window = GetWindow(mActiveWindows[i]);
		window->zIndex = i;
	}
}

bool UiManager::TranslateMouseMessage(const TigMouseMsg& mouseMsg)
{
	int flags = mouseMsg.flags;
	int x = mouseMsg.x, y = mouseMsg.y;
	TigMsgWidget newTigMsg;
	newTigMsg.createdMs = temple::GetRef<int>(0x11E74578); // system time as recorded by the Msg Process function
	newTigMsg.type = TigMsgType::WIDGET;
	newTigMsg.x = x;
	newTigMsg.y = y;

	int widIdAtCursor = GetWidgetAt(x, y);

	int globalWidId = mWidgetMouseHandlerWidgetId;

	// moused widget changed
	if ((flags & MSF_POS_CHANGE) && widIdAtCursor != globalWidId)
	{
		if (widIdAtCursor != -1 && mouseFuncs.GetCursorDrawCallbackId() == (uint32_t)mMouseMsgHandlerRenderTooltipCallback)
		{
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
		}

		if (globalWidId != -1)
		{
			bool enqueueExited = false;
			auto globalWid = GetWidget(globalWidId);
			// if window

			if (globalWid == nullptr) return false;  //Happens sometimes on shutdown

			if (globalWid->IsWindow())
			{
				auto prevHoveredWindow = (LgcyWindow*)globalWid;
				if (prevHoveredWindow->mouseState == LgcyWindowMouseState::Pressed) {
					prevHoveredWindow->mouseState = LgcyWindowMouseState::PressedOutside;
				} else if (prevHoveredWindow->mouseState != LgcyWindowMouseState::PressedOutside) {
					prevHoveredWindow->mouseState = LgcyWindowMouseState::Outside;
				}
				enqueueExited = true;
			}
			// button
			else if (globalWid->IsButton() && !globalWid->IsHidden())
			{
				auto buttonWid = GetButton(globalWidId);
				switch (buttonWid->buttonState) {
				case LgcyButtonState::Hovered:
					// Unhover
					buttonWid->buttonState = LgcyButtonState::Normal;
					sound.MssPlaySound(buttonWid->sndHoverOff);
					break;
				case LgcyButtonState::Down:
					// Down -> Released without click event
					buttonWid->buttonState = LgcyButtonState::Released;
					break;
				}
				if (!GetWidget(globalWid->parentId)->IsHidden())
				{
					enqueueExited = true;
				}
			}
			// scrollbar
			else if (globalWid->IsScrollBar())
			{
				if (globalWid->IsHidden() || GetWidget(globalWid->parentId)->IsHidden())
					enqueueExited = false;
				else
					enqueueExited = true;
			}

			if (enqueueExited)
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
				auto widAtCursorWindow = (LgcyWindow*)widAtCursor;
				if (widAtCursorWindow->mouseState == LgcyWindowMouseState::PressedOutside) {
					widAtCursorWindow->mouseState = LgcyWindowMouseState::Pressed;
				}
				else if (widAtCursorWindow->mouseState != LgcyWindowMouseState::Pressed) {
					widAtCursorWindow->mouseState = LgcyWindowMouseState::Hovered;
				}
			}
			else if (widAtCursor->type == LgcyWidgetType::Button) {
				auto buttonWid = (LgcyButton*)widAtCursor;
				if (buttonWid->buttonState != LgcyButtonState::Normal)
				{
					if (buttonWid->buttonState == LgcyButtonState::Released)
					{
						buttonWid->buttonState = LgcyButtonState::Down;
					}
				}
				else
				{
					buttonWid->buttonState = LgcyButtonState::Hovered;
					sound.MssPlaySound(buttonWid->sndHoverOn);
				}
			}
			newTigMsg.widgetId = widIdAtCursor;
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Entered;
			messageQueue->Enqueue(newTigMsg);
		}
		globalWidId = mWidgetMouseHandlerWidgetId = widIdAtCursor;
	}

	if (mouseMsg.flags & MouseStateFlags::MSF_POS_CHANGE_SLOW && globalWidId != -1 && !mouseFuncs.GetCursorDrawCallbackId())
	{
		mouseFuncs.SetCursorDrawCallback([&](int x, int y) {
			mMouseMsgHandlerRenderTooltipCallback(x, y, &mWidgetMouseHandlerWidgetId);
		}, (uint32_t)mMouseMsgHandlerRenderTooltipCallback);
	}

	if (mouseMsg.flags & MouseStateFlags::MSF_LMB_CLICK)
	{
		auto widIdAtCursor2 = GetWidgetAt(mouseMsg.x, mouseMsg.y); // probably redundant to do again, but just to be safe...
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
			mMouseButtonId = widIdAtCursor2;
			messageQueue->Enqueue(newTigMsg);
		}
	}

	if ((mouseMsg.flags & MouseStateFlags::MSF_LMB_RELEASED) && mMouseButtonId != -1)
	{
		auto button = GetButton(mMouseButtonId);
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
				return false;
			}
		}
		auto widIdAtCursor2 = GetWidgetAt(mouseMsg.x, mouseMsg.y); // probably redundant to do again, but just to be safe...
		newTigMsg.widgetId = mMouseButtonId;
		newTigMsg.widgetEventType = (widIdAtCursor2 != mMouseButtonId) ? TigMsgWidgetEvent::MouseReleasedAtDifferentButton : TigMsgWidgetEvent::MouseReleased;
		messageQueue->Enqueue(newTigMsg);
		mMouseButtonId = -1;
	}
	
	return false;
}

bool UiManager::ProcessMessage(TigMsg &msg)
{
	// Dispatch time update messages continuously to all advanced widgets
	if (msg.type == TigMsgType::UPDATE_TIME) {
		for (auto &entry : mActiveWidgets) {
			if (entry.second.advancedWidget) {
				entry.second.advancedWidget->OnUpdateTime(msg.createdMs);
			}
		}
	}

	switch (msg.type) {
	case TigMsgType::MOUSE:
		return ProcessMouseMessage(msg);
	case TigMsgType::WIDGET:
		return ProcessWidgetMessage(msg);
	default:
		// In order from top to bottom (back is top) using a index instead of an iterator because the iterator may invalidate
		for (size_t i = mActiveWindows.size(); i > 0; i--) {
			auto window = GetWidget(mActiveWindows[i-1]);

			if (!window->IsHidden() && window->CanHandleMessage()) {
				if (window->HandleMessage(msg)) {
					return true;
				}
			}
		}
		return false;
	}

}

gfx::Size UiManager::GetCanvasSize() const
{
	return { gameView->GetWidth(), gameView->GetHeight() };
}

uint32_t UiManager::QuickSave()
{
	uint32_t& mDoYouWantToQuitActive = temple::GetRef<uint32_t>(0x10BE8CF0);
	if (!mDoYouWantToQuitActive && !uiSystems->GetDlg().IsActive()) {

		if (modSupport.IsZMOD()) {
			py::tuple args = py::make_tuple();
			auto pyResult = pythonObjIntegration.ExecuteScript("game_save", "can_quick_save", args.ptr()); // def can_quick_save() -> int
			if (PyInt_Check(pyResult)) {
				auto result = _PyInt_AsInt(pyResult);
				if (!result) {
					return FALSE;
				}
			}
			Py_DECREF(pyResult);
		}

		return gameSystems->QuickSave() ? TRUE : FALSE;
	}
	return FALSE;
}

bool UiManager::ProcessWidgetMessage(TigMsg & msg)
{
	auto widgetId = msg.arg1;
	LgcyWidget *dispatchTo;
	while (widgetId != -1 && (dispatchTo = GetWidget(widgetId)) != nullptr)
	{
		LgcyWidget *parent = (dispatchTo->parentId != -1) ? GetWindow(dispatchTo->parentId) : nullptr;
		if ((!parent || !parent->IsHidden()) && !dispatchTo->IsHidden())
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

bool UiManager::ProcessMouseMessage(TigMsg & msg)
{
	// Handle if a widget requested mouse capture
	if (mMouseCaptureWidgetId != -1)
	{
		auto advWidget = GetAdvancedWidget(mMouseCaptureWidgetId);
		if (advWidget) {			
			advWidget->HandleMessage(msg);
			return true;
		} else {
			auto widget = GetWidget(mMouseCaptureWidgetId);
			if (widget && widget->CanHandleMessage()) {
				widget->HandleMessage(msg);
				return true;
			}
		}
		return false;
	}

	for (auto it = mActiveWindows.rbegin(); it != mActiveWindows.rend(); it++) {
		auto windowId = *it;
		auto window = GetWindow(windowId);

		if (!window || !window->IsWindow() || window->IsHidden() || !DoesWidgetContain(windowId, msg.arg1, msg.arg2)) {
			continue;
		}

		// Try dispatching the msg to all children of the window that are also under the mouse cursor, in reverse order of their
		// own insertion into the children list
		for (auto j = 0u; j < window->childrenCount; j++) {
			auto idx = window->childrenCount - 1 - j;
			auto childId = window->children[idx];

			if (DoesWidgetContain(childId, msg.arg1, msg.arg2))
			{
				auto child = GetWidget(childId);
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

size_t LgcyWidget::GetSize() const
{
	switch (type) {
	case LgcyWidgetType::Window:
		return 0x294;
	case LgcyWidgetType::Button:
		return 0xBC;
	case LgcyWidgetType::Scrollbar:
		return 0xB0;
	default:
		throw new TempleException("Cannot determine size of a widget that has an unknown type.");
	}
}

LgcyWindow::LgcyWindow()
{
	widgetId = -1;
	parentId = -1;
	type = LgcyWidgetType::Window;
	this->size = sizeof(LgcyWindow);
	x = 0;
	y = 0;
	xrelated = 0;
	yrelated = 0;
	height = 0;
	width = 0;
	render = 0;
	handleMessage = 0;
	renderTooltip = 0;
	childrenCount = 0;
	mouseState = LgcyWindowMouseState::Outside;
	field_28c = 0;
	zIndex = 0;
	flags = 0;
}

LgcyWindow::LgcyWindow(int x, int y, int w, int h) : LgcyWindow()
{
	this->x = x;
	this->y = y;
	this->xrelated = x;
	this->yrelated = y;
	this->width = w;
	this->height = h;
}

int LgcyWindow::AddChildButton(const std::string & btnName, int xRelative, int yRelative, int w, int h, LgcyWidgetRenderFn renderHandler, LgcyWidgetHandleMsgFn msgHandler, LgcyWidgetRenderTooltipFn tooltipHandler, bool useDefaultSounds)
{
	LgcyButton btn(btnName.c_str(), this->widgetId, xRelative, yRelative, w, h);
	btn.x += this->x; btn.y += this->y;
	btn.render = renderHandler;
	btn.handleMessage = msgHandler;
	
	if (nullptr != tooltipHandler){
		btn.renderTooltip = tooltipHandler;
	}
	if (useDefaultSounds)
		btn.SetDefaultSounds();
	int btnId = uiManager->AddButton(btn, this->widgetId);
	
	return btnId;
}

LgcyButton::LgcyButton() {
	memset(this, 0, sizeof(LgcyButton));
	type = LgcyWidgetType::Button;
}

void LgcyWidgetDeleter::operator()(LgcyWidget* p) {
	free(p);
}
