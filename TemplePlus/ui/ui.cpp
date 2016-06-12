#include "stdafx.h"
#include "ui.h"
#include "tig/tig.h"
#include <util/fixes.h>
#include <tig/tig_msg.h>
#include <sound.h>
#include <tig/tig_font.h>
#include <tig/tig_mouse.h>

Ui ui;
Widget** Ui::activeWidgets;


#pragma region UI System Specification
/*
	System spec used to define an UI subsystem (i.e. dialogs and such).
*/
typedef int (__cdecl *UiSystemInit)(const GameSystemConf& conf);
typedef void (__cdecl *UiSystemReset)();
typedef bool (__cdecl *UiSystemLoadModule)();
typedef void (__cdecl *UiSystemUnloadModule)();
typedef void (__cdecl *UiSystemShutdown)();
typedef bool (__cdecl *UiSystemSaveGame)(void* tioFile);
typedef bool (__cdecl *UiSystemLoadGame)(void* sth);
typedef bool (__cdecl *UiSystemResizeScreen)(const UiResizeArgs& resizeArgs);

struct UiSystem {
	const char* name;
	UiSystemInit init;
	UiSystemReset reset;
	UiSystemLoadModule loadModule;
	UiSystemUnloadModule unloadModule;
	UiSystemShutdown shutdown;
	UiSystemSaveGame savegame;
	UiSystemLoadGame loadgame;
	UiSystemResizeScreen resizeScreen;
};

const int UiSystemsCount = 43;
#pragma endregion

/*
	Native ToEE functions we use.
*/
typedef void (__cdecl *SaveCallback)();

static struct UiFuncs : temple::AddressTable {
	bool (__cdecl *Init)(const GameSystemConf* conf);
	bool (__cdecl *LoadModule)();
	void (__cdecl *UnloadModule)();
	void (__cdecl *Shutdown)();

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
	BOOL(__cdecl*AddWindow)(Widget* widget, unsigned size, int* widgetId, const char* codeFileName, int lineNumber);
	BOOL(__cdecl*AddButton)(WidgetType2* button, unsigned size, int* widgId, const char* codeFileName, int lineNumber);
	BOOL(__cdecl*ButtonSetButtonState)(int widgetId, int newState);
	BOOL(__cdecl*WidgetAndWindowRemove)(int widId);
	BOOL(__cdecl*WidgetSetHidden)(int widId, int hiddenState);
	BOOL(__cdecl*GetButtonState)(int widId, int* state);
	void(__cdecl*WidgetBringToFront)(int widId);
	void(*UiMouseMsgHandlerRenderTooltipCallback)(int x, int y, void* data); //


	ActiveWidgetListEntry* activeWidgetAllocList;
	Widget** activeWidgets;
	int* activeWidgetCount;
	int* visibleWindowCnt;

	// List of all UI subsystems (top level dialogs, basically)
	UiSystem* systems;

	// These are the original callbacks for game save/load
	SaveCallback GameLoad;
	SaveCallback GameSave;

	// Called to save UI related functions to the savegame in data2.sav
	SaveCallback* saveGameCallback;
	// Called to load UI related functions from the savegame in data2.sav
	SaveCallback* loadGameCallback;
	int *visibleWidgetWindows;
	int* uiWidgetMouseHandlerWidgetId;
	int* uiMouseButtonId;

	UiFuncs() {
		rebase(Init, 0x101156F0);
		rebase(LoadModule, 0x10115790);
		rebase(LoadImg, 0x101E8320);
		rebase(UnloadModule, 0x101152C0);
		rebase(Shutdown, 0x10115230);
		rebase(GetAsset, 0x1004A360);
		rebase(UpdateCombatUi, 0x1009A730);
		rebase(UpdatePartyUi, 0x1009A740);
		rebase(ShowWorldMap, 0x1015F140);
		rebase(WorldMapTravelByDialog, 0x10160450);
		rebase(ShowPartyPool, 0x10165E60);
		rebase(ShowCharUi, 0x10148E20);
		rebase(ShowWrittenUi, 0x10160F50);

		rebase(GameLoad, 0x101154B0);
		rebase(GameSave, 0x101152F0);


		
		rebase(systems, 0x102F6C10);
		rebase(uiWidgetMouseHandlerWidgetId, 0x10301324);
		rebase(uiMouseButtonId, 0x10301328);
		rebase(saveGameCallback, 0x103072C4);
		rebase(loadGameCallback, 0x103072C8);


		rebase(WidgetBringToFront, 0x101F8E40);
		rebase(BindButton, 0x101F8950);
		rebase(AddWindow,  0x101F8FD0);
		rebase(AddButton,  0x101F9460);
		rebase(WidgetAndWindowRemove, 0x101F9010);
		rebase(WidgetSetHidden, 0x101F9100);
		rebase(ButtonSetButtonState, 0x101F9510);
		rebase(GetButtonState,  0x101F9740);
		rebase(UiMouseMsgHandlerRenderTooltipCallback, 0x101F9870);
		
		rebase(visibleWidgetWindows, 0x10EF39F0);
		rebase(visibleWindowCnt, 0x10EF68D0);
		rebase(activeWidgetCount, 0x10EF68D8);
		rebase(activeWidgetAllocList, 0x10EF68DC);
		rebase(activeWidgets, 0x10EF68E0);
		
	}

	
} uiFuncs;


class UiReplacement : TempleFix
{
public:

	static Widget* WidgetGet( int widIdx)
	{
		if (widIdx >= 0 && widIdx < ACTIVE_WIDGET_CAP)
		{
			Widget * widg = uiFuncs.activeWidgets[widIdx];
			return widg;
		}
		return nullptr;
	};

	
	static WidgetType2 * GetButton(int widId)
	{
		WidgetType2 * result = (WidgetType2 *)WidgetGet(widId);
		if (!result || result->type != 2)
			return nullptr;
		return result;

	}
	
	static int WidgetRemove(int widId)
	{
		return reinterpret_cast<int(*)(int)>(temple::GetPointer<0x101F9420>())(widId);
	}

	static int WidgetType1RemoveChild(int parentId, int widId)
	{
		WidgetType1 * parent = (WidgetType1*)WidgetGet(parentId);
		if (parent && parent->type == 1)
		{
			for (size_t i = 0; i < parent->childrenCount;i++)
			{
				if (parent->children[i] == widId)
				{
					memcpy(&parent->children[i], &parent->children[i + 1], sizeof(int)* (parent->childrenCount - i - 1));
					parent->childrenCount--;
					auto child = WidgetGet(widId);
					if (child)
						child->parentId = -1;
					return 0;
				}
			}
		}
		return 1;
	};

	/*
		removes widget, including removing it from its parent's children list
	*/
	static int WidgetRemoveRegardParent(int widIdx)
	{
		Widget* widg = WidgetGet(widIdx);
		if (widg)
		{
			auto parent = widg->parentId;
			if (parent == -1)
				return WidgetRemove(widIdx);
			if (parent >= 0 && parent < ACTIVE_WIDGET_CAP)
			{
				if (WidgetType1RemoveChild(parent, widIdx) == 0)
					return WidgetRemove(widIdx);
			}
		}
		return 1;
	}

	static void Reset() {

		for (auto i = 0; i < UiSystemsCount; ++i) {
			auto resetFunc = uiFuncs.systems[i].reset;
			if (resetFunc) {
				resetFunc();
			}
		}
	}

	static int WidgetlistIndexof(int widId, int* widlist, int size)
	{

		for (int i = 0; i < size; i++)
		{
			if (widlist[i] == widId)
				return i;
		}

		return -1;
	}

	static int UiWidgetHandleMouseMsg(TigMouseMsg* mouseMsg);
	static int(__cdecl*orgUiWidgetHandleMouseMsg)(TigMouseMsg*);

	void apply() override
	{
		replaceFunction(0x1011DFE0, WidgetlistIndexof);
		replaceFunction(0x101F94D0, WidgetRemoveRegardParent);
		replaceFunction(0x101F90E0, WidgetGet);
		replaceFunction(0x101F9570, GetButton);
		replaceFunction(0x10115270, Reset);
		 orgUiWidgetHandleMouseMsg = replaceFunction(0x101F9970, UiWidgetHandleMouseMsg);

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
				 //auto uiCharEditorSkillsWnd = temple::GetPointer<WidgetType1>(0x10C7B628);
				 //int dummy = 1;


			 }
			 
			 

			 return tigFont.Draw(text, extents, style) != 0;

		 });


	}
} uiReplacement;

int(__cdecl*UiReplacement::orgUiWidgetHandleMouseMsg)(TigMouseMsg*);

int UiReplacement::UiWidgetHandleMouseMsg(TigMouseMsg* mouseMsg)
{
	return ui.UiWidgetHandleMouseMsg(mouseMsg);
}

static UiSystem& getUiSystem(const char* name) {
	// Search for the ui system to replace
	for (auto i = 0; i < UiSystemsCount; ++i) {
		if (!strcmp(name, uiFuncs.systems[i].name)) {
			return uiFuncs.systems[i];
		}
	}

	throw TempleException(format("Couldn't find UI system {}! Replacement failed.", name));
}

bool WidgetType1::Add(int* widIdOut){
	return ui.AddWindow(this, sizeof(WidgetType1), widIdOut, "ui.cpp", 325) != 0;
}

WidgetType2::WidgetType2(){
	memset(this, 0, sizeof(WidgetType2));
	type = 2;
}

WidgetType2::WidgetType2(char* ButtonName, int ParentId, int X, int Y, int Width, int Height){
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
	widgetFlags = 0;
	renderTooltip = nullptr;
	render = nullptr;
	handleMessage = nullptr;
	buttonState = 0;
	field98 = 0;
	type = 2;
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

WidgetType2::WidgetType2(char* ButtonName, int ParentId, TigRect& rect){
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
	widgetFlags = 0;
	renderTooltip = nullptr;
	render = nullptr;
	handleMessage = nullptr;
	buttonState = 0;
	field98 = 0;
	type = 2;
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

bool WidgetType2::Add(int* widIdOut){
	return ui.AddButton(this, sizeof(WidgetType2), widIdOut, "ui.cpp", 367) != 0;
}

int WidgetType3::GetY()
{
	auto getY = temple::GetRef<int(__cdecl)(int id, int& yOut)>(0x101FA150);
	int y = 0;
	if (getY(this->widgetId, y)){
		logger->warn("scrollbar error!");
		y = -1;
	}
	return y;
}

bool WidgetType3::Init(int X, int Y, int Height){

	x = X;
	y = Y;
	type = 3;
	parentId = -1;
	widgetId = -1;
	widgetFlags = 0;
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

bool WidgetType3::Add(int* widIdOut){
	return temple::GetRef<bool(__cdecl)(Widget*, size_t, int*, const char*, int)>(0x101F93D0)(this, sizeof(WidgetType3), widIdOut, "ui.cpp", 366);
}

bool Ui::GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int& textureIdOut) {
	return uiFuncs.GetAsset(assetType, static_cast<uint32_t>(assetIndex), textureIdOut, 0) == 0;
}

ImgFile* Ui::LoadImg(const char* filename) {
	return uiFuncs.LoadImg(filename);
}

void Ui::ResizeScreen(int bufferStuffId, int width, int height) {
	
	UiResizeArgs args;
	memset(&args, 0, sizeof(args));
	args.windowBufferStuffId = bufferStuffId;
	args.rect1.width = width;
	args.rect1.height = height;
	args.rect2.width = width;
	args.rect2.height = height;

	for (auto i = 0; i < UiSystemsCount; ++i) {
		auto resizeFunc = uiFuncs.systems[i].resizeScreen;
		if (resizeFunc) {
			resizeFunc(args);
		}
	}

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
	return (activeWidgets[widId]->widgetFlags & 1) != 0;
}

BOOL Ui::AddWindow(Widget* widget, unsigned size, int* widgetId, const char* codeFileName, int lineNumber)
{
	return uiFuncs.AddWindow(widget, size, widgetId, codeFileName, lineNumber);
}

BOOL Ui::ButtonInit(WidgetType2* widg, char* buttonName, int parentId, int x, int y, int width, int height)
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
	widg->widgetFlags = 0;
	widg->buttonState = 0;
	widg->renderTooltip = 0;
	widg->field98 = 0;
	widg->type = 2;
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

BOOL Ui::AddButton(WidgetType2* button, unsigned size, int* widgId, const char* codeFileName, int lineNumber)
{
	return uiFuncs.AddButton(button, size, widgId, codeFileName, lineNumber);
}

BOOL Ui::BindToParent(int parentId, int buttonId){
	return uiFuncs.BindButton(parentId, buttonId);
}

BOOL Ui::SetDefaultSounds(int widId){
	WidgetType2 widg;
	if (WidgetCopy(widId, &widg))
		return true;
	widg.sndDown = 3012;
	widg.sndClick = 3013;
	widg.hoverOn = 3010;
	widg.hoverOff = 3011;
	return WidgetSet(widId, &widg) != 0;
}

BOOL Ui::ButtonSetButtonState(int widgetId, int newState)
{
	return uiFuncs.ButtonSetButtonState(widgetId, newState);
}

BOOL Ui::WidgetRemoveRegardParent(int widIdx)
{
	return uiReplacement.WidgetRemoveRegardParent(widIdx);
}

BOOL Ui::WidgetAndWindowRemove(int widId)
{
	return uiFuncs.WidgetAndWindowRemove(widId);
}

BOOL Ui::WidgetSetHidden(int widId, int hiddenState)
{
	return uiFuncs.WidgetSetHidden(widId, hiddenState);
}


WidgetType1* Ui::WidgetGetType1(int widId){
	Widget* result = WidgetGet(widId);
	if (!result)
		return nullptr;
	if (result->type == 1)	{
		return static_cast<WidgetType1*>(result);
	}
	return nullptr;
}

WidgetType2* Ui::GetButton(int widId){
	auto result = WidgetGet(widId);
	if (!result || result->type != 2){
		return nullptr;
	}
	return static_cast<WidgetType2*>(result);
}

WidgetType3 * Ui::ScrollbarGet(int widId){
	auto result = WidgetGet(widId);
	if (!result || result->type != 3)
		return nullptr;
	return static_cast<WidgetType3*>(result);
}

Widget* Ui::WidgetGet(int widId)
{
	if (widId == -1)
		return nullptr;
	return uiFuncs.activeWidgets[widId];
}

int Ui::GetWindowContainingPoint(int x, int y)
{
	for (int i = 0; i < *uiFuncs.visibleWindowCnt; i++)
	{
		Widget* widg = WidgetGet(uiFuncs.visibleWidgetWindows[i]);
		if (widg)
		{
			if (widg->type == 1 && !(widg->widgetFlags & 1)
				&& WidgetContainsPoint(uiFuncs.visibleWidgetWindows[i], x,y)
				)
			{
				return uiFuncs.visibleWidgetWindows[i];
			}

		}
	}
	return -1;
}

BOOL Ui::GetButtonState(int widId, int* state){
	return uiFuncs.GetButtonState(widId, state);
}

bool Ui::GetButtonState(int widId, UiButtonState& state){
	auto widg = ui.GetButton(widId);
	if (!widg)
		return true;
	state = (UiButtonState)widg->buttonState;
	return false;
}

void Ui::WidgetBringToFront(int widId)
{
	return uiFuncs.WidgetBringToFront(widId);
}

int Ui::WidgetlistIndexof(int widgetId, int* widgetlist, int size)
{
	return uiReplacement.WidgetlistIndexof(widgetId, widgetlist, size);
}

BOOL Ui::WidgetContainsPoint(int widgetId, int x, int y)
{
	if (widgetId == -1)
		return false;
	auto widg = uiFuncs.activeWidgets[widgetId];
	if (widg == nullptr)
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
	if (wndWidget->widgetFlags & 1)
		return windowId;

	for (int i = wndWidget->childrenCount - 1; i >= 0; i-- )
	{
		auto childId = wndWidget->children[i];
		if (childId != -1)
		{
			auto childWid = uiFuncs.activeWidgets[childId];
			if (childWid != nullptr
				&& !(childWid->widgetFlags & 1)
				&& WidgetContainsPoint(childId, x,y))
			{
				return childId;
			}
		}
		
	}
	return windowId;
}

int Ui::UiWidgetHandleMouseMsg(TigMouseMsg* mouseMsg)
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
			if (globalWid->type == 1)
			{
				auto prevHoveredWindow = (WidgetType1*)globalWid;
				if (prevHoveredWindow->mouseState == WindowMouseState::Pressed) {
					prevHoveredWindow->mouseState = WindowMouseState::PressedOutside;
				} else if (prevHoveredWindow->mouseState != WindowMouseState::PressedOutside) {
					prevHoveredWindow->mouseState = WindowMouseState::Outside;
				}
				enqueue4 = true;
			} 
			// button
			else if (globalWid->type == 2 && !(globalWid->widgetFlags & 1))
			{
				auto buttonWid = uiReplacement.GetButton(globalWidId);
				switch (buttonWid->buttonState) {
				case 1:
					// Unhover
					buttonWid->buttonState = 0;
					sound.MssPlaySound(buttonWid->hoverOff);
					break;
				case 2:
					// Down -> Released without click event
					buttonWid->buttonState = 3;
					break;
				}
				if (!(WidgetGet(globalWid->parentId)->widgetFlags & 1))
				{
					enqueue4 = true;
				}
			}
			// scrollbar
			else if (globalWid->type == 3)
			{
				if (globalWid->widgetFlags & 1 || (WidgetGet(globalWid->parentId)->widgetFlags & 1 ))
					enqueue4 = false;
				else
					enqueue4 = true;
			} else if (globalWid->type > 3)
			{
				enqueue4 = true;
				logger->warn("Unknown widget type {} encountered!", globalWid->type);
			}

			if (enqueue4)
			{
				newTigMsg.widgetId = globalWidId;
				newTigMsg.widgetEventType = TigMsgWidgetEvent::Exited;
				msgFuncs.Enqueue(&newTigMsg);
			}
		}

		if (widIdAtCursor != -1)
		{
			auto widAtCursor = WidgetGet(widIdAtCursor);
			if (widAtCursor->type == 1)
			{
				auto widAtCursorWindow = WidgetGetType1(widIdAtCursor);
				if (widAtCursorWindow->mouseState == WindowMouseState::PressedOutside) {
					widAtCursorWindow->mouseState = WindowMouseState::Pressed;
				} else if (widAtCursorWindow->mouseState != WindowMouseState::Pressed) {
					widAtCursorWindow->mouseState = WindowMouseState::Hovered;
				}
			} else if (widAtCursor->type == 2) {
				auto buttonWid = uiReplacement.GetButton(widIdAtCursor);
				if (buttonWid->buttonState)
				{
					if (buttonWid->buttonState == 3)
					{
						buttonWid->buttonState = 2;
					}
				} 
				else
				{
					buttonWid->buttonState = 1;
					sound.MssPlaySound(buttonWid->hoverOn);
				}
			}
			newTigMsg.widgetId = widIdAtCursor;
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Entered;
			msgFuncs.Enqueue(&newTigMsg);
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
			auto button = uiReplacement.GetButton(widIdAtCursor2);
			if (button)
			{
				int buttonState = button->buttonState;
				if(buttonState >= 0)
				{
					if (buttonState == 1)
					{
						button->buttonState = 2;
						sound.MssPlaySound(button->sndDown);
					}
					else if (buttonState == 4)
					{
						return 0;
					}
				}
			}
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Clicked;
			newTigMsg.widgetId = widIdAtCursor2;
			*uiFuncs.uiMouseButtonId = widIdAtCursor2;
			msgFuncs.Enqueue(&newTigMsg);
		}
	}

	if ( (mouseMsg->flags & MouseStateFlags::MSF_LMB_RELEASED) && *uiFuncs.uiMouseButtonId != -1)
	{
		auto button = uiReplacement.GetButton(*uiFuncs.uiMouseButtonId);
		if (button)
		{
			switch (button->buttonState)
			{
			case 2:
				button->buttonState = 1;
				sound.MssPlaySound(button->sndClick);
				break;
			case 3:
				button->buttonState = 0;
				sound.MssPlaySound(button->sndClick);
				break;
			case 4:
				return 0;
			}
		}
		auto widIdAtCursor2 = GetAtInclChildren(mouseMsg->x, mouseMsg->y); // probably redundant to do again, but just to be safe...
		newTigMsg.widgetId = *uiFuncs.uiMouseButtonId;
		newTigMsg.widgetEventType = (widIdAtCursor2 != *uiFuncs.uiMouseButtonId)?TigMsgWidgetEvent::MouseReleasedAtDifferentButton : TigMsgWidgetEvent::MouseReleased;
		msgFuncs.Enqueue(&newTigMsg);
		*uiFuncs.uiMouseButtonId = -1;
	}

	return 0;
}

int Ui::WidgetSet(int widId, const Widget* widg)
{
	memcpy(activeWidgets[widId], widg, activeWidgets[widId]->size);
	return 0;
}

int Ui::WidgetCopy(int widId, Widget* widg)
{
	memcpy(widg, activeWidgets[widId], activeWidgets[widId]->size);
	return 0;
}


bool Ui::ScrollbarGetY(int widId, int * scrollbarY) {
	auto widget = (WidgetType3*) WidgetGet(widId);
	if (!widget || widget->type != 3)
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
	WidgetType3 widg;
	if (!ui.WidgetCopy(widId, &widg))
	{
		widg.yMax = yMax;
		WidgetSet(widId, &widg);
	}
}

BOOL Ui::ScrollbarSetY(int widId, int value){
	WidgetType3 scrollbarWid;
	auto result = WidgetCopy(widId, &scrollbarWid);
	if (!result)
	{
		scrollbarWid.scrollbarY = value;
		TigMsg msg;
		msg.createdMs = temple::GetRef<int>(0x11E74578);
		msg.type = TigMsgType::WIDGET;
		msg.arg2 = 5;
		msg.arg1 = scrollbarWid.parentId;
		msg.Enqueue();
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
#pragma region Loading and Unloading
UiLoader::UiLoader(const GameSystemConf& conf) {

	StopwatchReporter reporter("Loaded UI Systems in {}.");
	logger->info("Loading UI systems...");

	for (auto i = 0; i < UiSystemsCount; ++i) {
		auto& system = uiFuncs.systems[i];
		if (system.init) {
			logger->debug("   Initializing '{}'...", system.name);
			if (!system.init(conf)) {
				logger->error("Unable to initialize UI subsystem {}.", system.name);
				// Roll back initialization for all preceding systems
				while (--i > 0) {
					if (uiFuncs.systems[i].shutdown) {
						uiFuncs.systems[i].shutdown();
					}
				}
				throw TempleException(format("Unable to initialize UI subsystem {}.", system.name));
			}
		}
	}

}

UiLoader::~UiLoader() {

	logger->info("Shutting down UI systems...");

	for (auto i = UiSystemsCount - 1; i >= 0; --i) {
		auto& system = uiFuncs.systems[i];
		if (system.shutdown) {
			logger->debug("   Shutting down '{}'..", system.name);
			system.shutdown();
		}
	}

}

// NOTE: we take this argument to ensure that the UI has been loaded
UiModuleLoader::UiModuleLoader(const UiLoader&) {

	StopwatchReporter reporter("Loaded module for UI Systems in {}.");
	logger->info("Loading module for UI systems...");

	for (auto i = 0; i < UiSystemsCount; ++i) {
		auto& system = uiFuncs.systems[i];
		if (system.loadModule) {
			logger->debug("   Loading module for '{}'...", system.name);
			if (!system.loadModule()) {
				logger->error("Unable to load module for UI subsystem {}.", system.name);
				// Roll back initialization for all preceding systems
				while (--i > 0) {
					if (uiFuncs.systems[i].shutdown) {
						uiFuncs.systems[i].shutdown();
					}
				}
				throw TempleException(format("Unable to load module for UI subsystem {}.", system.name));
			}
		}
	}

	*uiFuncs.saveGameCallback = uiFuncs.GameSave;
	*uiFuncs.loadGameCallback = uiFuncs.GameLoad;
}

UiModuleLoader::~UiModuleLoader() {

	logger->info("Unloading modules for UI systems...");

	for (auto i = UiSystemsCount - 1; i >= 0; --i) {
		auto& system = uiFuncs.systems[i];
		if (system.unloadModule) {
			logger->debug("   Unloading module for '{}'..", system.name);
			system.unloadModule();
		}
	}

}
#pragma endregion
