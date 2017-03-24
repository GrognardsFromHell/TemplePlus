#pragma once

#include <map>
#include "common.h"
#include <tig/tig_msg.h>

class DungeonMaster
{



public:
	bool IsActive();
	void Show();
	void Hide();
	void Toggle();

	bool IsActionActive();
	void Render();

	bool HandleMsg(const TigMsg & msg);

	void InitEntry(int protoNum);

	struct Record{
		int protoId;
		std::string name;
		Race race;
		MonsterCategory monsterCat;
		MonsterSubcategoryFlag monsterSubtypes;
		std::vector<int> factions;
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;

	bool FilterResult(Record& record);
	std::map<int, Record > humanoids;
	bool mIsInited = false;
	int mTexId;

	std::map<int, Record > monsters;
	std::map<int, Record > weapons;

	int mObjSpawnProto = 0;
	int mCategoryFilter = 0;
	int mSubcategoryFilter = 0;
	int mActionTimeStamp = 0;
};

extern DungeonMaster dmSys;