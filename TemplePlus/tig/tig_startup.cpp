#include "stdafx.h"

#include <temple/dll.h>

#include "mainwindow.h"
#include "graphics/graphics.h"
#include "graphics/legacyvideosystem.h"
#include "messages/messagequeue.h"

#include "tig_startup.h"
#include "../tio/tio.h"

#include "../util/config.h"

using TigStartupFunction = int(const TigConfig*);
using TigShutdownFunction = void();

/*
	These function addresses point to empty functions that
	simply do nothing.
*/
static constexpr uint32_t TigStartupNoop = 0x10262530;
static constexpr uint32_t TigShutdownNoop = 0x100027F0;

class LegacyTigSystem : public TigSystem {
public:

	LegacyTigSystem(const TigConfig* config,
	                const std::string& name,
	                uint32_t startupAddr,
	                uint32_t shutdownAddr) : mName(name), mShutdownAddr(shutdownAddr) {
		auto startup = reinterpret_cast<TigStartupFunction*>(
			temple::GetPointer(startupAddr)
		);
		if (startup(config) != 0) {
			throw TempleException("Unable to initialize TIG system {}", name);
		}
	}

	~LegacyTigSystem() {
		logger->info("Shutting down {}", mName);
		auto shutdown = reinterpret_cast<TigShutdownFunction*>(
			temple::GetPointer(mShutdownAddr)
		);
		shutdown();
	}

private:
	std::string mName;
	uint32_t mShutdownAddr;
};

static struct TigInternal : temple::AddressTable {
	bool* consoleDisabled; // TODO: move to tig_console.h

	int (__cdecl *FindSound)(int soundId, char* filenameOut);

	TigInternal() {
		rebase(consoleDisabled, 0x10D26DB4);
		rebase(FindSound, 0x1003B9E0);
	}
} tigInternal;

static TigConfig createTigConfig(HINSTANCE hInstance);

TigSystem::~TigSystem() {
}

TigInitializer::TigInitializer(HINSTANCE hInstance)
	: mConfig(createTigConfig(hInstance)) {

	StopwatchReporter reporter("TIG initialized in {}");
	logger->info("Initializing TIG");

	tio_path_add("tig.dat");
	tio_path_add(".");

	// No longer used: mStartedSystems.emplace_back(StartSystem("memory.c", 0x101E04F0, 0x101E0510));
	// No longer used: mStartedSystems.emplace_back(StartSystem("debug.c", 0x101E4DE0, TigShutdownNoop));
	mMainWindow = std::make_unique<MainWindow>(hInstance);
	mGraphics = std::make_unique<Graphics>(*mMainWindow);
	mStartedSystems.emplace_back(StartSystem("idxtable.c", 0x101EC400, 0x101ECAD0));
	mStartedSystems.emplace_back(StartSystem("trect.c", TigStartupNoop, 0x101E4E40));
	mStartedSystems.emplace_back(StartSystem("color.c", 0x101ECB20, 0x101ED070));

	mLegacyVideoSystem = std::make_unique<LegacyVideoSystem>(*mMainWindow, *mGraphics);

	// mStartedSystems.emplace_back(StartSystem("video.c", 0x101DC6E0, 0x101D8540));
	
	mStartedSystems.emplace_back(StartSystem("shader", 0x101E3350, 0x101E2090));
	mStartedSystems.emplace_back(StartSystem("palette.c", 0x101EBE30, 0x101EBEB0));
	mStartedSystems.emplace_back(StartSystem("window.c", 0x101DED20, 0x101DF320));
	mStartedSystems.emplace_back(StartSystem("timer.c", 0x101E34E0, 0x101E34F0));
	// mStartedSystems.emplace_back(StartSystem("dxinput.c", 0x101FF910, 0x101FF950));
	// mStartedSystems.emplace_back(StartSystem("keyboard.c", 0x101DE430, 0x101DE2D0));

	mStartedSystems.emplace_back(StartSystem("texture.c", 0x101EDF60, 0x101EE0A0));
	mStartedSystems.emplace_back(StartSystem("mouse.c", 0x101DDF50, 0x101DDE30));
	mStartedSystems.emplace_back(StartSystem("message.c", 0x101DE460, 0x101DE4E0));
	// mMessageQueue = std::make_unique<MessageQueue>();
	// startedSystems.emplace_back(StartSystem("gfx.c", TigStartupNoop, TigShutdownNoop));
	mStartedSystems.emplace_back(StartSystem("strparse.c", 0x101EBF00, TigShutdownNoop));
	mStartedSystems.emplace_back(StartSystem("filecache.c", TigStartupNoop, TigShutdownNoop));
	if (!config.noSound) {
		mStartedSystems.emplace_back(StartSystem("sound.c", 0x101E3FA0, 0x101E48A0));
	}
	mStartedSystems.emplace_back(StartSystem("movie.c", 0x101F1090, TigShutdownNoop));
	mStartedSystems.emplace_back(StartSystem("wft.c", 0x101F98A0, 0x101F9770));
	mStartedSystems.emplace_back(StartSystem("font.c", 0x101EAEC0, 0x101EA140));
	mStartedSystems.emplace_back(StartSystem("console.c", 0x101E0290, 0x101DFBC0));
	mStartedSystems.emplace_back(StartSystem("loadscreen.c", 0x101E8260, TigShutdownNoop));

	*tigInternal.consoleDisabled = false; // tig init disables console by default
}

TigInitializer::TigSystemPtr TigInitializer::StartSystem(const std::string& name,
                                                         uint32_t startAddr,
                                                         uint32_t shutdownAddr) {

	logger->info("Starting TIG subsystem {}", name);

	return std::make_unique<LegacyTigSystem>(&mConfig, name, startAddr, shutdownAddr);

}

TigInitializer::~TigInitializer() {

	StopwatchReporter reporter("TIG shut down in {}");
	logger->info("Shutting down TIG");

	for (auto it = mStartedSystems.rbegin();
	     it != mStartedSystems.rend();
	     ++it) {
		it->reset();
	}

}

static TigConfig createTigConfig(HINSTANCE hInstance) {
	TigConfig tigConfig;
	memset(&tigConfig, 0, sizeof(tigConfig));
	tigConfig.minTexWidth = 1024;
	tigConfig.minTexHeight = 1024;
	// From ToEE's pov we now always run windowed
	tigConfig.flags = SF_VSYNC | SF_WINDOW;
	// This should no longer be used since we completely replaced this part of tig
	// tigConfig.wndproc = (int)windowproc;
	tigConfig.framelimit = 100;
	// NOTE: These are the actual render sizes, we use a different size for the window / presentation
	tigConfig.width = config.renderWidth;
	tigConfig.height = config.renderHeight;
	tigConfig.bpp = 32;
	tigConfig.hinstance = hInstance;
	tigConfig.findSound = tigInternal.FindSound;

	if (config.antialiasing) {
		tigConfig.flags |= SF_ANTIALIASING;
	}
	if (config.mipmapping) {
		tigConfig.flags |= SF_MIPMAPPING;
	}
	tigConfig.flags |= 0x1000; // Usage unknown
	tigConfig.soundSystem = "miles";

	// No longer used
	// tigConfig.createBuffers = tigInternal.GameCreateVideoBuffers;
	// tigConfig.freeBuffers = tigInternal.GameFreeVideoBuffers;

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
