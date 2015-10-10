#include "stdafx.h"
#include "ui.h"
#include "tig/tig.h"
#include <util/fixes.h>

Ui ui;




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

	ActiveWidgetListEntry* activeWidgetAllocList;
	Widget** activeWidgets;
	int* activeWidgetCount;

	// List of all UI subsystems (top level dialogs, basically)
	UiSystem* systems;

	// These are the original callbacks for game save/load
	SaveCallback GameLoad;
	SaveCallback GameSave;

	// Called to save UI related functions to the savegame in data2.sav
	SaveCallback* saveGameCallback;
	// Called to load UI related functions from the savegame in data2.sav
	SaveCallback* loadGameCallback;

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

		rebase(activeWidgetAllocList, 0x10EF68DC);
		rebase(activeWidgets, 0x10EF68E0);
		rebase(activeWidgetCount, 0x10EF68D8);
		rebase(systems, 0x102F6C10);
		rebase(saveGameCallback, 0x103072C4);
		rebase(loadGameCallback, 0x103072C8);

		rebase(WidgetBringToFront, 0x101F8E40);
		rebase(BindButton, 0x101F8950);
		rebase(AddWindow,  0x101F8FD0);
		rebase(AddButton,  0x101F9460);
		rebase(ButtonSetButtonState, 0x101F9510);
		rebase(WidgetAndWindowRemove, 0x101F9010);
		rebase(WidgetSetHidden, 0x101F9100);
		rebase(GetButtonState,  0x101F9740);
		
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
			for (int i = 0; i < parent->childrenCount;i++)
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
		removes widget, incluing removing it from its parent's children list
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

	static int WidgetlistIndexof(int widId, int* widlist, int size)
	{

		for (int i = 0; i < size; i++)
		{
			if (widlist[i] == widId)
				return i;
		}

		return -1;
	}

	const char* name() override
	{
		return "UiSys" "Function Replacements";
	}
	void apply() override
	{
		replaceFunction(0x1011DFE0, WidgetlistIndexof);
		replaceFunction(0x101F94D0, WidgetRemoveRegardParent);
		replaceFunction(0x101F90E0, WidgetGet);
		replaceFunction(0x101F9570, GetButton);
		
	}
} uiReplacement;

static UiSystem& getUiSystem(const char* name) {
	// Search for the ui system to replace
	for (auto i = 0; i < UiSystemsCount; ++i) {
		if (!strcmp(name, uiFuncs.systems[i].name)) {
			return uiFuncs.systems[i];
		}
	}

	throw TempleException(format("Couldn't find UI system {}! Replacement failed.", name));
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

BOOL Ui::BindButton(int parentId, int buttonId)
{
	return uiFuncs.BindButton(parentId, buttonId);
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

BOOL Ui::WidgetCopy(int widId, Widget* widgetOut)
{
	memcpy(widgetOut, uiFuncs.activeWidgets[widId], uiFuncs.activeWidgets[widId]->size);
	return 0;
}

BOOL Ui::GetButtonState(int widId, int* state)
{
	
	return uiFuncs.GetButtonState(widId, state);
}

void Ui::WidgetBringToFront(int widId)
{
	return uiFuncs.WidgetBringToFront(widId);
}

int Ui::WidgetlistIndexof(int widgetId, int* widgetlist, int size)
{
	return uiReplacement.WidgetlistIndexof(widgetId, widgetlist, size);
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
