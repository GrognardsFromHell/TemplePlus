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
};

extern AiSystem aiSys;


struct AiTacticDef
{
	char * name;
	uint32_t(__cdecl * aiFunc)(AiTactic *);
	uint32_t pad;
};

uint32_t _aiStrategyParse(objHndl objHnd, objHndl target);