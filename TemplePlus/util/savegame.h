
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

#include <gsl/gsl>

#include <gamesystems/map/sector.h>

#include "streams.h"
#include "animgoals/anim.h"

struct QuestSave {
	GameTime acceptedTime;
	uint32_t acceptedArea;
	uint32_t state;
};

struct ObjRefSave {
	ObjectId id;
	uint32_t mapId;
	locXY location;
};

struct AnimGoalSave {
	AnimGoalType type;
	ObjRefSave selfObj;
	ObjRefSave targetObj;
	ObjRefSave blockObj;
	ObjRefSave scratchObj;
	ObjRefSave parentObj;
	locXY targetLoc;
	locXY range;
	int animId;
	int animIdPrev;
	int animData;
	int animData2;
	int spellData;
	int skillData;
	int flagsData;
	int scratchVal1;
	int scratchVal2;
	int scratchVal3;
	int scratchVal4;
	int scratchVal5;
	int scratchVal6;
	int soundHandle;
};

struct AnimSlotSave {
	AnimSlotId id;
	uint32_t flags;
	int currentState;
	uint32_t field14;
	uint64_t field18;
	ObjRefSave animObj;
	std::vector<AnimGoalSave> goals;
	std::array<uint8_t, 0xF8> unk;
	uint64_t pathInfo;
	GameTime someTime;
	uint32_t field2c90;
};

struct SpellObjSave {
	ObjectId id;
	uint32_t casterPartSysName;
};

struct SpellSave {
	uint32_t spellId;
	uint32_t spellIdx;
	uint32_t active;

	uint32_t spellEnum;
	uint32_t spellEnumOrg;
	uint32_t flagsSth;
	ObjectId caster;
	uint32_t casterPartSysName; // 0 if none
	uint32_t spellClass;
	uint32_t spellLevel;
	uint32_t baseCasterLevel;
	uint32_t spellDc;
	uint32_t spellObjCount; // Guess
	objHndl unknownObj;
	std::array<SpellObjSave, 128> spellObjs; // Not really sure if this is spell objs
	size_t targetListNumItemsCopy;
	size_t targetListNumItems;
	std::array<SpellObjSave, 32> targets;
	uint32_t numProjectiles;
	std::array<ObjectId, 5> projectiles;
	LocFull aoeCenter;
	uint32_t duration;
	uint32_t durationRemaining;
	uint32_t spellRange;
	uint32_t savingThrowResult;
	uint32_t metaMagic;
	uint32_t spellId2;
};

struct TimeEventArgSave {
	int32_t intVal = 0;
	float floatVal = 0;
	ObjRefSave objectRefVal;
	locXY locVal = {0, 0};
	std::string pythonObj;
};

struct TimeEventSave {
	GameTime expiresAt;
	TimeEventType type;
	std::vector<TimeEventArgSave> args;
};

class SaveGameArchive {
public:
	static void Create(const std::string &folder,
		const std::string &filename);

	static void Unpack(const std::string &filename, 
		const std::string &folder);

private:
	static void AddFolder(const std::string &folder,
		OutputStream &indexStream,
		OutputStream &dataStream);

	static void UnpackFolder(const std::string &folder,
		InputStream &indexStream,
		InputStream &dataStream);

	static void UnpackFile(const std::string &folder,
		InputStream &indexStream,
		InputStream &dataStream);
};

class SaveGame {
public:

	void Load(const std::string &path);

private:

	void LoadDataSav(InputStream &in);

	std::vector<std::string> mCustomDescriptions;
	bool mIronman = false;
	uint32_t mIronmanSaveNumber = 0;
	std::string mIronmanSaveName;
	std::vector<SectorTime> mSectorTimes;
	std::vector<uint8_t> mGlobalVars;
	std::vector<uint8_t> mGlobalFlags;
	uint32_t mStoryState = 0;
	std::vector<int32_t> mEncounterQueue;

	std::string mMapDataPath;
	std::string mMapSavePath;
	std::set<uint32_t> mVisitedMaps;

	uint32_t mNextSpellId = 0;
	std::vector<SpellSave> mSpells;

	uint32_t mLightSchemeId = 0;
	uint32_t mLightSchemeHourOfDay = 0;
	std::set<uint32_t> mKnownAreas;
	std::vector<uint32_t> mSoundSchemeIds; // At most 2
	bool mSoundGameOverSound = false;
	std::vector<uint32_t> mSoundGameOverStash; // 2 scheme ids
	bool mSoundGameInCombat = false;
	std::vector<uint32_t> mSoundGameCombatStash; // 2 scheme ids
	std::vector<QuestSave> mQuests;

	bool mInCombat = false;

	GameTime mRealTime;
	GameTime mGameTime;
	GameTime mAnimTime;
	std::vector<TimeEventSave> mTimeEvents;

	uint32_t mNextAnimId = 0;
	uint32_t mActiveGoalCount = 0;
	bool mAnimCatchup = false;
	uint32_t mAnimActionId = 0;
	std::vector<uint32_t> mUnkAnimVals;
	std::map<size_t, AnimSlotSave> mAnimSlots;

	static AnimSlotSave ReadAnimSlot(InputStream &in);
	static ObjRefSave ReadObjRef(InputStream &in);

	static void ReadSystemFooter(InputStream &in, const char *system);
};
