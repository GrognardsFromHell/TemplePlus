#include "stdafx.h"
#include "util/fixes.h"
#include "tio/tio_utils.h"
#include "gamesystems/gamesystems.h"
#include "ui/ui.h"
#include "gamesystems/timeevents.h"
#include "party.h"
#include "maps.h"
#include "critter.h"
#include "python/python_integration_obj.h"
#include <tio/tio.h>
#include "gamesystem.h"
#include "util/savegame.h"
#include <tig/tig_texture.h>
#include <config\config.h>

struct GsiData {
	string filename;
	string displayName;
	string moduleName;
	int leaderPortrait;
	int leaderLevel;
	locXY leaderLoc;
	GameTime time;
	string leaderName;
	int mapNumber;
	int storyState;
};

static struct GameLibSaveAddresses : temple::AddressTable {

	int* isInSave;

	void(__cdecl *UiMmRelated)(int arg);
	void(__cdecl *UiMmRelated2)(int arg);

	bool (__cdecl *PackArchive)(const char *filename, const char *srcPath);

	bool (__cdecl *UiSaveGame)();
	
	GameLibSaveAddresses() {
		rebase(isInSave, 0x103072D0);
		rebase(UiMmRelated, 0x10111A00);
		rebase(UiMmRelated2, 0x10111A40);
		rebase(PackArchive, 0x101E5F80);
		rebase(UiSaveGame, 0x101152F0);
	}
} addresses;

struct InSaveGame {
	InSaveGame() {
		*addresses.isInSave = true;
	}
	~InSaveGame() {
		*addresses.isInSave = false;
	}
};

static GsiData GatherGsiData(const string &filename, const string &displayName) {
	GsiData result;
	result.filename = filename;
	result.displayName = displayName;
	result.moduleName = config.defaultModule; //"ToEE";
	
	auto leader = party.GetLeader();
	result.leaderName = objects.GetDisplayName(leader, leader);
	result.mapNumber = maps.GetCurrentMapId();
	result.leaderPortrait = critterSys.GetPortraitId(leader);
	result.leaderLevel = critterSys.GetLevel(leader);
	result.leaderLoc = objects.GetLocation(leader);
	result.storyState = party.GetStoryState();
	result.time = gameSystems->GetTimeEvent().GetTime();

	return result;
}

static bool SaveGsiData(const GsiData &saveInfo) {

	// Remove existing GSI for the slots
	string globPattern = fmt::format("save\\{}*.gsi", saveInfo.filename);
	TioFileList list;
	tio_filelist_create(&list, globPattern.c_str());
	for (int i = 0; i < list.count; ++i) {
		auto filename = fmt::format("save\\{}", list.files[i].name);
		tio_remove(filename.c_str());
	}
	tio_filelist_destroy(&list);

	auto filename = fmt::format("save\\{}{}.gsi", saveInfo.filename, saveInfo.displayName);
	auto file = tio_fopen(filename.c_str(), "wb");	
	if (!file) {
		logger->error("Unable to open {} for writing.", filename);
		return false;
	}

	int version = 0;
	if (tio_fwrite(&version, 4, 1, file) != 1) {
		logger->error("Unable to write version info for GSI.");
		tio_fclose(file);
		return false;
	}

	bool success = true;

	int stringLength = saveInfo.moduleName.length() + 1;
	success &= tio_fwrite(&stringLength, 4, 1, file) == 1;
	success &= tio_fwrite(saveInfo.moduleName.c_str(), stringLength, 1, file) == 1;

	stringLength = saveInfo.leaderName.length() + 1;
	success &= tio_fwrite(&stringLength, 4, 1, file) == 1;
	success &= tio_fwrite(saveInfo.leaderName.c_str(), stringLength, 1, file) == 1;

	success &= tio_fwrite(&saveInfo.mapNumber, 4, 1, file) == 1;
	success &= tio_fwrite(&saveInfo.time, 8, 1, file) == 1;
	success &= tio_fwrite(&saveInfo.leaderPortrait, 4, 1, file) == 1;
	success &= tio_fwrite(&saveInfo.leaderLevel, 4, 1, file) == 1;
	success &= tio_fwrite(&saveInfo.leaderLoc, 8, 1, file) == 1;
	success &= tio_fwrite(&saveInfo.storyState, 4, 1, file) == 1;

	stringLength = saveInfo.displayName.length() + 1;
	success &= tio_fwrite(&stringLength, 4, 1, file) == 1;
	success &= tio_fwrite(saveInfo.displayName.c_str(), stringLength, 1, file) == 1;

	tio_fclose(file);
	return success;
}

bool GameSystems::SaveGame(const string& filename, const string& displayName) {

	InSaveGame inSave;
	addresses.UiMmRelated(63);

	if (!TioDirExists("Save\\Current")) {
		return false;
	}

	auto fullPath = "Save\\Current\\data.sav"; // Full path
	auto file = tio_fopen(fullPath, "wb");

	if (!file) {
		logger->error("Error creating: Save\\Current\\data.sav");
		return false;
	}

	int saveVersion = 0;
	if (tio_fwrite(&saveVersion, 4, 1, file) != 1) {
		logger->error("Failed to write save version.");
		tio_fclose(file);
		tio_remove(fullPath);
		return false;
	}

	if (tio_fwrite(&mIronmanFlag, 4, 1, file) != 1) {
		logger->error("Failed to write ironman mode flag.");
		tio_fclose(file);
		tio_remove(fullPath);
		return false;
	}

	if (mIronmanFlag) {
		if (tio_fwrite(&mIronmanSaveNumber, 4, 1, file) != 1) {
			logger->error("Failed to write ironman save number");
			tio_fclose(file);
			tio_remove(fullPath);
			return false;
		}

		auto savenameLen = strlen(mIronmanSaveName) + 1;
		if (tio_fwrite(&savenameLen, 4, 1, file) != 1) {
			logger->error("Failed to write ironman save name length");
			tio_fclose(file);
			tio_remove(fullPath);
			return false;
		}
		
		if (tio_fwrite(mIronmanSaveName, 1, savenameLen, file) != savenameLen) {
			logger->error("Failed to write ironman save name");
			tio_fclose(file);
			tio_remove(fullPath);
			return false;
		}
	}

	uint64_t startOfData;
	tio_fgetpos(file, &startOfData);

	int i = 0;
	for (auto system : mSaveGameAwareSystems) {
		addresses.UiMmRelated2(i + 1);
		i++;

		if (!system->SaveGame(file)) {
			logger->error("Save function for game system {} failed.", system->GetName());
			tio_fclose(file);
			tio_remove(fullPath);
			return false;
		}

		uint32_t sentinel = 0xBEEFCAFE;
		if (tio_fwrite(&sentinel, 4, 1, file) != 1) {
			tio_fclose(file);
			tio_remove(fullPath);
			return false;
		}

		uint64_t endOfData;
		tio_fgetpos(file, &endOfData);
		logger->info(" wrote to: {}, Total: ({})", endOfData, endOfData - startOfData);
		startOfData = endOfData;
	}

	tio_fclose(file);

	{
		// user savehook
		auto saveHookArgs = Py_BuildValue("(s)", filename.c_str());
		pythonObjIntegration.ExecuteScript("templeplus.savehook", "save", saveHookArgs);
		Py_DECREF(saveHookArgs);
	}


	if (!addresses.UiSaveGame()) {
		tio_remove(fullPath);
		return false;
	}

	logger->info("UI savegame created");

	GsiData gsiData = GatherGsiData(filename, displayName);
	if (!SaveGsiData(gsiData)) {
		logger->error("Unable to save GSI file.");
		return false;
	}

	// Create savegame archive
	auto archiveName = fmt::format("save\\{}", filename);

	try {
		SaveGameArchive::Create("Save\\Current", archiveName);
	} catch (const std::exception &e) {
		logger->error("Unable to create save game archive: {}", e.what());
		return false;
	}

	addresses.UiMmRelated2(63);

	logger->info("Game save created successfully.");

	// Rename screenshots
	auto screenDestName = fmt::format("save\\{}l.jpg", filename);
	int textId;
	if (tio_fileexists(screenDestName.c_str())) {
		tio_remove(screenDestName.c_str());
	}
	tio_rename("save\\templ.jpg", screenDestName.c_str());
	textureFuncs.RegisterTextureOverride(screenDestName.c_str(), &textId); // override previous texture if any

	screenDestName = fmt::format("save\\{}s.jpg", filename);
	if (tio_fileexists(screenDestName.c_str())) {
		tio_remove(screenDestName.c_str());
	}
	tio_rename("save\\temps.jpg", screenDestName.c_str());
	textureFuncs.RegisterTextureOverride(screenDestName.c_str(), &textId);

	// co8 savehook
	auto saveHookArgs = Py_BuildValue("(s)", filename.c_str());
	pythonObjIntegration.ExecuteScript("templeplus.savehook", "post_save", saveHookArgs);
	Py_DECREF(saveHookArgs);

	return true;
}

bool GameSystems::SaveGameIronman()
{
	if (mIronmanFlag && mIronmanSaveName) {
		auto filename = fmt::format("iron{:04d}", mIronmanSaveNumber);
		return SaveGame(filename, mIronmanSaveName);
	}
	return false;
}

static class GameLibSaveReplacement : TempleFix {
public:
	static BOOL SaveGame(const char *filename, const char *displayName) {
		return gameSystems->SaveGame(filename, displayName) ? TRUE : FALSE;
	}

	void apply() override {
		replaceFunction(0x100042C0, SaveGame);

		// gamelib_ironman_save
		replaceFunction<int()>(0x10004870, []() {
			return gameSystems->SaveGameIronman() ? TRUE : FALSE;
		});
	}
} fix;
