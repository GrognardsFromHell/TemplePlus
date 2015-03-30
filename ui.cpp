#include "stdafx.h"
#include "ui.h"
#include "tig.h"

Ui ui;

#pragma region UI System Specification
/*
	System spec used to define an UI subsystem (i.e. dialogs and such).
*/
struct UiResizeArgs {
	int windowBufferStuffId;
	TigRect rect1;
	TigRect rect2;
};

typedef int(__cdecl *UiSystemInit)(const GameSystemConf *conf);
typedef void(__cdecl *UiSystemReset)();
typedef bool(__cdecl *UiSystemLoadModule)();
typedef void(__cdecl *UiSystemUnloadModule)();
typedef void(__cdecl *UiSystemShutdown)();
typedef bool(__cdecl *UiSystemSaveGame)(void *tioFile);
typedef bool(__cdecl *UiSystemLoadGame)(void *sth);
typedef bool(__cdecl *UiSystemResizeScreen)(const UiResizeArgs &resizeArgs);

struct UiSystemSpec {
	const char *name;
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
static struct UiFuncs : AddressTable {
	bool(__cdecl *Init)(const GameSystemConf *conf);
	bool(__cdecl *LoadModule)();
	void(__cdecl *UnloadModule)();
	void(__cdecl *Shutdown)();

	ImgFile *(__cdecl *LoadImg)(const char *filename);
	int(__cdecl *GetAsset)(UiAssetType assetType, uint32_t assetIndex, int &textureIdOut, int offset);

	ActiveWidgetListEntry* activeWidgetAllocList;
	Widget** activeWidgets;
	int *activeWidgetCount;

	UiSystemSpec *systems;

	UiFuncs() {
		rebase(Init, 0x101156F0);
		rebase(LoadModule, 0x10115790);
		rebase(LoadImg, 0x101E8320);
		rebase(UnloadModule, 0x101152C0);
		rebase(Shutdown, 0x10115230);
		rebase(GetAsset, 0x1004A360);

		rebase(activeWidgetAllocList, 0x10EF68DC);
		rebase(activeWidgets, 0x10EF68E0);
		rebase(activeWidgetCount, 0x10EF68D8);
		rebase(systems, 0x102F6C10);
	}
} uiFuncs;

bool Ui::GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int& textureIdOut) {
	return uiFuncs.GetAsset(assetType, static_cast<uint32_t>(assetIndex), textureIdOut, 0) == 0;
}

ImgFile* Ui::LoadImg(const char* filename) {
	return uiFuncs.LoadImg(filename);
}

UiLoader::UiLoader(const GameSystemConf &conf) {

	auto systemsLoaded = 0;
	
	for (int i = 0; i < UiSystemsCount; ++i) {
		
	}

}

UiLoader::~UiLoader() {
}

// NOTE: we take this argument to ensure that the UI has been loaded
UiModuleLoader::UiModuleLoader(const UiLoader&) {
}

UiModuleLoader::~UiModuleLoader() {
}
