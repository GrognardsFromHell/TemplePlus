#pragma once

#include <map>
#include "common.h"
#include <tig/tig_msg.h>
#include "spell_structs.h"
#include "skill.h"
//#include <infrastructure\vfs.h>

//struct VfsSearchResult;

class DungeonMaster
{



public:

	DungeonMaster();

	bool IsActive();
	bool IsMinimized();
	bool IsMoused();
	void Show();
	void Hide();
	void Toggle();

	bool IsUnavailable();
	bool IsActionActive();
	bool IsEditorActive();
	objHndl GetHoveredCritter();
	bool IsHandlingMsg();
	void SetIsHandlingMsg(bool b);;

	
	int GetDiceRollForcing();
	bool IsControllingNpcs();

	void Render();
	void RenderDmButton();
	void RenderMaps();
	void RenderEditedObj();
	void RenderVsParty();
	void RenderFudgeRolls();
	void RenderPathfinding();
	void RenderSector();

	bool HandleMsg(const TigMsg & msg);
	bool HandleSpawning(const TigMsg & msg);
	bool HandleCloning(const TigMsg& msg);
	bool HandleEditing(const TigMsg & msg);
	bool HandleMoving(const TigMsg &msg);
	bool HandlePathnode(const TigMsg& msg);
	bool HandlePaintTiles(const TigMsg& msg);

	void InitEntry(int protoNum);
	void InitCaches();
	void InitObjEditor();

	void GotoArena();
	void TransitionToMap(int mapId);
	objHndl GetTgtObj() { return mTgtObj;  }
	
	
	struct Record {
		int protoId;
		std::string name;
		std::string lowerName; // lower case name
	};

	struct CritterRecord: Record {
		Race race;
		MonsterCategory monsterCat;
		MonsterSubcategoryFlag monsterSubtypes;
		std::vector<int> factions;
		int hitDice;
		std::map<Stat, int> classLevels;
		std::map<SkillEnum, int> skillRanks; // is actually 2 * skill rank, display is halved (for supporting half ranks)
	};

	struct ItemRecord : Record {
		int itemType;
	};


	// describes how a critter is modified
	struct CritterBooster{
		std::vector<int> factions;
		std::map<Stat, int> classLevels; // how many extra class levels to hand out for each class
		std::vector<SpellStoreData> spellsKnown;
		std::vector<SpellStoreData> spellsMemorized;
	};


	struct ObjEditor : CritterRecord{
		std::vector<int> stats;
		std::vector<SpellStoreData> spellsKnown;
		std::vector<SpellStoreData> spellsMemorized;
		std::vector<feat_enums> feats;
		std::vector<int> scripts;
	};

	enum DungeonMasterAction : int {
		None,
		Spawn,
		Clone,
		Move,
		PathnodeCreate,
		PathnodeDelete,
		PathnodeMove,
		PaintTileBlock,
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;
	bool mJustOpened = false;

	void SetMoused(bool state);

	int mForceRollType; // 0 - normal, 1 - rolls 1s, 2 - roll avg (e.g. 10s), 3 - avg+2, 4 - rolls 20s
	bool mControlNpcs = false;

	void RenderMonster(CritterRecord& record);
	void RenderMonsterFilter();
	void RenderMonsterModify();
	void ApplyMonsterModify(objHndl handle);

	bool FilterResult(CritterRecord& record);

	void RenderItem(ItemRecord& record); // Give item menu
	void RenderItemFilter();

	bool FilterItemResult(ItemRecord& record);
	std::array<char, 256> mItemNameFilterBuf;
	std::string mItemNameFilter = "";

	
	std::map<int, CritterRecord > humanoids;
	bool mIsInited = false;

	std::map<int, CritterRecord > monsters;
	std::map<int, ItemRecord > items;

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

	// Item Filter
	int mItemProtoFilter = 0;

	objHndl mTgtObj = objHndl::null; // object under cursor
	objHndl mEditedObj = objHndl::null; // object being edited

	void SetObjEditor(objHndl handle);
	void ApplyObjEdit(objHndl handle);
};

extern DungeonMaster dmSys;