
#include "stdafx.h"
#include "temple_functions.h"
#include "tig_msg.h"
#include "tig_startup.h"
#include "tig_mouse.h"
#include "gamesystems.h"
#include "config.h"
#include "graphics.h"
#include "tig_shader.h"
#include "ui.h"
#include "ui_mainmenu.h"
#include "movies.h"

#include <boost/algorithm/string.hpp>    

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

GlobalPrimitive<bool, 0x10D26DB4> tigConsoleDisabled; // TODO: move to tig_console.h
GlobalPrimitive<bool, 0x10BDDD9C> noRandomEncounters;
GlobalPrimitive<bool, 0x10300974> msMouseZEnabled;
GlobalPrimitive<uint32_t, 0x102AF7C0> pathLimit;
GlobalPrimitive<uint32_t, 0x102AF7C4> pathTimeLimit;
GlobalPrimitive<uint32_t, 0x10BD3B6C> bufferstuffFlag;
GlobalPrimitive<uint32_t, 0x102F6A7C> bufferstuffWidth;
GlobalPrimitive<uint32_t, 0x102F6A80> bufferstuffHeight;
GlobalPrimitive<uint32_t, 0x108EDA9C> activeRngType;
GlobalPrimitive<bool, 0x108ED0D0> showDialogLineNo;
GlobalPrimitive<uint32_t, 0x10307374> scrollDistance;
GlobalPrimitive<uint32_t, 0x108254A0> mapFoggingInited;
GlobalPrimitive<uint32_t, 0x102F7778> uiOptionsSupportedModeCount;

struct StartupRelevantFuncs : AddressTable {
	
	int (__cdecl *FindSound)(int soundId, char *filenameOut);
	_tig_init TigInit;
	void (__cdecl *TigExit)();
	int (__cdecl *SetScreenshotKeyhandler)(TigMsgGlobalKeyCallback *callback);
	bool (__cdecl *TigWindowBufferstuffCreate)(int *bufferStuffIdx);
	void (__cdecl *TigWindowBufferstuffFree)(int bufferStuffIdx);
	void(__cdecl *RunBatchFile)(const char *filename);
	void(__cdecl *RunMainLoop)();

	void rebase(Rebaser rebase) override {
		rebase(FindSound, 0x1003B9E0);
		rebase(TigInit, 0x101DF5A0);
		rebase(TigExit, 0x101DF3D0);
		rebase(SetScreenshotKeyhandler, 0x101DCB30);
		rebase(TigWindowBufferstuffCreate, 0x10113EB0);
		rebase(TigWindowBufferstuffFree, 0x101DF2C0);		
		rebase(TigWindowBufferstuffFree, 0x101DF2C0);
		rebase(RunBatchFile, 0x101DFF10);
		rebase(RunMainLoop, 0x100010F0);		
	}

} startupRelevantFuncs;

static void applyGlobalConfig();
static TigConfig createTigConfig(HINSTANCE hInstance);
static void setMiles3dProvider();
static void addScreenshotHotkey();
static void applyGameConfig();
static bool setDefaultCursor();

int TempleMain(HINSTANCE hInstance, const wstring &commandLine) {
	TempleMutex mutex;

	if (!mutex.acquire()) {
		return 1;
	}

	/*
		Write ToEE global config vars from our config
	*/
	applyGlobalConfig();

	/*
		Initialize TIG
	*/
	auto tigConfig = createTigConfig(hInstance);

	// TODO: RAII
	auto result = startupRelevantFuncs.TigInit(&tigConfig);
	if (result) {
		LOG(error) << "Unable to initialize TIG: " << result;
		return 1;
	}
	tigConsoleDisabled = false; // tig init disables console by default

	setMiles3dProvider();
	addScreenshotHotkey();

	// It's pretty unclear what this is used for
	bufferstuffFlag = bufferstuffFlag | 0x40;
	bufferstuffWidth = tigConfig.width;
	bufferstuffHeight = tigConfig.height;
	
	// Hides the cursor during loading
	mouseFuncs.HideCursor();

	/*
		Initialize Game Systems
	*/
	GameSystemConf gameConf;
	memset(&gameConf, 0, sizeof(gameConf));
	gameConf.width = tigConfig.width;
	gameConf.height = tigConfig.height;
	gameConf.field_10 = 0x10002530; // Callback 1
	gameConf.renderfunc = 0x10002650; // Callback 1

	// TODO: RAII
	if (!startupRelevantFuncs.TigWindowBufferstuffCreate(&gameConf.bufferstuffIdx)) {
		return 1;
	}

	if (!gameSystemFuncs.Init(&gameConf)) {
		LOG(error) << "Unable to initialize game systems.";
		return 1;
	}

	/*
		Process options applicable after initialization of game systems
	*/
	applyGameConfig();

	if (!setDefaultCursor()) {
		return 1;
	}

	// TODO: RAII
	if (!uiFuncs.Init(&gameConf)) {
		return 1;
	}

	// The name here was previously overridable via the cmdline but should use a different mechanism in the future
	// TODO: RAII
	if (!gameSystemFuncs.LoadModule("ToEE")) {
		LOG(error) << "Unable to load module ToEE";
		return 1;
	}

	// Notify the UI system that the module has been loaded
	// TODO: RAII
	if (!uiFuncs.LoadModule()) {
		LOG(error) << "Unable to load UI module data.";
		return 1;
	}

	if (!config.skipIntro) {
		movieFuncs.PlayMovie("movies\\introcinematic.bik", 0, 0, 0);
	}

	// Show the main menu
	mouseFuncs.ShowCursor();
	uiMainMenuFuncs.ShowPage(0); 
	temple_set<0x10BD3A68>(1); // Purpose unknown and unconfirmed, may be able to remove

	// Run console commands from "startup.txt" (working dir)
	LOG(info) << "[Running Startup.txt]";
	startupRelevantFuncs.RunBatchFile("Startup.txt");
	LOG(info) << "[Beginning Game]";
		
	startupRelevantFuncs.RunMainLoop();

	startupRelevantFuncs.TigWindowBufferstuffFree(gameConf.bufferstuffIdx);
	startupRelevantFuncs.TigExit();
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
		pathLimit = *config.pathLimit;
	}
	if (config.pathTimeLimit) {
		pathTimeLimit = *config.pathTimeLimit;
	}
}

static TigConfig createTigConfig(HINSTANCE hInstance) {
	TigConfig tigConfig;
	memset(&tigConfig, 0, sizeof(tigConfig));
	tigConfig.minTexWidth = 1024;
	tigConfig.minTexHeight = 1024;
	// From ToEE's pov we now always run windowed (TODO: Make this conditional on the graphics replacement)
	tigConfig.flags = SF_VSYNC | SF_WINDOW;
	// This should no longer be used since we completely replaced this part of tig
	// tigConfig.wndproc = (int)windowproc;
	tigConfig.framelimit = 100;
	tigConfig.width = 1024; // These should be read from a cfg file
	tigConfig.height = 768;
	tigConfig.bpp = 32;
	tigConfig.hinstance = hInstance;
	tigConfig.findSound = startupRelevantFuncs.FindSound;

	if (config.antialiasing) {
		tigConfig.flags |= SF_ANTIALIASING;
	}
	if (config.mipmapping) {
		tigConfig.flags |= SF_MIPMAPPING;
	}
	tigConfig.flags |= 0x1000; // Usage unknown
	tigConfig.soundSystem = "miles";
	// TODO: These two might no longer be needed due to us reworking the video system
	tigConfig.createBuffers = videoFuncs.GameCreateVideoBuffers;
	tigConfig.freeBuffers = videoFuncs.GameFreeVideoBuffers;

	if (config.showFps) {
		tigConfig.flags |= SF_FPS;
	}
	if (config.noSound) {
		tigConfig.flags |= SF_NOSOUND;
	}
	if (config.doublebuffer) {
		tigConfig.flags |= SF_DOUBLEBUFFER;
	}
	if (config.animCatchup) {
		tigConfig.flags |= SF_ANIMCATCHUP;
	}
	return tigConfig;
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
		LOG(error) << "Unknown RNG type specified!";
		break;
	}
	
	// TODO Dice test
	// TODO Default party support (mem location 0x10BDDD1C)
	// TODO Start map support (mem location 0x10BD3A48)
	// TODO Dialog check
	showDialogLineNo = config.showDialogLineNos;
	scrollDistance = config.scrollDistance;

	// Removes the initialized flag from the fog subsystem
	if (config.disableFogOfWar) {
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

bool setDefaultCursor() {
	int cursorShaderId;
	auto result = shaderFuncs.GetId("art\\interface\\cursors\\MainCursor.mdf", &cursorShaderId);
	if (result) {
		LOG(error) << "Unable to load cursor material: " << result;
		return false;
	}

	result = mouseFuncs.SetCursor(cursorShaderId);
	if (result) {
		LOG(error) << "Unable to set default cursor: " << result;
		return false;
	}
	return true;
}
