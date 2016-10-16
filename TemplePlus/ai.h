
#pragma once

#include "common.h"
#include "tig\tig_mes.h"
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
	int field30;
	D20SpellData spellData;
	int field3C;
	SpellPacketBody spellPktBod;

	AiPacket(objHndl objIn);
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
	int healSpellChange;
	int combatMinDistanceFeet;
	int canOpenPortals;
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
	fighter = 1,
	defender = 2,
	caster = 3,
	healer = 4,
	flanker = 5,
	sniper = 6,
	magekiller = 7,
	berzerker = 8,
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
	AiStrategy ** aiStrategies;
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

	void aiTacticGetConfig(int i, AiTactic* aiTac, AiStrategy* aiStrat);
	uint32_t AiStrategyParse(objHndl objHnd, objHndl target);
	uint32_t AiStrategDefaultCast(objHndl objHnd, objHndl target, D20SpellData* spellData, SpellPacketBody* spellPkt);
	
	bool HasAiFlag(objHndl npc, AiFlag flag);
	void SetAiFlag(objHndl npc, AiFlag flag);
	void ClearAiFlag(objHndl npc, AiFlag flag);
	AiParamPacket GetAiParams(objHndl obj);

	void ShitlistAdd(objHndl npc, objHndl target);
	void ShitlistRemove(objHndl npc, objHndl target);
	void FleeAdd(objHndl npc, objHndl target);
	void StopAttacking(objHndl npc);
	void ProvokeHostility(objHndl agitator, objHndl provokedNpc, int rangeType, int flags); // rangeType - 0 is for 5 tiles, 1 is for 10 tiles, 2 is for 20 tiles, and 3 is unlimited
	
	objHndl GetCombatFocus(objHndl npc);
	objHndl GetWhoHitMeLast(objHndl npc);
	BOOL ConsiderTarget(objHndl obj, objHndl tgt); // checks if it's a good target
	void SetCombatFocus(objHndl npc, objHndl target);
	void SetWhoHitMeLast(objHndl npc, objHndl target);
	void GetAiFightStatus(objHndl obj, AiFightStatus* status, objHndl * target);
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
	int AttackThreatened(AiTactic* aiTac);
	int BreakFree(AiTactic* aiTac);
	int CastParty(AiTactic * aiTac);
	int CoupDeGrace(AiTactic * aiTac);
	int ChargeAttack(AiTactic * aiTac);
	int Default(AiTactic* aiTac);
	int Flank(AiTactic * aiTac);
	int GoMelee(AiTactic* aiTac);
	int PickUpWeapon(AiTactic* aiTac);	
	int Sniper(AiTactic *aiTac);
	int TargetClosest(AiTactic * aiTac);
	int TargetThreatened(AiTactic * aiTac);

	unsigned int Asplode(AiTactic * aiTactic);
	unsigned int WakeFriend(AiTactic* aiTac);


	void StrategyTabLineParseTactic(AiStrategy*, char * tacName, char * middleString, char* spellString);
	int StrategyTabLineParser(TabFileStatus* tabFile, int n, char ** strings);
	int AiOnInitiativeAdd(objHndl obj);
	AiCombatRole GetRole(objHndl obj);
	BOOL AiFiveFootStepAttempt(AiTactic * aiTac);

	void RegisterNewAiTactics();
	int GetStrategyIdx(const char* stratName) const; // get the strategy.tab index for given strategy name
	static int GetAiSpells(AiSpellList* aiSpell, objHndl obj, AiSpellType aiSpellType);
	static int ChooseRandomSpell(AiPacket* aiPkt);
	static int ChooseRandomSpellFromList(AiPacket* aiPkt, AiSpellList *);
	
	void AiProcess(objHndl obj);

	int AiTimeEventExpires(TimeEvent* evt);

private:
	void (__cdecl *_ShitlistAdd)(objHndl npc, objHndl target);
	void (__cdecl *_AiRemoveFromList)(objHndl npc, objHndl target, int listType);	
	void (__cdecl *_FleeAdd)(objHndl npc, objHndl target);
	void (__cdecl *_AiSetCombatStatus)(objHndl npc, int status, objHndl target, int unk);
	void (__cdecl *_StopAttacking)(objHndl npc);
	
};

extern AiSystem aiSys;

unsigned int _AiAsplode(AiTactic* aiTac);
unsigned int _AiWakeFriend(AiTactic* aiTac);


