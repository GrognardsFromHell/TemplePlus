
#include "stdafx.h"
#include "util/fixes.h"
#include "obj.h"
#include "gamesystems.h"
#include "mapsystem.h"
#include "map/sector.h"
#include <config/config.h>
#include "gamesystems\objects\objsystem.h"

enum esd_load_result : uint32_t {
	PARTIALLY_EXPLORED = 0,
	FULLY_UNEXPLORED = 1,
	FULLY_EXPLORED = 2
};

#pragma pack(push, 1)
struct flee_combat_info {
	int mapId;
	int field_4;
	LocAndOffsets loc;
	uint32_t enterX;
	uint32_t enterY;
	BOOL isFleeing;
	int field_24;
};

struct explored_sector_data {
	uint8_t allExplored;
	uint8_t exploredBitmap[(64 * 64 * 3 * 3) / 8];
};
#pragma pack(pop)

static class MapSystemHooks : public TempleFix {

	static void fleecombat_reset();
	static void fleecombat_set_prev_map(int mapNumber, LocAndOffsets *loc, uint32_t enterX, uint32_t enterY);
	static void fleecombat_get_info(flee_combat_info *a1);
	static BOOL fleecombat_has_info();
	static void fleecombat_set_fleeing(BOOL fleeing);
	static BOOL fleecombat_is_fleeing();

	// _show_specific_tip: void __usercall(int tipIndex@<eax>)
	// static void tip_ok_callback();
	static esd_load_result fog_esd_load(uint64_t sectorLoc, explored_sector_data *buffer);
	static BOOL fog_esd_save(uint64_t sectorLoc, explored_sector_data *fogSec);
//	static int map_field1c(int);
	static void map_preload_sectors_at(locXY loc);
	static int get_map_by_type(int mapType);
	static BOOL is_in_clear_map_objs();
	static BOOL is_map_open();
	static void map_get_sector_dirs(char **dataDirOut, char **saveDirOut);
	// static BOOL maps_visited_it_copyout(int a1, int a2, int *a3);
	static BOOL map_is_random_encounter_map(int mapId);
	static BOOL map_is_vignette_map(int mapId);
	static int maps_get_visited(int *pMapsVisited, int maxCount);
	static BOOL map_is_outdoor();
	static BOOL map_is_bedrest();
	static BOOL _map_save_preprocess();
	static int _map_save_objects();
	static BOOL _save_mob_obj_diff();
	static BOOL _map_save_dynamic();
	// sub_100704B0: int()
	// _map_read_mobile_mdy: BOOL __usercall@<eax>(const char *a1@<eax>)
	static void _map_clear_objects();
	static void map_remove_dispatchers();
	// _map_open_load_mapinfo: void __usercall(char *a1@<eax>)
	static int _map_disable_objects();
	// _MakeMapCacheName: char *__usercall@<eax>(char *string@<eax>)
	static const char *_map_get_art_callback_real(int a1);
	// get_mapid_from_mapidx: int __usercall@<eax>(int a1@<eax>)
	static int map_mod_load();
	static void show_game_tip();
	static BOOL maps_is_valid_map_id(int);
	static BOOL map_find_name(int mapId, const char **mapNameOut);
	static int map_get_current_map_id();
	static int map_get_highest_map_id();
	static BOOL map_try_get_startpos(int mapId, uint64_t *startX, uint64_t *startY);
	static BOOL map_get_area(int mapId, int *areaOut);
	static BOOL map_get_enter_movie(int mapId, int *movieIdOut, int ignoreVisited);
	static int _map_flush(int);
	static BOOL preprocess_mobs_for_map(const char *a1);
	static void map_mark_visited_map(objHndl obj);
	static void _map_close();
	// _read_map_mobiles: BOOL __usercall@<eax>(char *mapDataDir@<eax>, char *mapSaveDir)
	static BOOL map_has_fog_of_war(int mapId);
	static void map_get_startpos(int mapId, uint64_t *a2, uint64_t *a3);
	// map_reset: None
	// map_exit: int()
	// static int map_save(int);
	static int _map_open(char *dataDir, char *saveDir, int);
	static BOOL map_open_in_game(int mapId, BOOL preloadSectors, BOOL dontSaveCurrentMap);

	void apply() override {
		replaceFunction(0x1006f8f0, fleecombat_reset);
		replaceFunction(0x1006f920, fleecombat_set_prev_map);
		replaceFunction(0x1006f970, fleecombat_get_info);
		replaceFunction(0x1006f990, fleecombat_has_info);
		replaceFunction(0x1006f9a0, fleecombat_set_fleeing);
		replaceFunction(0x1006f9b0, fleecombat_is_fleeing);
		// replaceFunction(0x1006f9c0, _show_specific_tip);
		// replaceFunction(0x1006fa60, tip_ok_callback);
		replaceFunction(0x1006faa0, fog_esd_load);
		replaceFunction(0x1006fb50, fog_esd_save);
		// replaceFunction(0x1006fc60, map_field1c);
		replaceFunction(0x1006fc90, map_preload_sectors_at);
		replaceFunction(0x1006fd70, get_map_by_type);
		replaceFunction(0x1006fd90, is_in_clear_map_objs);
		replaceFunction(0x1006fda0, is_map_open);
		replaceFunction(0x1006fdb0, map_get_sector_dirs);
		// replaceFunction(0x1006fde0, maps_visited_it_copyout);
		replaceFunction(0x1006fe10, map_is_random_encounter_map);
		replaceFunction(0x1006fe30, map_is_vignette_map);
		replaceFunction(0x1006fe50, maps_get_visited);
		replaceFunction(0x1006fe80, map_is_outdoor);
		replaceFunction(0x1006fea0, map_is_bedrest);
		// replaceFunction(0x1006fec0, _map_save_preprocess);
		// replaceFunction(0x1006ff40, _map_save_objects);
		// replaceFunction(0x10070080, _save_mob_obj_diff);
		// replaceFunction(0x10070380, _map_save_dynamic);
		// replaceFunction(0x100704b0, sub_100704B0);
		// replaceFunction(0x10070610, _map_read_mobile_mdy);
		// replaceFunction(0x100706d0, _map_clear_objects);
		replaceFunction(0x10070780, map_remove_dispatchers);
		// replaceFunction(0x100707f0, _map_open_load_mapinfo);
		// replaceFunction(0x10070a00, _map_disable_objects);
		// replaceFunction(0x10070af0, _MakeMapCacheName);
		// replaceFunction(0x10070b50, _map_get_art_callback_real);
		// replaceFunction(0x10070b80, get_mapid_from_mapidx);
		// replaceFunction(0x10070ba0, map_mod_load);
		replaceFunction(0x10070eb0, show_game_tip);
		replaceFunction(0x10070ef0, maps_is_valid_map_id);
		replaceFunction(0x10070f30, map_find_name);
		replaceFunction(0x10070f90, map_get_current_map_id);
		replaceFunction(0x10070fb0, map_get_highest_map_id);
		replaceFunction(0x10070fd0, map_try_get_startpos);
		replaceFunction(0x10071070, map_get_area);
		replaceFunction(0x100710d0, map_get_enter_movie);
		// replaceFunction(0x10071170, _map_flush);
		// replaceFunction(0x10071350, preprocess_mobs_for_map);
		replaceFunction(0x10071700, map_mark_visited_map);
		// replaceFunction(0x10071780, _map_close);
		// replaceFunction(0x100717e0, _read_map_mobiles);
		replaceFunction(0x10071d70, map_has_fog_of_war);
		replaceFunction(0x10071dd0, map_get_startpos);
		// replaceFunction(0x10071e40, map_reset);
		// replaceFunction(0x10071fd0, map_exit);
		// replaceFunction(0x10072050, map_save);
		// replaceFunction(0x10072370, _map_open);
		replaceFunction(0x10072a90, map_open_in_game);

		static void (__cdecl*orgCheckFogForCritter)(objHndl , int) = replaceFunction<void(__cdecl)(objHndl, int)>(0x100327A0, [](objHndl handle, int idx){

			if (objSystem->IsValidHandle(handle))
			{
				orgCheckFogForCritter(handle, idx);
			} else
				logger->info("orgCheckFogForCritter encountered null handle!!");
		});

		// Hooked call to IsPortalOpen inside FogPerformCheckForCritter to account for OF_SEE_THROUGH
		redirectToLambda<BOOL(objHndl)>(0x10032A23, [](objHndl handle)->BOOL {
			if (!handle)
				return TRUE;
			auto flags = objects.GetFlags(handle);
			if (flags & OF_SEE_THROUGH) {
				return TRUE;
			}
			return objects.IsPortalOpen(handle) ? TRUE : FALSE;
			});
	}
	
} hooks;

void MapSystemHooks::fleecombat_reset()
{
	gameSystems->GetMap().ResetFleeTo();
}

void MapSystemHooks::fleecombat_set_prev_map(int mapNumber, LocAndOffsets * loc, uint32_t enterX, uint32_t enterY)
{
	locXY enterPos{ enterX, enterY };
	gameSystems->GetMap().SetFleeInfo(mapNumber, *loc, enterPos);
}

void MapSystemHooks::fleecombat_get_info(flee_combat_info * fleeOut)
{
	auto &fleeInfo = gameSystems->GetMap().GetFleeInfo();
	fleeOut->mapId = fleeInfo.mapId;
	fleeOut->loc = fleeInfo.location;
	fleeOut->enterX = fleeInfo.enterLocation.locx;
	fleeOut->enterY = fleeInfo.enterLocation.locy;
	fleeOut->isFleeing = fleeInfo.isFleeing ? TRUE : FALSE;

}

BOOL MapSystemHooks::fleecombat_has_info()
{
	return gameSystems->GetMap().HasFleeInfo() ? TRUE : FALSE;
}

void MapSystemHooks::fleecombat_set_fleeing(BOOL fleeing)
{
	gameSystems->GetMap().SetFleeing(fleeing == TRUE);
}

BOOL MapSystemHooks::fleecombat_is_fleeing()
{
	return gameSystems->GetMap().IsFleeing() ? TRUE : FALSE;
}

esd_load_result MapSystemHooks::fog_esd_load(uint64_t sectorLoc, explored_sector_data * buffer)
{
	SectorExplorationData data;
	auto state = gameSystems->GetMap().GetExplorationData({ sectorLoc }, &data);
	switch (state) {
	case SectorExplorationState::AllExplored:
		buffer->allExplored = 1;
		return esd_load_result::FULLY_EXPLORED;
	case SectorExplorationState::PartiallyExplored:
		buffer->allExplored = 0;
		for (size_t i = 0; i < sizeof(buffer->exploredBitmap); ++i) {
			buffer->exploredBitmap[i] = data.explorationBitmap[i];
		}
		return esd_load_result::PARTIALLY_EXPLORED;
	default:
	case SectorExplorationState::Unexplored:
		buffer->allExplored = 0;
		return esd_load_result::FULLY_UNEXPLORED;
	}
}

BOOL MapSystemHooks::fog_esd_save(uint64_t sectorLoc, explored_sector_data * fogSec)
{
	SectorLoc loc{ sectorLoc };
	SectorExplorationData data;

	if (fogSec->allExplored){
		for (size_t i = 0; i < sizeof(fogSec->exploredBitmap); ++i) {
			data.explorationBitmap[i] = 0xff;
		}
	}
	else{
		for (size_t i = 0; i < sizeof(fogSec->exploredBitmap); ++i) {
			data.explorationBitmap[i] = fogSec->exploredBitmap[i];
		}
	}

	gameSystems->GetMap().SetExplorationData(loc, data);
	return TRUE;
}

void MapSystemHooks::map_preload_sectors_at(locXY loc)
{
	gameSystems->GetMap().PreloadSectorsAround(loc);
}

int MapSystemHooks::get_map_by_type(int mapType)
{
	MapType type = MapType::None;
	switch (mapType) {
	case 1:
		type = MapType::StartMap;
		break;
	case 2:
		type = MapType::ShoppingMap;
		break;
	case 3:
		type = MapType::TutorialMap;
		break;
	}
	return gameSystems->GetMap().GetMapIdByType(type);
}

BOOL MapSystemHooks::is_in_clear_map_objs()
{
	return gameSystems->GetMap().IsClearingMap() ? TRUE : FALSE;
}

BOOL MapSystemHooks::is_map_open()
{
	return gameSystems->GetMap().IsMapOpen() ? TRUE : FALSE;
}

void MapSystemHooks::map_get_sector_dirs(char ** dataDirOut, char ** saveDirOut)
{
	static char sDataDir[MAX_PATH];
	static char sSaveDir[MAX_PATH];
	strcpy_s(&sDataDir[0], MAX_PATH, gameSystems->GetMap().GetSectorDataDir().c_str());
	strcpy_s(&sSaveDir[0], MAX_PATH, gameSystems->GetMap().GetSectorSaveDir().c_str());

	*dataDirOut = &sDataDir[0];
	*saveDirOut = &sSaveDir[0];
}

BOOL MapSystemHooks::map_is_random_encounter_map(int mapId)
{
	return gameSystems->GetMap().IsRandomEncounterMap(mapId);
}

BOOL MapSystemHooks::map_is_vignette_map(int mapId)
{
	return gameSystems->GetMap().IsVignetteMap(mapId);
}

int MapSystemHooks::maps_get_visited(int * pMapsVisited, int maxCount)
{
	int count = 0;
	for (auto &mapId : gameSystems->GetMap().GetVisitedMaps()) {
		if (count >= maxCount) {
			break;
		}
		pMapsVisited[count++] = mapId;
	}
	return count;
}

BOOL MapSystemHooks::map_is_outdoor()
{
	return gameSystems->GetMap().IsCurrentMapOutdoors();
}

BOOL MapSystemHooks::map_is_bedrest()
{
	return gameSystems->GetMap().IsCurrentMapBedrest();
}

void MapSystemHooks::map_remove_dispatchers()
{
	// TODO: This is kinda pointless. d20_exit is called after map_exit,
	// which should already have cleaned up all remaining objects
	// SO effectively? This is dead code.
}

void MapSystemHooks::show_game_tip()
{
	gameSystems->GetMap().ShowGameTip();
}

BOOL MapSystemHooks::maps_is_valid_map_id(int mapId)
{
	return gameSystems->GetMap().IsValidMapId(mapId) ? TRUE : FALSE;
}

BOOL MapSystemHooks::map_find_name(int mapId, const char ** mapNameOut)
{
	auto& mapName = gameSystems->GetMap().GetMapName(mapId);
	if (mapName.empty()) {
		*mapNameOut = nullptr;
		return FALSE;
	}
	
	*mapNameOut = mapName.c_str();
	return TRUE;
}

int MapSystemHooks::map_get_current_map_id()
{
	return gameSystems->GetMap().GetCurrentMapId();
}

int MapSystemHooks::map_get_highest_map_id()
{
	return gameSystems->GetMap().GetHighestMapId();
}

/* 0x10070fd0 */
BOOL MapSystemHooks::map_try_get_startpos(int mapId, uint64_t * startX, uint64_t * startY)
{
	auto startPos = gameSystems->GetMap().GetStartPos(mapId);
	*startX = startPos.locx;
	*startY = startPos.locy;
	if (startPos.locx == 0 || startPos.locy == 0) {
		return FALSE;
	}
	return TRUE;
}

BOOL MapSystemHooks::map_get_area(int mapId, int * areaOut)
{
	auto area = gameSystems->GetMap().GetArea(mapId);
	*areaOut = area;
	return (area != 0) ? TRUE : FALSE;
}

BOOL MapSystemHooks::map_get_enter_movie(int mapId, int * movieIdOut, int ignoreVisited)
{
	int movieId = gameSystems->GetMap().GetEnterMovie(mapId, ignoreVisited == TRUE);
	*movieIdOut = movieId;
	
	if (movieId != 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void MapSystemHooks::map_mark_visited_map(objHndl obj)
{
	gameSystems->GetMap().MarkVisitedMap(obj);
}

BOOL MapSystemHooks::map_has_fog_of_war(int mapId){
	return gameSystems->GetMap().IsUnfogged(mapId) ? FALSE : TRUE;
}

void MapSystemHooks::map_get_startpos(int mapId, uint64_t * startX, uint64_t * startY)
{
	auto startPos = gameSystems->GetMap().GetStartPos(mapId);
	*startX = startPos.locx;
	*startY = startPos.locy;
}

BOOL MapSystemHooks::map_open_in_game(int mapId, BOOL preloadSectors, BOOL dontSaveCurrentMap)
{
	auto res = gameSystems->GetMap().OpenMap(mapId, preloadSectors == TRUE, dontSaveCurrentMap == TRUE);
	return res ? TRUE : FALSE;
}
