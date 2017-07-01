#pragma once

#include <map>
#include "common.h"
#include <tig/tig_msg.h>
#include "spell_structs.h"
//#include <infrastructure\vfs.h>

//struct VfsSearchResult;

class DungeonMaster
{



public:
	bool IsActive();
	bool IsMinimized();
	bool IsMoused();
	void Show();
	void Hide();
	void Toggle();

	bool IsActionActive();
	bool IsEditorActive();
	objHndl GetHoveredCritter();
	bool IsHandlingMsg();
	void SetIsHandlingMsg(bool b);;

	void Render();
	void RenderDmButton();
	void RenderMaps();
	void RenderEditedObj();
	void RenderVsParty();

	bool HandleMsg(const TigMsg & msg);
	bool HandleSpawning(const TigMsg & msg);
	bool HandleCloning(const TigMsg& msg);
	bool HandleEditing(const TigMsg & msg);
	bool HandleMoving(const TigMsg &msg);

	void InitEntry(int protoNum);
	void InitCaches();

	void GotoArena();
	void TransitionToMap(int mapId);
	objHndl GetTgtObj() { return mTgtObj;  }
	

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
		std::vector<int> factions;
		std::map<Stat, int> classLevels; // how many extra class levels to hand out for each class
		std::vector<SpellStoreData> spellsKnown;
		std::vector<SpellStoreData> spellsMemorized;
	};


	struct ObjEditor : Record{
		std::vector<int> stats;
		std::vector<SpellStoreData> spellsKnown;
		std::vector<SpellStoreData> spellsMemorized;
		std::vector<feat_enums> feats;
		
	};

	enum DungeonMasterAction : int {
		None,
		Spawn,
		Clone,
		Move
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;
	bool mJustOpened = false;


	void RenderMonster(Record& record);
	void RenderMonsterFilter();
	void RenderMonsterModify();
	void ApplyMonsterModify(objHndl handle);

	bool FilterResult(Record& record);
	std::map<int, Record > humanoids;
	bool mIsInited = false;

	std::map<int, Record > monsters;
	std::map<int, Record > weapons;

	// Import from save internals
	bool PseudoLoad(std::string filename);
	std::vector<objHndl> dynHandlesFromSave;

	// Actions
	void ActivateAction(DungeonMasterAction actionType);
	void DeactivateAction();
	void ActivateSpawn(int protoId);
	void ActivateClone(objHndl handle);
	void ActivateMove(objHndl handle);
	void ActivateRivalPc(objHndl handle);
	
	// Action objects
	DungeonMasterAction mActionType = DungeonMasterAction::None;
	int mObjSpawnProto = 0;
	objHndl mCloningObj = objHndl::null;
	objHndl mMovingObj = objHndl::null;


	// Monster Filter
	int mCategoryFilter = 0;
	int mSubcategoryFilter = 0;
	int mActionTimeStamp = 0;
	int mFactionFilter = 0;

	objHndl mTgtObj = objHndl::null; // object under cursor
	objHndl mEditedObj = objHndl::null; // object being edited

	void SetObjEditor(objHndl handle);
	void ApplyObjEdit(objHndl handle);
};

extern DungeonMaster dmSys;