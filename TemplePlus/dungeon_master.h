#pragma once

#include <map>
#include "common.h"
#include <tig/tig_msg.h>

class DungeonMaster
{



public:
	bool IsActive();
	bool IsActionActive();
	void Render();

	bool HandleMsg(TigMsg & msg);

	void InitEntry(int protoNum);

	struct Record{
		int protoId;
		std::string name;
		Race race;
		MonsterCategory monsterCat;
		std::vector<int> factions;
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;

	bool FilterResult(Record& record);
	std::map<int, Record > humanoids;
	std::map<int, Record > monsters;
	std::map<int, Record > weapons;


	int mObjSpawnProto = 0;
};

extern DungeonMaster dmSys;