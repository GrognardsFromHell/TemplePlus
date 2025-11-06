
#pragma once

#include "common.h"
#include "tig\tig_mes.h"
#include "tig\tig_tabparser.h"
#include "d20.h"


#define AI_TACTICS_NEW_SIZE 100

struct TimeEvent;
struct Pathfinding;
struct AiTacticDef;
struct LegacySpellSystem;
struct ActionSequenceSystem;
struct LegacyD20System;
struct LegacyCombatSystem;
struct AiStrategy;
struct AiTactic;
struct AiSpellList
{
	std::vector<int> spellEnums;
	std::vector<D20SpellData> spellData;
};
struct AiPacket
{
	objHndl obj;
	objHndl target;
	int aiFightStatus; // see AiFightStatus
	int aiState2; // 1 - cast spell, 3 - use skill,  4 - scout point
	int spellEnum;
	SkillEnum skillEnum;
	objHndl scratchObj;
	objHndl leader;
	int soundMap;
	D20SpellData spellData;
	int field3C;
	SpellPacketBody spellPktBod;

	AiPacket(objHndl objIn);

	BOOL PacketCreate();
	BOOL WieldBestItem();
	BOOL SelectHealSpell();
	bool ShouldUseHealSpellOn(objHndl handle, BOOL healSpellRecommended);
	BOOL LookForEquipment();


	
	void FightStatusUpdate();
		bool HasScoutStandpoint();
		bool ScoutPointSetState();
		void ChooseRandomSpell_RegardInvulnerableStatus();
		objHndl PickRandomFromAiList();
		objHndl ConsiderCombatFocus();


	
	void ProcessCombat();
		void ProcessBackingOff();
		void ThrowSpell();
		void UseItem();
		void MoveToScoutPoint();
		void ScoutPointAttack();

		void DoWaypoints();
		void ProcessFighting();
		void FleeingStatusRefresh();
};
struct AiParamPacket
{ // most of this stuff is arcanum leftovers
	int hpPercentToTriggerFlee;
	int numPeopleToTriggerFlee;
	int lvlDiffToTriggerFlee;
	int pcHpPercentToPreventFlee;
	int fleeDistanceFeet;
	int reactionLvlToRefuseFollowingPc;
	int unused7;
	int unused8;
	int maxLvlDiffToAgreeToJoin;
	int reactionLoweredOnFriendlyFire;
	int hostilityThreshold;
	int unused12;
	int offensiveSpellChance;
	int defensiveSpellChance;
	int healSpellChance;
	int combatMinDistanceFeet;
	int canOpenPortals;

	void GetForCritter(objHndl handle);
};

enum AiFlag : uint64_t {
	FindingHelp = 0x1,
	WaypointDelay = 0x2,
	WaypointDelayed = 0x4,
	RunningOff = 0x8,
	Fighting = 0x10,
	CheckGrenade = 0x20,
	CheckWield = 0x40,
	CheckWeapon = 0x80,
	LookForWeapon = 0x100,
	LookForArmor = 0x200,
	LookForAmmo = 0x400,
	HasSpokenFlee = 0x800,
};

enum AiCombatRole: int32_t
{
	general = 0,
	fighter ,
	defender ,
	caster ,
	healer ,
	flanker ,
	sniper ,
	magekiller ,
	berzerker ,
	tripper,
	special
};

enum AiFightStatus : uint32_t {
	AIFS_NONE = 0,
	AIFS_FIGHTING = 1,
	AIFS_FLEEING =2,
	AIFS_SURRENDERED = 3,
	AIFS_FINDING_HELP = 4,
	AIFS_BEING_DRAWN = 5 // New TODO for Harpy Song
};

struct AiSystem : temple::AddressTable
{
	//AiStrategy ** aiStrategies;
	std::vector<AiStrategy> aiStrategies;
	IdxTableWrapper<AiStrategy> * aiCustomStrats;

	AiTacticDef * aiTacticDefs;
	AiTacticDef aiTacticDefsNew[AI_TACTICS_NEW_SIZE];
	
	static AiParamPacket * aiParams;
	uint32_t * aiStrategiesNum;
	LegacyCombatSystem * combat;
	LegacyD20System  * d20;
	ActionSequenceSystem * actSeq;
	LegacySpellSystem * spell;
	Pathfinding * pathfinding;
	
	AiSystem();

	AiStrategy* GetAiStrategy(uint32_t stratId);

	void aiTacticGetConfig(int i, AiTactic* aiTac, AiStrategy* aiStrat);
	uint32_t StrategyParse(objHndl objHnd, objHndl target);
	uint32_t AiStrategDefaultCast(objHndl objHnd, objHndl target, D20SpellData* spellData, SpellPacketBody* spellPkt);
	
	bool IsRunningOff(objHndl handle) const;
	bool HasAiFlag(objHndl npc, AiFlag flag) const;
	void SetAiFlag(objHndl npc, AiFlag flag);
	void ClearAiFlag(objHndl npc, AiFlag flag);
	AiParamPacket GetAiParams(objHndl obj);

	void ShitlistAdd(objHndl npc, objHndl target);
	void ShitlistRemove(objHndl npc, objHndl target);
	BOOL AiListFind(objHndl aiHandle, objHndl tgt, int typeToFind); // search for tgt in ai list field. 0 is for enemies, 1 is for allies
	void FleeAdd(objHndl npc, objHndl target);
	void StopAttacking(objHndl npc);
	void StopFleeing(objHndl npc);
	void ProvokeHostility(objHndl agitator, objHndl provokedNpc, int rangeType, int flags); // rangeType - 0 is for 5 tiles, 1 is for 10 tiles, 2 is for 20 tiles, and 3 is unlimited
	void TryLockOnTarget(objHndl handle, objHndl leader, objHndl target, int isAlways1, int isFlags1Set, int skipAiStatusUpdate);
	void TargetLockUnset(objHndl handle);
	BOOL RefuseFollowCheck(objHndl handle, objHndl leader);

	objHndl GetCombatFocus(objHndl npc);
	objHndl GetWhoHitMeLast(objHndl npc);
	BOOL ConsiderTarget(objHndl obj, objHndl tgt); // checks if it's a good target
	objHndl GetFriendsCombatFocus(objHndl handle, objHndl friendHandle, objHndl leader);
	objHndl FindSuitableTarget(objHndl handle); // was 0x1005CED0;
	int CannotHate(objHndl aiHandle, objHndl triggerer, objHndl aiLeader);
	int WillKos(objHndl aiHandle, objHndl triggerer); // does the triggerer provoke KOS hostility
	/*
	// returns 4 if aiHandle == tgt
	// returns 3 if  tgt == leader   or   tgt in party and (leader or aiHandle in party)
	// otherwise:
	// returns 0 if aiHandle is charmed
	// otherwise:
	// returns 1 if allegiance shared
	*/
	int GetAllegianceStrength(objHndl aiHandle, objHndl tgt);
	BOOL CannotHear(objHndl handle, objHndl tgt, int tileRangeIdx); // checks if a critter (handle) can observe tgt up to a specified range; regards magic effects such as Invisibility to Undead / Animals, and also makes a hidden listen check
	void SetCombatFocus(objHndl npc, objHndl target);
	void SetWhoHitMeLast(objHndl npc, objHndl target);
	void GetAiFightStatus(objHndl obj, AiFightStatus* status, objHndl * target);
	
	void AlertAllies(objHndl handle, objHndl alertFrom, int rangeIdx);
		void AlertAlly(objHndl handle, objHndl alertFrom, objHndl alertDispatcher, int rangeIdx);
	void AlertAllies2(objHndl handle, objHndl alertFrom);
	/*
	 updates AI flags based on a "Should flee" check
	 originally 10057A70
	*/
	void FightOrFlight(objHndl obj, objHndl tgt);

	
	/*
		Updates AI flags and handles fleeing using the below functions
	*/
	void FightStatusProcess(objHndl obj, objHndl newTgt);

		/*
			Plays the "Fleeing" voice line, and sequences a move action away from the fleeingFrom object
		*/
		void FleeProcess(objHndl obj, objHndl fleeingFrom);
	
		int UpdateAiFlags(objHndl ObjHnd, AiFightStatus aiFightStatus, objHndl target, int *soundMap);

	// AI Tactic functions
	// These generate action sequences for the AI and/or change the AI target
	int Approach(AiTactic* aiTac);
	int ApproachSingle(AiTactic* aiTac);
	int AttackThreatened(AiTactic* aiTac);
	int BreakFree(AiTactic* aiTac);
	int CastParty(AiTactic * aiTac);
	int CoupDeGrace(AiTactic * aiTac);
	int ChargeAttack(AiTactic * aiTac);
	int Default(AiTactic* aiTac);
	BOOL DefaultCast(AiTactic *aiTac);
	int Flank(AiTactic * aiTac);
	int GoMelee(AiTactic* aiTac);
	int PickUpWeapon(AiTactic* aiTac);	
	int Sniper(AiTactic *aiTac);
	BOOL ImprovePosition(AiTactic *aiTac);
	int TargetClosest(AiTactic * aiTac);
	BOOL TargetDamaged(AiTactic *aiTac);
	BOOL TargetFriendHurt(AiTactic* aiTac);
	int TargetThreatened(AiTactic * aiTac);
	BOOL UsePotion(AiTactic *aiTac);
	BOOL UseItem(AiTactic* aiTac);

	unsigned int Asplode(AiTactic * aiTactic);
	unsigned int WakeFriend(AiTactic* aiTac);
	int AiGoto(AiTactic* aiTac);
	int AiHalt(AiTactic* aiTac);
	int AiStop(AiTactic* aiTac);
	int AiTargetObj(AiTactic* aiTac);
	int AiTotalDefence(AiTactic* aiTac);
	int AiPythonAction(AiTactic* aiTac);
	int AiD20Action(AiTactic* aiTac);

	// Init
	void StrategyTabLineParseTactic(AiStrategy*, const char * tacName, const char * middleString, const char* spellString);
	void StrategyTabLineParseTacticMiddleString(AiStrategy* aiStrat, int idx, const char* middleString);
	void ParseStrategyLine(AiStrategy& stratOut, const std::vector<string>& strings);
	int StrategyTabLineParser(const TigTabParser* tabFile, int n, char ** strings);
	void InitCustomStrategies();
	void SetCustomStrategy(objHndl handle, const std::vector<std::string>& stringVector, int save);
	
	// Custom strats save/load
	bool CustomStrategiesSave();
	bool CustomStrategiesLoad();

	int AiOnInitiativeAdd(objHndl obj);
	AiCombatRole GetRole(objHndl obj);
	BOOL AiFiveFootStepAttempt(AiTactic * aiTac);

	void RegisterNewAiTactics();
	int GetStrategyIdx(const char* stratName) const; // get the strategy.tab index for given strategy name
	void AiListRemove(const objHndl& handle, const objHndl& tgt, int aiType);
	

	static int GetAiSpells(AiSpellList* aiSpell, objHndl obj, AiSpellType aiSpellType);
	static int ChooseRandomSpell(AiPacket* aiPkt);
	static int ChooseRandomSpellFromList(AiPacket* aiPkt, AiSpellList *);
	static int ChooseRandomSpellFromList(AiTactic * aiTac, AiSpellList * aiSpellList);

	BOOL IsPcUnderAiControl(objHndl handle);
	void AiProcess(objHndl obj);
	bool AiProcessHandleConfusion(objHndl handle, int confusionState);
	bool AiProcessPc(objHndl handle);

	int AiTimeEventExpires(TimeEvent* evt);
	
private:
	void (__cdecl *_ShitlistAdd)(objHndl npc, objHndl target);
	void (__cdecl *_AiRemoveFromList)(objHndl npc, objHndl target, int listType);	
	void (__cdecl *_FleeAdd)(objHndl npc, objHndl target);
	void (__cdecl *_StopFleeing)(objHndl npc);
	void (__cdecl *_StopAttacking)(objHndl npc);
	bool Is5FootStepWorth(AiTactic * aiTac);
	IdxTable< AiStrategy> mAiStrategiesCustom;
	std::vector<std::vector<std::string>> mAiStrategiesCustomSrc;
};

extern AiSystem aiSys;

unsigned int _AiAsplode(AiTactic* aiTac);
unsigned int _AiWakeFriend(AiTactic* aiTac);
unsigned int _AiGoto(AiTactic* aiTac);
unsigned int _AiHalt(AiTactic* aiTac);
unsigned int _AiStop(AiTactic* aiTac);
unsigned int _AiTargetObj(AiTactic* aiTac);
unsigned int _AiTotalDefence(AiTactic* aiTac);
unsigned int _AiPythonAction(AiTactic* aiTac);
unsigned int _AiD20Action(AiTactic* aiTac);
unsigned int _AiUseItem(AiTactic* aiTac);
unsigned int _AiApproachSingle(AiTactic* aiTac);
