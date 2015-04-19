
#pragma once

class Maps {
public:
	vector<int> GetVisited();
	int GetCurrentMapId();
	int GetCurrentAreaId();
	bool IsValidMapId(int mapId);
	void RevealFlag(int mapId, int flagId);
	bool IsCurrentMapOutdoor();
};

extern Maps maps;
