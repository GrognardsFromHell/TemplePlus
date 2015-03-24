
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
#include "exception.h"
#include "stopwatch.h"

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

	StartupRelevantFuncs() {
		rebase(FindSound, 0x1003B9E0);
		rebase(TigInit, 0x101DF5A0);
		rebase(TigExit, 0x101DF3D0);
		rebase(SetScreenshotKeyhandler, 0x101DCB30);
		rebase(TigWindowBufferstuffCreate, 0x10113EB0);
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

// RAII for TIG initialization
class TigInitializer {
public:
	TigInitializer(HINSTANCE hInstance) : mConfig(createTigConfig(hInstance)) {
		StopwatchReporter reporter("TIG initialized in {}");
		logger->info("Initializing TIG");
		auto result = startupRelevantFuncs.TigInit(&mConfig);
		if (result) {
			throw TempleException(format("Unable to initialize TIG: {}", result));
		}
		tigConsoleDisabled = false; // tig init disables console by default
	}
	~TigInitializer() {
		logger->info("Shutting down TIG");
		startupRelevantFuncs.TigExit();
	}
	const TigConfig &config() const {
		return mConfig;
	}
private:
	TigConfig mConfig;
};

class TigBufferstuffInitializer {
public:
	TigBufferstuffInitializer() {
		StopwatchReporter reporter("Game scratch buffer initialized in {}");
		logger->info("Creating game scratch buffer");
		if (!startupRelevantFuncs.TigWindowBufferstuffCreate(&mBufferIdx)) {
			throw TempleException("Unable to initialize TIG buffer");
		}
	}
	~TigBufferstuffInitializer() {
		logger->info("Freeing game scratch buffer");
		startupRelevantFuncs.TigWindowBufferstuffFree(mBufferIdx);
	}
	int bufferIdx() const {
		return mBufferIdx;
	}
private:
	int mBufferIdx = -1;
};

class GameSystemsInitializer {
public:
	GameSystemsInitializer(const TigConfig &tigConfig) {
		StopwatchReporter reporter("Game systems initialized in {}");
		logger->info("Loading game systems");

		memset(&mConfig, 0, sizeof(mConfig));
		mConfig.width = tigConfig.width;
		mConfig.height = tigConfig.height;
		mConfig.field_10 = 0x10002530; // Callback 1
		mConfig.renderfunc = 0x10002650; // Callback 1
		mConfig.bufferstuffIdx = tigBuffer.bufferIdx();

		gameSystemFuncs.NewInit(mConfig);
		// if (!gameSystemFuncs.Init(&mConfig)) {
		//	throw TempleException("Unable to initialize game systems!");
		// }
	}
	~GameSystemsInitializer() {
		logger->info("Unloading game systems");
		gameSystemFuncs.Shutdown();
	}
	const GameSystemConf &config() const {
		return mConfig;
	}
private:
	GameSystemConf mConfig;
	TigBufferstuffInitializer tigBuffer;
};

class UiInitializer {
public:
	UiInitializer(const GameSystemConf &config) {
		StopwatchReporter reporter("UI initialized in {}");
		logger->info("Loading UI systems");
		if (!uiFuncs.Init(&config)) {
			throw TempleException("Unable to initialize the UI systems.");
		}
	}
	~UiInitializer() {
		logger->info("Unloading UI systems");
		uiFuncs.Shutdown();
	}
};

class GameSystemsModuleInitializer {
public:
	GameSystemsModuleInitializer(const string &moduleName) {
		StopwatchReporter reporter("Game module loaded in {}");
		logger->info("Loading game module {}", moduleName);
		if (!gameSystemFuncs.LoadModule(moduleName.c_str())) {
			throw TempleException(format("Unable to load game module {}", moduleName));
		}
	}
	~GameSystemsModuleInitializer() {
		logger->info("Unloading game module");
		gameSystemFuncs.UnloadModule();
	}
};

class UiModuleInitializer {
public:
	UiModuleInitializer() {
		StopwatchReporter reporter("Module specific UI loaded in {}");
		logger->info("Loading module specific UI.");
		if (!uiFuncs.LoadModule()) {
			throw TempleException("Unable to load module specific UI data.");
		}
	}
	~UiModuleInitializer() {
		logger->info("Unloading module specific UI.");
		uiFuncs.UnloadModule();
	}
};


int TempleMain(HINSTANCE hInstance, const wstring &commandLine) {
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
	bufferstuffWidth = tig.config().width;
	bufferstuffHeight = tig.config().height;
	
	// Hides the cursor during loading
	mouseFuncs.HideCursor();

	GameSystemsInitializer gameSystems(tig.config());

	/*
		Process options applicable after initialization of game systems
	*/
	applyGameConfig();

	if (!setDefaultCursor()) {
		return 1;
	}

	UiInitializer ui(gameSystems.config());

	GameSystemsModuleInitializer gameModule(config.defaultModule);

	// Notify the UI system that the module has been loaded
	UiModuleInitializer uiModule;
	
	if (!config.skipIntro) {
		movieFuncs.PlayMovie("movies\\introcinematic.bik", 0, 0, 0);
	}

	// Show the main menu
	mouseFuncs.ShowCursor();
	uiMainMenuFuncs.ShowPage(0); 
	temple_set<0x10BD3A68>(1); // Purpose unknown and unconfirmed, may be able to remove

	// Run console commands from "startup.txt" (working dir)
	logger->info("[Running Startup.txt]");
	startupRelevantFuncs.RunBatchFile("Startup.txt");
	logger->info("[Beginning Game]");
		
	startupRelevantFuncs.RunMainLoop();

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
