
#include "stdafx.h"
#include "temple_functions.h"
#include "tig/tig_msg.h"
#include "tig/tig_startup.h"
#include "tig/tig_mouse.h"
#include "gamesystems/gamesystems.h"
#include "tig/tig_shader.h"
#include "ui/ui.h"
#include "ui/ui_mainmenu.h"
#include "movies.h"
#include "python/pythonglobal.h"
#include "mainloop.h"
#include "config/config.h"
#include "updater/updater.h"
#include "ui/ui_systems.h"

class TempleMutex {
public:
	TempleMutex() : mMutex(nullptr) {
		
	}

	~TempleMutex() {
		if (mMutex) {
			ReleaseMutex(mMutex);
		}
	}

	bool acquire() {
		mMutex = CreateMutexA(nullptr, FALSE, "TempleofElementalEvilMutex");
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			MessageBoxA(nullptr, "Temple of Elemental Evil is already running.", "Temple of Elemental Evil", MB_OK | MB_ICONERROR);
			return false;
		}
		return true;
	}
private:
	HANDLE mMutex;
};

temple::GlobalPrimitive<bool, 0x10BDDD9C> noRandomEncounters;
temple::GlobalPrimitive<bool, 0x10300974> msMouseZEnabled;
temple::GlobalPrimitive<uint32_t, 0x102AF7C0> pathLimit;
temple::GlobalPrimitive<uint32_t, 0x102AF7C4> pathTimeLimit;
temple::GlobalPrimitive<uint32_t, 0x10BD3B6C> bufferstuffFlag;
temple::GlobalPrimitive<uint32_t, 0x102F6A7C> bufferstuffWidth;
temple::GlobalPrimitive<uint32_t, 0x102F6A80> bufferstuffHeight;
temple::GlobalPrimitive<uint32_t, 0x108EDA9C> activeRngType;
temple::GlobalPrimitive<bool, 0x108ED0D0> showDialogLineNo;
temple::GlobalPrimitive<uint32_t, 0x10307374> scrollDistance;
temple::GlobalPrimitive<uint32_t, 0x108254A0> mapFoggingInited;
temple::GlobalPrimitive<uint32_t, 0x102F7778> uiOptionsSupportedModeCount;
temple::GlobalPrimitive<uint32_t, 0x10BD3A48> startMap;

struct StartupRelevantFuncs : temple::AddressTable {
	

	int (__cdecl *SetScreenshotKeyhandler)(TigMsgGlobalKeyCallback *callback);
	void (__cdecl *RunBatchFile)(const char *filename);
	void (__cdecl *RunMainLoop)();
	int (__cdecl *TempleMain)(HINSTANCE hInstance, HINSTANCE hInstancePrev, const char *commandLine, int showCmd);

	/*
		This is here just for the editor
	*/
	int(__cdecl *MapOpenInGame)(int mapId, int a3, int a4);

	StartupRelevantFuncs() {
		rebase(SetScreenshotKeyhandler, 0x101DCB30);		
		rebase(RunBatchFile, 0x101DFF10);
		rebase(RunMainLoop, 0x100010F0);		
		rebase(TempleMain, 0x100013D0);
		
		rebase(MapOpenInGame, 0x10072A90);
	}

} startupRelevantFuncs;

static void applyGlobalConfig();
static void setMiles3dProvider();
static void addScreenshotHotkey();
static void applyGameConfig();
static bool SetDefaultCursor();

#pragma pack(push, 1)
struct ObjPropDef {
	uint32_t protoPropIdx;
	uint32_t field_4;
	uint32_t PropBitmap_idx1;
	uint32_t PropBitmask;
	uint32_t PropBitmap_idx2;
	uint32_t storedInPropColl;
	uint32_t FieldTypeCode;
};
#pragma pack(pop)

int TempleMain(HINSTANCE hInstance, const string &commandLine) {

	if (!config.engineEnhancements) {
		temple::GetRef<int>(0x10307284) = 800;
		temple::GetRef<int>(0x10307288) = 600;
		if (config.skipLegal) {
			temple::GetRef<int>(0x102AB360) = 0; // Disable legal movies
		}
		const char *cmdLine = GetCommandLineA();

		return startupRelevantFuncs.TempleMain(hInstance, nullptr, cmdLine, SW_NORMAL);
	}

	TempleMutex mutex;

	if (!mutex.acquire()) {
		return 1;
	}
	
	/*
		Write ToEE global config vars from our config
	*/
	applyGlobalConfig();


	TigInitializer tig(hInstance);
	
	setMiles3dProvider();
	addScreenshotHotkey();

	// It's pretty unclear what this is used for
	bufferstuffFlag = bufferstuffFlag | 0x40;
	bufferstuffWidth = tig.GetConfig().width;
	bufferstuffHeight = tig.GetConfig().height;
	
	// Hides the cursor during loading
	mouseFuncs.HideCursor();

	GameSystems gameSystems(tig);
	
	/*
		Process options applicable after initialization of game systems
	*/
	applyGameConfig();

	if (!SetDefaultCursor()) {
		logger->error("Unable to set the default cursor");
		return 1;
	}
		
	UiSystems uiSystems(config.renderWidth, config.renderHeight);

	gameSystems.LoadModule(config.defaultModule); // TODO: RAII

	// Python should now be initialized. Do the global hooks
	PythonGlobalExtension::installExtensions();

	// Notify the UI system that the module has been loaded
	UiModuleLoader uiModuleLoader(uiSystems);
	
	if (!config.skipIntro) {
		movieFuncs.PlayMovie("movies\\introcinematic.bik", 0, 0, 0);
	}

	uiSystems.ResizeViewport(config.renderWidth, config.renderHeight);

	// Show the main menu
	mouseFuncs.ShowCursor();
	if (!config.editor) {
		uiMainMenuFuncs.ShowPage(0);
	} else {
		startupRelevantFuncs.MapOpenInGame(5001, 0, 1);
	}
	temple::GetRef<int>(0x10BD3A68) = 1; // Purpose unknown and unconfirmed, may be able to remove

	// Run console commands from "startup.txt" (working dir)
	logger->info("[Running Startup.txt]");
	startupRelevantFuncs.RunBatchFile("Startup.txt");
	logger->info("[Beginning Game]");	

	Updater updater;

	GameLoop loop(tig, gameSystems, updater);
	loop.Run();
	// startupRelevantFuncs.RunMainLoop();

	return 0;
}

static void applyGlobalConfig() {
	if (config.noRandomEncounters) {
		noRandomEncounters = true;
	}
	if (config.noMsMouseZ) {
		msMouseZEnabled = false;
	}
	if (config.pathLimit) {
		pathLimit = config.pathLimit;
	}
	if (config.pathTimeLimit) {
		pathTimeLimit = config.pathTimeLimit;
	}
}

void setMiles3dProvider() {
	// TODO Investigate whether we should set a specific 3D sound provider that works best on modern systems (Windows 7+)
}

void addScreenshotHotkey() {
	TigMsgGlobalKeyCallback spec;
	spec.keycode = 0xB7; // This is a DINPUT key for the print key
	spec.callback = nullptr; // Would override the default thing (which we override elsewhere)
	startupRelevantFuncs.SetScreenshotKeyhandler(&spec);
}

void applyGameConfig() {
	// MT is the default
	switch (config.rngType) {
	case RngType::MERSENNE_TWISTER: 
		activeRngType = 0;
		break;
	case RngType::ARCANUM: 
		activeRngType = 1;
		break;
	default: 
		logger->error("Unknown RNG type specified!");
		break;
	}
	
	// TODO Dice test
	// TODO Default party support (mem location 0x10BDDD1C)
	// TODO Start map support (mem location 0x10BD3A48)
	// TODO Dialog check
	showDialogLineNo = config.showDialogLineNos;
	scrollDistance = config.scrollDistance;

	// Removes the initialized flag from the fog subsystem
	//if (config.disableFogOfWar) {
	if (!_stricmp( tolower(config.fogOfWar).c_str(), "unfogged")){
		mapFoggingInited = mapFoggingInited & ~1;
	}

	/*
		This is only used for displaying the mode-dropdown in fullscreen mode,
		which we currently do not use anymore. I don't know why the main method
		sets this. It specifies the number of modes in the supported mode table 
		@ 102F76B8.
	*/
	uiOptionsSupportedModeCount = 12;

}

bool SetDefaultCursor() {
	int cursorShaderId;
	auto result = shaderFuncs.GetId("art\\interface\\cursors\\MainCursor.mdf", &cursorShaderId);
	if (result) {
		logger->error("Unable to load cursor material: {}", result);
		return false;
	}

	result = mouseFuncs.SetCursor(cursorShaderId);
	if (result) {
		logger->error("Unable to set default cursor: {}", result);
		return false;
	}
	return true;
}
