#pragma once
#include "common.h"
#include <spell_structs.h>
#include <EASTL/fixed_string.h>
#include <EASTL/hash_map.h>
#include "EASTL/vector.h"
#include "ui_chargen.h"

struct UiResizeArgs;
struct GameSystemConf;

struct KnownSpellInfo {
	int spEnum = 0;
	
	/* spFlag (not actually flag, just status)
	1 - denotes a replaceable spell (e.g. for sorcerers)
	2 - replaced spell (i.e. the slot had a previous spell and now has a new one)
	3 - new spell slot; use 0 for the spell level labels
	*/
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
	NoFeatInfoFlag = 0, 
	DisregardPrereqs = 4 
};



enum CharEditorStages : int {
	CE_Stage_Class = 0,
	CE_Stage_Stats,
	CE_Stage_Features,
	CE_Stage_Skills,
	CE_Stage_Feats,
	CE_Stage_Spells,
	CE_STAGE_COUNT
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

	void SetIsNewChar(bool state);
	bool IsNewChar(void);
	bool IsSelectingFeats();

	bool SpellsNeedReset();
	void SpellsNeedResetSet(bool value);

	// utilities
	bool SpellIsAlreadyKnown(int spEnum, int spellClass);
	bool SpellIsForbidden(int spEnum, int spellClass);


	void BonusFeatsClear();
	void SetBonusFeats(std::vector<FeatInfo> & fti);
	
	int GetNewLvl(Stat classCode = stat_level);
	bool IsClassBonusFeat(feat_enums feat);
	bool IsBonusFeatDisregardingPrereqs(feat_enums feat);

	eastl::vector<string> levelLabels;
	eastl::vector<string> spellLevelLabels;

protected:
	std::vector<KnownSpellInfo> mSpellInfo;
	std::vector<KnownSpellInfo> mAvailableSpells; // spells available for learning
	std::vector<FeatInfo> mExistingFeats, mSelectableFeats, mMultiSelectFeats, mMultiSelectMasterFeats, mBonusFeats;

	bool mSpellsNeedReset;

	bool mIsNewChar = false; // used in differentiating new character generation vs. levelup
};

extern Chargen chargen;

int HookedFeatMultiselectSub_101A8080(feat_enums feat);