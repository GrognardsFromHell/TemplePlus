#pragma once

#include <map>
#include "common.h"
#include <tig/tig_msg.h>
#include "spell_structs.h"

class DungeonMaster
{



public:
	bool IsActive();
	bool IsMinimized();
	void Show();
	void Hide();
	void Toggle();

	bool IsActionActive();
	void Render();
	void RenderEditedObj();

	bool HandleMsg(const TigMsg & msg);
	bool HandleSpawning(const TigMsg & msg);
	bool HandleEditing(const TigMsg & msg);

	void InitEntry(int protoNum);

	struct Record{
		int protoId;
		std::string name;
		Race race;
		MonsterCategory monsterCat;
		MonsterSubcategoryFlag monsterSubtypes;
		std::vector<int> factions;
		int hitDice;
		std::map<Stat, int> classLevels;
	};


	// describes how a critter is modified
	struct CritterBooster{
		std::map<Stat, int> classLevels; // how many extra class levels to hand out for each class
		std::vector<SpellStoreData> spellsKnown;
		std::vector<SpellStoreData> spellsMemorized;
	};


	struct ObjEditor{
		
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;

	void RenderMonster(Record& record);

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
	int mFactionFilter = 0;

	objHndl mTgtObj = objHndl::null; // object under cursor
	objHndl mEditedObj = objHndl::null; // object being edited
};

extern DungeonMaster dmSys;