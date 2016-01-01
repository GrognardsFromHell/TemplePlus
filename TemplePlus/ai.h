
#pragma once

#include "common.h"
#include "tig\tig_mes.h"
#include "d20.h"


#define AI_TACTICS_NEW_SIZE 100

struct Pathfinding;
struct AiTacticDef;
struct SpellSystem;
struct ActionSequenceSystem;
struct D20System;
struct CombatSystem;
struct AiStrategy;
struct AiTactic;
struct AiPacket
{
	objHndl obj;
	objHndl target;
	int aiFightStatus; // 0 - none;  1 - fighting;  2 - fleeing  ;  3 - surrendered ; 4 - finding help
	int aiState2; // 3 - use skill,  4 - scout point
	int field18;
	SkillEnum skillEnum;
	objHndl scratchObj;
	objHndl leader;
	int field30;
	D20SpellData spellData;
	int field3C;
	SpellPacketBody spellPktBod;

	AiPacket(objHndl objIn);
};

enum class AiFlag : uint64_t {
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

struct AiSystem : temple::AddressTable
{
	AiStrategy ** aiStrategies;
	AiTacticDef * aiTacticDefs;
	AiTacticDef aiTacticDefsNew[AI_TACTICS_NEW_SIZE];
	uint32_t * aiStrategiesNum;
	CombatSystem * combat;
	D20System  * d20;
	ActionSequenceSystem * actSeq;
	SpellSystem * spell;
	Pathfinding * pathfinding;
	AiSystem();
	void aiTacticGetConfig(int i, AiTactic* aiTac, AiStrategy* aiStrat);
	uint32_t AiStrategyParse(objHndl objHnd, objHndl target);
	
	bool HasAiFlag(objHndl npc, AiFlag flag);
	void SetAiFlag(objHndl npc, AiFlag flag);
	void ClearAiFlag(objHndl npc, AiFlag flag);

	void ShitlistAdd(objHndl npc, objHndl target);
	void ShitlistRemove(objHndl npc, objHndl target);
	void FleeAdd(objHndl npc, objHndl target);
	void StopAttacking(objHndl npc);
	
	objHndl GetCombatFocus(objHndl npc);
	objHndl GetWhoHitMeLast(objHndl npc);
	void SetCombatFocus(objHndl npc, objHndl target);
	void SetWhoHitMeLast(objHndl npc, objHndl target);

	int TargetClosest(AiTactic * aiTac);
	int TargetThreatened(AiTactic * aiTac);
	int Approach(AiTactic* aiTac);
	int CastParty(AiTactic * aiTac);
	int CoupDeGrace(AiTactic * aiTac);
	int ChargeAttack(AiTactic * aiTac);
	int PickUpWeapon(AiTactic* aiTac);
	int BreakFree(AiTactic* aiTac);
	int GoMelee(AiTactic* aiTac);
	int Sniper(AiTactic *aiTac);
	void UpdateAiFightStatus(objHndl objIn, int* aiState, objHndl* target);
	int UpdateAiFlags(objHndl ObjHnd, int aiFightStatus, objHndl target, int *soundMap);
	void StrategyTabLineParseTactic(AiStrategy*, char * tacName, char * middleString, char* spellString);
	int StrategyTabLineParser(TabFileStatus* tabFile, int n, char ** strings);
	int AiOnInitiativeAdd(objHndl obj);
	AiCombatRole GetRole(objHndl obj);
	BOOL AiFiveFootStepAttempt(AiTactic * aiTac);

	void RegisterNewAiTactics();
	unsigned int Asplode(AiTactic * aiTactic);
	unsigned int WakeFriend(AiTactic* aiTac);
	int Default(AiTactic* aiTac);
	int AttackThreatened(AiTactic* aiTac);
	void AiTurnSthg_1005EEC0(objHndl obj);
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


