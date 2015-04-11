#include "stdafx.h"
#include "ui.h"
#include "tig/tig.h"
#include "util/stopwatch.h"

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

static struct UiFuncs : AddressTable {
	bool (__cdecl *Init)(const GameSystemConf* conf);
	bool (__cdecl *LoadModule)();
	void (__cdecl *UnloadModule)();
	void (__cdecl *Shutdown)();

	ImgFile*(__cdecl *LoadImg)(const char* filename);
	int (__cdecl *GetAsset)(UiAssetType assetType, uint32_t assetIndex, int& textureIdOut, int offset);

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

		rebase(GameLoad, 0x101154B0);
		rebase(GameSave, 0x101152F0);

		rebase(activeWidgetAllocList, 0x10EF68DC);
		rebase(activeWidgets, 0x10EF68E0);
		rebase(activeWidgetCount, 0x10EF68D8);
		rebase(systems, 0x102F6C10);
		rebase(saveGameCallback, 0x103072C4);
		rebase(loadGameCallback, 0x103072C8);
	}
} uiFuncs;

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
