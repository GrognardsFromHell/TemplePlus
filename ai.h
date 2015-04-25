
#pragma once

#include "stdafx.h"
#include "common.h"
#include "tig\tig_mes.h"


struct Pathfinding;
struct AiTacticDef;
struct SpellSystem;
struct ActionSequenceSystem;
struct D20System;
struct CombatSystem;
struct AiStrategy;
struct AiTactic;

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

struct AiSystem : AddressTable
{
	AiStrategy ** aiStrategies;
	AiTacticDef * aiTacticDefs;
	uint32_t * aiStrategiesNum;
	CombatSystem * combat;
	D20System  * d20;
	ActionSequenceSystem * actSeq;
	SpellSystem * spell;
	Pathfinding * pathfinding;
	AiSystem();
	void aiTacticGetConfig(int i, AiTactic* aiTac, AiStrategy* aiStrat);
	uint32_t aiStrategyParse(objHndl objHnd, objHndl target);
	
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

private:
	void (__cdecl *_ShitlistAdd)(objHndl npc, objHndl target);
	void (__cdecl *_AiRemoveFromList)(objHndl npc, objHndl target, int listType);	
	void (__cdecl *_FleeAdd)(objHndl npc, objHndl target);
	void (__cdecl *_AiSetCombatStatus)(objHndl npc, int status, objHndl target, int unk);
	void (__cdecl *_StopAttacking)(objHndl npc);
};

extern AiSystem aiSys;


struct AiTacticDef
{
	char * name;
	uint32_t(__cdecl * aiFunc)(AiTactic *);
	uint32_t pad;
};

