#include "stdafx.h"
#include "common.h"
#include "ai.h"
#include "d20.h"
#include "combat.h"
#include "obj.h"
#include "action_sequence.h"
#include "spell.h"
#include "temple_functions.h"
#include "pathfinding.h"

class AiReplacements : public TempleFix
{
	macTempleFix(AI Replacements)
	{
		logger->info("Replacing AI functions...");
		macReplaceFun(100E50C0, _aiStrategyParse)
	}
} aiReplacements;


#pragma region AI System Implementation
struct AiSystem aiSys;

struct AiTactic {
	AiTacticDef * aiTac;
	uint32_t field4;
	objHndl performer;
	objHndl target;
	int32_t tacIdx;
	D20SpellData d20SpellData;
	uint32_t field24;
	SpellPacketBody spellPktBody;
};


struct AiStrategy
{
	uint32_t field0;
	AiTacticDef * aiTacDefs[20];
	uint32_t field54[20];
	SpellStoreData spellsKnown[20];
	uint32_t numTactics;
};

const auto TestSizeOfAiTactic = sizeof(AiTactic); // should be 2832 (0xB10 )
const auto TestSizeOfAiStrategy = sizeof(AiStrategy); // should be 808 (0x324)

AiSystem::AiSystem()
{
	combat = &combatSys;
	d20 = &d20Sys;
	actSeq = &actSeqSys;
	spell = &spellSys;
	pathfinding = &pathfindingSys;
	macRebase(aiStrategies, 11868F3C)
	macRebase(aiStrategiesNum,11868F40)
	macRebase(aiTacticDefs, 102E4398)
}

void AiSystem::aiTacticGetConfig(int tacIdx, AiTactic* aiTacOut, AiStrategy* aiStrat)
{
	SpellPacketBody * spellPktBody = &aiTacOut->spellPktBody;
	spell->spellPacketBodyReset(&aiTacOut->spellPktBody);
	aiTacOut->aiTac = aiStrat->aiTacDefs[tacIdx];
	aiTacOut->field4 = aiStrat->field54[tacIdx];
	aiTacOut->tacIdx = tacIdx;
	int32_t spellEnum = aiStrat->spellsKnown[tacIdx].spellEnum;
	spellPktBody->spellEnum = spellEnum;
	spellPktBody->spellEnumOriginal = spellEnum;
	if (spellEnum != -1)
	{
		aiTacOut->spellPktBody.objHndCaster = aiTacOut->performer;
		aiTacOut->spellPktBody.casterClassCode = aiStrat->spellsKnown[tacIdx].classCode;
		aiTacOut->spellPktBody.spellKnownSlotLevel = aiStrat->spellsKnown[tacIdx].spellLevel;
		spell->spellPacketSetCasterLevel(spellPktBody);
		d20->D20ActnSetSpellData(&aiTacOut->d20SpellData, spellEnum, spellPktBody->casterClassCode, spellPktBody->spellKnownSlotLevel, 0xFF, spellPktBody->metaMagicData);
	}
}

uint32_t AiSystem::aiStrategyParse(objHndl objHnd, objHndl target)
{
	AiTactic aiTac;
	combat->enterCombat(objHnd);
	AiStrategy* aiStrat = &(*aiStrategies)[objects.getInt32(objHnd, obj_f_critter_strategy)];
	if (!actSeq->TurnBasedStatusInit(objHnd)) return 0;
	
	actSeq->curSeqReset(objHnd);
	d20->GlobD20ActnInit();
	spell->spellPacketBodyReset(&aiTac.spellPktBody);
	aiTac.performer = objHnd;
	aiTac.target = target;

	for (uint32_t i = 0; i < aiStrat->numTactics; i++)
	{
		aiTacticGetConfig(i, &aiTac, aiStrat);
		hooked_print_debug_message("\n%s attempting %s...\n", description._getDisplayName(objHnd, objHnd), aiTac.aiTac->name);
		auto aiFunc = aiTac.aiTac->aiFunc;
		if (!aiFunc) continue;
		if (aiFunc(&aiTac)) {
			actSeq->sequencePerform();
			return 1;
		}
	}

	// if none of those work, use default
	aiTac.aiTac = &aiTacticDefs[0];
	aiTac.field4 = 0;
	aiTac.tacIdx = -1;
	hooked_print_debug_message("\n%s attempting %s...\n", description._getDisplayName(objHnd, objHnd), aiTac.aiTac->name);
	assert(aiTac.aiTac != nullptr);
	if (aiTac.aiTac->aiFunc(&aiTac))
	{
		actSeq->sequencePerform();
		return 1;
	}
	objHndl pathablePartyMember = pathfinding->canPathToParty(objHnd);
	if (pathablePartyMember)
	{
		if (aiTac.aiTac->aiFunc(&aiTac))
		{
			actSeq->sequencePerform();
			return 1;
		}
	}
	return 0;
}
#pragma endregion

#pragma region AI replacement functions


#pragma endregion 