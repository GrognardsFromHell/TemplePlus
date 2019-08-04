
#pragma once

#include <map>
#include <set>

#include "gamesystem.h"
#include "common.h"

struct SectorLoc;
struct GameSystemConf;
struct MapListEntry;
class D20System;
class PartySystem;
class TigInitializer;

// Contains info on how to flee from combat
struct MapFleeInfo {
	MapFleeInfo();

	bool isFleeing;
	int mapId;
	LocAndOffsets location;
	locXY enterLocation;
};

enum class MapType : uint32_t {
	None,
	StartMap,
	ShoppingMap,
	TutorialMap,
	ArenaMap // new in Temple+
};

struct SectorExplorationData {
	uint8_t explorationBitmap[64 * 64 * 3 * 3 / 8];
};

enum class SectorExplorationState : uint32_t {
	AllExplored,
	PartiallyExplored,
	Unexplored
};

class MapSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Map";
	MapSystem(TigInitializer &tigInitializer, 
		const GameSystemConf &config, 
		D20System &d20System,
		PartySystem &partySystem);
	~MapSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	bool PseudoLoad(GameSystemSaveFile* saveFile, std::string saveFolder, std::vector<objHndl> & dynHandles);

	void ResetFleeTo();
	const MapFleeInfo &GetFleeInfo() const {
		return mFleeInfo;
	}
	void SetFleeInfo(int mapId, LocAndOffsets loc, locXY enterLoc);
	bool HasFleeInfo() const;
	bool IsFleeing() const;
	void SetFleeing(bool fleeing);

	SectorExplorationState GetExplorationData(SectorLoc sectorLoc, 
		SectorExplorationData *dataOut);
	void SetExplorationData(SectorLoc sectorLoc,
		const SectorExplorationData &dataOut);

	// TODO: This is a candidate to be moved to the sector system
	void PreloadSectorsAround(locXY loc);

	int GetMapIdByType(MapType type);

	bool IsClearingMap() const {
		return mClearingMap;
	}

	bool IsMapOpen() const {
		return mMapOpen;
	}

	const std::string &GetSectorDataDir() const {
		return mSectorDataDir;
	}

	const std::string &GetSectorSaveDir() const {
		return mSectorSaveDir;
	}

	bool IsRandomEncounterMap(int mapId) const;

	bool IsVignetteMap(int mapId) const;

	const std::set<int>& GetVisitedMaps() const {
		return mVisitedMaps;
	}

	bool IsCurrentMapOutdoors() const;

	bool IsCurrentMapBedrest() const;

	// This is odd placement here
	void ClearDispatchers();

	void ClearObjects();

	void ShowGameTip();
	void ShowGameTip(int tipId);

	bool IsValidMapId(int mapId);

	int GetCurrentMapId() const;

	int GetHighestMapId() const;

	locXY GetStartPos(int mapId) const;

	const std::string& GetMapName(int mapId);
	const std::string& GetMapDescription(int mapId);
	bool IsMapOutdoors(int mapId);

	int GetEnterMovie(int mapId, bool ignoreVisited);

	void MarkVisitedMap(objHndl obj);

	bool IsUnfogged(int mapId);

	int GetArea(int mapId) const;

	bool OpenMap(int mapId, bool preloadSectors, bool dontSaveCurrentMap);

	void CloseMap();

private:
	void LoadTips();
	
	TigInitializer &mTig;
	D20System &mD20System;
	PartySystem &mPartySystem;

	// Startup tips related fields
	bool mEnableTips;
	std::string mTipsDialogTitle;
	std::string mTipsDialogOk;
	std::string mTipsDialogNext;
	std::string mTipsDialogShowTips;
	std::map<int, std::string> mTips;

	// Related to fleeing from combat (for whatever reason this is here)
	MapFleeInfo mFleeInfo;
	
	std::string mSectorDataDir;
	std::string mSectorSaveDir;

	// List of maps, scope: module
	std::map<int, MapListEntry> mMaps;
	const MapListEntry* mCurrentMap = nullptr;

	// Visited maps, scope: game session
	std::set<int> mVisitedMaps;

	// Picked when opening the map
	locXY mStartLoc;

	// Indicates that the map is currently being cleared
	bool mClearingMap = false;

	// Indicates that a map is currently opened
	bool mMapOpen = false;
	bool mMapClosed = false;

	
	// Store the currently opened map state to disk
	void FlushMap(int flags);

	void OpenMap(const MapListEntry* map);

	void LoadMapInfo(const std::string &dataDir);

	void ReadMapMobiles(const std::string &dataDir, const std::string &saveDir);
	void ReadDynamicMobiles(const std::string &saveDir);
	void MapLoadPostprocess();

	void SaveMapMobiles();
	void SaveSectors(int flags);

};
