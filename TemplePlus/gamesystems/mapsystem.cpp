
#include "stdafx.h"
#include "mapsystem.h"

#include "config/config.h"
#include "util/streams.h"

#include <infrastructure/mesparser.h>
#include <infrastructure/vfs.h>
#include <infrastructure/stringutil.h>
#include <infrastructure/binaryreader.h>

#include <temple/dll.h>
#include <temple/meshes.h>

#include <tio/tio.h>

#include "animgoals/anim.h"
#include "map/sector.h"
#include "obj.h"
#include "gamesystems/legacysystems.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/map/gmesh.h"
#include "tig/tig_startup.h"
#include "temple/soundsystem.h"

#include "gamesystem.h"
#include "gamesystems.h"
#include "legacymapsystems.h"

#include "particlesystems.h"

#include "clipping/clipping.h"
#include "graphics/mapterrain.h"
#include "objects/objevent.h"
#include "python/python_debug.h"
#include "gamesystems/objects/objsystem.h"

struct MapListEntry {
	int id;
	int flags = 0;
	std::string name;
	std::string description;
	int worldmap = 0;
	int area = 0;
	int movie = 0;
	int startPosX = 0;
	int startPosY = 0;
	MapType type = MapType::None;
	
	bool IsOutdoors() const {
		return (flags & 2) == 2;
	}
	bool IsBedrest() const {
		return (flags & 8) == 8;
	}
	bool IsUnfogged() const {
		return (flags & 4) == 4;
	}
};

static struct MapSystemAddresses : public temple::AddressTable {
public:
	BOOL (*FirstNonProtoHandle)(objHndl *obj, int *it);
	BOOL (*NextNonProtoHandle)(objHndl *obj, int *it);
	BOOL (*call_ui_tutorial_isactive)();

	using UiTipCallback = void(int);
	void (*ui_show_tip)(
		const char *title,
		const char *text,
		const char *okBtnLabel,
		const char *nextTipLabel,
		const char *showTipsLabel,
		BOOL *showTips,
		UiTipCallback *closeCallback
	);

	MapSystemAddresses() {
		rebase(FirstNonProtoHandle, 0x1009CC10);
		rebase(NextNonProtoHandle, 0x1009CCB0);
		rebase(call_ui_tutorial_isactive, 0x1009AB80);
		rebase(ui_show_tip, 0x1009AC40);
	}

} addresses;

MapSystem::MapSystem(TigInitializer &tig, const GameSystemConf &config, D20System &d20System, PartySystem &partySystem)
	: mTig(tig), mD20System(d20System), mPartySystem(partySystem) {

	LoadTips();
}

MapSystem::~MapSystem() {
}

void MapSystem::LoadModule() {
	auto mapList = MesFile::ParseFile("Rules\\MapList.mes");
	auto mapNames = MesFile::ParseFile("mes\\map_names.mes");

	std::vector<gsl::cstring_span<>> parts;

	bool alwaysFog = false;
	bool alwaysUnfog = false;
	if (!_stricmp(tolower(config.fogOfWar).c_str(), "always")) {
		alwaysFog = true;
	}
	else if (!_stricmp(tolower(config.fogOfWar).c_str(), "unfogged")) {
		alwaysUnfog = true;
	}
	

	for (auto &line : mapList) {
		MapListEntry entry;

		parts.clear();
		split(line.second, ',', parts, true);

		entry.id = line.first;
		entry.name.assign(parts[0].begin(), parts[0].end());
		entry.startPosX = atoi(parts[1].data());
		entry.startPosY = atoi(parts[2].data());
		entry.flags = 0;
		if (alwaysUnfog)
			entry.flags |= 4;

		

		// The rest are key value pairs
		for (size_t i = 3; i < parts.size(); ++i) {			
			auto subParts = split(parts[i], ':', true);
			auto key = subParts[0];
			auto value = subParts[1];

			if (!strncmp(key.data(), "Type", key.size())) {
				if (!strncmp(value.data(), "NONE", value.size())) {
					entry.type = MapType::None;
				} else if (!strncmp(value.data(), "START_MAP", value.size())) {
					entry.type = MapType::StartMap;
				} else if (!strncmp(value.data(), "SHOPPING_MAP", value.size())) {
					entry.type = MapType::ShoppingMap;
				} else if (!strncmp(value.data(), "TUTORIAL_MAP", value.size())) {
					entry.type = MapType::TutorialMap;
				}
			} else if (!strncmp(key.data(), "WorldMap", key.size())) {
				entry.worldmap = atoi(value.data());
			} else if (!strncmp(key.data(), "Area", key.size())) {
				entry.area = atoi(value.data());
			} else if (!strncmp(key.data(), "Movie", key.size())) {
				entry.movie = atoi(value.data());
			} else if (!strncmp(key.data(), "Flag", key.size())) {
				if (!strncmp(value.data(), "DAYNIGHT_XFER", value.size())) {
					entry.flags |= 1;
				} else if (!strncmp(value.data(), "OUTDOOR", value.size())) {
					entry.flags |= 2;
				} else if (!strncmp(value.data(), "UNFOGGED", value.size())) {
					if (!alwaysFog || entry.type == MapType::ShoppingMap)
						entry.flags |= 4;
				} else if (!strncmp(value.data(), "BEDREST", value.size())) {
					entry.flags |= 8;
				}
			}
			
		}
		mMaps[line.first] = entry;
	}

	for (auto &line : mapNames) {
		mMaps[line.first].description.assign( line.second  );
	}

	// get info from hardcoded map areas table (bleh)
	for (auto &entry: mMaps) {
		if (!entry.second.area) {
			auto mapArea = temple::GetRef<int(__cdecl)(int)>(0x1006EC30)(entry.second.id);
			entry.second.area = mapArea;
		}
	}

	
	if (config.debugObjects) {
		// Search for duplicate mob files
		std::map<std::string, int> objMapId;
		for (auto& it: mMaps) {
			auto &entry = it.second;
			auto dataDir = fmt::format("maps\\{}", entry.name);
			auto mobFiles = vfs->Search(dataDir + "\\*.mob");

			for (auto& mobFile : mobFiles) {
				auto found = objMapId.find(mobFile.filename);
				if (found != objMapId.end()) {
					

					auto file = tio_fopen( fmt::format("{}/{}", dataDir, mobFile.filename).c_str() , "rb");

					uint32_t header;
					if (tio_fread(&header, sizeof(header), 1, file) != 1) {
						throw TempleException("ObjSystem::LoadFromFile: Couldn't read the object header.");
					}

					if (header != 0x77) {
						throw TempleException("Expected object header 0x77, but got 0x{:x}", header);
					}

					ObjectId protoId;
					if (tio_fread(&protoId, sizeof(protoId), 1, file) != 1) {
						throw TempleException("Couldn't read the prototype id.");
					}

					logger->error("Duplicates MOB file found: {}/{} (was also on mapID = {}, now on {}). ProtoID: {}", dataDir, mobFile.filename, found->second, entry.id, protoId.GetPrototypeId());
					
					tio_fclose(file);
				}
				else {
					objMapId[mobFile.filename] = entry.id;
				}
			}

		}

	}

	RegisterDebugFunction("check_mobs", 
		[&]() {
			std::unordered_map<std::string, int> mobObjMapId;
			std::unordered_map<std::string, int> dynObjMapId;
			auto DEBUG_GUID_STR = "G_2524B3E6_4991_4A73_A73E_E2338AF4BA3C"; // guid to look for...
			ObjectId debugGuid;
			objHndl debugGuidHandle = objHndl::null;
			auto readObject = temple::GetPointer<BOOL(objHndl*, const char*)>(0x100DE690);

			// first, read all MOB files to find the "originals" (where applicable)
			for (auto& it : mMaps) {
				auto& entry = it.second;
				auto mapId = entry.id;
				auto dataDir = fmt::format("maps\\{}", entry.name);

				// Iterate all MOB files
				auto mobFiles = vfs->Search(dataDir + "\\*.mob");
				for (auto& mobFile : mobFiles) {
					auto file = tio_fopen(fmt::format("{}/{}", dataDir, mobFile.filename).c_str(), "rb");

					uint32_t header;
					if (tio_fread(&header, sizeof(header), 1, file) != 1) {
						throw TempleException("ObjSystem::LoadFromFile: Couldn't read the object header.");
					}

					if (header != 0x77) {
						throw TempleException("Expected object header 0x77, but got 0x{:x}", header);
					}

					ObjectId protoId;
					if (tio_fread(&protoId, sizeof(protoId), 1, file) != 1) {
						throw TempleException("Couldn't read the prototype id.");
					}
					if (!protoId.IsPrototype()) {
						throw TempleException("Expected a prototype id, but got type {} instead.", (int)protoId.subtype);
					}
					ObjectId objId;
					if (tio_fread(&objId, sizeof(objId), 1, file) != 1) {
						throw TempleException("Couldn't read the object id.");
			}
					tio_fclose(file);

					// check that the object GUID matches the filename
					auto objIdString = objId.ToString();
					if (nullptr == std::strstr(mobFile.filename.c_str(), objIdString.c_str())) {
						logger->error("Mismatch between MOB filename and GUID content! Filename: {} GUID: {}", mobFile.filename, objId);
					}

					// check for dupes
					auto found = mobObjMapId.find(objIdString);
					if (found != mobObjMapId.end()) {
						logger->error("Duplicates MOB file found: {}/{} (was also on mapID = {}, now on {}). ProtoID: {}", dataDir, mobFile.filename, found->second, entry.id, protoId.GetPrototypeId());
					}
					else {
						mobObjMapId[objIdString] = entry.id;
					}

					if (nullptr != std::strstr(objIdString.c_str(), DEBUG_GUID_STR)) {
						debugGuid = objId;
					}
				}

			}


			const int MAX_MAP_ID = 5135;
			

			for (auto& it : mMaps) {

				auto& entry = it.second;
				auto mapId = entry.id;
				auto saveDir = fmt::format("Save\\Current\\maps\\{}", entry.name);
				auto dataDir = fmt::format("maps\\{}", entry.name);

				

				if (mapId >= MAX_MAP_ID) {
					continue;
		}
		
				OpenMap(mapId, false, true);

				// debug:
				"G_2524B3E6_4991_4A73_A73E_E2338AF4BA3C";
				auto debugHandle = objSystem->GetHandleById(debugGuid);
				if (debugHandle) {
					debugGuidHandle = debugHandle;
				}
				logger->debug("*** Finished processing map {}; Debug Handle: {}; debugGuidHandle valid: {}", mapId, debugHandle, debugGuidHandle && objSystem->IsValidHandle(debugGuidHandle));
	}

			
		});
}

void MapSystem::UnloadModule() {
	mMaps.clear();
}

void MapSystem::Reset() {
	ResetFleeTo();

	logger->info("Resetting map");
	
	gameSystems->GetAnim().InterruptAll();
	
	CloseMap();
	mCurrentMap = nullptr; // TODO: Redundant?

	mVisitedMaps.clear();
}

#pragma pack(push, 1)
struct MapFleeSaveData {
	int mapId;
	int padding1;
	LocAndOffsets loc;
	int enterX;
	int enterY;
	BOOL isFleeing;
	int padding2;
};
#pragma pack(pop)

template<typename T>
static void SaveIdxTable(const std::map<int32_t, T> &values, TioFile* file) {
	const uint32_t header = 0xAB1EE1BA;
	if (tio_fwrite(&header, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to write the idx table header");
	}

	uint32_t bucketCount = 1;
	if (tio_fwrite(&bucketCount, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to write the idx table capacity");
	}
	
	const uint32_t dataSize = sizeof(T);
	if (tio_fwrite(&dataSize, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to write the idx table item size");
	}

	// ToEE seems to distinguish between capacity and size, but we don't
	size_t nodeCount = values.size();
	if (tio_fwrite(&nodeCount, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to write the idx table size");
	}

	for (auto &entry : values) {
		if (tio_fwrite(&entry.first, sizeof(int32_t), 1, file) != 1) {
			throw TempleException("Unable to write key for idx table entry.");
		}
		if (tio_fwrite(&entry.second, sizeof(T), 1, file) != 1) {
			throw TempleException("Unable to write value for idx table entry.");
		}
	}

	const uint32_t footer = 0xE1BAAB1E;
	if (tio_fwrite(&footer, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to write the idx table footer");
	}

}

template<typename T>
static map<int32_t,T> LoadIdxTable(TioFile* file) {
	uint32_t magicNumber;
	if (tio_fread(&magicNumber, sizeof(uint32_t), 1, file) != 1
		|| magicNumber != 0xAB1EE1BA) {
		throw TempleException("Unable to read the idx table header");
	}

	uint32_t bucketCount;
	if (tio_fread(&bucketCount, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to read the idx table capacity");
	}

	uint32_t dataSize;
	if (tio_fread(&dataSize, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to write the idx table item size");
	}
	
	std::map<int32_t, T> result;
	for (size_t bucket = 0; bucket < bucketCount; ++bucket) {
		size_t nodeCount;
		if (tio_fread(&nodeCount, sizeof(uint32_t), 1, file) != 1) {
			throw TempleException("Unable to write the idx table size");
		}

		for (size_t i = 0; i < nodeCount; ++i) {
			int32_t key;
			if (tio_fread(&key, sizeof(int32_t), 1, file) != 1) {
				throw TempleException("Unable to read key for idx table entry.");
			}
			T value;
			if (tio_fread(&value, sizeof(T), 1, file) != 1) {
				throw TempleException("Unable to read value for idx table entry.");
			}
			result[key] = value;
		}
	}

	if (tio_fread(&magicNumber, sizeof(uint32_t), 1, file) != 1
		|| magicNumber != 0xE1BAAB1E) {
		throw TempleException("Unable to read the idx table footer");
	}

	return result;

}

bool MapSystem::SaveGame(TioFile *file) {
	
	FlushMap(1);

	if (tio_fputs(mSectorDataDir.c_str(), file) == -1) {
		logger->error("Couldn't write sector data directory.");
		return false;		
	}
	if (tio_fputc('\n', file) == -1) {
		logger->error("Couldn't write sector directory separator.");
		return false;
	}
	if (tio_fputs(mSectorSaveDir.c_str(), file) == -1) {
		logger->error("Couldn't write sector save directory.");
		return false;
	}
	if (tio_fputc('\n', file) == -1) {
		logger->error("Couldn't write sector directory separator.");
		return false;
	}

	// Save visited maps
	std::map<int32_t, int32_t> visitedMaps;
	for (auto mapId : mVisitedMaps) {
		visitedMaps[mapId] = 1;
	}
	SaveIdxTable(visitedMaps, file);
	
	// Previously, the save functions were called here, but no map subsystem
	// actually had any save callbacks registered

	// Flush map will clear the dispatchers of the entire party
	// and map load postprocess will not restore them, so we
	// have to do that here
	mPartySystem.ForEachInParty([&](objHndl handle) {
		d20Sys.d20Status->D20StatusInit(handle);
	});

	// Save Flee data
	MapFleeSaveData fleeData;
	fleeData.mapId = mFleeInfo.mapId;
	fleeData.loc = mFleeInfo.location;
	fleeData.enterX = mFleeInfo.enterLocation.locx;
	fleeData.enterY = mFleeInfo.enterLocation.locy;
	fleeData.isFleeing = mFleeInfo.isFleeing ? TRUE : FALSE;

	auto mapFleeFh = vfs->Open("Save\\Current\\map_mapflee.bin", "wb");
	vfs->Write(&fleeData, sizeof(fleeData), mapFleeFh);
	vfs->Close(mapFleeFh);

	// Why does it call spell_save here directly? ... Who knows!
	gameSystems->GetSpell().Save(file);

	return true;

}

bool MapSystem::LoadGame(GameSystemSaveFile* saveFile) {
	// originally 10072C40

	char filename[260];
	if (!tio_fgets(filename, 260, saveFile->file)) {
		logger->error("Unable to load current map data directory");
		return false;
	}
	auto len = strnlen(filename, 259);
	if (len > 0 && filename[len - 1] == '\n') {
		filename[len - 1] = '\0';
	} else {
		logger->error("Map data directory not correctly terminated.");
		return false;
	}

	// Find the map entry corresponding to the map directory
	auto lastBackslash = strrchr(filename, '\\');
	const char* mapName;
	if (lastBackslash)
		mapName = lastBackslash + 1;
	else
		mapName = filename;
	
	MapListEntry *mapEntry = nullptr;
	for (auto& entry : mMaps) {
		if (!_stricmp(entry.second.name.c_str(), mapName)) {
			mapEntry = &entry.second;
			break;
		}
	}
	if (!mapEntry) {
		logger->error("No map was found that matches {}", filename);
		return false;
	}

	logger->info("Loading map id {}, name {}", mapEntry->id, mapEntry->name);

	if (!tio_fgets(filename, 260, saveFile->file)) {
		logger->error("Unable to load current map data directory");
		return false;
	}
	len = strnlen(filename, 259);
	if (len > 0 && filename[len - 1] == '\n') {
		filename[len - 1] = '\0';
	} else {
		logger->error("Map save directory not correctly terminated.");
		return false;
	}

	// NOTE: save dir is ignored
	
	auto visitedMaps = LoadIdxTable<int32_t>(saveFile->file);

	if (vfs->FileExists("Save\\Current\\map_mapflee.bin")) {
		auto mapFleeFh = vfs->Open("Save\\Current\\map_mapflee.bin", "rb");
		MapFleeSaveData fleeData;
		vfs->Read(&fleeData, sizeof(fleeData), mapFleeFh);
		mFleeInfo.mapId = fleeData.mapId;
		mFleeInfo.isFleeing = fleeData.isFleeing != 0;
		mFleeInfo.enterLocation.locx = fleeData.enterX;
		mFleeInfo.enterLocation.locy = fleeData.enterY;
		mFleeInfo.location = fleeData.loc;
		vfs->Close(mapFleeFh);
	}
	
	OpenMap(mapEntry);

	mVisitedMaps.clear();
	for (auto &entry : visitedMaps) {
		mVisitedMaps.insert(entry.first);
	}

	return gameSystems->GetSpell().Load(saveFile);

}

const std::string &MapSystem::GetName() const {
	static std::string name("Map");
	return name;
}

bool MapSystem::PseudoLoad(GameSystemSaveFile * saveFile, std::string saveFolder, std::vector<objHndl> & dynHandles)
{
	char filename[260];
	if (!tio_fgets(filename, 260, saveFile->file)) {
		logger->error("Unable to load current map data directory");
		return false;
	}
	auto len = strnlen(filename, 259);
	if (len > 0 && filename[len - 1] == '\n') {
		filename[len - 1] = '\0';
	}
	else {
		logger->error("Map data directory not correctly terminated.");
		return false;
	}

	// Find the map entry corresponding to the map directory
	auto lastBackslash = strrchr(filename, '\\');
	const char* mapName;
	if (lastBackslash)
		mapName = lastBackslash + 1;
	else
		mapName = filename;

	MapListEntry *mapEntry = nullptr;
	for (auto& entry : mMaps) {
		if (!_stricmp(entry.second.name.c_str(), mapName)) {
			mapEntry = &entry.second;
			break;
		}
	}
	if (!mapEntry) {
		logger->error("No map was found that matches {}", filename);
		return false;
	}

	logger->info("Loading map id {}, name {}", mapEntry->id, mapEntry->name);

	if (!tio_fgets(filename, 260, saveFile->file)) {
		logger->error("Unable to load current map data directory");
		return false;
	}
	len = strnlen(filename, 259);
	if (len > 0 && filename[len - 1] == '\n') {
		filename[len - 1] = '\0';
	}
	else {
		logger->error("Map save directory not correctly terminated.");
		return false;
	}

	// NOTE: save dir is ignored

	auto visitedMaps = LoadIdxTable<int32_t>(saveFile->file);
	if (saveFolder.size() < 2)
		return false;

	if (saveFolder[saveFolder.size() - 1] != '\\') {
		saveFolder.resize(saveFolder.size() + 1);
		saveFolder[saveFolder.size() - 1] = '\\';
	}
	std::string mapfleeFile = fmt::format("{}map_mapflee.bin", saveFolder);
	if (vfs->FileExists(mapfleeFile.c_str()) ) {
		auto mapFleeFh = vfs->Open(mapfleeFile.c_str(), "rb");
		MapFleeSaveData fleeData;
		vfs->Read(&fleeData, sizeof(fleeData), mapFleeFh);
		vfs->Close(mapFleeFh);
	}

	
	// Pseudo Open Map

	auto dataDir = fmt::format("maps\\{}", mapEntry->name);
	auto saveDir = fmt::format("{}maps\\{}", saveFolder, mapEntry->name);

	logger->info("Loading Map: {}", dataDir);

	if (!vfs->DirExists(dataDir)) {
		logger->error("Cannot open map '{}' because it doesn't exist.", dataDir);
		return false;
	}

	vfs->MkDir(saveDir);


	auto mdyFilename = fmt::format("{}\\mobile.mdy", saveDir);

	if (!vfs->FileExists(mdyFilename)) {
		logger->info("Skipping dynamic mobiles because {} doesn't exist.", mdyFilename);
		return false;
	}

	logger->info("Loading dynamic mobiles from {}", mdyFilename);

	auto mdyfh = tio_fopen(mdyFilename.c_str(), "rb");
	if (!mdyfh) {
		throw TempleException("Unable to open dynamic mobile file for reading: {}", mdyFilename);
	}

	dynHandles.clear();
	while (true) {
		try {
			auto handle = objSystem->LoadFromFile(mdyfh);
			auto d = description.getDisplayName(handle);
			std::string handleDesc;
			if (d)
				handleDesc = fmt::format("{}", d);
			else
				handleDesc = fmt::format("Unnamed");
			logger->debug("PseudoLoad: dynamic object {} ({})", handleDesc,	objSystem->GetObject(handle)->id.ToString());
			dynHandles.push_back(handle);
		}
		catch (TempleException &e) {
			logger->error("Unable to load object: {}", e.what());
			break;
		}
		
	}

	if (!tio_feof(mdyfh)) {
		tio_fclose(mdyfh);
		throw TempleException("Error while reading dynamic mobile file {}", mdyFilename);
	}

	tio_fclose(mdyfh);

	logger->info("Done reading dynamic mobiles.");

	// Post Process

	objSystem->ForEachObj([&](objHndl handle, GameObjectBody& obj) {
		if (!obj.IsStatic()) {
			obj.UnfreezeIds();
		}

		/*if (obj.HasFlag(OF_TELEPORTED)) {
			TimeEvent e;
			e.system = TimeEventType::Teleported;
			e.params[0].handle = handle;
			gameSystems->GetTimeEvent().ScheduleNow(e);
			obj.SetFlag(OF_TELEPORTED, false);
			return;
		}*/

		//// Initialize everything's D20 state
		//if (gameSystems->GetParty().IsInParty(handle)) {
		//	return; // The party is initialized elsewhere (in the Party system)
		//}

		// This logic is a bit odd really. Apparently obj_f_dispatcher will not be -1 for non-critters anyway?
		if (obj.IsNPC() || obj.GetInt32(obj_f_dispatcher) == -1) {
			d20Sys.d20Status->D20StatusInit(handle);
		}

	});

	
	return true;
}

void MapSystem::ResetFleeTo()
{
	mFleeInfo = MapFleeInfo();
}

void MapSystem::SetFleeInfo(int mapId, LocAndOffsets loc, locXY enterLoc)
{
	mFleeInfo.mapId = mapId;
	mFleeInfo.enterLocation = enterLoc;
	mFleeInfo.location = loc;
}

bool MapSystem::HasFleeInfo() const
{
	return mFleeInfo.mapId != 0;
}

bool MapSystem::IsFleeing() const {
	return mFleeInfo.isFleeing;
}

void MapSystem::SetFleeing(bool fleeing)
{
	mFleeInfo.isFleeing = fleeing;
}

SectorExplorationState MapSystem::GetExplorationData(SectorLoc sectorLoc, SectorExplorationData * dataOut)
{
	auto filename(fmt::format("{}\\esd{}", mSectorSaveDir, sectorLoc.raw));

	auto fh(vfs->Open(filename.c_str(), "rb"));

	if (!fh) {
		return SectorExplorationState::Unexplored;
	}

	uint8_t fullyExplored = 0;
	vfs->Read(&fullyExplored, 1, fh);

	if (fullyExplored) {
		vfs->Close(fh);
		return SectorExplorationState::AllExplored;
	}

	vfs->Read(&dataOut->explorationBitmap[0], sizeof(dataOut->explorationBitmap), fh);
	vfs->Close(fh);
	return SectorExplorationState::PartiallyExplored;
}

void MapSystem::SetExplorationData(SectorLoc sectorLoc, const SectorExplorationData & dataOut)
{
	// First determine whether the entirety of the sector has been explored
	bool allExplored = true;
	for (auto &i : dataOut.explorationBitmap) {
		if (i != 0xFF) {
			allExplored = false;
			break;
		}
	}

	auto filename(fmt::format("{}\\esd{}", mSectorSaveDir, sectorLoc.raw));
	
	auto fh = vfs->Open(filename.c_str(), "wb");
	if (!fh) {
		logger->error("Unable to write sector exploration data to {}", filename);
		return;
	}
	
	uint8_t allExploredByte = allExplored ? 1 : 0;
	vfs->Write(&allExploredByte, 1, fh);
	
	if (!allExplored) {
		vfs->Write(&dataOut.explorationBitmap[0], sizeof(dataOut.explorationBitmap), fh);
	}

	vfs->Close(fh);	
}

void MapSystem::PreloadSectorsAround(locXY loc)
{
	// Center sector
	SectorLoc sectorLoc{ loc };
	int startX = (int) sectorLoc.x() - 1;
	int startY = (int) sectorLoc.y() - 1;

	Sector* sector;
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			SectorLoc loc{ startX + x, startY + y };						
			if (sectorSys.SectorLock(loc, &sector)) {
				sectorSys.SectorUnlock(loc);
			}

		}
	}

}

int MapSystem::GetMapIdByType(MapType type)
{
	for (auto &entry : mMaps) {
		if (entry.second.type == type) {
			return entry.first;
		}
	}
	if (type == MapType::ArenaMap){
		return 5119;
	}
	return 0;
}

bool MapSystem::IsRandomEncounterMap(int mapId) const
{
	auto maxRange = 5077;
	if (temple::Dll::GetInstance().IsExpandedWorldmapDll())
		maxRange = 5078;

	return mapId >= 5070 && mapId <= maxRange;
}

bool MapSystem::IsVignetteMap(int mapId) const
{
	return mapId >= 5096 && mapId <= 5104;
}

bool MapSystem::IsCurrentMapOutdoors() const
{
	if (mCurrentMap) {
		return mCurrentMap->IsOutdoors();
	}
	return false;
}

bool MapSystem::IsCurrentMapBedrest() const
{
	if (mCurrentMap) {
		return mCurrentMap->IsBedrest();
	}
	return false;
}

void MapSystem::ClearDispatchers()
{

	int it;
	objHndl handle;
	if (addresses.FirstNonProtoHandle(&handle, &it)) {
		do {
			if (!objects.IsStatic(handle)) {
				mD20System.RemoveDispatcher(handle);
			}
		} while (addresses.NextNonProtoHandle(&handle, &it));
	}

}

void MapSystem::ClearObjects()
{
	Expects(!mClearingMap);

	mClearingMap = true;

	gameSystems->GetTimeEvent().ClearForMapClose();

	auto freeRenderState = temple::GetPointer<void(objHndl)>(0x10021930);
	auto removeMapObj = temple::GetPointer<void(objHndl)>(0x100219B0);

	int it;
	objHndl handle;
	if (addresses.FirstNonProtoHandle(&handle, &it)) {
		do {
			if (!objects.IsStatic(handle)) {
				freeRenderState(handle);
				removeMapObj(handle);
			}
		} while (addresses.NextNonProtoHandle(&handle, &it));
	}

	gameSystems->GetMapSector().Clear();
	gameSystems->GetObj().CompactIndex();

	mClearingMap = false;
}

void MapSystem::ShowGameTip()
{
	if (mEnableTips)
	{
		if (!addresses.call_ui_tutorial_isactive())
		{
			auto tip = config.GetVanillaInt("startup_tip");
			if (tip >= 0) {
				mEnableTips = true;
				ShowGameTip(tip);
			}
		}
	}
}

void MapSystem::ShowGameTip(int tipId)
{
	// Roll over to the first tip
	if (tipId >= (int) mTips.size()) {
		tipId = 0;
	}

	// Have to make this static for c_str to be reliable
	static std::string sTipText = mTips[tipId];
	static MapSystem *sMapSystems = this;
	static BOOL sShowTips = TRUE;

	sTipText = mTips[tipId];
	addresses.ui_show_tip(
		mTipsDialogTitle.c_str(),
		sTipText.c_str(),
		mTipsDialogOk.c_str(),
		mTipsDialogNext.c_str(),
		mTipsDialogShowTips.c_str(),
		&sShowTips,
		[](int okButton) {
			if (okButton) {
				if (!sShowTips) {
					config.SetVanillaInt("startup_tip", -1);
				}
			} else {
				auto nextTip = config.GetVanillaInt("startup_tip");
				sMapSystems->ShowGameTip(nextTip);
			}
		});
	
	config.SetVanillaInt("startup_tip", tipId + 1);
}

bool MapSystem::IsValidMapId(int mapId)
{
	return mMaps.find(mapId) != mMaps.end();
}

int MapSystem::GetCurrentMapId() const
{
	if (mCurrentMap) {
		return mCurrentMap->id;
	}
	return 0;
}

int MapSystem::GetHighestMapId() const
{
	int highestId = 0;
	for (auto & entry : mMaps) {
		if (entry.second.id > highestId) {
			highestId = entry.second.id;
		}
	}	
	return highestId;
}

locXY MapSystem::GetStartPos(int mapId) const
{
	locXY result{ 0, 0 };

	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		result.locx = it->second.startPosX;
		result.locy = it->second.startPosY;
	}

	return result;
}

const std::string & MapSystem::GetMapName(int mapId)
{
	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		return it->second.name;
	}

	static std::string sEmptyName;
	return sEmptyName;
}

const std::string & MapSystem::GetMapDescription(int mapId)
{
	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		return it->second.description;
	}

	static std::string sEmptyName;
	return sEmptyName;
}

bool MapSystem::IsMapOutdoors(int mapId)
{
	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		return it->second.IsOutdoors();
	}
	return false;
}

int MapSystem::GetEnterMovie(int mapId, bool ignoreVisited)
{
	if (!ignoreVisited && mVisitedMaps.find(mapId) != mVisitedMaps.end()) {
		return 0;
	}

	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		return it->second.movie;
	}
	return 0;
}

void MapSystem::MarkVisitedMap(objHndl obj)
{
	if (objects.GetType(obj) != obj_t_pc) {
		return;
	}

	if (!mCurrentMap) {
		return;
	}
	
	int mapId = mCurrentMap->id;
	if (!IsRandomEncounterMap(mapId) && !IsVignetteMap(mapId)) {
		mVisitedMaps.insert(mapId);
	}
}

bool MapSystem::IsUnfogged(int mapId)
{
	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		return it->second.IsUnfogged();
	}
	return false;
}

int MapSystem::GetArea(int mapId) const
{
	auto it = mMaps.find(mapId);
	if (it != mMaps.end()) {
		return it->second.area;
	}
	return 0;
}

/* 10072A90 */
bool MapSystem::OpenMap(int mapId, bool preloadSectors, bool dontSaveCurrentMap)
{
	auto it = mMaps.find(mapId);
	if (it == mMaps.end()) {
		logger->warn("Trying to open unknown map id {}", mapId);
		return false;
	}

	auto &mapEntry = it->second;
	
	mPartySystem.SaveCurrent();

	if (IsMapOpen() && !dontSaveCurrentMap) {
		FlushMap(0);
	}

	mTig.GetSoundSystem().ProcessEvents();

	auto validateSector = temple::GetPointer<BOOL(uint8_t)>(0x1009F550);

	if (!validateSector(1))
	{
		throw TempleException("Object system validate failed pre-load.");
	}

	
	OpenMap(&mapEntry);

	mStartLoc.locx = mapEntry.startPosX;
	mStartLoc.locy = mapEntry.startPosY;

	gameSystems->GetLocation().CenterOn(mapEntry.startPosX, mapEntry.startPosY);
	
	mTig.GetSoundSystem().ProcessEvents();

	if (preloadSectors) {
		PreloadSectorsAround(mStartLoc);
	}

	gameSystems->GetTimeEvent().LoadForCurrentMap();
	mPartySystem.RestoreCurrent();
	gameSystems->GetCritter().UpdateNpcHealingTimers();
	
	auto uiAfterMapload = temple::GetPointer<void()>(0x1009A540);
	uiAfterMapload();

	if (!validateSector(1)) {
		throw TempleException("Object system validate failed post-load.");
	}

	return true;
}

void MapSystem::CloseMap()
{
	if (!mMapClosed)
	{
		mMapClosed = true;
		
		static auto turnBasedReset = temple::GetPointer<void()>(0x10092A50);
		turnBasedReset();

		gameSystems->GetAAS().FreeAll();
		
		mMapOpen = false;
		
		for (auto mapSystem : gameSystems->GetMapCloseAwareSystems()) {
			mapSystem->CloseMap();
		}

		ClearObjects();
		gameSystems->GetParticleSys().RemoveAll();

		mSectorSaveDir = "";
		mSectorDataDir = "";
	}
}

void MapSystem::LoadTips()
{
	auto tips(MesFile::ParseFile("mes/tips.mes"));

	mTipsDialogTitle = tips[0];
	mTipsDialogOk = tips[1];
	mTipsDialogNext = tips[2];
	mTipsDialogShowTips = tips[3];

	auto count = std::stoi(tips[99]);

	for (int i = 0; i < count; ++i) {
		mTips[i] = tips[100 + i];
	}

	if (config.GetVanillaInt("startup_tip") >= 0) {
		mEnableTips = true;
	} else {
		mEnableTips = false;
	}

}

void MapSystem::FlushMap(int flags) {
	
	// Freeze all IDs
	objSystem->ForEachObj([&](objHndl handle, GameObjectBody& obj) {
		// The assumption seems to be that static objs are not saved here
		if (!obj.IsStatic()) {
			obj.FreezeIds();
		}
		gameSystems->GetD20().RemoveDispatcher(handle);
	});

	SaveMapMobiles();

	// Unfreeze the IDs
	MapLoadPostprocess();

	// Both are fogging related, but I have no idea how they differ?
	gameSystems->GetMapFogging().SaveEsd();

	if (mCurrentMap) {
		gameSystems->GetMapFogging().SaveExploredTileData(mCurrentMap->id);
	}

	// Previously a "map.sbf" file was saved here, which is only used
	// by the old scripting system though
	SaveSectors(flags);
	// Previously several other subsystems saved their data here if they were
	// in editor mode

	// Flushes townmap data for the current map
	static auto ui_townmap_flush = temple::GetPointer<void()>(0x100521b0);
	ui_townmap_flush();

	// flushes the ObjectEvents (which are tied to spell objects anyway and should go away)
	if (!flags){
		objEvents.FlushEvents();
	}
	

}

#pragma pack(push, 1)
struct MapProperties {
	// ID for terrain art
	int groundArtId;
	int y;
	uint64_t limitX;
	uint64_t limitY;
};
#pragma pack(pop)

void MapSystem::OpenMap(const MapListEntry *mapEntry)
{

	auto dataDir = fmt::format("maps\\{}", mapEntry->name);
	auto saveDir = fmt::format("Save\\Current\\maps\\{}", mapEntry->name);

	logger->info("Loading Map: {}", dataDir);

	// Close opened map 
	CloseMap();

	if (!vfs->DirExists(dataDir)) {
		throw TempleException("Cannot open map '{}' because it doesn't exist.", dataDir);
	}

	vfs->MkDir(saveDir);

	mSectorSaveDir = saveDir;
	mSectorDataDir = dataDir;
	
	gameSystems->GetHeight().SetDataDirs(dataDir, saveDir);
	gameSystems->GetPathNode().SetDataDirs(dataDir, saveDir);

	logger->info("Reading map properties from map.prp");

	auto prpFilename = fmt::format("{}\\map.prp", dataDir);
	auto prpContent = vfs->ReadAsBinary(prpFilename);
	Expects(prpContent.size() >= 24);

	BinaryReader reader(prpContent);
	auto mapProperties = reader.Read<MapProperties>();

	// Previously startloc.txt was loaded here, but that only seems to be relevant 
	// for worlded, and not for the actual game.

	ReadMapMobiles(dataDir, saveDir);

	gameSystems->GetHeight().Clear();

	gameSystems->GetAnim().ClearGoalDestinations();

	gameSystems->GetClipping().Load(dataDir);

	gameSystems->GetGMesh().Load(dataDir);
		
	gameSystems->GetLight().Load(dataDir);
	
	gameSystems->GetMapFogging().LoadFogColor(dataDir);

	gameSystems->GetPathNode().Load(dataDir, saveDir);
	
	MapLoadPostprocess();

	// TODO: a global sector_datadir and sector_savedir were set here
	// But only  seemed to be used from within map systems and at one spot 
	// in editor mode

	gameSystems->GetTerrain().Load(mapProperties.groundArtId);

	// Previously a "map.sbf" file was loaded here, which is only used
	// by the old scripting system though

	gameSystems->GetLocation().SetLimits(mapProperties.limitX, mapProperties.limitY);

	gameSystems->GetSector().SetLimits(mapProperties.limitX / 64, mapProperties.limitY / 64);

	gameSystems->GetMapSector().SetDirectories(dataDir, saveDir);
	gameSystems->GetSectorVB().SetDirectories(dataDir, saveDir);
	
	auto center = gameSystems->GetLocation().GetLimitsCenter();
	gameSystems->GetLocation().CenterOn(center.locx, center.locy);

	mCurrentMap = mapEntry;

	if (mapEntry->IsUnfogged()) {
		gameSystems->GetMapFogging().Disable();
	} else {
		gameSystems->GetMapFogging().Enable();
		gameSystems->GetMapFogging().LoadExploredTileData(mapEntry->id);
	}

	gameSystems->GetLight().SetMapId(mapEntry->id);
	
	gameSystems->GetScroll().SetMapId(mapEntry->id);
	
	LoadMapInfo(dataDir);
	
	// Previously there was a function call here that disabled objects outside the
	// Arkanum demo bounds, which are obviously unused here.

	mMapOpen = true;
	mMapClosed = false;

	logger->info("Finished loading the map");
}

void MapSystem::LoadMapInfo(const std::string & dataDir)
{
	auto filename = fmt::format("{}\\mapinfo.txt", dataDir);

	// This file is optional
	std::string mapInfoContent;
	try {
		mapInfoContent = vfs->ReadAsString(filename);
	} catch (TempleException &e) {
		logger->info("Couldn't find or read optional file: {} ({})", filename, e.what());
		return;
	}

	for (auto& line : split(mapInfoContent, '\n', true)) {

		auto parts = split(line, ':', true);
		if (parts.size() != 2) {
			continue;
		}

		auto prop = tolower(parts[0]);
		if (prop == "lightscheme") {
			auto lightSchemeId = std::stoi(parts[1]);
			gameSystems->GetLightScheme().SetLightSchemeId(lightSchemeId);
			auto hour = gameSystems->GetLightScheme().GetHourOfDay();
			gameSystems->GetLightScheme().SetLightScheme(0, hour);
		} else if (prop == "soundscheme") {
			auto schemes = split(parts[1], ',', true);
			auto scheme1 = std::stoi(schemes[0]);
			auto scheme2 = std::stoi(schemes[1]);
			gameSystems->GetSoundGame().SetSoundSchemeIds(scheme1, scheme2);
		} else if (prop == "reverb") {
			auto reverbs = split(parts[1], ',', true);
			auto roomType = std::stoi(reverbs[0]);
			auto reverbDry = std::stoi(reverbs[1]);
			auto reverbWet = std::stoi(reverbs[2]);
			mTig.GetSoundSystem().SetReverb(roomType, reverbDry, reverbWet);
		} else if (prop == "ground") {
			auto groundId = std::stoi(parts[1]);
			gameSystems->GetTerrain().Load(groundId);
		} else {
			logger->warn("Unknown command in map extension file {}: {}", filename, line);
		}

	}

}

/* 0x100717E0 */
void MapSystem::ReadMapMobiles(const std::string &dataDir, const std::string &saveDir)
{
	auto readObject = temple::GetPointer<BOOL(objHndl*, const char*)>(0x100DE690);
	
	// Read all mobiles that shipped with the game files
	auto mobFiles = vfs->Search(dataDir + "\\*.mob");

	logger->info("ReadMapMobiles: Loading {} map mobiles from {}", mobFiles.size(), dataDir);

	for (auto &mobFile : mobFiles) {
		auto filename = fmt::format("{}\\{}", dataDir, mobFile.filename);
		objHndl handle;
		if (!readObject(&handle, filename.c_str())) {
			logger->warn("Unable to load mobile object {} for level {}",
				filename, dataDir);
		} else //if (config.debugMessageEnable)
		{
			// Temple+: added check for OF_DYNAMIC (fixes "Jerkstop" issue)
			auto obj = objSystem->GetObject(handle);
			logger->trace("ReadMapMobiles: \t\tLoaded MOB obj {} ({})", handle, obj->id.ToString() );
			auto flags = obj->GetFlags();
			if (flags & OF_DYNAMIC) {
				logger->error("ReadMapMobiles: \t\t\tMOB file flagged OF_DYNAMIC!!! Unsetting.");
				obj->SetFlag(OF_DYNAMIC, false);
			}
		}
	}

	logger->info("ReadMapMobiles: \t\tDone loading map mobiles");

	// Read all mobile differences that have accumulated for this map in the save dir
	auto diffFilename = fmt::format("{}\\mobile.md", saveDir);

	if (vfs->FileExists(diffFilename)) {
		logger->info("ReadMapMobiles: Loading mobile diffs from {}", diffFilename);

		auto fh = tio_fopen(diffFilename.c_str(), "rb");
		if (!fh) {
			throw TempleException("Could not read mobile diffs from {}", diffFilename);
		}

		ObjectId objId;

		while (!tio_feof(fh)) {
			auto pos = tio_ftell(fh);

			if (tio_fread(&objId, sizeof(ObjectId), 1, fh) != 1) {
				if (tio_feof(fh)) {
					break;
				}
				tio_fclose(fh);
				throw TempleException("Unable to read next object id from mobile diff file {} @ {}", 
					diffFilename, pos);
			}

			// Get the active handle for the mob so we can apply diffs to it
			auto handle = gameSystems->GetObj().GetHandleById(objId);
			if (!handle) {
				tio_fclose(fh);
				throw TempleException("Could not retrieve handle for {} to apply differences to from {}", 
					objId, diffFilename);
			}

			auto obj = objSystem->GetObject(handle);
			obj->LoadDiffsFromFile(handle, fh);
			logger->trace("Loaded {} ({}) from diff file.", handle, objSystem->GetObject(handle)->id.ToString());

			if (objects.GetFlags(handle) & OF_EXTINCT) {
				logger->trace("{} is extinct.", handle);
				gameSystems->GetObj().Remove(handle);
			}
		}

		tio_fclose(fh);

		logger->info("ReadMapMobiles: \t\tDone loading map mobile diffs");

	} else {
		logger->info("ReadMapMobiles: \t\tSkipping mobile diffs, because {} is missing", diffFilename);
	}

	// Destroy all mobiles that had previously been destroyed	
	auto desFilename = fmt::format("{}\\mobile.des", saveDir);

	if (vfs->FileExists(desFilename)) {
		logger->info("ReadMapMobiles: Loading destroyed mobile file from {}", desFilename);

		auto desContent = vfs->ReadAsBinary(desFilename);
		BinaryReader reader(desContent);

		while (!reader.AtEnd()) {
			auto objId = reader.Read<ObjectId>();
			auto handle = gameSystems->GetObj().GetHandleById(objId);
			
			if (handle) {
				auto obj = objSystem->GetObject(handle);
				auto flags = obj->GetFlags();
				
				logger->debug("ReadMapMobiles: \t\t{} ({}) is destroyed.", handle, obj->id.ToString());
				gameSystems->GetObj().Remove(handle);
			}
		}
		
		logger->info("ReadMapMobiles: Done loading destroyed map mobiles");
	} else {
		logger->info("ReadMapMobiles: Skipping destroyed mobile files, because {} is missing", desFilename);
	}
	
	ReadDynamicMobiles(saveDir);

}

void MapSystem::ReadDynamicMobiles(const std::string & saveDir)
{
	auto filename = fmt::format("{}\\mobile.mdy", saveDir);

	if (!vfs->FileExists(filename)) {
		logger->info("ReadDynamicMobiles: Skipping dynamic mobiles because {} doesn't exist.", filename);
		return;
	}

	logger->info("ReadDynamicMobiles: Loading dynamic mobiles from {}", filename);

	auto fh = tio_fopen(filename.c_str(), "rb");
	if (!fh) {
		throw TempleException("Unable to open dynamic mobile file for reading: {}", filename);
	}

	while (true) {
		try {
			auto handle = objSystem->LoadFromFile(fh);
			logger->trace("ReadDynamicMobiles: \t\tLoaded dynamic object {} ({})", handle ,	objSystem->GetObject(handle)->id.ToString());
		} catch (TempleException &e) {
			logger->error("Unable to load object: {}", e.what());
			break;
		}
	}

	if (!tio_feof(fh)) {
		tio_fclose(fh);
		throw TempleException("Error while reading dynamic mobile file {}", filename);
	}

	tio_fclose(fh);

	logger->info("ReadDynamicMobiles: Done reading dynamic mobiles.");

}

void MapSystem::MapLoadPostprocess()
{

	gameSystems->GetD20().ResetRadialMenus();

	objSystem->ForEachObj([&](objHndl handle, GameObjectBody& obj) {
		
		auto flags = obj.GetFlags();
		auto isDestroyed = flags & (OF_DESTROYED | OF_OFF);
		logger->trace("MapLoadPostProcess: obj {} GUID: {} destroyed/off? {}", handle, obj.id.ToString(), isDestroyed);
		if (objects.IsEquipment(handle)) {

			auto parentId = obj.GetObjectId(obj_f_item_parent);
			auto parentHndl = objSystem->GetHandleById(parentId);
			logger->trace("  Parent GUID: {} handle: {}", parentId.ToString(), parentHndl);
			if ( (flags & OF_INVENTORY) && !parentHndl ) {
				logger->trace("    Unparented inventory item?? {}", handle);
			}
			else if (parentHndl && (flags & OF_INVENTORY) == 0) {
				logger->trace("    Parented item not flagged as inventory?? {}", handle);
			}
			
			
			
		}
		if (!obj.IsStatic()) {
			obj.UnfreezeIds();
		}

		// Temple+: added safeguard against nulled inventory items
		if (obj.IsCritter()) {
			obj.PruneNullInventoryItems();
		}
		

		if (obj.HasFlag(OF_TELEPORTED)) {
			TimeEvent e;
			e.system = TimeEventType::Teleported;
			e.params[0].handle = handle;
			gameSystems->GetTimeEvent().ScheduleNow(e); // calls 0x10025050 which does some sector light updates mainly
			obj.SetFlag(OF_TELEPORTED, false);
			return;
		}

		// Initialize everything's D20 state
		if (gameSystems->GetParty().IsInParty(handle)) {
			return; // The party is initialized elsewhere (in the Party system)
		}

		// This logic is a bit odd really. Apparently obj_f_dispatcher will not be -1 for non-critters anyway?
		if (obj.IsNPC() || obj.GetInt32(obj_f_dispatcher) == -1) {
			d20Sys.d20Status->D20StatusInit(handle);
		}

	});

}

void MapSystem::SaveMapMobiles() {

	// This file will contain the differences from the mobile object stored in the sector's data files
	auto diffFilename = fmt::format("{}\\mobile.md", mSectorSaveDir);
	auto diffOut = std::make_unique<VfsOutputStream>(diffFilename);

	// This file will contain the dynamic objects that have been created on this map
	auto dynFilename = fmt::format("{}\\mobile.mdy", mSectorSaveDir);
	auto dynOut = std::make_unique<VfsOutputStream>(dynFilename);


	// This file will contain the object ids of mobile sector objects that have been destroyed
	auto destrFilename = fmt::format("{}\\mobile.des", mSectorSaveDir);
	auto destrFh = vfs->Open(destrFilename.c_str(), "ab");
	if (!destrFh) {
		throw TempleException("Unable to open {} for writing.", destrFilename);
	}
	
	auto prevDestroyedObjs = vfs->Length(destrFh) / sizeof(ObjectId);
	auto destroyedObjs = 0;
	auto dynamicObjs = 0;
	auto diffObjs = 0;

	objSystem->ForEachObj([&](objHndl handle, GameObjectBody& obj) {
		
		if (obj.IsStatic()) {
			return; // Diffs for static objects are stored in the sector files
		}

		// Dynamic objects are stored in their entirety in mobile.mdy
		if (obj.HasFlag(OF_DYNAMIC)) {
			// If a dynamic object has been destroyed, it wont be recreated on mapload
			// anyway (since there is no mob file for it)
			if (obj.HasFlag(OF_DESTROYED) || obj.HasFlag(OF_EXTINCT)) {
				//logger->debug("Skipping dynamic object {} for writing destroyed objs ({})", handle, objSystem->GetObject(handle)->id.ToString());
				return;
			}
			// TODO: Replace with proper VFS usage
			obj.Write(*dynOut);
			++dynamicObjs;
			return;
		}

		if (!obj.hasDifs) {
			//logger->debug("Skipping object with diffs {} for writing destroyed objs ({})", handle, objSystem->GetObject(handle)->id.ToString());
			return; // Object is unchanged
		}

		if (obj.HasFlag(OF_DESTROYED) || obj.HasFlag(OF_EXTINCT)) {
			if (obj.HasFlag(OF_EXTINCT))
			{
				logger->trace("SaveMapMobiles: \tWriting extinct object {} ({}) to mobile.des file.", handle,	objSystem->GetObject(handle)->id.ToString());
			} else
			{
				logger->trace("SaveMapMobiles: \tWriting destroyed object {} ({}) to mobile.des file.", handle,	objSystem->GetObject(handle)->id.ToString());
			}
			// Write the object id of the destroyed obj to mobile.des
			vfs->Write(&obj.id, sizeof(obj.id), destrFh);
			++destroyedObjs;
		} else {
			//logger->debug("Writing object {} to diff file ({})", description.getDisplayName(handle),	objSystem->GetObject(handle)->id.ToString());
			// Write the object id followed by a diff record to mobile.md
			diffOut->WriteObjectId(obj.id);

			// TODO: Replace with proper VFS usage
			obj.WriteDiffsToStream(*diffOut);
			++diffObjs;
		}

	});
		
	diffOut.reset();
	vfs->Close(destrFh);
	dynOut.reset();

	logger->info("Saved {} dynamic, {} destroyed ({} previously), and {} mobiles with differences in {}",
		dynamicObjs, destroyedObjs, prevDestroyedObjs, diffObjs, mSectorSaveDir);

	if (!destroyedObjs && !prevDestroyedObjs) {
		tio_remove(destrFilename.c_str());
	}
	if (!dynamicObjs) {
		tio_remove(dynFilename.c_str());
	}

}

void MapSystem::SaveSectors(int flags)
{
	auto saveStatics = temple::GetRef<int(__cdecl)(char )>(0x10082C00);
	saveStatics(flags);

}

MapFleeInfo::MapFleeInfo()
{
	isFleeing = false;
	mapId = 0;
	location.location.locx = 0;
	location.location.locy = 0;
	location.off_x = 0;
	location.off_y = 0;
	enterLocation.locx = 0;
	enterLocation.locy = 0;
}
