#include "stdafx.h"
#include "util/fixes.h"
#include "tio/tio_utils.h"
#include "ui/ui.h"
#include "gamesystem.h"
#include "python/python_integration_obj.h"

#include "gamesystems/gamesystems.h"
#include "tio/tio.h"
#include "gamesystems/legacy.h"
#include "util/savegame.h"
#include <infrastructure/vfs.h>
#include <mod_support.h>
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"

static struct GameLibLoadAddresses : temple::AddressTable {
	/*
		Signals to two other locations of the code, that a savegame is being loaded:
		- map_sector_mapclose
		- AI combat related
	*/
	int* isInLoad;
	
	void (__cdecl *UiMmRelated)(int arg);
	void (__cdecl *UiMmRelated2)(int arg);

	BOOL (__cdecl *UnpackArchive)(const char *filename, const char *destPath);

	BOOL (__cdecl *UiLoadGame)();

	GameLibLoadAddresses() {
		rebase(isInLoad, 0x103072D4);
		rebase(UiMmRelated, 0x10111A00);
		rebase(UiMmRelated2, 0x10111A40);
		rebase(UnpackArchive, 0x101E5A10);
		rebase(UiLoadGame, 0x101154B0);
	}
} addresses;

struct InLoadGame {
	InLoadGame() {
		*addresses.isInLoad = true;
	}
	~InLoadGame() {
		*addresses.isInLoad = false;
	}
};

bool GameSystems::LoadGame(const std::string& filename) {

	logger->debug("Loading savegame from {}", filename);

	InLoadGame inLoadGame; // Sets the global flag that indicates we're loading a save; destructor unsets it
	
	addresses.UiMmRelated(63);

	if (!vfs->DirExists("Save\\Current")) {
		logger->error("Could not find save folder Save\\Current");
		return false;
	}
	
	logger->info("begin removing files...");
	
	if (!vfs->CleanDir("Save\\Current")) {
		logger->error("Error clearing folder Save\\Current");
		return false;
	}

	logger->info("Restoring save archive...");
	
	auto path = fmt::format("save\\{}", filename);
	try {
		SaveGameArchive::Unpack(path.c_str(), "Save\\Current");
	} catch (const std::exception &e) {
		logger->error("Error restoring savegame archive {} to Save\\Current: {}", path, e.what());
		return false;
	}

	// this is for later
	// ::SaveGame saveGame;
	// saveGame.Load("Save\\Current");

	auto file = tio_fopen("Save\\Current\\data.sav", "rb");
	if (!file) {
		logger->error("gamelib_load(): Error reading data.sav\n");
		return false;
	}

	int saveVersion;
	if (tio_fread(&saveVersion, 4, 1, file) != 1) {
		logger->error("Error reading save version");
		tio_fclose(file);
		return false;
	}
	
	if (saveVersion != 0) {
		logger->error("Save game version mismatch error. Expected 0, read {}", saveVersion);
		tio_fclose(file);
		return false;
	}
	
	if (tio_fread(&mIronmanFlag, 4, 1, file) != 1) {
		logger->error("Unable to read ironman flag");
		tio_fclose(file);
		return false;
	}

	if (mIronmanFlag) {
		if (tio_fread(&mIronmanSaveNumber, 4, 1, file) != 1) {
			logger->error("Unable to read ironman save number.");
			tio_fclose(file);
			return false;
		}

		int savenameSize;
		if (tio_fread(&savenameSize, 4, 1, file) != 1) {
			logger->error("Unable to read ironman savename size");
			tio_fclose(file);
			return false;
		}

		// Other parts of toee may free this pointer so we have to use their free/alloc
		if (mIronmanSaveName) {
			free(mIronmanSaveName);
		}
		mIronmanSaveName = (char*) malloc(savenameSize);
		
		if (tio_fread(mIronmanSaveName, 1, savenameSize, file) != savenameSize) {
			logger->error("Unable to read ironman savegame name.");
			tio_fclose(file);
			return false;
		}
	}

	uint64_t startPos;
	tio_fgetpos(file, &startPos);

	logger->info("Starting position for game data: {}", startPos);

	addresses.UiMmRelated2(1);

	GameSystemSaveFile saveFile;
	saveFile.file = file;
	saveFile.saveVersion = saveVersion;
	
	int i = 0;
	for (auto system : mSaveGameAwareSystems) {

		logger->debug("Loading savegame for system {} ({})", system->GetName(), i);
		
		if (!system->LoadGame(&saveFile)) {
			logger->error("Loading save game data for game system {} ({}) failed", system->GetName(), i);
			tio_fclose(file);
			return false;
		}

		// Report on the current file position
		uint64_t afterPos;
		tio_fgetpos(file, &afterPos);
		logger->debug(" read {} bytes up to {}", afterPos - startPos, afterPos);
		startPos = afterPos;

		// Read the sentinel value
		uint32_t sentinel;
		if (tio_fread(&sentinel, 4, 1, file) != 1) {
			logger->error("Unable to read the sentinel value.");
			tio_fclose(file);
			return false;
		}

		if (sentinel != 0xBEEFCAFE) {
			logger->error("Invalid sentinel value read: 0x{:x}", sentinel);
			tio_fclose(file);
			return false;
		}

		addresses.UiMmRelated2(i + 1);
		++i;
	}
	tio_fclose(file);

	logger->info("Loading UI data from save game.");
	if (!addresses.UiLoadGame()) {
		logger->error("Loading UI data failed");
		return false;
	}

	{
		// user savehook
		auto saveHookArgs = Py_BuildValue("(s)", filename.c_str());
		pythonObjIntegration.ExecuteScript("templeplus.savehook", "load", saveHookArgs);
		Py_DECREF(saveHookArgs);
	}

	addresses.UiMmRelated2(63);
	
	logger->info("Completed loading of save game");
	
	uiSystems->GetParty().Update();
	
	if (temple::Dll::GetInstance().HasCo8Hooks()){
		// Co8 load hook
		auto loadHookArgs = Py_BuildValue("(s)", filename.c_str());
		pythonObjIntegration.ExecuteScript("templeplus.savehook", "post_load", loadHookArgs);
		Py_DECREF(loadHookArgs);

		if (modSupport.IsCo8NCEdition()){
			modSupport.SetNCGameFlag(true);
		} else
		{
			modSupport.SetNCGameFlag(false);
		}
	}

	return true;
}

static class GameLibLoadReplacement : TempleFix {
public:
	static bool __cdecl LoadGame(const char *filename) {
		return gameSystems->LoadGame(filename);
	}

	void apply() override {
		replaceFunction(0x100028D0, LoadGame);
	}
} fix;
