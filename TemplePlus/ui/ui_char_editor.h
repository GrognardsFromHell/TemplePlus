#pragma once
#include "common.h"
#include <spell_structs.h>
#include <EASTL/fixed_string.h>
#include <EASTL/hash_map.h>
#include "EASTL/vector.h"

struct UiResizeArgs;
struct GameSystemConf;

struct CharEditorSelectionPacket{
	int abilityStats[6];
	int numRerolls; // number of rerolls
	int isPointbuy;
	char rerollString[128];
	Stat statBeingRaised;
	int raceId; // 7 is considered invalid
	int genderId; // 2 is considered invalid
	int height0;
	int height1;
	int modelScale;
	int hair0;
	int hair1;
	Stat classCode;
	int deityId;
	int domain1;
	int domain2;
	Alignment alignment; 
	int alignmentChoice; // 1 is for Positive Energy, 2 is for Negative Energy
	feat_enums feat0;
	feat_enums feat1;
	feat_enums feat2;
	int skillPointsAdded[42]; // idx corresponds to skill enum
	int skillPointsSpent;
	int availableSkillPoints;
	int spellEnums[SPELL_ENUM_MAX_VANILLA];
	int spellEnumsAddedCount;
	int spellEnumToRemove; // for sorcerers who swap out spells
	int wizSchool;
	int forbiddenSchool1;
	int forbiddenSchool2;
	feat_enums feat3;
	feat_enums feat4;
	int portraitId;
	char voiceFile[256];
	int voiceId; // -1 is considered invalid
};

struct KnownSpellInfo {
	int spEnum = 0;
	uint8_t spFlag = 0;
	int spellClass = 0;
	int spellLevel = 0;
	KnownSpellInfo() { spEnum = 0; spFlag = 0; spellClass = 0; };
	KnownSpellInfo(int SpellEnum, int SpellFlag);
	KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass);
	KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass, int isDomain);

};

enum FeatInfoFlag
{
	None = 0, 
	DisregardPrereqs = 4 
};
struct FeatInfo {
	int featEnum;
	int flag = 0;
	int minLevel = 1;
	FeatInfo(int FeatEnum) : featEnum(FeatEnum) {};
	FeatInfo(std::string &featName);
};


struct LegacyCharEditorSystem{
	const char* name;
	BOOL(__cdecl *systemInit)(GameSystemConf *);
	BOOL(__cdecl *systemResize)(void *);
	int systemReset; // unused
	void(__cdecl *free)();
	void(__cdecl *hide)();
	void(__cdecl *show)();
	BOOL(__cdecl *checkComplete)(); // checks if the char editing stage is complete (thus allowing you to move on to the next stage). This is checked at every render call.
	void(__cdecl *finalize)(CharEditorSelectionPacket &selPkt, objHndl &handle); //applies the configuration when clicking the "finish" button
	void(__cdecl *reset)(CharEditorSelectionPacket & editSpec);
	BOOL(__cdecl *activate)(); // inits values and sets appropriate states for buttons based on gameplay logic (e.g. stuff exclusive to certain classes etc.)
};


class CharEditorSystem {
public:
	virtual const std::string &GetName() const = 0;
	virtual ~CharEditorSystem() = default;
	//CharEditorSystem(const GameSystemConf &) = delete;
	virtual BOOL Resize(const UiResizeArgs &) =0;
	int pad; // possibly some unused callback?
	void(__cdecl *free)();
	void(__cdecl *hide)();
	void(__cdecl *show)();
	int(__cdecl *checkComplete)(); // checks if the char editing stage is complete (thus allowing you to move on to the next stage). This is checked at every render call.
	int(__cdecl*debugMaybe)();
	void(__cdecl *reset)(CharEditorSelectionPacket & editSpec);
	BOOL(__cdecl *activate)(); // inits values and sets appropriate states for buttons based on gameplay logic (e.g. stuff exclusive to certain classes etc.)
};


class CharEditorClassSystem : public CharEditorSystem {
public:
	static constexpr auto Name = "char_editor_class";
	CharEditorClassSystem(const GameSystemConf &config);
	~CharEditorClassSystem();
	const std::string &GetName() const override;
};

const auto testSizeOfCharEditorSelectionPacket = sizeof(CharEditorSelectionPacket); // should be 3640 (0xE38)

/*
	Used by UiCharEditor and UiPcCreation systems
*/
class Chargen {

public:
	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();
	std::vector<KnownSpellInfo>& GetKnownSpellInfo();
	std::vector<KnownSpellInfo> &GetAvailableSpells();
	int * GetRolledStats();
	int GetRolledStatIdx(int x, int y, int *xyOut = nullptr); // gets the index of the Rolled Stats button according to the mouse position. Returns -1 if none.

	bool SpellsNeedReset();
	void SpellsNeedResetSet(bool value);

	// utilities
	bool SpellIsAlreadyKnown(int spEnum, int spellClass);
	bool SpellIsForbidden(int spEnum);


	eastl::vector<string> levelLabels;
	eastl::vector<string> spellLevelLabels;

protected:
	std::vector<KnownSpellInfo> mSpellInfo;
	std::vector<KnownSpellInfo> mAvailableSpells; // spells available for learning
	std::vector<FeatInfo> mExistingFeats, mSelectableFeats, mMultiSelectFeats, mMultiSelectMasterFeats, mBonusFeats;

	bool mSpellsNeedReset;
};

extern Chargen chargen;

int HookedFeatMultiselectSub_101A8080(feat_enums feat);