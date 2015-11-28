
#include "stdafx.h"
#include "maps.h"
#include <temple/dll.h>
#include "location.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"

struct MapAddresses : temple::AddressTable {
	
	/*
		Writes at most size visited map ids to pMapIdsOut and returns
		the real number of visited maps, which can be larger.
	*/
	size_t (__cdecl *GetVisitedMaps)(int *pMapIdsOut, int size);

	/*
		Gets the id of the current area, which is based on the current map.
		There is a hardcoded table in this function that should be replaced.
	*/
	int (__cdecl *GetCurrentArea)();

	/*
		Gets the id of the current map.
	*/
	int (__cdecl *GetCurrentMap)();

	/*
		Returns true if the given map id is valid.
	*/
	bool (__cdecl *IsValidMapId)(int mapId);
	
	/*
		Returns true if the current map is an outdoor map.
	*/
	bool (__cdecl *IsCurrentMapOutdoor)();

	/*
		Reveals a Flag on the Townmap UI.
	*/
	void (__cdecl *RevealFlag)(int mapId, int flagId);

	/*
		Gets a jump point definition.
	*/
	bool (__cdecl *GetJumpPoint)(int jmpPntID, char *mapNameOut, size_t mapNameOutSize, int *mapNumOut, locXY *locXYOut);
	
	locXY (*GetMapCenterTile)();
	
	int * mapLoaded;
	int * mapIsOpen;

	MapAddresses() {
		rebase(GetVisitedMaps, 0x1006FE50);
		rebase(GetCurrentArea, 0x1006ED50);
		rebase(GetCurrentMap, 0x10070F90);
		rebase(IsValidMapId, 0x10070EF0);
		rebase(RevealFlag, 0x10128360);
		rebase(IsCurrentMapOutdoor, 0x1006FE80);
		rebase(GetJumpPoint, 0x100BDE20);
		rebase(mapLoaded, 0x10AA9524);
		rebase(GetMapCenterTile, 0x1002A170);
		rebase(mapIsOpen, 0x10AA9588);
	}
};
MapAddresses mapAddresses;

class GameSystemReplacements : TempleFix
{
public:
	const char* name() override { return "GameSystems" "Function Replacements"; }

	
	static int(__cdecl*orgField1C)(GameSystemConf* conf);
	static int field1c(GameSystemConf * conf) {
		return orgField1C(conf);
	};

	void apply() override
	{
		orgField1C = replaceFunction(0x1006FC60, field1c); // doesn't seem to get called anywhere, not even when editor mode is enabled. Possibly ripped out code.
	}
} gameSystemFix;

int(__cdecl*GameSystemReplacements::orgField1C)(GameSystemConf*) ;

Maps maps;

vector<int> Maps::GetVisited() {

	vector<int> result(100);
	auto realCount = mapAddresses.GetVisitedMaps(result.data(), result.size());

	if (realCount > result.size()) {
		result.resize(realCount);
		realCount = mapAddresses.GetVisitedMaps(result.data(), result.size());
	}
	assert(realCount <= result.size());
	result.resize(realCount);

	return result;

}

int Maps::GetCurrentMapId() {
	return mapAddresses.GetCurrentMap();
}

int Maps::GetCurrentAreaId() {
	return mapAddresses.GetCurrentArea();
}

bool Maps::IsValidMapId(int mapId) {
	return mapAddresses.IsValidMapId(mapId);
}

void Maps::RevealFlag(int mapId, int flagId) {
	mapAddresses.RevealFlag(mapId, flagId);
}

bool Maps::IsCurrentMapOutdoor() {
	return mapAddresses.IsCurrentMapOutdoor();
}

locXY Maps::GetMapCenterTile() {
	return mapAddresses.GetMapCenterTile();
}

BOOL Maps::IsMapOpen()
{
	return *mapAddresses.mapIsOpen;
}

bool Maps::GetJumpPoint(int id, JumpPoint& jumpPoint, bool withMapName) {
	char mapName[256];
	size_t mapNameLen = withMapName ? 256 : 0;
	bool result = mapAddresses.GetJumpPoint(id, mapName, mapNameLen, &jumpPoint.mapId, &jumpPoint.location);
	if (result) {
		jumpPoint.mapName = mapName;
	}
	return result;
}
