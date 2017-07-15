
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
#include "anim.h"
#include "float_line.h"
#include "action_sequence.h"


void PyPerformTouchAttack_PatchedCallToHitProcessing(D20Actn * pd20A, D20Actn d20A, uint32_t savedesi, uint32_t retaddr, PyObject * pyObjCaller, PyObject * pyTupleArgs);
void enlargeSpellRestoreModelScaleHook(objHndl objHnd);
void enlargeSpellIncreaseModelScaleHook(objHndl objHnd);

// Spell Condition Fixes (for buggy spell effects)
class SpellConditionFixes : public TempleFix {
public:
#define SPFIX(fname) static int fname ## (DispatcherCallbackArgs args);
	void VampiricTouchFix();
	void enlargePersonModelScaleFix(); // fixes ambiguous float point calculations that resulted in cumulative roundoff errors
	static void SpellDamageWeaponlikeHook(objHndl tgt, objHndl caster, int dicePacked, DamageType damType, int attackPower, D20ActionType actionType, int spellId, D20CAF flags ); // allows for sneak attack damage on chill touch

	static int ImmunityCheckHandler(DispatcherCallbackArgs args);

	static int CalmEmotionsActionInvalid(DispatcherCallbackArgs args);
	static bool ShouldRemoveCalmEmotions(objHndl handle, DispIoD20Signal *evtObj, DispatcherCallbackArgs args);

	static int StinkingCloudObjEvent(DispatcherCallbackArgs args);
	static int GreaseSlippage(DispatcherCallbackArgs args);
	static int ColorSprayUnconsciousOnAdd(DispatcherCallbackArgs args);
	static int HasteBonusAttack(DispatcherCallbackArgs args);
	static int BootsOfSpeedBonusAttack(DispatcherCallbackArgs args);
	static int BlinkDefenderMissChance(DispatcherCallbackArgs args);

	static int WebOnMoveSpeed(DispatcherCallbackArgs args);
	static int MelfsAcidArrowDamage(DispatcherCallbackArgs args);

	static int InvisibSphereDismiss(DispatcherCallbackArgs args);

	static BOOL MindFogSaveThrowHook(objHndl tgt, objHndl caster, int spellDc, SavingThrowType saveType, int flags, int spellId);
	static bool ShouldRemoveInvisibility(objHndl handle, DispIoD20Signal *evtObj, DispatcherCallbackArgs args);

	static int InvisibilityAooWillTake(DispatcherCallbackArgs args);
	static int AidOnAddTempHp(DispatcherCallbackArgs args);

	static int GhoulTouchAttackHandler(DispatcherCallbackArgs args);

	void apply() override {

		// Ghoul touch - not allowing saving throw
		replaceFunction(0x100D4A00, GhoulTouchAttackHandler);

		// Aid Spell fixed amount of HP gained to be 1d8 + 1/caster level
		replaceFunction(0x100CBE00, AidOnAddTempHp);

		// Calm Emotions ActionInvalid check
		replaceFunction(0x100C6630, CalmEmotionsActionInvalid);

		// Invisibility Sphere lacking a Dismiss handler
		{
			SubDispDefNew sdd;
			sdd.dispType = dispTypeD20Signal;
			sdd.dispKey = DK_SIG_Dismiss_Spells;
			sdd.dispCallback = InvisibSphereDismiss;
			write(0x102DAFB8, &sdd, sizeof(sdd)); // in place of Teleport_Reconnect which does nothing
		}



		// Invisibility Spell
		replaceFunction(0x100E87D0, InvisibilityAooWillTake); // fixes infinite AoO issue for Greater Invisibility and other spells which cause an inivisbility effect (like Sleet Storm)

		static int(__cdecl*orgSpell_remove_spell)(DispatcherCallbackArgs) = replaceFunction<int(DispatcherCallbackArgs)>(0x100D7620, [](DispatcherCallbackArgs args) {
			// fixes not removing Invisibility if target != caster
			// fixes handling of Calm Emotions

			DispIoD20Signal *evtObj = nullptr;
			if (args.dispIO)
				evtObj = dispatch.DispIoCheckIoType6(args.dispIO);


			if (args.dispKey == DK_SIG_Sequence) {
				logger->warn("Caught a DK_SIG_Sequence, make sure we are removing spell properly...");
			}

			switch (args.dispKey) {
			case DK_SIG_Killed:
			case DK_SIG_Critter_Killed:
			case DK_SIG_Sequence:
			case DK_SIG_Spell_Cast:
			case DK_SIG_Action_Recipient:
			case DK_SIG_Concentration_Broken:
			case DK_SIG_TouchAttackAdded:
			case DK_SIG_Teleport_Prepare:
			case DK_SIG_Teleport_Reconnect:
				break;
			default:
				if (evtObj && evtObj->data1 != args.GetCondArg(0))
					return 0;
				break;
			}


			auto spellId = args.GetCondArg(0);
			SpellPacketBody spPkt(spellId);

			if (!spPkt.spellEnum){
				logger->error("Error getting spell packet ID {}", spellId);
				return 0;
			}

			auto spellModRemove = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100CBAB0);

			switch(spPkt.spellEnum){
			case 37:
				d20Sys.d20SendSignal(args.objHndCaller, DK_SIG_Spell_End, spellId, 0);
				spPkt.EndPartsysForTgtObj(args.objHndCaller);
				pySpellIntegration.SpellSoundPlay(&spPkt, SpellEvent::EndSpellCast);
				if (!spPkt.RemoveObjFromTargetList(args.objHndCaller)) {
					logger->error("Cannot END spell - could not remove target!");
					return FALSE;
				}
				spellSys.SpellEnd(spellId, 0);
				return 0;
			default:
				break;
			}

			if (spPkt.spellEnum == 48){ // Calm Emotions
				if (!ShouldRemoveCalmEmotions(args.objHndCaller, evtObj, args))
					return 0;

				pySpellIntegration.SpellSoundPlay(&spPkt, SpellEvent::EndSpellCast);
				d20Sys.d20SendSignal(spPkt.caster, DK_SIG_Spell_End, spellId, 0);
				//spPkt.EndPartsysForTgtObj(args.objHndCaller);
				gameSystems->GetParticleSys().End(spPkt.casterPartsysId);
				gameSystems->GetParticleSys().CreateAtObj("sp-Calm Emotions-END", spPkt.caster);
				spPkt.DoForTargetList([&](objHndl tgtHndl){
					d20Sys.d20SendSignal(tgtHndl, DK_SIG_Spell_End, spellId, 0);
					spPkt.EndPartsysForTgtObj(tgtHndl);
					spPkt.RemoveObjFromTargetList(tgtHndl);
				});
				d20Sys.d20SendSignal(spPkt.caster, DK_SIG_Remove_Concentration, spellId, 0);
				/*if (!spPkt.RemoveObjFromTargetList(args.objHndCaller)){
					logger->error("Cannot END spell - could not remove target!");
					return FALSE;
				}*/
				// todo: make this a template
				spellSys.SpellEnd(spellId, 0);
				spellModRemove(args);
				return 0;
			}

			if (spPkt.spellEnum == 253){
				if (ShouldRemoveInvisibility(args.objHndCaller, evtObj, args) && spPkt.targetCount && spPkt.targetListHandles[0])
					d20Sys.d20SendSignal(spPkt.targetListHandles[0], DK_SIG_Spell_End, spellId, 0);
			}
			return orgSpell_remove_spell(args);

		});


		//// spell mod end handler
		//static int(__cdecl*orgSpellEndModHandler)(DispatcherCallbackArgs) = replaceFunction<int(DispatcherCallbackArgs)>(0x100E9680, [](DispatcherCallbackArgs args)
		//{
		//	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
		//	if (dispIo->data1 != args.GetCondArg(0)){
		//		auto dummy = 1;
		//	}
		//	return orgSpellEndModHandler(args);
		//});

		// Spell damage conversion to Weapon-like spell damage (to support Sneak Attack with spells)
		redirectCall(0x100DCDC4, SpellDamageWeaponlikeHook); // Chill Touch
		redirectCall(0x100C94F3, SpellDamageWeaponlikeHook); // Produce Flame
		redirectCall(0x100DDA04, SpellDamageWeaponlikeHook); // Shocking Grasp
		redirectCall(0x100DDEB4, SpellDamageWeaponlikeHook); // Vampiric Touch
		replaceFunction(0x100CE940, MelfsAcidArrowDamage);

		VampiricTouchFix();
		enlargePersonModelScaleFix();

		// Replaces sp-WebOn movement speed hook
		replaceFunction(0x100CB700, WebOnMoveSpeed);

		// Fix for Blink defender miss chance (the conditions for true seeing reducing the chance to 20% was flipped)
		replaceFunction(0x100C62D0, BlinkDefenderMissChance);

		// Fix for Haste stacking
		replaceFunction(0x100C87C0, HasteBonusAttack);
		replaceFunction(0x10102190, BootsOfSpeedBonusAttack);

		// Fix for Color Spray not knocking critters down
		replaceFunction(0x100CCA00, ColorSprayUnconsciousOnAdd);

		// Stinking Cloud Fix
		replaceFunction(0x100CAF30, StinkingCloudObjEvent);

		// Grease fix for Freedom of Movement
		replaceFunction(0x100C8270, GreaseSlippage);

		static int (*orgImmunityCheckHandler )(DispatcherCallbackArgs)= replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>(0x100ED650, [](DispatcherCallbackArgs args)
		{
			if (!ImmunityCheckHandler(args))
				return orgImmunityCheckHandler(args);
			return 0;
		});
		

		// Fix for Shocking Grasp doing d8 instead of d6 damage
		char sgWriteVal = 6;
		write(0x100DD9DF + 1, &sgWriteVal, sizeof(char));

		// Fix for Restoration spell not curing Charisma damage (it iterated from 0 to 4 instead of 0 to 5)
		char restoWriteVal = 6;
		write(0x100CF637 + 2, &restoWriteVal, sizeof(char));

		// EffectTooltip for Cause Fear
		{
			SubDispDefNew sdd(dispTypeEffectTooltip, DK_NONE, [](DispatcherCallbackArgs args)->int {
				auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
				auto remainingDuration = args.GetCondArg(1);
				auto spellId = args.GetCondArg(0);
				SpellPacketBody spellPkt(spellId);
				std::string text;
				if (args.GetCondArg(2) == 0) { // frightened
					text = fmt::format("({}) \n {}: {}/{}", combatSys.GetCombatMesLine(209), combatSys.GetCombatMesLine(175), remainingDuration, spellPkt.duration);
				}
				else // shaken
				{
					text = fmt::format("({})", combatSys.GetCombatMesLine(208));
				}
				//auto text = fmt::format("\n {}: {}/{}", combatSys.GetCombatMesLine(175), remainingDuration, spellPkt.duration);
				dispIo->Append(args.GetData1(), spellPkt.spellEnum, text.c_str());
				return 0;
			}, 117, 0);
			write(0x102D232C, &sdd, sizeof(SubDispDefNew));
		}

		// Tooltip for Cause Fear
		{
			SubDispDefNew sdd(dispTypeTooltip, DK_NONE, [](DispatcherCallbackArgs args)->int {
				DispIoTooltip *dispIo = dispatch.DispIoCheckIoType9(args.dispIO);
				auto mesLine = combatSys.GetCombatMesLine(args.subDispNode->subDispDef->data1);
				int numstrings = dispIo->numStrings;
				if (numstrings >= 10)
					return 0;

				auto spellId = args.GetCondArg(0);
				SpellPacketBody spellPkt(spellId);
				std::string text;
				if (args.GetCondArg(2) == 0) { // frightened
					text = fmt::format("{} ({})", mesLine, combatSys.GetCombatMesLine(209));
				}
				else // shaken
				{
					text = fmt::format("{} ({})", mesLine, combatSys.GetCombatMesLine(208));
				}


				int idx = 0;
				for (idx = 0; idx < numstrings && idx < 10; idx++)
				{
					if (!strcmp(dispIo->strings[idx], &text[0]))
						break;
				}
				if (idx == numstrings) // reached the end and not found
				{
					strncpy(dispIo->strings[numstrings], &text[0], 0x100);
					dispIo->numStrings++;
				}
				return 0;
			}, 52, 0);
			write(0x102D2318, &sdd, sizeof(SubDispDefNew));
		}
	
		// Divine Power BAB bouns type change from 12 to 40 so it stacks with Weapon Enh Bonus but doesn't stack with itself
		char divPowWriteVal = 40;
		write(0x100C7426 + 1, &divPowWriteVal, 1);


		redirectCall(0x100D56A3, MindFogSaveThrowHook);

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

void SpellConditionFixes::SpellDamageWeaponlikeHook(objHndl tgt, objHndl caster, int dicePacked, DamageType damType, int attackPower, D20ActionType actionType, int spellId, D20CAF flags){
	*(int*)&flags |= D20CAF_HIT;
	damage.DealWeaponlikeSpellDamage(tgt, caster, Dice::FromPacked(dicePacked), damType, attackPower, 100, 103, actionType, spellId, flags);
}

int SpellConditionFixes::ImmunityCheckHandler(DispatcherCallbackArgs args)
{
	/*
		this function serves as a hook for the complete (and much longer...) immunity handler. 
		Its purpose is to fix issues with:

		1. Protection from Alignment overzealously protecting against friendly spells
		2. Necklace of Adaptation protecting against Cloudkill and Stinking Cloud spell effects
		3. Death Ward issue

		return 0 to let the original handler handle
		return 1 to prevent immunity
	*/

	/*
		the dispatch is performed from the functions:
		0x100C3810 CheckSpellResistance
		0x1008D830 PerformCastSpellProcessTargets - sets the spellPkt field of dispIo23 , and sets flag = 1
	*/

	auto dispIo23 = dispatch.DispIoCheckIoType23(args.dispIO);
	if (dispIo23->returnVal == 1)
		return 1;
	



	DispIoTypeImmunityTrigger dispIo21;
	dispIo21.condNode = args.subDispNode->condNode;
	auto dispatcher = objSystem->GetObject(args.objHndCaller)->GetDispatcher();
	if (!dispatch.dispatcherValid(dispatcher))
		return 1;
	
	// check which immunity trigger this condition handles (if any)
	int immType = DK_IMMUNITY_SPELL;
	for (; immType <= DK_IMMUNITY_SPECIAL; immType++){
		dispatch.DispatcherProcessor(dispatcher, dispTypeImmunityTrigger, immType, &dispIo21);
		if (dispIo21.interrupt == 1)
			break;
	}
	if (immType > DK_IMMUNITY_SPECIAL)
		return 1;

	/*
		Necklace of Adaptation immunity to Cloudkill and Stinking Cloud
	*/
	if (immType == DK_IMMUNITY_SPECIAL) // immunity special
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

		// fix for Co8 now-unnecessary Monster Plant application to Undead
		if (args.GetData1() == 1  // Monster Plant
			&& critterSys.IsCategoryType(args.objHndCaller, MonsterCategory::mc_type_undead)) {
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
			return 1;
		}
	}


	if (immType != 10)
		return 0;

	// get the spellpacket of the protective spell (i.e. the one this hook belongs to)
	auto immSpellId = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	SpellPacketBody immSpellPkt;
	if (!spellSys.GetSpellPacketBody(immSpellId, &immSpellPkt))
		return 1;

	if (dispIo23->flag == 0)
		return 0;


	auto offendingSpellPkt = dispIo23->spellPkt;
	SpellEntry offendingSpellEntry;
	spellSys.spellRegistryCopy(offendingSpellPkt->spellEnum, &offendingSpellEntry);

	// Death Ward
	if (immSpellPkt.spellEnum == 101){ 
		auto spClass = offendingSpellPkt->spellClass;
		
		if (offendingSpellEntry.spellDescriptorBitmask & 0x10)
			return 0; // Has Death descriptor - do as usual (should filter it out)

		// otherwise,
		// if offending spell is not a death domain spell, skip the immunity

		if (!spellSys.isDomainSpell(spClass) || spClass != Domain_Death){
				return 1;
		}
	}


	// Prot from alignment spells
	if (immSpellPkt.spellEnum == 368
		|| immSpellPkt.spellEnum >= 370 && immSpellPkt.spellEnum <= 372){



		if (!(offendingSpellEntry.spellDescriptorBitmask & 0x4000)) //Mind Affecting
			return 0;

		if (offendingSpellEntry.spellSchoolEnum != School_Enchantment) // only enchantment effects should be warded from Prot. From Evil
			return 1;


		auto caster = dispIo23->spellPkt->caster;

		if (!caster || !objects.IsCritter(caster))
			return 0;

		if (offendingSpellEntry.aiTypeBitmask && (1 << ai_action_defensive))
		{
			if (critterSys.IsFriendly(caster, args.objHndCaller)) // because some spells are actually both ways, such as prayer
				return 1;
		}
	}
	

	return 0;
}

int SpellConditionFixes::CalmEmotionsActionInvalid(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
	auto d20a = (D20Actn*)dispIo->data1;
	if (!d20a)
		return 0;
	if (!d20Sys.IsActionOffensive(d20a->d20ActType, d20a->d20ATarget)){
		return 0;
	}
	if (!critterSys.IsFriendly(d20a->d20APerformer, d20a->d20ATarget)){
		dispIo->return_val = 1;
		dispIo->data1 = 0;
		dispIo->data2 = 0;
	}
	return 0;
}

bool SpellConditionFixes::ShouldRemoveCalmEmotions(objHndl handle, DispIoD20Signal* evtObj, DispatcherCallbackArgs args){
	if (!evtObj){
		if (args.dispType == dispTypeBeginRound || args.dispKey == DK_SIG_Dismiss_Spells
			|| args.dispType == dispTypeConditionAddPre)
			return true;
		return false;
	}
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);
	if (!spellPkt.spellEnum){
		return false;
	}
	if (args.dispKey == DK_SIG_Killed){
		d20Sys.d20SendSignal(handle, DK_SIG_Spell_End, spellPkt.spellId, 0); // is this a bug???? todo
		return true;
	}
	if (evtObj->dispIOType == dispIOTypeDispelCheck){
		return true;
	}

	if (evtObj->dispIOType != dispIoTypeSendSignal) // e.g. for dispel
		return false;

	if (args.dispKey == DK_SIG_Concentration_Broken)
		return true;

	if (args.dispKey == DK_SIG_Action_Recipient){
		auto d20a = (D20Actn*)evtObj->data1;
		if (!d20a)
			return true;
		if (d20a->d20ActType == D20A_CAST_SPELL) { // bug? what about scrolls and such? should probably check spellId instead. TODO
			auto actionSpellId = d20a->spellId;
			return actionSpellId != spellId;
		}

		if (d20Sys.IsActionOffensive(d20a->d20ActType, d20a->d20ATarget)) {
			if (!critterSys.IsFriendly(d20a->d20APerformer, d20a->d20ATarget))
				return true;
		}
		return false;
	}
	
	return true;
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
	
}
int SpellConditionFixes::GreaseSlippage(DispatcherCallbackArgs args){

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);
	
	if (!spellPkt.spellEnum){
		logger->warn("sp-Grease hit: spell has ended, terminating!");
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	if (d20Sys.d20Query(args.objHndCaller, DK_QUE_Critter_Has_Freedom_of_Movement))
		return 0;

	if (d20Sys.d20Query(args.objHndCaller, DK_QUE_Untripable))
		return 0;

	if (!spellPkt.SavingThrow(args.objHndCaller, D20STF_NONE)) {
		histSys.CreateRollHistoryLineFromMesfile(48, args.objHndCaller, objHndl::null);
		combatSys.FloatCombatLine(args.objHndCaller, 104);
		conds.AddTo(args.objHndCaller, "Prone", {});
		animationGoals.PushAnimate(args.objHndCaller, 64);
	}

	return 0;

}

int SpellConditionFixes::ColorSprayUnconsciousOnAdd(DispatcherCallbackArgs args){

	auto spellId = args.GetCondArg(0);
	floatSys.FloatSpellLine(args.objHndCaller, 20024, FloatLineColor::Red);
	Dice dice(2, 4, 0);
	auto dur = dice.Roll();
	args.SetCondArg(1, dur);
	conds.AddTo(args.objHndCaller, "Prone", {});
	animationGoals.PushAnimate(args.objHndCaller, 64);

	return 0;
}

int SpellConditionFixes::HasteBonusAttack(DispatcherCallbackArgs args){
	args.dispIO->AssertType(dispIOTypeD20ActionTurnBased);
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	if (dispIo->bonlist) {
		dispIo->bonlist->AddBonus(1, 34, 174); // Haste
	}

	return 0;
}

int SpellConditionFixes::BootsOfSpeedBonusAttack(DispatcherCallbackArgs args){
	if (args.GetCondArg(3)) {
		args.dispIO->AssertType(dispIOTypeD20ActionTurnBased);
		auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
		if (dispIo->bonlist) {
			dispIo->bonlist->AddBonus(1, 34, 174); // Haste
		}
	}

	return 0;
}

int SpellConditionFixes::BlinkDefenderMissChance(DispatcherCallbackArgs args){
	args.dispIO->AssertType(dispIOTypeAttackBonus);
	auto dispIo = static_cast<DispIoAttackBonus*>(args.dispIO);

	auto missChance = 50;
	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;
	if (d20Sys.d20Query(attacker, DK_QUE_Critter_Has_True_Seeing) 
		|| d20Sys.d20Query(attacker, DK_QUE_Critter_Can_See_Invisible)){ // reduce to 20% miss chance
		missChance = 20;
	}

	if (d20Sys.d20Query(attacker, DK_QUE_Critter_Can_See_Ethereal)){
		missChance = (missChance == 50) ? 20 : 0;
	}
	if (missChance){
		dispIo->bonlist.AddBonus(missChance, 19, args.GetData2());
	}
	return 0;
}

int SpellConditionFixes::WebOnMoveSpeed(DispatcherCallbackArgs args){
	args.dispIO->AssertType(dispIOTypeMoveSpeed);
	auto dispIo = static_cast<DispIoMoveSpeed*>(args.dispIO);
	if (!d20Sys.d20Query(args.objHndCaller, DK_QUE_Critter_Has_Freedom_of_Movement)) {
		dispIo->bonlist->SetOverallCap(1, 0, 0, args.GetData2());
		dispIo->bonlist->SetOverallCap(2, 0, 0, args.GetData2());
	}
	
	return 0;
}

int SpellConditionFixes::MelfsAcidArrowDamage(DispatcherCallbackArgs args){
	auto isCritical = args.GetCondArg(2);
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	floatSys.FloatCombatLine(args.objHndCaller, 46); // acid damage
	Dice damDice(2, 4, 0);
	int flags = D20CAF_HIT;

	if (isCritical){
		flags |= D20CAF_CRITICAL;
		args.SetCondArg(2, 0);
	}
	if (spPkt.durationRemaining < spPkt.duration ) // only the first shot gets sneak attack damage
		flags |= D20CAF_NO_PRECISION_DAMAGE;

	damage.DealWeaponlikeSpellDamage(spPkt.targetListHandles[0], spPkt.caster, damDice, DamageType::Acid, 1, 100, 103, D20A_CAST_SPELL, spellId, (D20CAF)flags);

	return 0;
}

int SpellConditionFixes::InvisibSphereDismiss(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	if (!dispIo) {
		return 1;
	}

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	if (!spPkt.spellEnum)
		return 0;

	if (dispIo->data1 != spellId)
		return 0;

	auto spellRemove = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100D7620);
	auto spellModRemove = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100CBAB0);

	if (spPkt.spellEnum == 315 || args.GetData1() == 1 || spPkt.targetCount > 0) {
		floatSys.FloatSpellLine(args.objHndCaller, 20000, FloatLineColor::White); // a spell has expired
		spellRemove(args);
		spellModRemove(args);
	}

	return 0;
}

BOOL SpellConditionFixes::MindFogSaveThrowHook(objHndl tgt, objHndl caster, int spellDc, SavingThrowType saveType, int flags, int spellId){
	return damage.SavingThrowSpell(tgt, caster, spellDc, SavingThrowType::Will, 0, spellId);
}

bool SpellConditionFixes::ShouldRemoveInvisibility(objHndl handle, DispIoD20Signal * evtObj, DispatcherCallbackArgs args){
	if (!evtObj){
		if (args.dispType == dispTypeBeginRound || args.dispKey == DK_SIG_Dismiss_Spells || args.dispType == dispTypeConditionAddPre){
			auto spellId = args.GetCondArg(0);
			d20Sys.d20SendSignal(handle, DK_SIG_Spell_End, spellId, 0);
			return true;
		}
		return false;
	}
	if (args.dispKey == DK_SIG_Killed || args.dispKey == DK_SIG_Dismiss_Spells
		|| args.dispType == dispTypeBeginRound || args.dispType == dispTypeConditionAddPre)
		return true;

	auto spellId = args.GetCondArg(0);
	if (evtObj->dispIOType != dispIoTypeSendSignal){
		if (evtObj->dispIOType == dispIOTypeDispelCheck){
			d20Sys.d20SendSignal(handle, DK_SIG_Spell_End, spellId, 0);
			return true;
		}
		return false;
	}

	ActnSeq *seq = (ActnSeq*)evtObj->data1;
	if (!seq)
		return true;

	if (!seq->d20ActArrayNum)
		return false;

	auto objIsInvisible = d20Sys.d20QueryWithData(handle, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Invisibility"), 0);
	if (!objIsInvisible)
		return false;

	for (auto i=0; i < seq->d20ActArrayNum; i++){
		auto &d20a = seq->d20ActArray[i];
		auto d20aSpellId = d20a.spellId;
		if (d20a.d20ActType != D20A_CAST_SPELL){
			auto tgt = d20a.d20ATarget;
			if (d20Sys.IsActionOffensive(d20a.d20ActType, d20a.d20ATarget)){
				d20Sys.d20SendSignal(handle, DK_SIG_Spell_End, spellId, 0);
				return true;
			}
		} 
		else if (d20aSpellId != spellId){
			SpellPacketBody spellPkt(d20aSpellId);
			if (!spellPkt.spellEnum){
				logger->warn("RemoveInvisibility: Error, unable to retrieve spell.");
				return false;
			}
			for (auto j = 0; j < spellPkt.targetCount; j++){
				if (spellSys.IsSpellHarmful(spellPkt.spellEnum, spellPkt.caster, spellPkt.targetListHandles[j])){
					d20Sys.d20SendSignal(handle, DK_SIG_Spell_End, spellId, 0);
					return true;
				}
			}
		}
	}

	return false;
}
 
int SpellConditionFixes::InvisibilityAooWillTake(DispatcherCallbackArgs args){

	auto spellId = args.GetCondArg(0);
	if (!spellId)
		return 0;

	SpellPacketBody spellPkt(spellId);
	if (!spellPkt.spellEnum)
		return 0;

	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
	if (spellPkt.spellEnum == 253 || spellPkt.spellEnum == 256 || spellPkt.spellEnum == 257){
		dispIo->return_val = 0;
	}
	else{
		if (dispIo->return_val > 0) // vanilla forgot to check this before setting the value! It would cause infinite AoOs for Greater Invis. and Sleet Storm because it override the AOO condition's value, which would be 0 (is it even necessary???)
			dispIo->return_val = 1;
	}
	return 0;
}

int SpellConditionFixes::AidOnAddTempHp(DispatcherCallbackArgs args){

	auto spellId = args.GetCondArg(0);

	SpellPacketBody spellPkt(spellId);
	if (!spellPkt.spellEnum){
		logger->error("AidOnAddTempHp: unable to get spell packet");
		return 0;
	}

	auto tempHpAmt = Dice::Roll(1, 8);
	if ((int)spellPkt.casterLevel > 0){
		tempHpAmt += min(10, (int)spellPkt.casterLevel);
	}

	floatSys.FloatSpellLine(args.objHndCaller, 20005, FloatLineColor::White, fmt::format("[{}]", tempHpAmt).c_str(), nullptr); // %d Temp HP Gained
	logger->debug("_begin_aid(): gained {} temporary hit points", tempHpAmt);

	conds.AddTo(args.objHndCaller, "Temporary_Hit_Points", {spellId, args.GetCondArg(1), tempHpAmt});

	return 0;
}

int SpellConditionFixes::GhoulTouchAttackHandler(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto d20a = (D20Actn*)dispIo->data1;
	
	if (!d20a)
		return 0;

	if (!(d20a->d20Caf & D20CAF_HIT)){
		combatSys.FloatCombatLine(args.objHndCaller, 69);
		return 0;
	}

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);
	if (!spellPkt.spellEnum)
		return 0;

	pySpellIntegration.SpellSoundPlay(&spellPkt, SpellEvent::AreaOfEffectHit);
	combatSys.FloatCombatLine(args.objHndCaller, 68);
	pySpellIntegration.SpellSoundPlay(&spellPkt, SpellEvent::SpellStruck);
	
	
	auto tgt = d20a->d20ATarget;
	
	if (spellSys.CheckSpellResistance(&spellPkt, tgt) == TRUE){
		args.RemoveSpellMod();
		args.RemoveCondition();
		return 0;
	}

	// Fixed target not getting a saving throw
	if (spellPkt.SavingThrow(tgt, D20SavingThrowFlag::D20STF_SPELL_SCHOOL_NECROMANCY)){
		args.RemoveSpellMod();
		args.RemoveCondition();
		return 0;
	}

	auto duration = Dice::Roll(1, 6, 2);
	spellPkt.duration = duration;
	if (!conds.AddTo(tgt, "sp-Ghoul Touch Paralyzed", { spellId, duration, 0 })){
		logger->debug("GhoulTouchAttackHandler: unable to add condition");
		return 0;
	}

	auto gtParticles = gameSystems->GetParticleSys().CreateAtObj("sp-Ghoul Touch", tgt);
	if (!conds.AddTo(tgt, "sp-Ghoul Touch Stench", {spellId, duration, 0 , gtParticles })){
		logger->debug("GhoulTouchAttackHandler: unable to add condition");
	}

	auto casterParticles = spellPkt.targetListPartsysIds[0];
	

	spellPkt.targetListHandles[0] = tgt;
	spellPkt.targetListPartsysIds[0] = gtParticles;
	if (!spellPkt.UpdateSpellsCastRegistry()){
		logger->debug("GhoulTouchAttackHandler: Unable to save spell packet.");
		return 0;
	}

	spellPkt.UpdatePySpell();

	// the following also updates the spell packet
	gameSystems->GetParticleSys().End(casterParticles);
	args.RemoveSpellMod();
	args.RemoveCondition();
	if (!spellPkt.RemoveObjFromTargetList(args.objHndCaller)){
		logger->debug("GhoulTouchAttackHandler: Cannot remove target");
		return 0;
	}
}
