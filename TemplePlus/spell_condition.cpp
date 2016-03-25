
#include "stdafx.h"
#include "common.h"
#include "util\fixes.h"
#include "python\pythonglobal.h"
#include "python\python_header.h"
#include "obj.h"
#include "d20.h"
#include "gamesystems/objects/objsystem.h"
#include "condition.h"
#include "critter.h"
#include "combat.h"
#include "gamesystems/gamesystems.h"
#include "history.h"
#include "python/python_integration_spells.h"
#include "gamesystems/objects/objevent.h"
#include "gamesystems/particlesystems.h"
#include "damage.h"


void PyPerformTouchAttack_PatchedCallToHitProcessing(D20Actn * pd20A, D20Actn d20A, uint32_t savedesi, uint32_t retaddr, PyObject * pyObjCaller, PyObject * pyTupleArgs);
void enlargeSpellRestoreModelScaleHook(objHndl objHnd);
void enlargeSpellIncreaseModelScaleHook(objHndl objHnd);

class SpellConditionFixes : public TempleFix {
public:
	const char* name() override {
		return "Spell Condition Fixes (for buggy spell effects)";
	}

	void VampiricTouchFix();
	void enlargePersonModelScaleFix(); // fixes ambiguous float point calculations that resulted in cumulative roundoff errors
	
	static int ImmunityCheckHandler(DispatcherCallbackArgs args);

	static int StinkingCloudObjEvent(DispatcherCallbackArgs args);

	void apply() override {
		
		VampiricTouchFix();
		enlargePersonModelScaleFix();

		replaceFunction(0x100CAF30, StinkingCloudObjEvent);

		static int (*orgImmunityCheckHandler )(DispatcherCallbackArgs)= replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>(0x100ED650, [](DispatcherCallbackArgs args)
		{
			if (!ImmunityCheckHandler(args))
				return orgImmunityCheckHandler(args);
			return 0;
		});
		

		// Fix for Shocking Grasp doing d8 instead of d6 damage
		char sgWriteVal = 6;
		write(0x100DD9DF + 1, &sgWriteVal, sizeof(char));
	}
} spellConditionFixes;


void SpellConditionFixes::VampiricTouchFix()
{
	writeHex(0x102E0A24, "EA 00");
	writeHex(0x102E0A28, "D0 43 0C 10");
	writeHex(0x102E0A2C, "F0 09  2E 10");

	writeHex(0x102E0A8C, "D0 8F 0E 10");

	writeHex(0x102E0AC4, "A6");
	writeHex(0x102E0AC8, "20 76 0D 10");

	writeHex(0x102E0B00, "A6");
	writeHex(0x102E0B04, "B0 BA 0C 10");


	//Perform Touch Attack mod:
	redirectCall(0x100B2CC9, PyPerformTouchAttack_PatchedCallToHitProcessing);
	return;
}

void SpellConditionFixes::enlargePersonModelScaleFix()
{
	redirectCall(0x100CD45C, enlargeSpellIncreaseModelScaleHook);
	redirectCall(0x100D84DE, enlargeSpellRestoreModelScaleHook); // sp152 enlarge 
	redirectCall(0x100D9C22, enlargeSpellRestoreModelScaleHook); // sp404 righteous might

}

int SpellConditionFixes::ImmunityCheckHandler(DispatcherCallbackArgs args)
{
	/*
		the dispatch is performed from the functions:
		0x100C3810 CheckSpellResistance
		0x1008D830 PerformCastSpell
	*/
	auto dispIo23 = dispatch.DispIoCheckIoType23(args.dispIO);
	if (dispIo23->returnVal == 1)
		return 1;
	



	DispIoTypeImmunityTrigger dispIo21;
	dispIo21.condNode = args.subDispNode->condNode;
	auto dispatcher = objSystem->GetObject(args.objHndCaller)->GetDispatcher();
	if (!dispatch.dispatcherValid(dispatcher))
		return 1;
	
	int immType = 10;
	for (; immType <= 16; immType++){
		dispatch.DispatcherProcessor(dispatcher, dispTypeImmunityTrigger, immType, &dispIo21);
		if (dispIo21.interrupt == 1)
			break;
	}
	if (immType > 16)
		return 1;


	if (immType == 0x10) // immunity special
	{
		if (args.subDispNode->subDispDef->data1 == 0x4) //  necklace of adaptation (NEW!)
		{
			if (dispIo23->spellPkt->spellEnum == 65 || dispIo23->spellPkt->spellEnum == 460) // Cloudkill and Stinking Cloud
			{
				histSys.CreateFromFreeText(fmt::format("{} protected by Necklace of Adaptation!\n",description.getDisplayName(args.objHndCaller )).c_str());
				dispIo23->returnVal = 1;
				return 1;
			}

		}
	}

	if (immType != 10)
		return 0;

	auto immSpellId = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	SpellPacketBody immSpellPkt;
	if (!spellSys.GetSpellPacketBody(immSpellId, &immSpellPkt))
		return 1;

	if (dispIo23->flag == 0)
		return 0;


	if (immSpellPkt.spellEnum != 368
		&& (immSpellPkt.spellEnum < 370 || immSpellPkt.spellEnum > 372)) {
		// prot from alignment spells
		return 0;
	}
	SpellEntry offendingSpellEntry;
	spellSys.spellRegistryCopy(dispIo23->spellPkt->spellEnum, &offendingSpellEntry);

	if (!(offendingSpellEntry.spellDescriptorBitmask & 0x4000)) //Mind Affecting
		return 0;

	auto caster = dispIo23->spellPkt->caster;
	
	if (!caster)
		return 0;

	if (!objects.IsCritter(caster))
		return 0;

	if ( offendingSpellEntry.aiTypeBitmask && ( 1<<ai_action_defensive)  )
	{
		if (critterSys.IsFriendly(caster, args.objHndCaller)) // because some spells are actually both ways, such as prayer
			return 1;
	}

	return 0;
}



void PyPerformTouchAttack_PatchedCallToHitProcessing( D20Actn * pd20A, D20Actn d20A, uint32_t savedesi, uint32_t retaddr, PyObject * pyObjCaller, PyObject * pyTupleArgs)
{
	auto tupSize = PyTuple_Size(pyTupleArgs);
	uint32_t shouldPerformMeleeTouchAttack = 0;
	if (tupSize > 1)
	{
		PyObject * pyarg2 = PyTuple_GetItem(pyTupleArgs, 1);
		if (PyType_IsSubtype(pyarg2->ob_type, &PyInt_Type))
		{
			if (PyLong_AsLong( pyarg2) != 0)
			{
				pd20A->d20Caf = D20CAF_TOUCH_ATTACK; // sans D20CAF_RANGED
			}
		}
	}
	

	d20Sys.ToHitProc(pd20A);
	return;
	
}

void __cdecl enlargeSpellRestoreModelScaleHook(objHndl objHnd)
{
	// patches for spellRemove function (disgusting hardcoded shit! bah!)
	uint32_t modelScale = objects.getInt32(objHnd, obj_f_model_scale);
	modelScale *= 5;
	modelScale /= 9;
	objects.setInt32(objHnd, obj_f_model_scale, modelScale);
}

void enlargeSpellIncreaseModelScaleHook(objHndl objHnd)
{
	uint32_t modelScale = objects.getInt32(objHnd, obj_f_model_scale);
	modelScale *= 9;
	modelScale /= 5;
	objects.setInt32(objHnd, obj_f_model_scale, modelScale);
}



int SpellConditionFixes::StinkingCloudObjEvent(DispatcherCallbackArgs args)
{
	DispIoObjEvent* dispIo = dispatch.DispIoCheckIoType17(args.dispIO);
	auto condEvtId = args.GetCondArg(2);
	if (dispIo->evtId == condEvtId)	{

		auto spellId = args.GetCondArg(0);
		SpellPacketBody spellPkt(spellId);
		if (!spellPkt.spellId){
			logger->warn("StinkingCloudObjEvent: Unable to fetch spell! ID {}", spellId);
			return 0;
		}

		/*
			AoE Entered; 
			 - add the target to the Spell's Target List
			 - Do a saving throw
		*/
		if (args.dispKey == DK_OnEnterAoE && (args.GetData1() == 0))
		{

			pySpellIntegration.SpellSoundPlay(&spellPkt, SpellEvent::SpellStruck);
			pySpellIntegration.SpellTrigger(spellId, SpellEvent::AreaOfEffectHit);

			if (spellSys.CheckSpellResistance(&spellPkt, dispIo->tgt) == 1)
				return 0;

			auto partsysId = gameSystems->GetParticleSys().CreateAtObj("sp-Stinking Cloud Hit", dispIo->tgt);
			spellPkt.AddTarget(dispIo->tgt, partsysId, 1);
			if (damage.SavingThrowSpell(dispIo->tgt, spellPkt.caster, spellPkt.dc, SavingThrowType::Fortitude, 0, spellPkt.spellId ))	{
				/* 
					save succeeded; add the "Hit Pre" condition, which will attempt 
					to apply the condition in the subsequent turns
				*/
				conds.AddTo(dispIo->tgt, "sp-Stinking Cloud Hit Pre", { static_cast<int>(spellPkt.spellId), spellPkt.durationRemaining, static_cast<int>(dispIo->evtId) });
			} else
			{
				/*
					Save failed; apply the condition
				*/
				conds.AddTo(dispIo->tgt,"sp-Stinking Cloud Hit", { static_cast<int>(spellPkt.spellId), spellPkt.durationRemaining, static_cast<int>(dispIo->evtId), 0 });
			}
		}
		/*
			AoE exited;
			 - If "Hit Pre" (identified by data1 = 223), remove the condition so the character doesn't keep making saves outside the cloud
			 - If "Hit" (identified by data1 = 222), reduce the remaining duration to 1d4+1
		*/
		else if (args.dispKey == DK_OnLeaveAoE)
		{
			if (args.GetData1() == 222) // the sp-Stinking Cloud Hit condition
			{
				args.SetCondArg(3,1);
				auto rollResult = Dice::Roll(1,4,1);
				args.SetCondArg(1, rollResult); // sets the remaining duration to 1d4+1
				histSys.CreateFromFreeText(fmt::format("{} exited Stinking Cloud; Nauseated for {} more rounds.\n", description.getDisplayName(dispIo->tgt), rollResult).c_str());
			}
			else if (args.GetData1() == 223) // the sp-Stinking Cloud Hit Pre condition
			{
				// remove the condition (cloud has been exited)
				// it will get re-added if the target re-enters via this same callback (see above)
				conds.ConditionRemove(dispIo->tgt, args.subDispNode->condNode);
			}
			
		}
		
		if (!spellPkt.UpdateSpellsCastRegistry() )	{
			logger->warn("StinkingCloudObjEvent: Unable to save update SpellPacket!");
			return 0;
		}
		pySpellIntegration.UpdateSpell(spellId);
	}
	return 0;
	
};