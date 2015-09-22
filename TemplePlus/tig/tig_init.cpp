
#include "stdafx.h"
#include "tig_init.h"

#include "../util/config.h"
#include <temple/dll.h>

static struct TigInternal : temple::AddressTable {
	bool *consoleDisabled; // TODO: move to tig_console.h

	_tig_init TigInit;
	void(__cdecl *TigExit)();
	int(__cdecl *FindSound)(int soundId, char *filenameOut);
	
	TigInternal() {
		rebase(consoleDisabled, 0x10D26DB4);
		rebase(FindSound, 0x1003B9E0);
		rebase(TigInit, 0x101DF5A0);
		rebase(TigExit, 0x101DF3D0);
	}
} tigInternal;

static TigConfig createTigConfig(HINSTANCE hInstance);

TigInitializer::TigInitializer(HINSTANCE hInstance) : mConfig(createTigConfig(hInstance)) {
	StopwatchReporter reporter("TIG initialized in {}");
	logger->info("Initializing TIG");
	auto result = tigInternal.TigInit(&mConfig);
	if (result) {
		throw TempleException(format("Unable to initialize TIG: {}", result));
	}
	*tigInternal.consoleDisabled = false; // tig init disables console by default
}

TigInitializer::~TigInitializer() {
	logger->info("Shutting down TIG");
	tigInternal.TigExit();
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
