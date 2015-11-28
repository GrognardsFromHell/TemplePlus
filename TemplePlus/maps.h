
#pragma once

#include "location.h"

struct JumpPoint {
	int id;
	string mapName;
	int mapId;
	locXY location;
};

class Maps {
public:
	vector<int> GetVisited();
	int GetCurrentMapId();
	int GetCurrentAreaId();
	bool IsValidMapId(int mapId);
	void RevealFlag(int mapId, int flagId);
	bool IsCurrentMapOutdoor();
	BOOL IsMapOpen();


	locXY GetMapCenterTile();

	/*
		Retrieves information about a jump point.
	*/
	bool GetJumpPoint(int id, JumpPoint &jumpPoint, bool withMapName = false);
	
};

extern Maps maps;
