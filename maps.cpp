
#include "stdafx.h"
#include "maps.h"
#include <util/addresses.h>

struct MapAddresses : AddressTable {
	
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
	
	MapAddresses() {
		rebase(GetVisitedMaps, 0x1006FE50);
		rebase(GetCurrentArea, 0x1006ED50);
		rebase(GetCurrentMap, 0x10070F90);
		rebase(IsValidMapId, 0x10070EF0);
		rebase(RevealFlag, 0x10128360);
		rebase(IsCurrentMapOutdoor, 0x1006FE80);
	}
};

MapAddresses mapAddresses;
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
