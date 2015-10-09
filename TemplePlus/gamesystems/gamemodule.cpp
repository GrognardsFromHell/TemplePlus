
#include "stdafx.h"
#include "gamemodule.h"
#include "legacy.h"

GameModule::GameModule(const std::string& moduleName) {
	StopwatchReporter reporter("Game module loaded in {}");
	logger->info("Loading game module {}", moduleName);
	if (!gameSystemFuncs.LoadModule(moduleName.c_str())) {
		throw TempleException(format("Unable to load game module {}", moduleName));
	}
}

GameModule::~GameModule() {
	logger->info("Unloading game module");
	gameSystemFuncs.UnloadModule();
}
