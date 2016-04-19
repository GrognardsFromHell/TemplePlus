#include "stdafx.h"
#include "common.h"
#include "d20.h"
#include "action_sequence.h"
#include "obj.h"
#include "temple_functions.h"
#include "tig\tig_mes.h"
#include "float_line.h"
#include "combat.h"
#include "description.h"
#include "location.h"
#include "pathfinding.h"
#include "turn_based.h"
#include "config/config.h"
#include "critter.h"
#include "util/fixes.h"
#include "condition.h"
#include "ui/ui_picker.h"
#include "ui/ui_turn_based.h"
#include "party.h"
#include "history.h"
#include "ui/ui.h"
#include "ai.h"
#include "ui/ui_intgame_turnbased.h"
#include "d20_obj_registry.h"
#include "anim.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"


static struct ActnSeqAddresses : temple::AddressTable {

	int(__cdecl *TouchAttackAddToSeq)(D20Actn* d20Actn, ActnSeq* actnSeq, TurnBasedStatus* turnBasedStatus);
	int(__cdecl *ActionAddToSeq)();
	int(__cdecl*ActionSequenceChecksWithPerformerLocation)();
	void(__cdecl* ActionSequenceRevertPath)(int d20ActnNum);
	void(__cdecl*Counterspell_sthg)(ReadiedActionPacket* readiedAction);
	PickerArgs * actSeqPicker;
	D20Actn * actSeqPickerAction;
	ReadiedActionPacket * readiedActionCache;
	ActnSeq** actSeqInterrupt;
	int* seqSthg_10B3D59C;
	int* seqSthg_10B3D5C4;
	int * cursorState;
	int *aooShaderId;
	int * aooShaderLocationsNum;
	AooShaderPacket * aooShaderLocations; // size 64 array
	BOOL(*IsLastSimulsPerformer)(objHndl obj);

	ActnSeqAddresses()
	{
		rebase(Counterspell_sthg, 0x10091710);
		rebase(ActionSequenceRevertPath, 0x10093180);
		rebase(TouchAttackAddToSeq, 0x10096760);
		rebase(IsLastSimulsPerformer, 0x10096FA0);
		rebase(ActionSequenceChecksWithPerformerLocation, 0x10097000);
		rebase(ActionAddToSeq, 0x10097C20);

		rebase(aooShaderLocations, 0x10B3B948);
		rebase(aooShaderLocationsNum, 0x10B3D598);
		rebase(seqSthg_10B3D59C, 0x10B3D59C);
		rebase(cursorState, 0x10B3D5AC);
		rebase(seqSthg_10B3D5C4, 0x10B3D5C4);

		rebase(readiedActionCache, 0x1186A900);
		rebase(actSeqPickerAction, 0x118CD400);
		rebase(actSeqPicker, 0x118CD460);
		rebase(actSeqInterrupt, 0x118CD574);

		rebase(aooShaderId, 0x1186A8E8);

	}

	
}addresses;


static class ActnSeqReplacements : public TempleFix
{
public:
	static int(__cdecl* orgChooseTargetCallback)(void *);
	static int(__cdecl*orgSeqRenderFuncMove) (D20Actn* d20a, UiIntgameTurnbasedFlags flags);
	static void AooShaderPacketAppend(LocAndOffsets* loc, int aooShaderId);
	//static int(__cdecl*orgSeqRenderAooMovement)(D20Actn*, UiIntgameTurnbasedFlags);
	static int SeqRenderAooMovement(D20Actn*, UiIntgameTurnbasedFlags);
	static int ChooseTargetCallback(void* a)
	{
		logger->debug("Choose Target Callback");
		return orgChooseTargetCallback(a); // doesn't seem to be used in practice???
	}

	static int SequenceSwitch(objHndl obj)
	{
		return actSeqSys.SequenceSwitch(obj);
	};

	static void GreybarReset()
	{
		actSeqSys.GreybarReset();
	}

	static int ActionFrameProcess(objHndl obj)
	{
		return actSeqSys.ActionFrameProcess(obj);
	}

	static void PerformOnAnimComplete(objHndl obj, int animId)
	{
		return actSeqSys.PerformOnAnimComplete(obj, animId);
	}

	static void TurnStart(objHndl obj);
	
	static void(__cdecl*orgTurnStart)(objHndl obj);


	static ActionErrorCode AddToSeqSimple(D20Actn*, ActnSeq*, TurnBasedStatus*);
	static ActionErrorCode AddToSeqWithTarget(D20Actn*, ActnSeq*, TurnBasedStatus*);

	static int ActionAddToSeq();
	static void ActSeqGetPicker();

	static int SeqRenderFuncMove(D20Actn* d20a, UiIntgameTurnbasedFlags flags);
	void ActSeqApply();
	void NaturalAttackOverwrites();

	void apply() override{
		ActSeqApply();
		NaturalAttackOverwrites();
		replaceFunction(0x1008B140, SequenceSwitch);
		orgSeqRenderFuncMove = replaceFunction(0x1008D090, SeqRenderFuncMove);
		//orgSeqRenderAooMovement =
		replaceFunction(0x10091D60, SeqRenderAooMovement);
		replaceFunction(0x100933F0, ActionFrameProcess);
		orgTurnStart = replaceFunction(0x10099430, TurnStart);
		replaceFunction(0x100999E0, GreybarReset);
		replaceFunction(0x10099CF0, PerformOnAnimComplete);

		// AddToSeqSpellCast
		replaceFunction<ActionErrorCode(D20Actn*, ActnSeq*, TurnBasedStatus*)>(0x100958A0,[](D20Actn* d20a, ActnSeq* seq, TurnBasedStatus* tbStat)->ActionErrorCode
		{
			if (d20Sys.d20Query(d20a->d20APerformer, DK_QUE_Prone))
			{
				D20Actn d20aGetup = *d20a;
				d20aGetup.d20ActType = D20A_STAND_UP;
				seq->d20ActArray[ seq->d20ActArrayNum++] = d20aGetup;
			}

			uint32_t spellEnum;
			d20Sys.ExtractSpellInfo(&d20a->d20SpellData, &spellEnum, nullptr, nullptr, nullptr, nullptr ,nullptr);
			SpellEntry spellEntry;
			auto srcResult = spellSys.spellRegistryCopy(spellEnum, &spellEntry);
			if (srcResult
				&& spellEntry.spellRangeType == SpellRangeType::SRT_Touch
				&& static_cast<UiPickerType>(spellEntry.modeTargetSemiBitmask) == UiPickerType::Single
				&& !(seq->ignoreLos & 1)
			)
			{
				int dummy = 1;
				return (ActionErrorCode)actSeqSys.AddToSeqWithTarget(d20a, seq, tbStat);
			}
			seq->d20ActArray[seq->d20ActArrayNum++] = *d20a;
			return ActionErrorCode::AEC_OK;
			
		});
		
		replaceFunction<void(__cdecl)(objHndl)>(0x100934E0, [](objHndl handle)
		{
			actSeqSys.ActionTypeAutomatedSelection(handle);
		});
		replaceFunction<int(__cdecl)()>(0x10091580, []()
		{
			return actSeqSys.ReadyVsApproachOrWithdrawalCount();
		});
	}
} actSeqReplacements;

int(__cdecl* ActnSeqReplacements::orgChooseTargetCallback)(void * );
int(__cdecl* ActnSeqReplacements::orgSeqRenderFuncMove) (D20Actn* d20a, UiIntgameTurnbasedFlags flags);
void(__cdecl* ActnSeqReplacements::orgTurnStart)(objHndl obj);



#pragma region Action Sequence System Implementation

ActionSequenceSystem actSeqSys;

ActionSequenceSystem::ActionSequenceSystem()
{
	d20 = &d20Sys;
	combat = &combatSys;
	pathfinding = &pathfindingSys;
	location = &locSys;
	object = &objects;
	turnbased = &tbSys;
	rebase(actSeqCur, 0x1186A8F0);
	rebase(tbStatus118CD3C0,0x118CD3C0); 

	rebase(actnProcState, 0x10B3D5A4);


	
	
	
	rebase(actionMesHandle, 0x10B3BF48);
	rebase(seqFlag_10B3D5C0, 0x10B3D5C0);
	rebase(actSeqPickerActive, 0x10B3D5A0);
	rebase(actSeqArray, 0x118A09A0);


	
	
	rebase(numSimultPerformers, 0x10B3D5B8);
	rebase(simultPerformerQueue, 0x118A06C0);

	rebase(simulsIdx,0x10B3D5BC); 

	
	rebase(_actionPerformProjectile, 0x1008AC70);
	rebase(_actSeqSpellHarmful, 0x1008AD10);
	rebase(_sub_1008BB40, 0x1008BB40);
	rebase(getRemainingMaxMoveLength, 0x1008B8A0);
	//rebase(TrimPathToRemainingMoveLength, 0x1008B9A0);

	rebase(_CrossBowSthgReload_1008E8A0, 0x1008E8A0);

	rebase(ActionCostReload, 0x100903B0);

	rebase(_TurnBasedStatusUpdate, 0x10093950);
	rebase(_combatTriggerSthg, 0x10093890);
	rebase(_ProcessPathForReadiedActions,      0x100939D0); 

	rebase(_actionPerformRecursion, 0x100961C0);
	rebase(_sub_10096450,           0x10096450);


	rebase(_AOOSthgSub_10097D50, 0x10097D50);

	rebase(_AOOSthg2_100981C0, 0x100981C0);
	rebase(_curSeqNext,        0x10098ED0);

	rebase(_InterruptNonCounterspell, 0x10099320);
	rebase(_InterruptCounterspell, 0x10099360);
	
	rebase(seqPickerTargetingType,0x118CD3B8); 
	rebase(seqPickerD20ActnType,0x118A0980); 
	rebase(seqPickerD20ActnData1,0x118CD570); 

	int _transMatrix[7*5] = { 0, - 1, -1, -1, -1,
		1, 0, -1, -1, -1,
		2, 0, 0, -1, -1,
		3, 0, 0, 0, -1,
		4, 2, 1, -1, 0,
		5, 2, 2, -1, 0,
		6, 5, 2, -1, 3 };
	memcpy(turnBasedStatusTransitionMatrix, _transMatrix, sizeof(turnBasedStatusTransitionMatrix));
}


void ActionSequenceSystem::curSeqReset(objHndl obj) 
{ // initializes the sequence pointed to by actSeqCur and assigns it to objHnd
	ActnSeq * curSeq = *actSeqCur;
	PathQueryResult * pqr;

	// release path finding queries 
	for (auto i = 0; i < curSeq->d20ActArrayNum; i++)
	{
		pqr = curSeq->d20ActArray[i].path;
		if (pathfinding->pathQueryResultIsValid(pqr))	pqr->occupiedFlag = 0;
		curSeq->d20ActArray[i].path = nullptr;
	}

	curSeq->d20ActArrayNum = 0;
	curSeq->d20aCurIdx = -1;
	curSeq->prevSeq = nullptr;
	curSeq->interruptSeq = nullptr;
	curSeq->seqOccupied = SEQF_NONE;
	if (obj != d20->globD20Action->d20APerformer)
	{
		*seqPickerTargetingType = D20TC_Invalid;
		*seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
		*seqPickerD20ActnData1 = 0;
	}

	d20->globD20Action->d20APerformer = obj;
	d20->D20ActnInit(obj, d20->globD20Action);
	curSeq->performer = obj;
	curSeq->targetObj = 0;
	location->getLocAndOff(obj, &curSeq->performerLoc);
	curSeq->ignoreLos = 0;
	*seqFlag_10B3D5C0 = 0;
}

void ActionSequenceSystem::ActSeqSpellReset() const
{
	auto actSeqSpellReset = temple::GetRef<void(__cdecl)()>(0x100930A0);
	actSeqSpellReset();
}

void ActionSequenceSystem::ActSeqGetPicker()
{
	auto tgtClassif = d20Sys.TargetClassification(d20Sys.globD20Action);
	if (tgtClassif == D20TargetClassification::D20TC_ItemInteraction)
	{
		if (d20Sys.d20Defs[d20Sys.globD20Action->d20ActType].flags & D20ADF_UseCursorForPicking)
		{
			*seqPickerD20ActnType = d20Sys.globD20Action->d20ActType;  // seqPickerD20ActnType
			*seqPickerD20ActnData1 = d20Sys.globD20Action->data1;
			*seqPickerTargetingType = D20TC_ItemInteraction;
			return;
		}
		addresses.actSeqPicker->flagsTarget = UiPickerFlagsTarget::None;
		addresses.actSeqPicker->modeTarget = UiPickerType::Single;
		addresses.actSeqPicker->incFlags = UiPickerIncFlags::NonCritter;
		addresses.actSeqPicker->excFlags = UiPickerIncFlags::None;
		addresses.actSeqPicker->callback = reinterpret_cast<PickerCallback>(0x10096570);//(int)ChooseTargetCallback;
		addresses.actSeqPicker->spellEnum = 0;
		addresses.actSeqPicker->caster = d20Sys.globD20Action->d20APerformer;
		*actSeqPickerActive = 1;
		uiTurnBased.ShowPicker(addresses.actSeqPicker,nullptr);
		*addresses.actSeqPickerAction = *d20Sys.globD20Action;
		return;
	}

	if (tgtClassif == D20TargetClassification::D20TC_SingleIncSelf)
	{
		if (d20Sys.d20Defs[d20Sys.globD20Action->d20ActType].flags & D20ADF_UseCursorForPicking)
		{
			*seqPickerD20ActnType = d20Sys.globD20Action->d20ActType; //seqPickerD20ActnType
			*seqPickerD20ActnData1 = d20Sys.globD20Action->data1;
			*seqPickerTargetingType = D20TC_SingleIncSelf;
			return;
		}
		addresses.actSeqPicker->flagsTarget = UiPickerFlagsTarget::None;
		addresses.actSeqPicker->modeTarget = UiPickerType::Single;
		addresses.actSeqPicker->incFlags = static_cast<UiPickerIncFlags>((uint64_t)UiPickerIncFlags::Self | (uint64_t)UiPickerIncFlags::Other | (uint64_t)UiPickerIncFlags::Dead);
		addresses.actSeqPicker->excFlags = UiPickerIncFlags::NonCritter;
		addresses.actSeqPicker->callback = reinterpret_cast<PickerCallback>(0x10096570);//(int)ChooseTargetCallback;
		addresses.actSeqPicker->spellEnum = 0;
		addresses.actSeqPicker->caster = d20Sys.globD20Action->d20APerformer;

		*actSeqPickerActive = 1;
		uiTurnBased.ShowPicker(addresses.actSeqPicker, nullptr);
		*addresses.actSeqPickerAction = *d20Sys.globD20Action;
		return;
	}

	if (tgtClassif == D20TargetClassification::D20TC_SingleExcSelf)
	{
		if (d20Sys.d20Defs[d20Sys.globD20Action->d20ActType].flags & D20ADF_UseCursorForPicking)
		{
			*seqPickerD20ActnType = d20Sys.globD20Action->d20ActType;
			*seqPickerD20ActnData1 = d20Sys.globD20Action->data1;
			*seqPickerTargetingType = D20TC_SingleExcSelf;
			return;
		}
		addresses.actSeqPicker->flagsTarget = UiPickerFlagsTarget::None;
		addresses.actSeqPicker->modeTarget = UiPickerType::Single;
		addresses.actSeqPicker->incFlags = static_cast<UiPickerIncFlags>((uint64_t)UiPickerIncFlags::Other | (uint64_t)UiPickerIncFlags::Dead);
		addresses.actSeqPicker->excFlags = static_cast<UiPickerIncFlags>((uint64_t)UiPickerIncFlags::Self | (uint64_t)UiPickerIncFlags::NonCritter);
		addresses.actSeqPicker->callback = reinterpret_cast<PickerCallback>(0x10096570);//(int)ChooseTargetCallback;
		addresses.actSeqPicker->spellEnum = 0;
		addresses.actSeqPicker->caster = d20Sys.globD20Action->d20APerformer;
		*actSeqPickerActive = 1;
		uiTurnBased.ShowPicker(addresses.actSeqPicker, nullptr);
		*addresses.actSeqPickerAction = *d20Sys.globD20Action;
		return;
	}

	if (tgtClassif == D20TC_CallLightning)
	{
		if (d20Sys.globD20Action->d20ActType == D20A_SPELL_CALL_LIGHTNING)
		{
			int callLightningId = d20Sys.d20QueryReturnData(d20Sys.globD20Action->d20APerformer, DK_QUE_Critter_Can_Call_Lightning, 0, 0);
			SpellPacketBody spellPkt;
			spellSys.GetSpellPacketBody(callLightningId, &spellPkt);
			auto baseCasterLevelMod = dispatch.Dispatch35BaseCasterLevelModify(d20Sys.globD20Action->d20APerformer, &spellPkt);
			addresses.actSeqPicker->range = spellSys.GetSpellRangeExact(SpellRangeType::SRT_Medium, baseCasterLevelMod, d20Sys.globD20Action->d20APerformer);
			addresses.actSeqPicker->radiusTarget = 5;
		} else
		{
			addresses.actSeqPicker->range = 0;
			addresses.actSeqPicker->radiusTarget = 0;
		}
		addresses.actSeqPicker->flagsTarget = UiPickerFlagsTarget::Range;
		addresses.actSeqPicker->modeTarget = UiPickerType::Area;
		addresses.actSeqPicker->incFlags = static_cast<UiPickerIncFlags>((uint64_t)UiPickerIncFlags::Self | (uint64_t)UiPickerIncFlags::Other );
		addresses.actSeqPicker->excFlags = static_cast<UiPickerIncFlags>( (uint64_t)UiPickerIncFlags::Dead | (uint64_t)UiPickerIncFlags::NonCritter);
		addresses.actSeqPicker->callback = reinterpret_cast<PickerCallback>(0x10096570);//(int)ChooseTargetCallback;
		addresses.actSeqPicker->spellEnum = 0;
		addresses.actSeqPicker->caster = d20Sys.globD20Action->d20APerformer;
		*actSeqPickerActive = 1;
		uiTurnBased.ShowPicker(addresses.actSeqPicker, nullptr);
		*addresses.actSeqPickerAction = *d20Sys.globD20Action;
		return;
	}

	if (tgtClassif == D20TC_CastSpell)
	{
		unsigned int spellEnum, spellClass, spellLevel, metaMagicData;
		D20SpellDataExtractInfo(&d20Sys.globD20Action->d20SpellData,
			&spellEnum, nullptr, &spellClass, &spellLevel,nullptr,&metaMagicData);
		auto curSeq = *actSeqSys.actSeqCur;
		curSeq->spellPktBody.spellRange *= ((MetaMagicData)metaMagicData).metaMagicEnlargeSpellCount + 1;
		SpellEntry spellEntry;
		if (spellSys.spellRegistryCopy(spellEnum, &spellEntry))
		{
			spellEntry.radiusTarget *= ((MetaMagicData)metaMagicData).metaMagicWidenSpellCount + 1;
		}
		PickerArgs pickArgs;
		spellSys.pickerArgsFromSpellEntry(&spellEntry, &pickArgs, curSeq->spellPktBody.caster, curSeq->spellPktBody.baseCasterLevel);
		pickArgs.spellEnum = spellEnum;
		pickArgs.callback = reinterpret_cast<PickerCallback>(0x10096CC0);
		*actSeqPickerActive = 1;
		// uiTurnBased.ShowPicker(&pickArgs, &curSeq->spellPktBody); //0x10B3D5D0
		uiPicker.ShowPicker(pickArgs, &curSeq->spellPktBody);
		*addresses.actSeqPickerAction = *d20Sys.globD20Action;
		return;
	}

}

void ActionSequenceSystem::ActionTypeAutomatedSelection(objHndl handle)
{
	
	auto setGlobD20Action = [](D20ActionType actType, int data1){
		d20Sys.globD20Action->d20ActType = actType;
		d20Sys.globD20Action->data1 = data1;
	};


	D20TargetClassification targetingType = D20TC_Movement; // default
	if(handle) {
		auto obj = gameSystems->GetObj().GetObject(handle);
		switch (obj->type)
		{
		case obj_t_portal:
		case obj_t_scenery:
			targetingType = D20TC_Movement;
			break;

		case obj_t_container:
		case obj_t_projectile:
		case obj_t_weapon:
		case obj_t_ammo:
		case obj_t_armor:
		case obj_t_money:
		case obj_t_food:
		case obj_t_scroll:
		case obj_t_key:
		case obj_t_written:
		case obj_t_generic:
		case obj_t_trap:
		case obj_t_bag:
			targetingType = D20TC_ItemInteraction;
			break;
		case obj_t_pc:
		case obj_t_npc:
			targetingType = D20TC_SingleExcSelf;
		default:
			targetingType = D20TC_Movement;
			break;
		}
	}

	if (*seqPickerTargetingType == targetingType
		|| *seqPickerTargetingType ==D20TC_SingleIncSelf && targetingType == D20TC_SingleExcSelf
		|| *seqPickerTargetingType != D20TC_Invalid)
	{
		setGlobD20Action(*seqPickerD20ActnType, *seqPickerD20ActnData1);
		return;
	}


	// if no targeting type defined for picker:
	if (!handle){
		setGlobD20Action(D20A_UNSPECIFIED_MOVE, 0);
		return;
	} 

	auto obj = gameSystems->GetObj().GetObject(handle);
	switch (obj->type){
	case obj_t_portal:
	case obj_t_scenery:
		setGlobD20Action(D20A_UNSPECIFIED_MOVE, 0);
		return;
	case obj_t_projectile:
	case obj_t_weapon:
	case obj_t_ammo:
	case obj_t_armor:
	case obj_t_money:
	case obj_t_food:
	case obj_t_scroll:
	case obj_t_key:
	case obj_t_written:
	case obj_t_generic:
	case obj_t_trap:
	case obj_t_bag:
		setGlobD20Action(D20A_PICKUP_OBJECT, 0);
		return;
	case obj_t_container:
		setGlobD20Action(D20A_OPEN_CONTAINER, 0);
		return;
	case obj_t_pc:
	case obj_t_npc:
		break;
	default: 
		return;
	}
	
	// Critters

	auto d20a = d20Sys.globD20Action;
	auto performer = d20a->d20APerformer;
	if (critterSys.IsFriendly(handle,performer) || critterSys.AllegianceShared(handle, performer))
	{
		if (!d20Sys.d20Query(performer, DK_QUE_HoldingCharge)) {
			setGlobD20Action(D20A_UNSPECIFIED_MOVE, 0);
			return;
		}
	} else if (critterSys.IsDeadNullDestroyed(handle) && temple::GetRef<BOOL(__cdecl)(objHndl)>(0x101391C0)(handle))
	{
		setGlobD20Action(D20A_OPEN_CONTAINER, 0);
		return;
	}

	setGlobD20Action(D20A_UNSPECIFIED_ATTACK, 0);

}

void ActionSequenceSystem::TurnStart(objHndl obj)
{
	logger->debug("*** NEXT TURN *** starting for {} ({}). CurSeq: {}", description.getDisplayName(obj), obj, (void*)(*actSeqSys.actSeqCur));
	auto objBody = gameSystems->GetObj().GetObject(obj);
	//orgTurnStart(obj);

	auto& dword_1189FB60 = temple::GetRef<int>(0x1189FB60);
	dword_1189FB60 = 0;
	auto& seqSthg_10B3D59C = temple::GetRef<int>(0x10B3D59C);
	seqSthg_10B3D59C = 0;
	auto& seqSthg_10B3D5C4 = temple::GetRef<int>(0x10B3D5C4);
	seqSthg_10B3D5C4 = 0;

	if (d20Sys.globD20Action->d20APerformer && d20Sys.globD20Action->d20APerformer != obj) {
		d20Sys.d20SendSignal(d20Sys.globD20Action->d20APerformer, DK_SIG_EndTurn, 0, 0);
	}

	if (!combatSys.isCombatActive())
		return;

	// check for interrupter sequence
	if (*addresses.actSeqInterrupt) {

		// switch sequences
		auto actSeq = *addresses.actSeqInterrupt;
		*actSeqCur = actSeq;
		*addresses.actSeqInterrupt = actSeq->interruptSeq;
		auto curIdx = actSeq->d20aCurIdx;
		logger->info("Switching to Interrupt sequence, actor {}", description.getDisplayName(actSeq->performer));
		if (curIdx <  actSeq->d20ActArrayNum) {

			auto d20a = &actSeq->d20ActArray[curIdx];

			if (InterruptNonCounterspell(d20a))
				return;

			if (d20a->d20ActType == D20A_CAST_SPELL) {
				auto d20SpellData = &d20a->d20SpellData;
				if (d20Sys.d20QueryWithData(actSeq->performer, DK_QUE_SpellInterrupted, d20SpellData, 0)) {
					d20a->d20Caf &= ~D20CAF_NEED_ANIM_COMPLETED;
					animationGoals.Interrupt(actSeq->performer, AnimGoalPriority::AGP_5, 0);
				}
			}
		}


		sequencePerform();
		return;
	}

	// clean readied actions for obj
	ReadyVsRemoveForObj(obj);

	// clean previously occupied sequences
	d20Sys.globD20ActnSetPerformer(obj);
	for (int i = 0; i < ACT_SEQ_ARRAY_SIZE; i++) {

		if ((actSeqArray[i].seqOccupied & 1)
			&& actSeqArray[i].performer == obj) {
			actSeqArray[i].seqOccupied &= ~1;
			logger->info("Clearing outstanding sequence [{}]", i);
		}
	}

	// allocate new sequence
	if (!AllocSeq(obj)) {
		*actSeqCur = &actSeqArray[0];
		curSeqReset(obj);
	}

	// apply newround condition and do BeginRound stuff
	if (!d20Sys.d20Query(obj, DK_QUE_NewRound_This_Turn)) {
		dispatch.Dispatch48BeginRound(obj, 1);
		conds.AddTo(obj, "NewRound_This_Turn", {});
	}

	// dispatch TurnBasedStatusInit
	TurnBasedStatus * tbStat = &(*actSeqSys.actSeqCur)->tbStatus;
	tbStat->hourglassState = 4;
	tbStat->tbsFlags = 0;
	tbStat->idxSthg = -1;
	tbStat->surplusMoveDistance = 0;
	tbStat->attackModeCode = 0;
	tbStat->baseAttackNumCode = 0;
	tbStat->numBonusAttacks = 0;
	tbStat->numAttacks = 0;
	tbStat->errCode = AEC_OK;

	DispIOTurnBasedStatus dispIo;
	dispIo.tbStatus = tbStat;
	dispatch.dispatchTurnBasedStatusInit(obj, &dispIo);

	// Enqueue simuls
	static void(*simulsEnqueue)() = temple::GetRef<void(__cdecl)()>(0x100922B0);
	simulsEnqueue();

	if (objects.IsPlayerControlled(obj) && critterSys.IsDeadOrUnconscious(obj)) {
		logger->info("Action for {} ending turn (unconscious)...", description.getDisplayName(d20Sys.globD20Action->d20APerformer));
		combatSys.CombatAdvanceTurn(obj);
		return;
	}

	if (objBody->GetFlags() & OF_OFF) {
		logger->info("Action for {} ending turn (OF_OFF)", description.getDisplayName(d20Sys.globD20Action->d20APerformer));
		combatSys.CombatAdvanceTurn(obj);
		return;
	}

	if (d20Sys.d20Query(obj, DK_QUE_Prone)) {
		if ((*actSeqSys.actSeqCur)->tbStatus.hourglassState >= 1)
		{
			d20Sys.D20ActnInit(d20Sys.globD20Action->d20APerformer, d20Sys.globD20Action);
			d20Sys.globD20Action->d20ActType = D20A_STAND_UP;
			d20Sys.globD20Action->data1 = 0;
			ActionAddToSeq();
			sequencePerform();
		}
	}
}

int ActionSequenceSystem::ActionAddToSeq()
{
	auto curSeq = *actSeqCur;
	auto d20ActnType = d20Sys.globD20Action->d20ActType;
	int actnCheckResult = 0;
	TurnBasedStatus tbStatus = curSeq->tbStatus;
	if (d20ActnType == D20A_CAST_SPELL
		&& curSeq->spellPktBody.spellEnum >= 600)
	{
		curSeq->tbStatus.tbsFlags |= TBSF_CritterSpell; // perhaps bug that it's not affecting the local copy?? TODO
		curSeq->spellPktBody.spellEnumOriginal = curSeq->spellPktBody.spellEnum;
	}
	auto actnCheckFunc = d20Sys.d20Defs[d20ActnType].actionCheckFunc;
	if (actnCheckFunc)
		actnCheckResult = actnCheckFunc(d20Sys.globD20Action, &tbStatus);
	if (objects.IsPlayerControlled(curSeq->performer))
	{
		if (actnCheckResult)
		{
			if (actnCheckResult == AEC_TARGET_INVALID)
			{
				actSeqSys.ActSeqGetPicker();
				return AEC_TARGET_INVALID;
			}
			if (actnCheckResult != AEC_TARGET_TOO_FAR)
			{
				auto errorString = actSeqSys.ActionErrorString(actnCheckResult);
				floatSys.floatMesLine(curSeq->performer, 1, FloatLineColor::Red, errorString);
				return actnCheckResult;
			}
		} else if (!d20Sys.TargetCheck(d20Sys.globD20Action))
		{
			actSeqSys.ActSeqGetPicker();
			return AEC_TARGET_INVALID;
		}
	} else
	{
		d20Sys.TargetCheck(d20Sys.globD20Action);
	}
	auto result =  actSeqSys.addD20AToSeq(d20Sys.globD20Action, curSeq);
	return result;
	// return addresses.ActionAddToSeq();
}

uint32_t ActionSequenceSystem::addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq)
{
	D20ActionType d20aType = d20a->d20ActType;
	if (d20aType == D20A_NONE){ return 0xE; }

	*actnProcState =  d20->d20Defs[d20aType].addToSeqFunc(d20a, actSeq, &actSeq->tbStatus);

	ActnSeq * curSeq = *actSeqCur;
	for (int d20aIdx = curSeq->d20aCurIdx + 1; d20aIdx < curSeq->d20ActArrayNum; d20aIdx++)
	{
		if (actSeq->tbStatus.tbsFlags & 0x40)
		{
			curSeq->d20ActArray[d20aIdx].d20Caf |= D20CAF_FULL_ATTACK;
		}
	}
	return *actnProcState;
}

uint32_t ActionSequenceSystem::isPerforming(objHndl obj)
{
	for (auto i = 0; i < ACT_SEQ_ARRAY_SIZE; i++)
	{
		if (actSeqArray[i].performer == obj && (actSeqArray[i].seqOccupied & SEQF_PERFORMING))
		{
			return 1;
		}
	}
	return 0;
}

ActionErrorCode ActionSequenceSystem::AddToSeqSimple(D20Actn* d20a, ActnSeq * actSeq, TurnBasedStatus* tbStat)
{
	memcpy(actSeq + sizeof(D20Actn) * actSeq->d20ActArrayNum++, d20a, sizeof(D20Actn));
	return AEC_OK;
}

int ActionSequenceSystem::AddToSeqWithTarget(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStatus)
{
	int result; 
	int actNum; 
	float reach; 
	objHndl target; 
	TurnBasedStatus tbStatusCopy; 
	D20Actn d20aCopy; 

	target = d20a->d20ATarget;
	actNum = actSeq->d20ActArrayNum;
	if (!target)
		return AEC_TARGET_INVALID;
	
	// check if target is within reach
	reach = critterSys.GetReach(d20a->d20APerformer, d20a->d20ActType);
	if (location->DistanceToObj(d20a->d20APerformer, d20a->d20ATarget) < reach)
	{
		memcpy(&actSeq->d20ActArray[actSeq->d20ActArrayNum++], d20a, sizeof(D20Actn));
		return AEC_OK;
	}

	// if not, add a move sequence
	memcpy(&d20aCopy, d20a, sizeof(d20aCopy));
	d20aCopy.d20ActType = D20A_UNSPECIFIED_MOVE;
	location->getLocAndOff(target, &d20aCopy.destLoc);
	result = moveSequenceParse(&d20aCopy, actSeq, tbStatus, 0.0, reach, 1 );
	if (!result)
	{
		tbStatusCopy = *tbStatus;
		memcpy(&actSeq->d20ActArray[actSeq->d20ActArrayNum++], d20a, sizeof(D20Actn));
		if (actNum < actSeq->d20ActArrayNum)
		{
			for (int dummy = 0 ; actNum < actSeq->d20ActArrayNum; actNum++)
			{
				result = TurnBasedStatusUpdate(&tbStatusCopy, &actSeq->d20ActArray[actNum]);
				if (result)
				{
					tbStatusCopy.errCode = result;
					return result;
				}
				auto actionCheckFunc = d20Sys.d20Defs[actSeq->d20ActArray[actNum].d20ActType].actionCheckFunc;
				if (actionCheckFunc)
				{
					result = actionCheckFunc(&actSeq->d20ActArray[actNum], &tbStatusCopy);
					if (result)
						return result;
				}
			}
			if (actNum >= actSeq->d20ActArrayNum)
				return AEC_OK;
			tbStatusCopy.errCode = result;
			if (result)
				return result;
		}
		return AEC_OK;
	}
	return result;
}

void ActionSequenceSystem::ProcessPathForReadiedActions(D20Actn* d20a, CmbtIntrpts* intrpts)
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}
	__asm{
		mov ecx, this;
		mov esi, [ecx]._ProcessPathForReadiedActions;
		mov eax, intrpts;
		push eax;
		mov eax, d20a;
		call esi;
		add esp, 4;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx} 
}

BOOL ActionSequenceSystem::HasReadiedAction(objHndl obj)
{
	auto readiedActionCache = addresses.readiedActionCache;
	for (int i = 0; i < READIED_ACTION_CACHE_SIZE ; i++)
	{
		if (readiedActionCache[i].interrupter == obj && readiedActionCache[i].flags == 1) {
			// TODO: bug?
			return 1;
		}
	}
	return 0;
}

void ActionSequenceSystem::ProcessPathForAoOs(objHndl obj, PathQueryResult* pqr, AoOPacket* aooPacket, float aooFreeDistFeet)
{// aooFreeDistFeet specifies the minimum distance traveled before an AoO is registered (e.g. for Withdrawal it will receive 5 feet)
	auto truncateLengthFeet = aooFreeDistFeet;
	auto aooDistFeet = aooFreeDistFeet;
	aooPacket->obj = obj;
	aooPacket->path = pqr;
	aooPacket->numAoOs = 0;
	auto pathLength = pathfindingSys.pathLength(pqr);
	if (aooFreeDistFeet > pathLength)
		return;
	//truncateLengthFeet = pathLength;
	LocAndOffsets truncatedLoc;
	pathfindingSys.TruncatePathToDistance(aooPacket->path, &truncatedLoc, truncateLengthFeet); // this is the first possible distance where an Aoo might occur
	int enemyCount = 0;
	objHndl* enemies = combatSys.GetHostileCombatantList(obj, &enemyCount );

	while (truncateLengthFeet <  pathLength - 2.0)
	{

		// obj is moving away from truncatedLoc
		// if an enemy can hit you from when you're in the current truncatedLoc
		// it means you incur an AOO

		// loop over enemies to catch interceptions
		for (int enemyIdx = 0; enemyIdx < enemyCount; enemyIdx++)
		{
			auto enemy = enemies[enemyIdx];
			int interrupterIdx = 0;
			bool hasInterrupted = false;
			for (interrupterIdx = 0; interrupterIdx < aooPacket->numAoOs; interrupterIdx++)
			{
				if (enemy == aooPacket->interrupters[interrupterIdx])
				{
					hasInterrupted = true;
					break;
				}
					
			}
			if (!hasInterrupted && enemy != aooPacket->interrupters[interrupterIdx])
			{
				if (d20Sys.d20QueryWithData(enemy, DK_QUE_AOOPossible, obj ))
				{
					if (combatSys.CanMeleeTargetAtLoc(enemy, obj, &truncatedLoc))
					{
						aooPacket->interrupters[aooPacket->numAoOs] = enemy;
						aooPacket->aooDistFeet[aooPacket->numAoOs] = aooDistFeet;
						aooPacket->aooLocs[aooPacket->numAoOs++] = truncatedLoc;
						if (aooPacket->numAoOs >= 32)
							return;
					}
				}
			}
		}

		// advanced the truncatedLoc by 4 feet along the path
		truncateLengthFeet = truncateLengthFeet + 4.0;
		aooDistFeet = truncateLengthFeet;

		if (truncateLengthFeet < pathLength - 2.0)
			pathfindingSys.TruncatePathToDistance(aooPacket->path, &truncatedLoc, truncateLengthFeet);


		/*
		if (aooDistFeet >= pathLength)
		{
			aooDistFeet = pathLength - 0.0001;
		}*/

		
			
	}
	delete [] enemies;
	
	return;
}

uint32_t ActionSequenceSystem::moveSequenceParse(D20Actn* d20aIn, ActnSeq* actSeq, TurnBasedStatus* tbStat, float distToTgtMin, float reach, int nonspecificMoveType)
{
	D20Actn d20aCopy;
	TurnBasedStatus tbStatCopy = *tbStat;
	PathQuery pathQ;
	LocAndOffsets locAndOffCopy = d20aIn->destLoc;
	LocAndOffsets * actSeqPerfLoc;
	ActionCostPacket actCost;

	//logger->debug("Parsing move sequence for {}, d20 action {}", description.getDisplayName(d20aIn->d20APerformer), d20ActionNames[d20aIn->d20ActType]);
	
	seqCheckFuncs(&tbStatCopy);
	
	// if prone, add a standup action
	if (d20->d20Query(d20aIn->d20APerformer, DK_QUE_Prone))
	{
		d20aCopy=* d20aIn;
		auto nextd20a = &actSeq->d20ActArray[actSeq->d20ActArrayNum];
		d20aCopy.d20ActType = D20A_STAND_UP;
		*nextd20a = d20aCopy;
		++actSeq->d20ActArrayNum; 
	}

	d20aCopy = *d20aIn;
	auto d20a = d20aIn;


	actSeqPerfLoc = &actSeq->performerLoc;
	pathQ.critter = d20aCopy.d20APerformer;
	pathQ.from = actSeq->performerLoc;


	if (d20a->d20ATarget)
	{
		pathQ.targetObj = d20a->d20ATarget;
		if (pathQ.critter == pathQ.targetObj)
			return ActionErrorCode::AEC_TARGET_INVALID;

		const float twelve = 12.0;
		const float fourPointSevenPlusEight = 4.714045f + 8.0f;
		pathQ.flags = static_cast<PathQueryFlags>(PathQueryFlags::PQF_TO_EXACT | PathQueryFlags::PQF_HAS_CRITTER | PathQueryFlags::PQF_800
			| PathQueryFlags::PQF_TARGET_OBJ | PathQueryFlags::PQF_ADJUST_RADIUS | PathQueryFlags::PQF_ADJ_RADIUS_REQUIRE_LOS);
		
		
		if (reach < 0.1){ reach = 3.0; }
		actSeq->targetObj = d20a->d20ATarget;
		pathQ.distanceToTargetMin = distToTgtMin * twelve;
		pathQ.tolRadius = reach * twelve - fourPointSevenPlusEight;
	} else
	{
		pathQ.to = d20aIn->destLoc;
		pathQ.flags = static_cast<PathQueryFlags>(PathQueryFlags::PQF_TO_EXACT | PathQueryFlags::PQF_HAS_CRITTER | PathQueryFlags::PQF_800 
			 | PathQueryFlags::PQF_ALLOW_ALTERNATIVE_TARGET_TILE);
	}

	if (d20a->d20Caf & D20CAF_UNNECESSARY)
	{
		*reinterpret_cast<int*>(&pathQ.flags) |= PQF_A_STAR_TIME_CAPPED;
	} 


	if (d20aCopy.path && d20aCopy.path >= pathfindingSys.pathQArray && d20aCopy.path < &pathfindingSys.pathQArray[PQR_CACHE_SIZE]) 
		d20aCopy.path->occupiedFlag = 0; // frees the last path used in the d20a


	// get new path result slot
	auto pqResult = pathfindingSys.FetchAvailablePQRCacheSlot();
	d20aCopy.path = pqResult;
	if (!pqResult) 
		return ActionErrorCode::AEC_TARGET_INVALID;

	// find path
	*pathfindingSys.rollbackSequenceFlag = 0;
	if ( (d20aCopy.d20Caf & D20CAF_CHARGE ) || d20a->d20ActType == D20A_RUN || d20a->d20ActType == D20A_DOUBLE_MOVE)
	{
		*reinterpret_cast<int*>(&pathQ.flags) |= PQF_DONT_USE_PATHNODES; // so it runs in a straight line
	}
	int pathResult = pathfinding->FindPath(&pathQ, pqResult);
	if (!pathResult)
	{
		if (pqResult->flags & PF_TIMED_OUT) 
		{
			*pathfindingSys.rollbackSequenceFlag = 1;
		}

		if (pathQ.targetObj)
			logger->debug("MoveSequenceParse: FAILED PATH... {} attempted from {} to {} ({})", description.getDisplayName(pqResult->mover), pqResult->from, pqResult->to, description.getDisplayName(pathQ.targetObj));
		else
			logger->debug("MoveSequenceParse: FAILED PATH... {} attempted from {} to {}", description.getDisplayName(pqResult->mover), pqResult->from, pqResult->to);
		


		/*if (config.pathfindingDebugMode)
		{ 
			pathResult = pathfinding->FindPath(&pathQ, pqResult);
			if (pathResult)
			{
				*pathfindingSys.rollbackSequenceFlag = 0;
				logger->debug("Second time's the charm... from {} to {}", pqResult->from, pqResult->to);
			}
		}*/
		if (!pathResult)
		{
			if (pqResult >= pathfindingSys.pathQArray && pqResult < &pathfindingSys.pathQArray[PQR_CACHE_SIZE])
				pqResult->occupiedFlag = 0;
			return AEC_TARGET_INVALID;
		}
	}

	auto pathLength = pathfinding->pathLength(pqResult);
	d20aCopy.destLoc = pqResult->to;
	d20aCopy.distTraversed = pathLength;

	if (pathLength < 0.1)
	{
		// logger->debug("Path length zero! Leak??");
		releasePath(pqResult);
		return 0;
	}
	if (!combat->isCombatActive()){	d20aCopy.distTraversed = 0;		pathLength = 0.0;	}

	float remainingMaxMoveLength = 0;
	if (getRemainingMaxMoveLength(d20a, &tbStatCopy, &remainingMaxMoveLength)) // deducting moves that have already been spent, but also a raw calculation (not taking 5' step and such into account)
	{
		if (remainingMaxMoveLength < 0.1)	{releasePath(d20aCopy.path);	return AEC_TARGET_TOO_FAR;	}
		if (static_cast<long double>(remainingMaxMoveLength) < pathLength)
		{
			auto temp = 1;;
			if (TrimPathToRemainingMoveLength(&d20aCopy, remainingMaxMoveLength, &pathQ)){ releasePath(d20aCopy.path); return temp; }
			pqResult = d20aCopy.path;
			pathLength = remainingMaxMoveLength;
		}
	}


	/*
	this is 0 for specific move action types like 5' step, Move, Run, Withdraw; 
	*/
	if (nonspecificMoveType) 
	{
		float baseMoveDist = dispatch.Dispatch29hGetMoveSpeed(d20aCopy.d20APerformer, nullptr);
		if (!(d20aCopy.d20Caf & D20CAF_CHARGE))
		{
			if (pathLength > (long double)tbStatCopy.surplusMoveDistance)
			{
				if ( 2*baseMoveDist + tbStatCopy.surplusMoveDistance < (long double)pathLength){ releasePath(pqResult); return 8; }
				if (tbStatCopy.surplusMoveDistance + baseMoveDist < (long double)pathLength){ d20aCopy.d20ActType = D20A_DOUBLE_MOVE; goto LABEL_53; }
				else if (pathLength <= 5.0)
				{
					if (d20a->d20ActType != D20A_UNSPECIFIED_MOVE)
					{
						d20->d20Defs[d20a->d20ActType].actionCost(d20a, &tbStatCopy, &actCost);
						if (actCost.hourglassCost == 4 || !tbStatCopy.hourglassState) 
						{
							d20aCopy.d20ActType = D20A_5FOOTSTEP; goto LABEL_53;
						}
					} else if (!tbStatCopy.hourglassState)
					{
						d20aCopy.d20ActType = D20A_5FOOTSTEP;
						if (!(tbStatCopy.tbsFlags & (TBSF_Movement | TBSF_Movement2))){	goto LABEL_53; }
					}
				}

			}
			d20aCopy.d20ActType = D20A_MOVE;
		} 
		else if ( 2* baseMoveDist >= (long double)pathLength) d20aCopy.d20ActType = D20A_RUN;	
		else	{	releasePath(pqResult); return AEC_TARGET_TOO_FAR;	}
	}

LABEL_53:
	if (config.pathfindingDebugMode)
	{
		if (!critterSys.IsPC(d20a->d20APerformer))
		{
			int dummy = 1;
		}
	}

	 *actSeqPerfLoc = pqResult->to;
	CmbtIntrpts intrpts;
	intrpts.numItems = 0;
	ProcessPathForReadiedActions(&d20aCopy, &intrpts);
	ProcessSequenceForAoOs(actSeq, &d20aCopy);
	addReadiedInterrupts(actSeq, &intrpts);
	updateDistTraversed(actSeq);
	return 0;
}

void ActionSequenceSystem::releasePath(PathQueryResult* pqr)
{
	if (pqr)
	{
		if (pqr >= pathfinding->pathQArray && pqr < &pathfinding->pathQArray[PQR_CACHE_SIZE])
		{
			pqr->occupiedFlag = 0;
		}
	}
}

void ActionSequenceSystem::addReadiedInterrupts(ActnSeq* actSeq, CmbtIntrpts* intrpts)
{
	D20Actn d20aNew;
	int32_t intrptNum = intrpts->numItems;
	if (intrptNum <= 0) return;
	
	d20aNew.d20ATarget = actSeq->performer;
	d20aNew.d20Caf = (D20CAF)0;
	d20aNew.d20ActType = D20A_READIED_INTERRUPT;
	d20aNew.data1 = 1;
	d20aNew.path = nullptr;
	ReadiedActionPacket * readied ;
	for (int i = 0; i < intrptNum; i++)
	{
		readied = intrpts->readyPackets[i];
		d20aNew.d20APerformer = readied->interrupter;
		memcpy(&(actSeq->d20ActArray[actSeq->d20ActArrayNum]), &d20aNew, sizeof(d20aNew));
		++actSeq->d20ActArrayNum;
	}
}

void ActionSequenceSystem::updateDistTraversed(ActnSeq* actSeq)
{
	PathQueryResult * path;
	int numd20s = actSeq->d20ActArrayNum;
	for (int i = 0; i < numd20s; i++)
	{
		path = actSeq->d20ActArray[i].path;
		if (path){	actSeq->d20ActArray[i].distTraversed = pathfinding->pathLength(path);	}
	}
}

uint32_t ActionSequenceSystem::actSeqOkToPerform()
{
	ActnSeq * curSeq = *actSeqCur;
	if ( (curSeq->seqOccupied & SEQF_PERFORMING) && (curSeq->d20aCurIdx >= 0) && curSeq->d20aCurIdx < (int32_t)curSeq->d20ActArrayNum )
	{
		auto caflags = curSeq->d20ActArray[curSeq->d20aCurIdx].d20Caf;
		if (caflags & D20CAF_NEED_PROJECTILE_HIT){ return 0; }
		return (caflags & D20CAF_NEED_ANIM_COMPLETED) == 0;
	}
	return 1;
}

TurnBasedStatus* ActionSequenceSystem::curSeqGetTurnBasedStatus()
{
	if (*actSeqCur)
	{
		return &(*actSeqCur)->tbStatus;
	} 
	return nullptr;
}

const char* ActionSequenceSystem::ActionErrorString(uint32_t actnErrorCode)
{
	MesLine mesLine;
	mesLine.key = actnErrorCode + 1000;
	mesFuncs.GetLine_Safe(*actionMesHandle, &mesLine);
	return mesLine.value;
}

std::ostream &operator<<(std::ostream &os, const ActnSeq *d) {
	return os << reinterpret_cast<uint32_t>(d);
}

uint32_t ActionSequenceSystem::AllocSeq(objHndl objHnd)
{
	ActnSeq * curSeq = *actSeqCur;
	if (curSeq && !(curSeq->seqOccupied & SEQF_PERFORMING))
	{
		*actSeqCur = nullptr;
	}
	for (auto i = 0; i < ACT_SEQ_ARRAY_SIZE; i++)
	{
		if ( (actSeqArray[i].seqOccupied & SEQF_PERFORMING ) == 0)
		{
			*actSeqCur = &actSeqArray[i];
			if (combat->isCombatActive())
				logger->debug("AllocSeq: \t Sequence Allocate[{}]({})({}): Resetting Sequence. ", i, (void*)*actSeqCur, objHnd);
			curSeqReset(objHnd);
			return 1;
		} 
	}
	logger->debug("Sequence Allocation for {} failed!  Bad things imminent. All sequences were taken!\n", 
		(void*)*actSeqCur, objHnd);
	return 0;
}

uint32_t ActionSequenceSystem::AssignSeq(objHndl objHnd)
{
	ActnSeq * prevSeq = *actSeqCur;
	if (AllocSeq(objHnd))
	{
		if (combat->isCombatActive())
		{
			if (prevSeq != nullptr)
			{
				logger->debug("Pushing sequence from {} ({}) to {} ({})", object->description.getDisplayName(prevSeq->performer), prevSeq->performer, object->description._getDisplayName(objHnd, objHnd), objHnd);
			} else
			{
				logger->debug("Allocated sequence for {} ({}) ", object->description.getDisplayName(objHnd), objHnd, objHnd);
			}
		}
		(*actSeqCur)->prevSeq = prevSeq;
		(*actSeqCur)->seqOccupied |= SEQF_PERFORMING;
		return 1;
	}
	return 0;
}

uint32_t ActionSequenceSystem::TurnBasedStatusInit(objHndl objHnd)
{
	TurnBasedStatus * tbStatus;
	DispIOTurnBasedStatus dispIOtB;

	if (combat->isCombatActive())
	{
		if (turnbased->turnBasedGetCurrentActor() == objHnd) return 1;
	} else if ( !isPerforming(objHnd))
	{
		d20->globD20ActnSetPerformer(objHnd);
		*actSeqCur = nullptr;
		AssignSeq(objHnd);
		ActnSeq * curSeq = *actSeqCur;
		tbStatus = &curSeq->tbStatus;
		tbStatus->hourglassState = 4;
		tbStatus->tbsFlags = (D20CAF)0;
		tbStatus-> idxSthg= -1;
		tbStatus-> baseAttackNumCode= 0;
		tbStatus->attackModeCode = 0;
		tbStatus-> numBonusAttacks= 0;
		tbStatus-> numAttacks= 0;
		tbStatus-> errCode= 0;
		tbStatus-> surplusMoveDistance = 0;
		dispatch.dispIOTurnBasedStatusInit(&dispIOtB);
		dispIOtB.tbStatus = &curSeq->tbStatus;
		dispatch.dispatchTurnBasedStatusInit(objHnd, &dispIOtB);
		curSeq->seqOccupied &= 0xffffFFFE; // unset "occupied" byte flag
		return 1;
	}
	return 0;
}

void ActionSequenceSystem::ActSeqCurSetSpellPacket(SpellPacketBody* spellPktBody, int ignoreLos)
{
	(*actSeqCur)->spellPktBody = *spellPktBody;
	if (ignoreLos)
	{
		logger->info("Set CurSeq ignoreLos to {}", ignoreLos);
	}
	(*actSeqCur)->ignoreLos = ignoreLos;
}

int ActionSequenceSystem::GetNewHourglassState(objHndl performer, D20ActionType d20ActionType, int d20Data1, int radMenuActualArg, D20SpellData* d20SpellData)
{
	if (!*actSeqCur || *actSeqCur == (ActnSeq*)0xFFFFF4EC)
		return -1;
	
	TurnBasedStatus tbStatus;
	memcpy(&tbStatus, &(*actSeqCur)->tbStatus, sizeof(TurnBasedStatus));

	D20Actn d20a;
	d20a.d20ActType = d20ActionType;
	d20a.d20APerformer = performer;
	d20a.radialMenuActualArg = radMenuActualArg;
	d20a.data1 = d20Data1;
	d20a.d20Caf = D20CAF_NONE;
	d20a.d20ATarget = 0i64;
	d20a.distTraversed = 0;
	d20a.spellId = 0;
	d20a.path = nullptr;
	if (d20SpellData)
	{
		d20a.d20SpellData = *d20SpellData;
	}
	if (TurnBasedStatusUpdate(&tbStatus, &d20a))
		return -1;
	return tbStatus.hourglassState;
}

int ActionSequenceSystem::GetHourglassTransition(int hourglassCurrent, int hourglassCost)
{
	if (hourglassCurrent == -1)
		return hourglassCurrent;
	return turnBasedStatusTransitionMatrix[hourglassCurrent][hourglassCost];
}

BOOL ActionSequenceSystem::SequenceSwitch(objHndl obj)
{
	int seqIdx = -1;
	for (int i = 0; i < ACT_SEQ_ARRAY_SIZE; i++)
	{
		auto seq = &actSeqArray[i];
		if (seq->seqOccupied & SEQF_PERFORMING)
		{
			if (seq->performer == obj)
			{
				seqIdx = i;
			}
		} 
		else
		{
			if (seq->prevSeq && seq->prevSeq->performer == obj)
				return 0;
			if (seq->interruptSeq)
			{
				if (seq->interruptSeq->performer == obj)
					return 0;
			}
		}
	}

	if (seqIdx >= 0)
	{
		logger->debug("SequenceSwitch: \t doing for {} ({}). Previous Current Seq: {}", description.getDisplayName(obj), obj, (void*)(*actSeqCur));
		*actSeqCur = &actSeqArray[seqIdx];
		logger->debug("SequenceSwitch: \t new Current Seq: {}", (void*)(*actSeqCur));
		return 1;
	}
	return 0;
}

void ActionSequenceSystem::GreybarReset()
{
	if (!combatSys.isCombatActive())
		return;
	auto actor = tbSys.turnBasedGetCurrentActor();
	if ( !isPerforming(actor) && IsSimulsCompleted() && !IsLastSimultPopped(actor))
	{
		logger->debug("GREYBAR DEHANGER for {} ({}) ending turn...", description.getDisplayName(actor), actor);
		combatSys.CombatAdvanceTurn(actor);
		return;
	}

	logger->debug("Greybar reset function");
	for (int i = 0; i < ACT_SEQ_ARRAY_SIZE; i++)
	{
		auto seq = &actSeqArray[i];
		if (!(seq->seqOccupied & SEQF_PERFORMING) || !SequenceSwitch(seq->performer))
			continue;
		int actNum = seq->d20ActArrayNum;
		for (int j = 0; j < actNum; j++)
		{
			seq->d20ActArray[j].d20Caf &= ~(D20CAF_NEED_ANIM_COMPLETED | D20CAF_NEED_PROJECTILE_HIT);
		}
		while (1)
		{
			ActionPerform();
			bool foundOtherSeq = false;
			for (int j = 0; j < ACT_SEQ_ARRAY_SIZE;j++)
			{
				if ( (actSeqArray[j].seqOccupied & 1) && actSeqArray[j].performer == seq->performer)
				{
					foundOtherSeq = true;
					break;
				}
			}
			if (!foundOtherSeq)
				break;

			auto curSeq = *actSeqCur;
			if (curSeq && (curSeq->seqOccupied & SEQF_PERFORMING))
			{
				int curSeqActionIdx = curSeq->d20aCurIdx;
				if (curSeqActionIdx >= 0  && curSeqActionIdx < curSeq->d20ActArrayNum)
				{
					auto d20caf = curSeq->d20ActArray[curSeqActionIdx].d20Caf;

					if ( (d20caf & D20CAF_NEED_PROJECTILE_HIT) || (d20caf & D20CAF_NEED_ANIM_COMPLETED))
						break;
				}
			}
		}
	}
}

ActionErrorCode ActionSequenceSystem::ActionSequenceChecksRegardLoc(LocAndOffsets* loc, TurnBasedStatus * tbStatus, int d20aIdx, ActnSeq* actSeq)
{
	TurnBasedStatus tbStatCopy = *tbStatus;
	LocAndOffsets locCopy = *loc;
	int d20aNum = actSeq->d20ActArrayNum;
	ActionErrorCode result = AEC_OK;

	if (d20aIdx >= d20aNum)
		return AEC_OK;
	
	for (int i = d20aIdx; i < d20aNum; i++)
	{
		auto d20a = &actSeq->d20ActArray[i];
		auto d20aType = d20a->d20ActType;
		auto tgtCheckFunc = d20Sys.d20Defs[d20aType].tgtCheckFunc;
		if (tgtCheckFunc)
		{
			result = static_cast<ActionErrorCode>(tgtCheckFunc(d20a, &tbStatCopy));
			if (result != AEC_OK)
			{
				return result;
			}
		}
		result = static_cast<ActionErrorCode>(TurnBasedStatusUpdate(d20a, &tbStatCopy));
		if (result != AEC_OK)
		{
			tbStatCopy.errCode = result; // ??? WTF maybe debug leftover
			return result;
		}
			
		auto actionCheckFunc = d20Sys.d20Defs[d20aType].actionCheckFunc;
		if (actionCheckFunc)
		{
			result = static_cast<ActionErrorCode>(actionCheckFunc(d20a,&tbStatCopy));
			if (result != AEC_OK)
			{
				return result;
			}
				
		}

		auto locCheckFunc = d20Sys.d20Defs[d20aType].locCheckFunc;
		if (locCheckFunc)
		{
			result = static_cast<ActionErrorCode>(locCheckFunc(d20a,&tbStatCopy, &locCopy));
			if (result != AEC_OK)
			{
				return result;
			}
		}

		auto path = d20a->path;
		if (path)
		{
			locCopy = path->to;
		}

	}
	tbStatCopy.errCode = result;
	return result;
}

ActionErrorCode ActionSequenceSystem::ActionSequenceChecksWithPerformerLocation()
{
	LocAndOffsets loc;
	locSys.getLocAndOff((*actSeqCur)->performer, &loc);
	if ((*actSeqCur)->d20ActArrayNum)
	{
		return ActionSequenceChecksRegardLoc(&loc, &(*actSeqCur)->tbStatus, 0, *actSeqCur);
	}
	return ActionErrorCode::AEC_NO_ACTIONS;

	// return addresses.ActionSequenceChecksWithPerformerLocation();
}

void ActionSequenceSystem::ActionSequenceRevertPath(int d20ANum)
{
	return addresses.ActionSequenceRevertPath(d20ANum);
}

bool ActionSequenceSystem::GetPathTargetLocFromCurD20Action(LocAndOffsets* loc)
{
	int actNum = (*actSeqCur)->d20ActArrayNum;
	if (actNum <=0 )
		return 0;
	Path * d20Path = (*actSeqCur)->d20ActArray[actNum - 1].path;
	if (d20Path == nullptr)
		return 0;
	*loc = d20Path->to;
	return 1;
	
}

int ActionSequenceSystem::TrimPathToRemainingMoveLength(D20Actn* d20a, float remainingMoveLength, PathQuery* pathQ)
{ //1008B9A0

	auto pqrTrimmed = pathfindingSys.FetchAvailablePQRCacheSlot();
	auto pathLengthTrimmed = remainingMoveLength - 0.1;
	
	
	pathfindingSys.GetPartialPath(d20a->path, pqrTrimmed, 0.0, pathLengthTrimmed);
	
	
	if (pathfindingSys.pathQueryResultIsValid(d20a->path))
	{
		d20a->path->occupiedFlag = 0;
	}
	d20a->path = pqrTrimmed;
	auto destination = pqrTrimmed->to;
	if (!pathfindingSys.PathDestIsClear(pathQ, d20a->d20APerformer, destination))
	{
		d20a->d20Caf |= D20CAF_ALTERNATE;
	}
	d20a->d20Caf |= D20CAF_TRUNCATED;
	return 0;
}

uint32_t ActionSequenceSystem::ActionCostNull(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus, ActionCostPacket* actionCostPacket)
{
	actionCostPacket->hourglassCost = 0;
	actionCostPacket->chargeAfterPicker = 0;
	actionCostPacket->moveDistCost = 0;
	return 0;
}

void ActionSequenceSystem::ProcessSequenceForAoOs(ActnSeq* actSeq, D20Actn* d20a)
{
	AoOPacket aooPacket;

	float distFeet = 0.0;
	auto pqr = d20a->path;
	auto d20ActionTypePostAoO = d20a->d20ActType;
	bool addingAoOStatus = 1;
	if (d20ActionTypePostAoO == D20A_5FOOTSTEP)
	{
		actSeq->d20ActArray[actSeq->d20ActArrayNum++] = *d20a;
		return;
	}
	if (d20ActionTypePostAoO == D20A_DOUBLE_MOVE)
	{
		distFeet = 5.0;
		d20ActionTypePostAoO = D20A_MOVE;
	}
	ProcessPathForAoOs(d20a->d20APerformer, d20a->path, &aooPacket, distFeet);
	if (aooPacket.numAoOs == 0)
	{
		actSeq->d20ActArray[actSeq->d20ActArrayNum++] = *d20a;
		return;
	}
	D20Actn d20aAoOMovement(D20A_AOO_MOVEMENT);
	LocAndOffsets truncLoc;
	pathfindingSys.TruncatePathToDistance(pqr, &truncLoc, 0.0);
	float startDistFeet = 0.0, endDistFeet = pathfindingSys.pathLength(d20a->path), aooDistFeet;
	PathQueryResult * pqrTrunc;
	for (int i = 0; i < aooPacket.numAoOs;i++)
	{
		if (aooPacket.aooDistFeet[i] != startDistFeet)
		{
			d20aAoOMovement = *d20a;
			pqrTrunc = pathfindingSys.FetchAvailablePQRCacheSlot();
			aooDistFeet = aooPacket.aooDistFeet[i];
			d20aAoOMovement.path = pqrTrunc;
			if (!pathfindingSys.GetPartialPath(pqr, pqrTrunc, startDistFeet, aooDistFeet))
			{
				if (pathfindingSys.pathQueryResultIsValid(pqrTrunc))
					pqrTrunc->occupiedFlag = 0;
				if (pathfindingSys.pathQueryResultIsValid(pqr))
					pqr->occupiedFlag = 0;
				return;
			}
			startDistFeet = aooPacket.aooDistFeet[i];
			d20aAoOMovement.destLoc = aooPacket.aooLocs[i];
			d20aAoOMovement.distTraversed = pathfindingSys.pathLength(pqrTrunc);
			actSeq->d20ActArray[actSeq->d20ActArrayNum] = d20aAoOMovement;
			if (!addingAoOStatus)
				actSeq->d20ActArray[actSeq->d20ActArrayNum].d20ActType = d20ActionTypePostAoO;
			addingAoOStatus = 0;
			actSeq->d20ActArrayNum++;
			if ( actSeq->d20ActArrayNum >= 32)
			{
				if (pathfindingSys.pathQueryResultIsValid(pqr))
					pqr->occupiedFlag = 0;
				return;
			}
		}
		d20aAoOMovement.d20APerformer = aooPacket.interrupters[i];
		d20aAoOMovement.d20ATarget = d20a->d20APerformer;
		d20aAoOMovement.destLoc = aooPacket.aooLocs[i];
		d20aAoOMovement.d20Caf = 0;
		d20aAoOMovement.distTraversed = 0;
		d20aAoOMovement.path = nullptr;
		d20aAoOMovement.d20ActType = D20A_AOO_MOVEMENT;
		d20aAoOMovement.data1 = 1;
		actSeq->d20ActArray[actSeq->d20ActArrayNum++] = d20aAoOMovement;
		if (actSeq->d20ActArrayNum >= 32)
		{
			if (pathfindingSys.pathQueryResultIsValid(pqr))
				pqr->occupiedFlag = 0;
			return;
		}
	}
	d20aAoOMovement = *d20a;
	auto pqrLastStretch = pathfindingSys.FetchAvailablePQRCacheSlot();
	d20aAoOMovement.path = pqrLastStretch;
	if (!pathfindingSys.GetPartialPath(pqr, pqrLastStretch, startDistFeet, endDistFeet))
	{
		if (pathfindingSys.pathQueryResultIsValid(pqr))
			pqr->occupiedFlag = 0;
		if (pathfindingSys.pathQueryResultIsValid(pqrLastStretch))
			pqrLastStretch->occupiedFlag = 0;
		return;
	}
	d20aAoOMovement.destLoc = d20a->destLoc;
	d20aAoOMovement.distTraversed = pathfindingSys.pathLength(pqrLastStretch);
	if (!addingAoOStatus)
		d20aAoOMovement.d20ActType = d20ActionTypePostAoO;
	actSeq->d20ActArray[actSeq->d20ActArrayNum++] = d20aAoOMovement;
	if (actSeq->d20ActArrayNum != 32)
	{
		pqr->occupiedFlag = 0;
	}
	return;

	/*{ __asm push ecx __asm push esi __asm push ebx __asm push edi};
	__asm{
		mov ecx, this;
		mov esi, [ecx]._sub_1008BB40;
		mov eax, d20a;
		push eax;
		mov ebx, actSeq;
		call esi;
		add esp, 4;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx}*/
}

int ActionSequenceSystem::CrossBowSthgReload_1008E8A0(D20Actn* d20a, ActnSeq* actSeq)
{
	int result = 0;
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi};;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._CrossBowSthgReload_1008E8A0;
		mov eax, d20a;
		push eax;
		mov ebx, actSeq;
		call esi;
		add esp, 4;
		mov result, eax;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx} 
	return result;
}

uint32_t ActionSequenceSystem::TurnBasedStatusUpdate(D20Actn* d20a, TurnBasedStatus* tbStatus)
{
	uint32_t result = 0;
	TurnBasedStatus tbStatCopy;
	
	memcpy(&tbStatCopy, tbStatus, sizeof(TurnBasedStatus));

	int actProcResult = ActionCostProcess(&tbStatCopy, d20a);
	if (!actProcResult)
	{
		memcpy(tbStatus, &tbStatCopy, sizeof(TurnBasedStatus));
		return 0;
	}

	auto tbsCheckFunc = d20Sys.d20Defs[d20a->d20ActType].turnBasedStatusCheck;
	if (tbsCheckFunc)
	{
		if (tbsCheckFunc(d20a, &tbStatCopy))
		{
			return actProcResult;
		}
		actProcResult = ActionCostProcess(&tbStatCopy, d20a);
	}

	if (!actProcResult)
	{
		memcpy(tbStatus, &tbStatCopy, sizeof(TurnBasedStatus));
		return 0;
	}
	return actProcResult;
	/*
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi};
	__asm{
		mov ecx, this;
		mov esi, tbStat;
		push esi;
		mov esi, [ecx]._TurnBasedStatusUpdate;
		mov ebx, d20a;
		call esi;
		add esp, 4;

		mov result, eax;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx} 
	return result;
	*/
}

uint32_t ActionSequenceSystem::SequencePathSthgSub_10096450(ActnSeq* actSeq, uint32_t idx ,TurnBasedStatus * tbStat)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;


mov esi, idx;
push esi;
mov esi, [ecx]._sub_10096450;
mov ebx, tbStat;
mov eax, actSeq;
push eax;
call esi;
add esp, 8;

mov result, eax;
pop ebx;
pop ecx;
pop esi;
	}
	return result;
}

uint32_t ActionSequenceSystem::seqCheckFuncs(TurnBasedStatus* tbStatus)
{
	LocAndOffsets seqPerfLoc;
	ActnSeq * curSeq = *actSeqCur;
	uint32_t result = 0;

	if (!curSeq)
	{
		memset(tbStatus, 0, sizeof(TurnBasedStatus));		return 0;
	}

	*tbStatus = curSeq->tbStatus;
	objects.loc->getLocAndOff(curSeq->performer, &seqPerfLoc);
	for (int32_t i = 0; i < curSeq->d20ActArrayNum; i++)
	{
		auto d20a = &curSeq->d20ActArray[i];
		if (curSeq->d20ActArrayNum <= 0) return 0;


		auto d20type = curSeq->d20ActArray[i].d20ActType;
		auto tgtCheckFunc = d20->d20Defs[d20type].tgtCheckFunc;
		if (tgtCheckFunc)
		{
			result = tgtCheckFunc(&curSeq->d20ActArray[i], tbStatus);	if (result) break;
		}


		result = TurnBasedStatusUpdate(d20a, tbStatus);
		if (result)
		{
			tbStatus->errCode = result;	break;
		}

		auto actCheckFunc = d20->d20Defs[d20a->d20ActType].actionCheckFunc;
		if (actCheckFunc)
		{
			result = actCheckFunc(d20a, tbStatus);		if (result) break;
		}

		auto locCheckFunc = d20->d20Defs[d20type].locCheckFunc;
		if (locCheckFunc)
		{
			result = locCheckFunc(d20a, tbStatus, &seqPerfLoc); if (result) break;
		}

		auto path = curSeq->d20ActArray[i].path;
		if (path)
			seqPerfLoc = path->to;
	}
	if (result)
	{
		if (!*actSeqCur) { memset(tbStatus, 0, sizeof(TurnBasedStatus)); }
		else
		{
			*tbStatus = (*actSeqCur)->tbStatus;
		}
	}

	return result;

}

void ActionSequenceSystem::DoAoo(objHndl obj, objHndl target)
{
	AssignSeq(obj);
	auto curSeq = *actSeqCur;
	curSeq->performer = obj;

	logger->debug("AOO - {} ({}) is interrupting {} ({})",
		description.getDisplayName(obj), obj,
		description.getDisplayName(target), target);

	if (obj != d20Sys.globD20Action->d20APerformer)
	{
		*seqPickerTargetingType = D20TC_Invalid;
		*seqPickerD20ActnType = D20A_UNSPECIFIED_ATTACK;
		*seqPickerD20ActnData1 = 0;
	}
	d20Sys.globD20Action->d20APerformer = obj;
	auto tbStat = &curSeq->tbStatus;
	tbStat->tbsFlags = TBSF_NONE;
	tbStat->surplusMoveDistance = 0.0;
	tbStat->attackModeCode = 0;
	tbStat->baseAttackNumCode = 0;
	tbStat->numBonusAttacks = 0;
	tbStat->numAttacks = 0;
	tbStat->errCode = 0;
	tbStat->hourglassState = 2;
	tbStat->idxSthg = -1;
	curSeq->seqOccupied |= SEQF_2;

	d20Sys.D20ActnInit(obj, d20Sys.globD20Action);
	if (critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary))
	{
		d20Sys.globD20Action->data1 = ATTACK_CODE_PRIMARY + 1;
	}
	else
	{
		if (dispatch.DispatchD20ActionCheck(d20Sys.globD20Action, tbStat, dispTypeGetCritterNaturalAttacksNum) <= 0 )
		{
			d20Sys.globD20Action->data1 = ATTACK_CODE_PRIMARY + 1;
		} else
		{
			d20Sys.globD20Action->data1 = ATTACK_CODE_NATURAL_ATTACK + 1;
		}
	}
	d20Sys.globD20Action->d20Caf |= D20CAF_ATTACK_OF_OPPORTUNITY;
	d20Sys.globD20Action->d20ActType = D20A_ATTACK_OF_OPPORTUNITY;
	auto tgtLoc = objects.GetLocationFull(target);
	d20Sys.GlobD20ActnSetTarget(target, &tgtLoc);
	ActionAddToSeq();
	d20Sys.d20SendSignal(obj, DK_SIG_AOOPerformed, target);
	// _AOOSthgSub_10097D50(objHnd1, objHnd2);
}

int32_t ActionSequenceSystem::DoAoosByAdjcentEnemies(objHndl obj)
{
	int status = 0;

	objHndl enemies[40];

	int numEnemies = combatSys.GetEnemiesCanMelee(obj, enemies);

	for (int i = 0; i < numEnemies; i++)
	{
		auto enemy = enemies[i];
		bool okToAoo = true;
		if (objects.GetFlags(enemy) & OF_INVULNERABLE) // bug? or maybe it also assumes the obj is "trapped" somehow, like in otiluke's resilient sphere
			continue;
		
		
		for (int j = 0; j < ACT_SEQ_ARRAY_SIZE; j++)
		{
			if ( (actSeqArray[j].seqOccupied & 1)
				&& actSeqArray[j].performer == enemy)
			{
				okToAoo = false;
				logger->debug("DoAoosByAdjacentEnemies({}({})): Action Aoo for {} ({}) while they are performing...", description.getDisplayName(obj), obj , description.getDisplayName(enemy), enemy);
			}
		}

		if (!okToAoo)
			continue;
		if (critterSys.IsFriendly(obj, enemy))
			continue;
		if (!d20Sys.d20QueryWithData(enemy, DK_QUE_AOOPossible, obj))
			continue;
		if (!d20Sys.d20QueryWithData(enemy, DK_QUE_AOOWillTake, obj))
			continue;
		DoAoo(enemy, obj);
		status = 1;
	}

	return status;
	// return _AOOSthg2_100981C0(obj);
}

int32_t ActionSequenceSystem::InterruptNonCounterspell(D20Actn* d20a)
{
	auto readiedAction = ReadiedActionGetNext(nullptr, d20a);
	if (!readiedAction)
		return 0;

	while (1){
		
		if (readiedAction->readyType != RV_Counterspell) {

			if (d20a->d20ActType == D20A_CAST_SPELL){
				if (d20a->d20APerformer && readiedAction->interrupter){
					auto isFriendly = critterSys.IsFriendly(readiedAction->interrupter, d20a->d20APerformer);
					auto sharedAlleg = critterSys.AllegianceShared(readiedAction->interrupter, d20a->d20APerformer);
					if (!sharedAlleg && !isFriendly)
						break;
				}	
			}

			if (d20a->d20ActType == D20A_READIED_INTERRUPT ) {
				if (d20a->d20ATarget && readiedAction->interrupter) {
					auto isFriendly = critterSys.IsFriendly(readiedAction->interrupter, d20a->d20ATarget);
					auto sharedAlleg = critterSys.AllegianceShared(readiedAction->interrupter, d20a->d20ATarget);
					if (!sharedAlleg && !isFriendly)
						break;
				}
			}
		}

		readiedAction = ReadiedActionGetNext(readiedAction, d20a);
		if (!readiedAction)
			return 0;
	}
	InterruptSwitchActionSequence(readiedAction);
	return 1;

	/*uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, [ecx]._InterruptNonCounterspell;
		mov eax, d20a;
		call esi;
		mov result, eax;
		pop ecx;
		pop esi;
	}
	return result;*/
}

int32_t ActionSequenceSystem::InterruptCounterspell(D20Actn* d20a)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;
		mov esi, [ecx]._InterruptCounterspell;
		mov ebx, d20a;
		call esi;
		mov result, eax;
		pop ebx;
		pop ecx;
		pop esi;
	}
	return result;

}

int ActionSequenceSystem::ReadyVsApproachOrWithdrawalCount()
{
	int result = 0;
	auto readiedCache = addresses.readiedActionCache;
	for (int i = 0; i < READIED_ACTION_CACHE_SIZE; i++)
	{
		if (readiedCache[i].flags == 1 && !(!readiedCache[i].interrupter)
			&&	(readiedCache[i].readyType == ReadyVsTypeEnum::RV_Approach
				|| readiedCache[i].readyType == ReadyVsTypeEnum::RV_Withdrawal ))
			result++;
	}
	return result;
}

void ActionSequenceSystem::ReadyVsRemoveForObj(objHndl obj)
{
	auto readiedCache = addresses.readiedActionCache;
	for (int i = 0; i < READIED_ACTION_CACHE_SIZE; i++){
		if (readiedCache[i].flags == 1 && readiedCache[i].interrupter == obj) {
			readiedCache[i].flags = 0;
			readiedCache[i].interrupter = 0i64;
		}
	}
}

ReadiedActionPacket* ActionSequenceSystem::ReadiedActionGetNext(ReadiedActionPacket* prevReadiedAction, D20Actn* d20a)
{
	auto readiedActionCache = addresses.readiedActionCache;
	int i0=0;

	// find the prevReadiedAction in the array (kinda retarded since it's a pointer???)
	if (prevReadiedAction)
	{
		for (i0 = 0; i0 < READIED_ACTION_CACHE_SIZE; i0++)
		{
			if (&readiedActionCache[i0] == prevReadiedAction)
			{
				i0++;
				break;
			}
		}
		if (i0 >= READIED_ACTION_CACHE_SIZE) return nullptr;
	}


	for (int i = i0; i < READIED_ACTION_CACHE_SIZE; i++)
	{
		if (!(readiedActionCache[i].flags & 1))
			continue;
		switch (readiedActionCache[i].readyType)
		{
			case RV_Spell:
			case RV_Counterspell:
				if (critterSys.IsFriendly(d20a->d20APerformer, readiedActionCache[i].interrupter))
					continue;
				if (d20a->d20ActType == D20A_CAST_SPELL)
					return &readiedActionCache[i];
				break;
			case RV_Approach:
			case RV_Withdrawal:
			default:
				if (d20a->d20ActType != D20A_READIED_INTERRUPT)
					continue;
				if (d20a->d20APerformer != readiedActionCache[i].interrupter)
					continue;
				return &readiedActionCache[i];
		}
	}

	return nullptr;
	
}

void ActionSequenceSystem::InterruptSwitchActionSequence(ReadiedActionPacket* readiedAction)
{
	if (readiedAction->readyType == RV_Counterspell)
		return addresses.Counterspell_sthg(readiedAction);
	
	(*actSeqCur)->interruptSeq = *addresses.actSeqInterrupt;
	*addresses.actSeqInterrupt = *actSeqCur;
	combatSys.FloatCombatLine((*actSeqCur)->performer, 158); // Action Interrupted
	logger->debug("{} interrupted by {}!", description.getDisplayName((*actSeqCur)->performer), description.getDisplayName( readiedAction->interrupter));
	histSys.CreateRollHistoryLineFromMesfile(7, readiedAction->interrupter, (*addresses.actSeqInterrupt)->performer);
	AssignSeq(readiedAction->interrupter);
	(*actSeqCur)->prevSeq = nullptr;
	auto curActor = tbSys.turnBasedGetCurrentActor();
	tbSys.CloneInitiativeFromObj(readiedAction->interrupter, curActor);
	tbSys.turnBasedSetCurrentActor(readiedAction->interrupter);
	(*actSeqCur)->performer = readiedAction->interrupter;
	d20Sys.globD20ActnSetPerformer(readiedAction->interrupter);
	(*actSeqCur)->tbStatus.hourglassState = 3;
	(*actSeqCur)->tbStatus.tbsFlags = 0;
	(*actSeqCur)->tbStatus.idxSthg = -1;
	(*actSeqCur)->tbStatus.surplusMoveDistance = 0;
	(*actSeqCur)->tbStatus.attackModeCode = 0;
	(*actSeqCur)->tbStatus.baseAttackNumCode = 0;
	(*actSeqCur)->tbStatus.numBonusAttacks = 0;
	(*actSeqCur)->tbStatus.numAttacks = 0;
	(*actSeqCur)->tbStatus.errCode = 0;
	(*actSeqCur)->seqOccupied &= 0xFFFFfffe;
	d20Sys.D20ActnInit(d20Sys.globD20Action->d20APerformer, d20Sys.globD20Action);
	readiedAction->flags = 0;
	party.CurrentlySelectedClear();
	if (objects.IsPlayerControlled(readiedAction->interrupter))
	{
		party.AddToCurrentlySelected(readiedAction->interrupter);
	} else
	{
		combatSys.TurnProcessAi(readiedAction->interrupter);
	}

	if (d20Sys.d20Query(readiedAction->interrupter, DK_QUE_Prone))
	{
		if ((*actSeqCur)->tbStatus.hourglassState >= 1 )
		{
			d20Sys.D20ActnInit(d20Sys.globD20Action->d20APerformer, d20Sys.globD20Action);
			d20Sys.globD20Action->d20ActType = D20A_STAND_UP;
			d20Sys.globD20Action->data1 = 0;
			ActionAddToSeq();
			sequencePerform();
		}
	}
	ui.UpdateCombatUi();

}

uint32_t ActionSequenceSystem::combatTriggerSthg(ActnSeq* actSeq)
{
	uint32_t result;
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._combatTriggerSthg;
		mov ebx, actSeq;
		call esi;
		mov result, eax;
	}
	macAsmEpil;
	
	return result;
}

ActionErrorCode ActionSequenceSystem::seqCheckAction(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	ActionErrorCode errorCode = (ActionErrorCode)TurnBasedStatusUpdate(d20a, tbStat);
	if (errorCode)
	{
		tbStat->errCode = errorCode;
		return errorCode;
	}
	
	auto actionCheckFunc = d20->d20Defs[ d20a->d20ActType].actionCheckFunc;
	if (actionCheckFunc)
	{
		return (ActionErrorCode)actionCheckFunc(d20a, tbStat);
	}
	return AEC_OK;
}

uint32_t ActionSequenceSystem::curSeqNext()
{
	ActnSeq* curSeq = *actSeqCur;
	objHndl performer = curSeq->performer;
	SpellPacketBody spellPktBody;
	curSeq->seqOccupied &= 0xffffFFFE; //unset "occupied" flag
	logger->debug("CurSeqNext: \t Sequence Completed for {} ({}) (sequence {})",
		description._getDisplayName(curSeq->performer, curSeq->performer),
		curSeq->performer, (void*)curSeq);

	int d20ActArrayNum = curSeq->d20ActArrayNum;
	if (d20ActArrayNum > 0)
	{
		d20Sys.d20SendSignal(
			(*actSeqCur)->d20ActArray[d20ActArrayNum - 1].d20APerformer,
			DK_SIG_Sequence,
			(int)*actSeqCur, 0);
	}
		

	// do d20SendSignal for DK_SIG_Action_Recipient
	for (int d20aIdx = 0; d20aIdx < (*actSeqCur)->d20ActArrayNum; d20aIdx++)
	{
		D20Actn* d20a = &(*actSeqCur)->d20ActArray[d20aIdx];
		auto d20aType = d20a->d20ActType;
		if (d20aType == D20A_CAST_SPELL)
		{
			auto spellId = d20a->spellId;
			if (spellId)
			{
				if (spellSys.GetSpellPacketBody(spellId, &spellPktBody ))
				{
					for (int i = 0; i < spellPktBody.orgTargetCount; i++)
					{
						if (spellPktBody.targetListHandles[i])
						{
							d20Sys.d20SendSignal(spellPktBody.targetListHandles[i],
								DK_SIG_Action_Recipient,
								(int)d20a,0);
						}
					}
				} else
				{
					logger->warn("CurSeqNext(): \t  unable to retrieve spell packet!");
				}
			}
		}
		else
		{
			auto d20aTarget = d20a->d20ATarget;
			bool triggersCombat = (d20Sys.d20Defs[d20aType].flags & D20ADF_TriggersCombat) != 0;
			if (triggersCombat || (d20aType == D20A_LAY_ON_HANDS_USE && critterSys.IsUndead(d20aTarget)) )
			{
				if (d20aTarget)
				{
					d20Sys.d20SendSignal(d20aTarget,
						DK_SIG_Action_Recipient,
						(int)d20a, 0);
				}
			} else
			{
				d20Sys.d20SendSignal((*actSeqCur)->performer, DK_SIG_Action_Recipient,
					(int)d20a, 0);
			}
		}
	}

	if (SequencePop())
	{
		if ((*actSeqCur)->d20aCurIdx + 1 < (*actSeqCur)->d20ActArrayNum)
			ActionPerform();
		return 0;
	}

	AssignSeq(performer);
	logger->debug("CurSeqNext: \t  Resetting Sequence ({})", (void*)*actSeqCur);
	curSeqReset(d20Sys.globD20Action->d20APerformer);
	if (combatSys.isCombatActive())
	{
		// look for stuff that terminates / interrupts the turn
		if (HasReadiedAction(d20Sys.globD20Action->d20APerformer))
		{
			logger->debug("CurSeqNext: \t Action for {} ({}) ending turn (readied action)...",
				description.getDisplayName(d20Sys.globD20Action->d20APerformer),
				d20Sys.globD20Action->d20APerformer);
			combatSys.CombatAdvanceTurn(tbSys.turnBasedGetCurrentActor());
			return 1;
		}
		if (ShouldAutoendTurn(&(*actSeqCur)->tbStatus))
		{
			logger->debug("CurSeqNext: \t Action for {} ({}) ending turn (autoend)...",
				description.getDisplayName(d20Sys.globD20Action->d20APerformer),
				d20Sys.globD20Action->d20APerformer);
			combatSys.CombatAdvanceTurn(tbSys.turnBasedGetCurrentActor());
			return 1;
		}
		if (!objects.IsPlayerControlled((*actSeqCur)->performer))
		{
			if (isSimultPerformer((*actSeqCur)->performer)
				|| *actnProcState || *addresses.seqSthg_10B3D59C > 5)
			{
				combatSys.CombatAdvanceTurn(tbSys.turnBasedGetCurrentActor());
			}
			else
			{
				(*actSeqCur)->seqOccupied &= 0xFFFFfffe;
				(*addresses.seqSthg_10B3D59C )++;
				aiSys.AiProcess((*actSeqCur)->performer);
			}
		}
		if (combatSys.isCombatActive()
			&& !(*actSeqPickerActive)
			&& (*actSeqCur)
			&& objects.IsPlayerControlled((*actSeqCur)->performer)
			&& (*actSeqCur)->tbStatus.baseAttackNumCode
			+ (*actSeqCur)->tbStatus.numBonusAttacks
					> (*actSeqCur)->tbStatus.attackModeCode
			)
		{ // I think this is for doing full attack?
			//if (d20Sys.d20Query((*actSeqCur)->performer, DK_QUE_Trip_AOO))
			//*seqPickerD20ActnType = D20A_TRIP;
			//else
			if (*seqPickerD20ActnType != D20A_TRIP)
				*seqPickerD20ActnType = D20A_STANDARD_ATTACK;
			*seqPickerD20ActnData1 = 0;
			*seqPickerTargetingType = D20TC_SingleExcSelf;
		}
	}
	return 1;
	//return _curSeqNext();
}

int ActionSequenceSystem::SequencePop()
{
	ActnSeq*  curSeq = *actSeqCur;
	ActnSeq*  prevSeq = (*actSeqCur)->prevSeq;
	curSeq->seqOccupied &= 0xFFFFfffe;
	logger->debug("Popping sequence ( {} )", (void*)curSeq);
	*actSeqCur = prevSeq;
	curSeq->prevSeq = nullptr;
	if (!prevSeq)
		return 0;
	auto curSeqPerformer = curSeq->performer;
	auto prevSeqPerformer = prevSeq->performer;
	logger->debug("Popping sequence from {} ({}) to {} ({})", 
		description.getDisplayName(curSeqPerformer),
		curSeqPerformer,
		description.getDisplayName(prevSeqPerformer),
		prevSeqPerformer);
	TurnBasedStatus tbStatNew;
	tbStatNew.hourglassState = 4;
	tbStatNew.tbsFlags = 0;
	tbStatNew.idxSthg = -1;
	tbStatNew.surplusMoveDistance = 0.0;
	tbStatNew.attackModeCode = 0;
	tbStatNew.baseAttackNumCode = 0;
	tbStatNew.numBonusAttacks = 0;
	tbStatNew.errCode = 0;
	DispIOTurnBasedStatus dispIo;
	dispatch.dispIOTurnBasedStatusInit(&dispIo);
	dispIo.tbStatus = &tbStatNew;
	d20Sys.globD20ActnSetPerformer(prevSeqPerformer);
	auto dispatcher = objects.GetDispatcher(prevSeqPerformer);
	if (dispatch.dispatcherValid(dispatcher))
	{
		dispatch.DispatcherProcessor(dispatcher, dispTypeTurnBasedStatusInit, 0, &dispIo);
	}
	if (critterSys.IsDeadOrUnconscious(prevSeqPerformer) && prevSeq->spellPktBody.spellEnum)
		spellSys.spellPacketBodyReset(&prevSeq->spellPktBody);
	if ( d20Sys.d20Query(prevSeqPerformer, DK_QUE_Prone) && prevSeq->tbStatus.hourglassState >= 1)
	{
		prevSeq->d20ActArrayNum = 0;
		d20Sys.D20ActnInit(d20Sys.globD20Action->d20APerformer, d20Sys.globD20Action);
		d20Sys.globD20Action->d20ActType = D20A_STAND_UP;
		d20Sys.globD20Action->data1 = 0;
		ActionAddToSeq();
	}
	*addresses.actSeqInterrupt = prevSeq->interruptSeq;
	prevSeq->interruptSeq = nullptr;
	return 1;
}

bool ActionSequenceSystem::ShouldAutoendTurn(TurnBasedStatus* tbStat)
{
	auto curActor = tbSys.turnBasedGetCurrentActor();
	if (config.GetVanillaInt("end_turn_default") && *addresses.seqSthg_10B3D5C4 || critterSys.IsDeadOrUnconscious(curActor))
	{
		return 1;
	}

	int endTurnTimeValue = config.GetVanillaInt("end_turn_time");
	
	if (endTurnTimeValue)
	{
		if (endTurnTimeValue == 1)
		{
			if (tbStat->hourglassState > 0)
				return 0;
		}
		if (endTurnTimeValue == 2)
		{
			if (tbStat->hourglassState > 1)
				return 0;
		}
		
	} 
	else if ( tbStat->hourglassState > 0)
	{
		return 0;
	}

	if (tbStat->surplusMoveDistance > 4.0)
	{
		if (endTurnTimeValue != 2
			|| tbStat->hourglassState > 0
			|| dispatch.Dispatch29hGetMoveSpeed(curActor,nullptr) < tbStat->surplusMoveDistance)
		{
			return 0;
		}
	}

	if (tbStat->attackModeCode < tbStat->baseAttackNumCode
		|| tbStat->numBonusAttacks
		|| !(tbStat->tbsFlags & 3) && !endTurnTimeValue)
	{
		return 0;
	}
	return 1;
}

void ActionSequenceSystem::ActionPerform()
{
	// in principle this cycles through actions,
	// but if it succeeds in performing one it will return
	MesLine mesLine;
	TurnBasedStatus tbStatus;
	int32_t * curIdx ; 
	D20Actn * d20a = nullptr;
	while (1)
	{
		ActnSeq * curSeq = *actSeqCur;
		curIdx = &curSeq->d20aCurIdx; // the action idx is inited to -1 for fresh sequences
		++*curIdx;

		// if the performer has become unconscious, abort
		objHndl performer = curSeq->performer;
		if (critterSys.IsDeadOrUnconscious(performer))
		{
			curSeq->d20ActArrayNum = curSeq->d20aCurIdx;
			logger->debug("PerformActions: \t Unconscious actor {} - cutting sequence", objects.description._getDisplayName(performer, performer));
		}

		// if have finished up the actions - do CurSeqNext
		if (curSeq->d20aCurIdx >= (int32_t)curSeq->d20ActArrayNum) 
			break;	
		
		tbStatus = curSeq->tbStatus;
		d20a = &curSeq->d20ActArray[*curIdx];
		
		auto errCode = SequencePathSthgSub_10096450( curSeq, *curIdx,&tbStatus);
		if (errCode)
		{
			
			mesLine.key = errCode + 1000;
			mesFuncs.GetLine_Safe(*actionMesHandle, &mesLine);
			logger->debug("PerformActions: \t Action unavailable for {} ({}): {}", 
				objects.description.getDisplayName(d20a->d20APerformer), d20a->d20APerformer, mesLine.value );
			*actnProcState = errCode;
			curSeq->tbStatus.errCode = errCode;
			objects.floats->floatMesLine(performer, 1, FloatLineColor::Red, mesLine.value);
			curSeq->d20ActArrayNum = curSeq->d20aCurIdx;
			break;
		}

		if (seqFlag_10B3D5C0[0]){	seqFlag_10B3D5C0[1] = 1;	}

		d20->d20aTriggerCombatCheck(curSeq, *curIdx);

		if (d20a->d20ActType != D20A_AOO_MOVEMENT)
		{
			if ( d20->D20ActionTriggersAoO(d20a, &tbStatus) && DoAoosByAdjcentEnemies(d20a->d20APerformer))
			{
				logger->debug("PerformActions: \t Sequence Preempted {} ({})", description._getDisplayName(d20a->d20APerformer, d20a->d20APerformer), d20a->d20APerformer);
				--*(curIdx);
				sequencePerform();
			} else
			{
				memcpy(&curSeq->tbStatus, &tbStatus, sizeof(tbStatus));
				*(uint32_t*)(&curSeq->tbStatus.tbsFlags) |= TBSF_AoO;
				InterruptCounterspell(d20a);
				logger->debug("PerformActions: \t Performing action for {} ({}): {}",
					description.getDisplayName(d20a->d20APerformer), 
					d20a->d20APerformer,
					(int)d20a->d20ActType);


				if (config.newFeatureTestMode && d20a->path != nullptr)
				{
					std::vector<LocAndOffsets> directionsDebug;
					for (int i = 0; i < d20a->path->nodeCount && i < 200; i++)
					{
						directionsDebug.emplace_back(d20a->path->nodes[i]);
					}
					if (d20a->d20ATarget)
						logger->debug("Move Action: {} going from {} to {} ({}), nodes used: {}", description.getDisplayName(d20a->path->mover), d20a->path->from, d20a->path->to, description.getDisplayName(d20a->d20ATarget), directionsDebug);
					else
						logger->debug("Move Action: {} going from {} to {}, nodes used: {}", description.getDisplayName(d20a->path->mover), d20a->path->from, d20a->path->to, directionsDebug);
				}

				ActionErrorCode performResult = static_cast<ActionErrorCode>(d20->d20Defs[d20a->d20ActType].performFunc(d20a));
				InterruptNonCounterspell(d20a);
			}
			return;
		}

		// otherwise, this is an AOO Movement - do an AoO check
		if (d20->tumbleCheck(d20a))
		{
			DoAoo(d20a->d20APerformer, d20a->d20ATarget);
			curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].d20Caf |= D20CAF_AOO_MOVEMENT;
			sequencePerform();
			return;
		}
	}
	if ( projectileCheckBeforeNextAction())
	{
		curSeqNext();
		return;
	}
	return;

}

void ActionSequenceSystem::sequencePerform()
{
	// check if OK to perform
	if (*actSeqPickerActive)
	{
		return;
	}

	// is curSeq ok to perform?
	if (!actSeqOkToPerform())
	{
		logger->debug("SequencePerform: \t Sequence given while performing previous action - aborted.");
		d20->D20ActnInit(d20->globD20Action->d20APerformer, d20->globD20Action);
		return;
	}

	if (*addresses.actSeqInterrupt)
	{
		int dummy = 1;
		if (*addresses.actSeqInterrupt == *actSeqCur)
		{
			int asd = 1;
		}
	}

	// try to perform the sequence and its actions
	ActnSeq * curSeq = *actSeqCur;
	if (combat->isCombatActive() || !actSeqSpellHarmful(curSeq) || !combatTriggerSthg(curSeq) ) // POSSIBLE BUG: I think this can cause spells to be overridden (e.g. when the temple priests prebuff simulataneously with you, and you get the spell effect instead) TODO
	{
		if (curSeq != *actSeqCur)
		{
			logger->debug("SequencePerform: Switched sequence slot from combat trigger!");
			curSeq = *actSeqCur;
		}
		logger->debug("SequencePerform: \t {} performing sequence ({})...", description.getDisplayName(curSeq->performer), (void*)curSeq);
		if (isSimultPerformer(curSeq->performer))
		{ 
			logger->debug("simultaneously...");
			if (!simulsOk(curSeq))
			{
				if (simulsAbort(curSeq->performer)) logger->debug("sequence not allowed... aborting simuls (pending).");
				else logger->debug("sequence not allowed... aborting subsequent simuls.");
				return;
			}
			logger->debug("succeeded...");
			
		} else
		{
			logger->debug("independently.");
		}
		*actnProcState = 0;
		curSeq->seqOccupied |= SEQF_PERFORMING;
		ActionPerform();
		for (curSeq = *actSeqCur; isPerforming(curSeq->performer); curSeq = *actSeqCur) // I think actionPerform can modify the sequence, so better be safe
		{
			if (curSeq->seqOccupied & SEQF_PERFORMING)
			{
				auto curIdx = curSeq->d20aCurIdx;
				if (curIdx >= 0 && curIdx < curSeq->d20ActArrayNum)
				{
					auto caflags = curSeq->d20ActArray[curIdx].d20Caf;
					if ( (caflags & D20CAF_NEED_PROJECTILE_HIT) || (caflags & D20CAF_NEED_ANIM_COMPLETED ) )
					break;
				}
			}
			ActionPerform();
		}
	}
}

void ActionSequenceSystem::ActionBroadcastAndSignalMoved()
{
	auto d20a = &(*actSeqCur)->d20ActArray[(*actSeqCur)->d20aCurIdx];
	d20ObjRegistrySys.D20ObjRegistrySendSignalAll(DK_SIG_Broadcast_Action, d20a, 0);
	
	auto distTrav = (int)d20a->distTraversed;
	switch (d20a->d20ActType)
	{
	case D20A_UNSPECIFIED_MOVE:
	case D20A_5FOOTSTEP:
	case D20A_MOVE:
	case D20A_DOUBLE_MOVE:
	case D20A_RUN:
	case D20A_CHARGE:
		d20Sys.d20SendSignal(d20a->d20APerformer, DK_SIG_Combat_Critter_Moved, distTrav, 0);
	default:
		return;
	}
}

int ActionSequenceSystem::ActionFrameProcess(objHndl obj)
{
	logger->debug("ActionFrameProcess: \t for {} ({})", description.getDisplayName(obj), obj);
	if (!isPerforming(obj))
	{
		logger->debug("Not performing!");
		return 0;
	}

	auto curSeq = *actSeqCur;
	if (curSeq == nullptr)
	{
		logger->debug("No sequence!");
		return 0;
	}

	if (curSeq->performer != obj)
	{
		logger->debug("..Switching sequence from {}", (void*)curSeq);
		if (!SequenceSwitch(obj))
		{
			logger->debug("..failed!");
			return 0;
		}
		curSeq = *actSeqCur;
		logger->debug("..to {}", (void*)curSeq);
	}

	auto d20a = &curSeq->d20ActArray[curSeq->d20aCurIdx];
	if (obj != d20a->d20APerformer)
	{
		return 0;
	}
	if (d20a->d20Caf & D20CAF_ACTIONFRAME_PROCESSED)
	{
		logger->debug("ActionFrameProcess: \t Action frame already processed.");
		return 0;
	}
	d20a->d20Caf |= D20CAF_ACTIONFRAME_PROCESSED;
	auto actFrameFunc = d20Sys.d20Defs[d20a->d20ActType].actionFrameFunc;
	if (!actFrameFunc)
		return 0;
	
	logger->debug("ActionFrameProcess: \t Calling action frame function");
	return actFrameFunc(d20a);	
}

void ActionSequenceSystem::PerformOnAnimComplete(objHndl obj, int animId)
{
	// do checks:

	// obj is performing
	if (!isPerforming(obj))
	{
		if (animId)
		{
			logger->debug("PerformOnAnimComplete: \t Animation {} Completed for {} ({}); Not performing.", animId, description.getDisplayName(obj), obj);
		}
		return;
	}

	logger->debug("PerformOnAnimComplete: \t Animation {} Completed for {} ({})", animId, description.getDisplayName(obj), obj);

	// does the Current Sequence belong to obj?
	auto curSeq = *actSeqCur;
	auto curSeq0 = curSeq;
	if (!curSeq || curSeq->performer != obj || !(curSeq->seqOccupied & SEQF_PERFORMING))
	{
		logger->debug("\tCurrent sequence performer is {} ({}), Switching sequence...", description.getDisplayName(curSeq->performer),curSeq->performer);
		if (!SequenceSwitch(obj))
		{
			logger->debug("\tFailed.");
			return;
		}
		curSeq = *actSeqCur;
		logger->debug("\tNew Current sequence performer is {} ({})", description.getDisplayName(curSeq->performer), curSeq->performer);
	}

	// is the animId ok?
	auto d20a = &curSeq->d20ActArray[curSeq->d20aCurIdx];
	if ( (animId != -1 && animId != 0xccccCCCC & (animId != 0 )) 
		&& d20a->animID != animId)
	{
		logger->debug("PerformOnAnimComplete: \t Wrong anim ID!");
		*actSeqCur = curSeq0;
		return;
	}

	// is the action performer correct?
	if (obj != d20a->d20APerformer)
	{
		logger->debug("PerformOnAnimComplete: \t Wrong performer!");
		*actSeqCur = curSeq0;
		return;
	}
	
	// does the Action do anything when anim completed?
	auto d20caf = d20a->d20Caf;
	if (!(d20caf & D20CAF_NEED_ANIM_COMPLETED))
		return;

	logger->debug("PerformOnAnimComplete: \t Performing.");
	d20a->d20Caf &= ~D20CAF_NEED_ANIM_COMPLETED;

	if (!(d20caf & D20CAF_ACTIONFRAME_PROCESSED))
	{
		ActionFrameProcess(obj);
	}

	if (actSeqOkToPerform())
	{
		ActionBroadcastAndSignalMoved();
		ActionPerform();
		while( isPerforming((*actSeqCur)->performer))
		{
			if (!actSeqOkToPerform())
				break;
			ActionPerform();
		}
	}
}

bool ActionSequenceSystem::projectileCheckBeforeNextAction()
{
	// 
	ActnSeq * curSeq = *actSeqCur;
	if (curSeq->d20aCurIdx < curSeq->d20ActArrayNum){ return 0; }
	if (curSeq->d20ActArrayNum <= 0){ return 1; }
	for (auto i = 0; i < curSeq->d20ActArrayNum; i++)
	{
		if (d20->d20Defs[curSeq->d20ActArray[i].d20ActType].performFunc 
			&& (curSeq->d20ActArray[i].d20Caf & D20CAF_NEED_PROJECTILE_HIT)){
			return 0;
		}
	}
	return 1;
}

uint32_t ActionSequenceSystem::actSeqSpellHarmful(ActnSeq* actSeq)
{
	uint32_t result;
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._actSeqSpellHarmful;
		mov ebx, actSeq;
		call esi;
		mov result, eax;
	}
	macAsmEpil;
	return result;
}

uint32_t ActionSequenceSystem::isSimultPerformer(objHndl objHnd)
{
	uint32_t numPerfs = *numSimultPerformers;
	objHndl * perfs = simultPerformerQueue;
	assert(numPerfs < 10000);
	if (numPerfs <= 0){ return 0; }
	for (uint32_t i = 0; i < numPerfs; i++) 
	{
		if (perfs[i] == objHnd){ return 1; }
	}
	return 0;
}

uint32_t ActionSequenceSystem::simulsOk(ActnSeq* actSeq)
{
	auto numd20as = actSeq->d20ActArrayNum;
	if (numd20as <= 0){ return 1; }
	auto d20a = &actSeq->d20ActArray[0];
	auto numStdAttkActns = 0;
	while (d20Sys.d20Defs[d20a->d20ActType].flags & D20ADF_SimulsCompatible)
	{
		++d20a;
		++numStdAttkActns;
		if (numStdAttkActns >= numd20as) return 1;
	}
	if (isSomeoneAlreadyActingSimult(actSeq->performer))
	{
		return 0;
	} else
	{
		*numSimultPerformers = 0;
		*simultPerformerQueue = 0i64;
		logger->debug("first simul actor, proceeding");

	}
	return 1;

}

uint32_t ActionSequenceSystem::simulsAbort(objHndl objHnd)
{
	// aborts sequence; returns 1 if objHnd is not the first in queue
	if (!combat->isCombatActive()) return 0;
	uint32_t isFirstInQueue = 1;
	auto numSimuls = *numSimultPerformers;
	if (numSimuls <= 0) return 0;

	for (uint32_t i = 0; i < numSimuls; i++)
	{
		if (objHnd == simultPerformerQueue[i])
		{
			if (isFirstInQueue)
			{
				*numSimultPerformers = 0;
				*simultPerformerQueue = 0i64;
				return 0;
			}
			else{
				*numSimultPerformers = *simulsIdx;
				memcpy(tbStatus118CD3C0, &(*actSeqCur)->tbStatus, sizeof(TurnBasedStatus));
				logger->debug("Simul aborted {} ({})", description._getDisplayName(objHnd, objHnd), *simulsIdx);
				return 1;
			}
		}

		if (isPerforming(simultPerformerQueue[i]))	isFirstInQueue = 0;
	}
	
	return 0;
}

uint32_t ActionSequenceSystem::isSomeoneAlreadyActingSimult(objHndl objHnd)
{
	if (*numSimultPerformers == 0) return 0;
	assert(*numSimultPerformers < 10000);
	for (uint32_t i = 0; i < *numSimultPerformers; i++)
	{
		if (objHnd == simultPerformerQueue[i]) return 0;

		auto perf = simultPerformerQueue[i];
		for (auto j = 0; j < ACT_SEQ_ARRAY_SIZE; j++)
		{
			if (actSeqArray[j].seqOccupied &&actSeqArray[j].performer == perf) return 1;
		}
	}
	return 0;
}

BOOL ActionSequenceSystem::IsSimulsCompleted()
{
	auto func =  temple::GetRef<BOOL(__cdecl)()>(0x10092110);
	return func();
}

BOOL ActionSequenceSystem::IsLastSimultPopped(objHndl obj)
{
	return obj == simultPerformerQueue[*numSimultPerformers];
}

BOOL ActionSequenceSystem::IsLastSimulsPerformer(objHndl obj)
{
	return addresses.IsLastSimulsPerformer(obj);
}

BOOL ActionSequenceSystem::SimulsAdvance()
{
	*simulsIdx = *numSimultPerformers - 1;
	auto actor = tbSys.turnBasedGetCurrentActor();
	for (int i = 0; i < *numSimultPerformers;i++)
	{
		if (actor == simultPerformerQueue[i])
		{
			*simulsIdx = i;
			break;
		}
	}
	if (*simulsIdx >= *numSimultPerformers - 1)
		return 0;
	logger->debug("Advancing to simul current {}", ++*simulsIdx);
	return 1;
}

int ActionSequenceSystem::ActionCostFullAttack(D20Actn* d20, TurnBasedStatus* tbStat, ActionCostPacket* acp)
{
	acp->chargeAfterPicker = 0;
	acp->moveDistCost = 0;
	acp->hourglassCost = 4;
	int flags = d20->d20Caf;
	if (d20->d20Caf & D20CAF_FREE_ACTION || !combat->isCombatActive() )  
		acp->hourglassCost = 0;
	if (tbStat->attackModeCode >= tbStat->baseAttackNumCode && tbStat->hourglassState >= 4 && !tbStat->numBonusAttacks)
	{
		FullAttackCostCalculate(d20, tbStat, (int*)&tbStat->baseAttackNumCode, (int*) &tbStat->numBonusAttacks,
			(int*)&tbStat->numAttacks,(int*) &tbStat->attackModeCode);
		tbStat->surplusMoveDistance = 0;
		tbStat->tbsFlags = tbStat->tbsFlags | TBSF_FullAttack;
	}

	return 0;
}

void ActionSequenceSystem::FullAttackCostCalculate(D20Actn* d20a, TurnBasedStatus* tbStatus, int* baseAttackNumCode, int* bonusAttacks, int* numAttacks, int* attackModeCode)
{
	objHndl  performer = d20a->d20APerformer;
	int usingOffhand = 0;
	int _attackTypeCodeHigh = 1;
	int _attackTypeCodeLow = 0;
	int numAttacksBase = 0;
	auto mainWeapon = inventory.ItemWornAt(performer, 3);
	auto offhand = inventory.ItemWornAt(performer, 4);

	if (offhand)
	{
		if (objects.GetType(offhand) != obj_t_armor)
		{
			_attackTypeCodeHigh = ATTACK_CODE_OFFHAND + 1; // originally 5
			_attackTypeCodeLow = ATTACK_CODE_OFFHAND; // originally 4
			usingOffhand = 1;
		}
	}
	if (mainWeapon)
	{
		int weapFlags = objects.getInt32(mainWeapon, obj_f_weapon_flags);
		if (weapFlags & OWF_RANGED_WEAPON)
			d20a->d20Caf |= D20CAF_RANGED;

	}
	if (!mainWeapon && !offhand)
	{
		numAttacksBase = dispatch.DispatchD20ActionCheck(d20a, tbStatus, dispTypeGetCritterNaturalAttacksNum);
		if (numAttacksBase > 0)
		{
			_attackTypeCodeHigh = ATTACK_CODE_NATURAL_ATTACK + 1; // originally 10
			_attackTypeCodeLow = ATTACK_CODE_NATURAL_ATTACK; // originally 9
		}
	}

	if (numAttacksBase <= 0)
	{
		numAttacksBase = dispatch.DispatchD20ActionCheck(d20a, tbStatus, dispTypeGetNumAttacksBase);
	}

	*bonusAttacks = dispatch.DispatchD20ActionCheck(d20a, tbStatus, dispTypeGetBonusAttacks);
	*numAttacks = usingOffhand + numAttacksBase + *bonusAttacks;
	*attackModeCode = _attackTypeCodeLow;
	*baseAttackNumCode = numAttacksBase + _attackTypeCodeHigh - 1 + usingOffhand;
}

int ActionSequenceSystem::TouchAttackAddToSeq(D20Actn* d20Actn, ActnSeq* actnSeq, TurnBasedStatus* turnBasedStatus)
{
	return addresses.TouchAttackAddToSeq(d20Actn, actnSeq, turnBasedStatus);
}

int ActionSequenceSystem::ActionCostProcess(TurnBasedStatus* tbStat, D20Actn* d20a)
{
	ActionCostPacket actCost;
	int hourglassCost;

	int result = d20Sys.d20Defs[d20a->d20ActType].actionCost(d20a, tbStat, &actCost);
	if (result)
		return result;

	
	hourglassCost = actCost.hourglassCost;
	if (tbStat->hourglassState == -1)
		tbStat->hourglassState = -1;
	else
		tbStat->hourglassState = turnBasedStatusTransitionMatrix[tbStat->hourglassState][actCost.hourglassCost];

	if (tbStat->hourglassState == -1) // this is an error state I think
	{
		result = AEC_NOT_ENOUGH_TIME3;
		if (hourglassCost <= 4)
		{
			switch (hourglassCost)
			{
			case 1:
				tbStat->errCode = AEC_NOT_ENOUGH_TIME2;
				return AEC_NOT_ENOUGH_TIME2;
			case 2:
				tbStat->errCode = AEC_NOT_ENOUGH_TIME1;
				return AEC_NOT_ENOUGH_TIME1;
			case 4:
				tbStat->errCode = AEC_NOT_ENOUGH_TIME3;
				return AEC_NOT_ENOUGH_TIME3;
			case 3:
				tbStat->errCode = AEC_NOT_ENOUGH_TIME3;
				break;
			default:
				break;
			}
		}

	}

	if (tbStat->surplusMoveDistance >= actCost.moveDistCost)
	{
		tbStat->surplusMoveDistance -= actCost.moveDistCost;
		if ( actCost.chargeAfterPicker <= 0 
			|| actCost.chargeAfterPicker + tbStat->attackModeCode <= tbStat->baseAttackNumCode + tbStat->numBonusAttacks)
		{
			if ((int) tbStat->numBonusAttacks < actCost.chargeAfterPicker)
				tbStat->attackModeCode += actCost.chargeAfterPicker;
			else
				tbStat->numBonusAttacks -= actCost.chargeAfterPicker;
			if (tbStat->attackModeCode == tbStat->baseAttackNumCode && !tbStat->numBonusAttacks)
				tbStat->tbsFlags &= ~TBSF_Movement2;  
			result = AEC_OK;
		}
		else
		{
			result = AEC_NOT_ENOUGH_TIME1;
		}
			
	}
	else
	{
		result = (tbStat->tbsFlags & TBSF_Movement2) != 0 ? AEC_ALREADY_MOVED : AEC_TARGET_OUT_OF_RANGE;
	}

	return result;

		
	
}

int ActionSequenceSystem::TurnBasedStatusUpdate(TurnBasedStatus* tbStat, D20Actn* d20a)
{
	return TurnBasedStatusUpdate(d20a, tbStat);
}

int ActionSequenceSystem::UnspecifiedAttackAddToSeqRangedMulti(ActnSeq* actSeq, D20Actn* d20a, TurnBasedStatus* tbStat)
{
	D20Actn d20aCopy;
	int baseAttackNumCode = tbStat->baseAttackNumCode;
	int numBonusAttacks = tbStat->numBonusAttacks;
	int attackModeCode = tbStat->attackModeCode;
	d20a->d20Caf |= D20CAF_RANGED;

	int bonusAttackNumCode = attackModeCode + numBonusAttacks;
	int attackCode = attackModeCode + 1;
	auto weapon = inventory.ItemWornAt(d20a->d20APerformer, 3);
	if (weapon)
	{
		WeaponAmmoType ammoType = (WeaponAmmoType)objects.getInt32(weapon, obj_f_weapon_ammo_type);
		if (ammoType > wat_dagger && ammoType <= wat_bottle) // thrown weapons   TODO: should this include daggers??
		{
			d20a->d20Caf |= D20CAF_THROWN;
			if (ammoType != wat_shuriken && !feats.HasFeatCount(d20a->d20APerformer, FEAT_QUICK_DRAW))
			{
				baseAttackNumCode = attackModeCode + 1;
				bonusAttackNumCode = attackModeCode;
			}
		}
	}

	if (d20->d20Query(d20a->d20APerformer, DK_QUE_Prone))
	{
		memcpy(&d20aCopy, d20a, sizeof(D20Actn));
		d20aCopy.d20ActType = D20A_STAND_UP;
		memcpy(&actSeq->d20ActArray[actSeq->d20ActArrayNum++], &d20aCopy, sizeof(D20Actn));
	}

	for (int i = bonusAttackNumCode - attackModeCode; i > 0; i--)
	{
		AttackAppend(actSeq, d20a, tbStat, attackCode);
	}

	for (int i = baseAttackNumCode - attackModeCode; i > 0; i--)
	{
		AttackAppend(actSeq, d20a, tbStat, attackCode);
		attackCode++;
	}
	return 0;
}

int ActionSequenceSystem::UnspecifiedAttackAddToSeqMeleeMulti(ActnSeq* actSeq, TurnBasedStatus* tbStat, D20Actn* d20a)
{
	int  attackModeCode = tbStat->attackModeCode;
	int  baseAttackNumCode = tbStat->baseAttackNumCode;
	int  attackCode = attackModeCode + 1;

	for (int i = tbStat->numBonusAttacks; i > 0; i--)
	{
		AttackAppend(actSeq, d20a, tbStat, attackCode);
	}

	for (int i = baseAttackNumCode - attackModeCode; i > 0; i--)
	{

		AttackAppend(actSeq, d20a, tbStat, attackCode);
		attackCode++;
	}
	return 0;
}

int ActionSequenceSystem::UnspecifiedAttackAddToSeq(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat)
{
	objHndl objHnd = d20a->d20ATarget;
	objHndl performer = d20a->d20APerformer;
	D20Actn d20aCopy;
		memcpy(&d20aCopy, d20a, sizeof(D20Actn));
	int d20aNumInitial = actSeq->d20ActArrayNum;
	ActionCostPacket acp;
	int junk =0;
	int numAttacks = 0;


	*seqFlag_10B3D5C0 = 1;
	if (!objHnd) return 9;
	if (d20->d20Query(d20a->d20APerformer, DK_QUE_HoldingCharge))
		return TouchAttackAddToSeq(d20a, actSeq, tbStat);
	float reach = critterSys.GetReach(d20a->d20APerformer, d20a->d20ActType);
	auto weapon = inventory.ItemWornAt(performer, 3);
	TurnBasedStatus tbStatCopy;
	memcpy(&tbStatCopy, tbStat, sizeof(TurnBasedStatus));
	if (weapon)
	{
		int weapFlags = objects.getInt32(weapon, obj_f_weapon_flags);
		if (weapFlags & OWF_RANGED_WEAPON)
		{
			d20aCopy.d20Caf |= D20CAF_RANGED;
			if (inventory.IsNormalCrossbow(weapon))
			{
				ActionCostReload(d20a, &tbStatCopy, &acp);
				if (acp.hourglassCost)
				{
					d20aCopy.d20ActType = D20A_STANDARD_RANGED_ATTACK;
					return CrossBowSthgReload_1008E8A0(&d20aCopy, actSeq);
				}
			}
			FullAttackCostCalculate(&d20aCopy, &tbStatCopy, &junk, &junk, &numAttacks, &junk );
			d20aCopy.d20ActType = D20A_FULL_ATTACK;
			if (numAttacks > 1 && !TurnBasedStatusUpdate(&tbStatCopy, &d20aCopy))
			{
				memcpy(&actSeq->d20ActArray[actSeq->d20ActArrayNum++], &d20aCopy, sizeof(D20Actn));
				d20aCopy.d20ActType = inventory.IsThrowingWeapon(weapon) != 0 ? D20A_THROW : D20A_STANDARD_RANGED_ATTACK;
				return UnspecifiedAttackAddToSeqRangedMulti(actSeq, &d20aCopy, &tbStatCopy);
			}
			memcpy(&d20aCopy, d20a, sizeof(D20Actn));
			d20aCopy.d20ActType = inventory.IsThrowingWeapon(weapon) != 0 ? D20A_THROW : D20A_STANDARD_RANGED_ATTACK;
			int result = TurnBasedStatusUpdate(&tbStatCopy, &d20aCopy);
			if (!result)
			{
				int attackCode = tbStatCopy.attackModeCode;
				AttackAppend(actSeq, &d20aCopy, tbStat, attackCode);
			}
			return result;
		}
	}
	location->getLocAndOff(objHnd, &d20aCopy.destLoc);
	if (location->DistanceToObj(performer, objHnd) > reach)
	{
		d20aCopy.d20ActType = D20A_UNSPECIFIED_MOVE;
		int result = moveSequenceParse(&d20aCopy, actSeq, tbStat, 0.0, reach, 1);
		if (result)
			return result;
		memcpy(&tbStatCopy, tbStat, sizeof(TurnBasedStatus));

	}

	// run the check function for all the new actions (if there are any)
	for (int i = d20aNumInitial; i < actSeq->d20ActArrayNum; i++)
	{
		int result = seqCheckAction(&actSeq->d20ActArray[i], &tbStatCopy);
		if (result) return result;

	}

	memcpy(&d20aCopy, d20a, sizeof(D20Actn));
	FullAttackCostCalculate(&d20aCopy, &tbStatCopy, &junk, &junk, &numAttacks, &junk);
	if (numAttacks > 1)
	{
		d20aCopy.d20ActType = D20A_FULL_ATTACK;
		if (!TurnBasedStatusUpdate(&tbStatCopy, &d20aCopy))
		{
			memcpy(&actSeq->d20ActArray[actSeq->d20ActArrayNum++], &d20aCopy, sizeof(D20Actn));
			d20aCopy.d20ActType = D20A_STANDARD_ATTACK;
			return UnspecifiedAttackAddToSeqMeleeMulti(actSeq, &tbStatCopy, &d20aCopy);
		}
	}
	memcpy(&d20aCopy, d20a, sizeof(D20Actn));
	d20aCopy.d20ActType = D20A_STANDARD_ATTACK;
	int result = TurnBasedStatusUpdate(&tbStatCopy, &d20aCopy);
	if (!result)
	{
		int attackCode = tbStatCopy.attackModeCode;
		AttackAppend(actSeq, &d20aCopy, tbStat, attackCode);
	}
	return result;

}

void ActionSequenceSystem::AttackAppend(ActnSeq* actSeq, D20Actn* d20a, TurnBasedStatus* tbStat, int attackCode)
{
	memcpy(& (actSeq->d20ActArray[actSeq->d20ActArrayNum]), d20a, sizeof(D20Actn));
	actSeq->d20ActArray[actSeq->d20ActArrayNum].data1 = attackCode;
	if (attackCode == ATTACK_CODE_OFFHAND + 2 || attackCode == ATTACK_CODE_OFFHAND + 4 || attackCode == ATTACK_CODE_OFFHAND + 6)
	{
		if (attackCode == ATTACK_CODE_OFFHAND + 2)
		{
			actSeq->d20ActArray[actSeq->d20ActArrayNum].d20Caf |= D20CAF_SECONDARY_WEAPON;
		}
		else if (attackCode == ATTACK_CODE_OFFHAND + 4)
		{
			if ( feats.HasFeatCount(d20a->d20APerformer, FEAT_IMPROVED_TWO_WEAPON_FIGHTING) 
				|| feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, (Stat)0,0))
				actSeq->d20ActArray[actSeq->d20ActArrayNum].d20Caf |= D20CAF_SECONDARY_WEAPON;
		} else
		{
			if (feats.HasFeatCount(d20a->d20APerformer, FEAT_GREATER_TWO_WEAPON_FIGHTING) 
				|| feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, (Stat)0,0))
				actSeq->d20ActArray[actSeq->d20ActArrayNum].d20Caf |= D20CAF_SECONDARY_WEAPON;
		}
		
	}
	if (tbStat->tbsFlags & 0x40)
		actSeq->d20ActArray[actSeq->d20ActArrayNum].d20Caf |= D20CAF_FULL_ATTACK;
	actSeq->d20ActArrayNum++;
}

int ActionSequenceSystem::StdAttackTurnBasedStatusCheck(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	int hgState = tbStat->hourglassState;

	if (tbStat->attackModeCode < tbStat->baseAttackNumCode || hgState < 2)
		return 1; // Not enough time error

	
	if (hgState != -1)
		hgState = turnBasedStatusTransitionMatrix[hgState][2];
	tbStat->hourglassState = hgState;

	if (inventory.ItemWornAt(d20a->d20APerformer, 3) || dispatch.DispatchD20ActionCheck(d20a, tbStat, dispTypeGetCritterNaturalAttacksNum)  <= 0)
		tbStat->attackModeCode = 0;
	else
		tbStat->attackModeCode = ATTACK_CODE_NATURAL_ATTACK;
	tbStat->baseAttackNumCode = tbStat->attackModeCode + 1;
	tbStat->numBonusAttacks = 0;
	tbStat->numAttacks = 0;
	return 0;
}
#pragma endregion


#pragma region hooks

uint32_t _addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq)
{
	return actSeqSys.addD20AToSeq(d20a, actSeq);
}


uint32_t _StdAttackTurnBasedStatusCheck(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	return actSeqSys.StdAttackTurnBasedStatusCheck(d20a, tbStat);
}

uint32_t _seqCheckAction(D20Actn* d20a, TurnBasedStatus* iO)
{
	return actSeqSys.seqCheckAction(d20a, iO);
}

uint32_t _isPerforming(objHndl objHnd)
{
	return actSeqSys.isPerforming(objHnd);
}

uint32_t _actSeqOkToPerform()
{
	return actSeqSys.actSeqOkToPerform();
}

void _actionPerform()
{
	actSeqSys.ActionPerform();
};


uint32_t _isSimultPerformer(objHndl objHnd)
{
	return actSeqSys.isSimultPerformer(objHnd);
}

uint32_t _seqCheckFuncsCdecl(TurnBasedStatus* tbStatus)
{
	return actSeqSys.seqCheckFuncs(tbStatus);
}




uint32_t __cdecl _moveSequenceParseCdecl(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* actnSthg, float distSthg, float reach, int flagSthg)
{
	return actSeqSys.moveSequenceParse(d20a, actSeq, actnSthg, distSthg, reach, flagSthg);
};

uint32_t __declspec(naked) _moveSequenceParseUsercallWrapper(ActnSeq* actSeq, TurnBasedStatus* actnSthg, float distSthg, float reach, int flagSthg)
{ //, D20_Action *d20aIn@<eax>
	macAsmProl; // esp = esp0 - 16
	__asm{
		mov ebx, [esp + 36]; // flagSthg @ esp0+20 , esp = esp0-16
		push ebx;
		mov esi, [esp + 36];  // esp = esp0-20,  reach @ esp0+16
		push esi;
		mov ebx, [esp + 36];
		push ebx;
		mov esi, [esp + 36];
		push esi;
		mov ebx, [esp + 36];
		push ebx;
		push eax;
		mov esi, _moveSequenceParseCdecl;
		call esi;
		add esp, 24; 
	}
	macAsmEpil;
	__asm retn;
}
	
uint32_t _unspecifiedMoveAddToSeq(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* actnSthg)
{
	return actSeqSys.moveSequenceParse(d20a, actSeq, actnSthg, 0.0, 0.0, 1);
}

int _UnspecifiedAttackAddToSeq(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat)
{
	return actSeqSys.UnspecifiedAttackAddToSeq(d20a, actSeq, tbStat);
}

int _ActionCostFullAttack(D20Actn* d20a, TurnBasedStatus* tbStat, ActionCostPacket* acp)
{
	return actSeqSys.ActionCostFullAttack(d20a, tbStat, acp);
}

void _sequencePerform()
{
	actSeqSys.sequencePerform();
}

void _curSeqReset(objHndl objHnd)
{
	actSeqSys.curSeqReset(objHnd);
}

uint32_t _allocSeq(objHndl objHnd)
{
	return actSeqSys.AllocSeq(objHnd);
}

uint32_t _assignSeq(objHndl objHnd)
{
	return actSeqSys.AssignSeq(objHnd);
}

TurnBasedStatus* _curSeqGetTurnBasedStatus()
{
	return actSeqSys.curSeqGetTurnBasedStatus();
}

uint32_t _turnBasedStatusInit(objHndl objHnd)
{
	return actSeqSys.TurnBasedStatusInit(objHnd);
}

const char* __cdecl _ActionErrorString(uint32_t actnErrorCode)
{
	return actSeqSys.ActionErrorString(actnErrorCode);
}

void _ActSeqCurSetSpellPacket(SpellPacketBody* spellPktBody, int flag)
{
	actSeqSys.ActSeqCurSetSpellPacket(spellPktBody, flag);
}


int _GetNewHourglassState(objHndl performer, D20ActionType d20aType, int d20Data1, int radMenuActualArg, D20SpellData* d20SpellData)
{
	return actSeqSys.GetNewHourglassState(performer, d20aType, d20Data1, radMenuActualArg, d20SpellData);
}
#pragma endregion


void ActnSeqReplacements::ActSeqGetPicker()
{

	actSeqSys.ActSeqGetPicker();
	
	//addresses.actSeqPicker;
	//int dummy = 1;

	//orgActSeq_100977A0();

	// dummy = 1;

}

int ActnSeqReplacements::SeqRenderFuncMove(D20Actn* d20a, UiIntgameTurnbasedFlags flags)
{
	if (d20a == nullptr)
	{
		return 0;
	}
	auto pqr = d20a->path;
	if (pqr && (flags & UITB_ShowPathPreview ) && (pqr->flags & PF_COMPLETE) )
	{
		uiIntgameTb.CreateMovePreview(pqr, (UiIntgameTurnbasedFlags)(flags & UITB_IsLastSequenceActionWithPath));
	} 

	
	if (config.pathfindingDebugMode)
	{
		// draw the nodes
		for (int i = 0; i < pqr->nodeCount; i++)
		{
			uiIntgameTb.RenderCircle(pqr->nodes[i], 0.0, 0x8078e9dd, 0xf078e9dd, 14.0);
		}
		// draw the d20 destination
		uiIntgameTb.RenderCircle(d20a->destLoc, 1.0, 0x80ffFFff, 0xefff0000, 9.0);
		uiIntgameTb.RenderCircle(pqr->to, 1.0, 0x00ffFFff, 0xefFFffFF, 16.0);
			
	}
	

	*addresses.cursorState = 3;
	if (d20a->d20Caf & D20CAF_TRUNCATED)
	{
		*addresses.cursorState = 19;
	}

	if (d20a->path)
	{
		uiIntgameTb.PathpreviewGetFromToDist(d20a->path);
	}
	return 0;
	//return orgSeqRenderFuncMove(d20a, flags);
}

void ActnSeqReplacements::AooShaderPacketAppend(LocAndOffsets* loc, int aooShaderId)
{
	auto shaderNum = *addresses.aooShaderLocationsNum;
	if (shaderNum < MAX_AOO_SHADER_LOCATIONS)
	{
		addresses.aooShaderLocations[shaderNum].loc = *loc;
		addresses.aooShaderLocations[shaderNum].shaderId = aooShaderId;
		++(*addresses.aooShaderLocationsNum);
	}
}

int ActnSeqReplacements::SeqRenderAooMovement(D20Actn* d20a, UiIntgameTurnbasedFlags flags)
{
	if (d20a && (flags & UITB_ShowPathPreview))
	{
		auto perfLoc = objects.GetLocationFull(d20a->d20APerformer); // this is the critter doing the aoo
		AooShaderPacketAppend(&d20a->destLoc, *addresses.aooShaderId);
		uiIntgameTb.AooInterceptArrowDraw(&perfLoc, &d20a->destLoc);
		if (config.pathfindingDebugMode)
		{
			uiIntgameTb.RenderCircle(d20a->destLoc, -1, 0x90af30af, 0, 23);
		}
	}
	return 0;
}

void ActnSeqReplacements::TurnStart(objHndl obj){
	actSeqSys.TurnStart(obj);
}

ActionErrorCode ActnSeqReplacements::AddToSeqSimple(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat){
	return actSeqSys.AddToSeqSimple(d20a, actSeq, tbStat);
}

ActionErrorCode ActnSeqReplacements::AddToSeqWithTarget(D20Actn*d20a, ActnSeq*actSeq, TurnBasedStatus* tbStat){
	return static_cast<ActionErrorCode>(actSeqSys.AddToSeqWithTarget(d20a, actSeq, tbStat));
}

int ActnSeqReplacements::ActionAddToSeq()
{
	return actSeqSys.ActionAddToSeq();
}

void ActnSeqReplacements::ActSeqApply()
{
	replaceFunction(0x10089F70, _curSeqGetTurnBasedStatus);
	replaceFunction(0x10089FA0, _ActSeqCurSetSpellPacket);

	replaceFunction(0x1008A050, _isPerforming);
	replaceFunction(0x1008A100, _addD20AToSeq);
	replaceFunction(0x1008A1B0, _ActionErrorString);
	replaceFunction(0x1008A980, _actSeqOkToPerform);
	replaceFunction(0x1008BFA0, AddToSeqSimple);

	replaceFunction(0x1008C4F0, _StdAttackTurnBasedStatusCheck);
	replaceFunction(0x1008C6A0, _ActionCostFullAttack);



	replaceFunction(0x100925E0, _isSimultPerformer);

	replaceFunction(0x10094910, _GetNewHourglassState);
	replaceFunction(0x10094A00, _curSeqReset);
	replaceFunction(0x10094CA0, _seqCheckFuncsCdecl);
	replaceFunction(0x10094C60, _seqCheckAction);


	replaceFunction(0x10094E20, _allocSeq);

	replaceFunction(0x10094EB0, _assignSeq);

	replaceFunction(0x10094F70, _moveSequenceParseUsercallWrapper);

	replaceFunction(0x10095450, AddToSeqWithTarget);
	replaceFunction(0x10095860, _unspecifiedMoveAddToSeq);

	replaceFunction(0x10095FD0, _turnBasedStatusInit);


	replaceFunction(0x100961C0, _sequencePerform);

	orgChooseTargetCallback = replaceFunction(0x10096570, ChooseTargetCallback);

	replaceFunction(0x100968B0, _UnspecifiedAttackAddToSeq);
	replaceFunction(0x100996E0, _actionPerform);

	replaceFunction(0x100977A0, ActSeqGetPicker);
	replaceFunction(0x10097C20, ActionAddToSeq);

}

void ActnSeqReplacements::NaturalAttackOverwrites()
{

	int writeVal = ATTACK_CODE_NATURAL_ATTACK;
	write(0x1008C542 + 3, &writeVal, 4);


	// new D20Defs

	// addD20AToSeq    replaced
	//writeVal = (int)&d20Sys.d20Defs[0].addToSeqFunc;
	//write(0x1008A125 + 2, &writeVal, sizeof(int));

	//sub_1008A180  D20ActionBreaksConcentration
	writeVal = (int)&d20Sys.d20Defs[0].flags;
	write(0x1008A18A + 2, &writeVal, sizeof(int));


	//1008A4B0   D20ActnTargetClassification
	writeVal = (int)&d20Sys.d20Defs[0].flags;
	write(0x1008A4B8 + 2, &writeVal, sizeof(int));


	// D20ActionTriggersAoO     partially replaced
	writeVal = (int)&d20Sys.d20Defs[0].flags;
	write(0x1008A9DA + 2, &writeVal, sizeof(int));
	write(0x1008AA10 + 2, &writeVal, sizeof(int));


	//projectileCheckBeforeNextAction
	writeVal = (int)&d20Sys.d20Defs[0].projectilePerformFunc;
	write(0x1008AC99 + 2, &writeVal, sizeof(int));

	// sub_1008ACC0 IsActionOffensive
	writeVal = (int)&d20Sys.d20Defs[0].flags;
	write(0x1008ACCA + 2, &writeVal, sizeof(int));

	// actSeqSpellHarmful
	//  flags
	writeVal = (int)&d20Sys.d20Defs[0].flags;
	write(0x1008AD59 + 2, &writeVal, sizeof(int));

	// d20aTriggerCombatCheck
	//  flags
	writeVal = (int)&d20Sys.d20Defs[0].flags;
	write(0x1008AEBD + 2, &writeVal, sizeof(int));


	// ActionCostProcess
	// 1008B052 actionCost
	writeVal = (int)&d20Sys.d20Defs[0].actionCost;
	write(0x1008B052 + 2, &writeVal, sizeof(int));


	// isSimulsOk
	// 100926B8 flags + 2 (BYTE2)
	writeVal = ((int)&d20Sys.d20Defs[0].flags) + 2;
	write(0x100926B8 + 2, &writeVal, sizeof(int));


	// ActionFrameProcess
	writeVal = (int)&d20Sys.d20Defs[0].actionFrameFunc;
	write(0x100934BB + 2, &writeVal, sizeof(int));

	// TurnBasedStatusUpdate
	// 10093980  aiCheck
	writeVal = (int)&d20Sys.d20Defs[0].turnBasedStatusCheck;
	write(0x10093980 + 2, &writeVal, sizeof(int));


	// seqCheckAction
	// 10094C85 actionCheckFunc 
	writeVal = (int)&d20Sys.d20Defs[0].actionCheckFunc;
	write(0x10094C85 + 2, &writeVal, sizeof(int));


	// seqCheckFuncs
	writeVal = (int)&d20Sys.d20Defs[0].tgtCheckFunc;
	write(0x10094D1C + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].actionCheckFunc;
	write(0x10094D57 + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].locCheckFunc;
	write(0x10094D81 + 2, &writeVal, sizeof(int));
	// 10094D1C  tgtCheck
	// 10094D57 actionCheckFunc
	// 10094D81 locCheckFunc

	// moveSequenceParse    replaced
	// 1009538C actionCost
	/*writeVal = (int)&d20Sys.d20Defs[0].actionCost;
	write(0x1009538C + 2, &writeVal, sizeof(int));*/

	// seqAddWithTarget
	writeVal = (int)&d20Sys.d20Defs[0].actionCheckFunc;
	write(0x1009559F + 2, &writeVal, sizeof(int));


	// sub_100960B0
	writeVal = (int)&d20Sys.d20Defs[0].tgtCheckFunc;
	write(0x10096100 + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].actionCheckFunc;
	write(0x1009613C + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].locCheckFunc;
	write(0x1009615D + 2, &writeVal, sizeof(int));

	//  sub_10096390
	writeVal = (int)&d20Sys.d20Defs[0].actionCheckFunc;
	write(0x10096424 + 2, &writeVal, sizeof(int));

	// SequencePathSthgSub_10096450
	writeVal = (int)&d20Sys.d20Defs[0].tgtCheckFunc;
	write(0x10096493 + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].locCheckFunc;
	write(0x100964C9 + 2, &writeVal, sizeof(int));
	writeVal = ((int)&d20Sys.d20Defs[0].flags) + 2;
	write(0x100964F9 + 2, &writeVal, sizeof(int));

	// sub_10097060  CursorRenderUpdate
	// 100970BB pickerMaybe
	// 100970EB pickerMaybe
	// 10097111 pickerMaybe
	writeVal = (int)&d20Sys.d20Defs[0].seqRenderFunc;
	write(0x100970BB + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].seqRenderFunc;
	write(0x100970EB + 2, &writeVal, sizeof(int));
	writeVal = (int)&d20Sys.d20Defs[0].seqRenderFunc;
	write(0x10097111 + 2, &writeVal, sizeof(int));

	// sub_10097320 HourglassUpdate     replaced
	//writeVal = (int)&d20Sys.d20Defs[0].flags;
	//write(0x10097330 + 2, &writeVal, sizeof(int));
	//writeVal = (int)&d20Sys.d20Defs[0].flags;
	//write(0x1009753B + 2, &writeVal, sizeof(int));
	//writeVal = (int)&d20Sys.d20Defs[0].turnBasedStatusCheck;
	//write(0x1009754D + 2, &writeVal, sizeof(int));
	//writeVal = (int)&d20Sys.d20Defs[0].seqRenderFunc;
	//write(0x100976C9 + 2, &writeVal, sizeof(int));

	// sub_100977A0  ActSeqGetPicker    replaced
	// 100977C2 flags
	// 10097825 flags
	// 100978EF flags
	//writeVal = (int)&d20Sys.d20Defs[0].flags;
	//write(0x100977C2 + 2, &writeVal, sizeof(int));
	//writeVal = (int)&d20Sys.d20Defs[0].flags;
	//write(0x10097825 + 2, &writeVal, sizeof(int));
	//writeVal = (int)&d20Sys.d20Defs[0].flags;
	//write(0x100978EF + 2, &writeVal, sizeof(int));

	// ActionAddToSeq
	writeVal = (int)&d20Sys.d20Defs[0].actionCheckFunc;
	write(0x10097C7E + 2, &writeVal, sizeof(int));

	// curSeqNext   replaced
	// 10099026 flags+1 (BYTE1)
	//writeVal = ((int)&d20Sys.d20Defs[0].flags) + 1;
	//write(0x10099026 + 2, &writeVal, sizeof(int));


	//actionPerform      replaced
	// 100998D4 performFunc
	//writeVal = (int)&d20Sys.d20Defs[0].performFunc;
	//write(0x100998D4 + 2, &writeVal, sizeof(int));


	// sub_10099B10
	// 10099C2E projectilePerformFunc
	writeVal = (int)&d20Sys.d20Defs[0].projectilePerformFunc;
	write(0x10099C2E + 2, &writeVal, sizeof(int));
}