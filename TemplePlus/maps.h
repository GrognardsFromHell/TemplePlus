
#pragma once

#include "location.h"

struct JumpPoint {
	int id;
	string mapName;
	int mapId;
	locXY location;
};

struct LgcyJumpPoint {
	int id;
	char* mapName;
	int mapId;
	locXY location;
};

struct DayNightXfer
{
	ObjectId objId;
	int defaultMapMaybe; // possibly "initial map"
	int dayMapId;
	LocAndOffsets loc;
	int nightMapId;
	int field34;
	LocAndOffsets nightLoc;
	DayNightXfer* next;
	int field4C;
};

class Maps {
public:
	vector<int> GetVisited();
	int GetCurrentMapId();
	int GetCurrentAreaId();
	bool IsValidMapId(int mapId);
	void RevealFlag(int mapId, int flagId);
	bool IsCurrentMapOutdoor();


	locXY GetMapCenterTile();

	/*
		Retrieves information about a jump point.
	*/
	bool GetJumpPoint(int id, JumpPoint &jumpPoint, bool withMapName = false);
	
};

extern Maps maps;
