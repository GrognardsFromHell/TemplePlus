
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

	void apply() override {
		
		VampiricTouchFix();
		enlargePersonModelScaleFix();
		static int (*orgImmunityCheckHandler )(DispatcherCallbackArgs)= replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>(0x100ED650, [](DispatcherCallbackArgs args)
		{
			if (!ImmunityCheckHandler(args))
				return orgImmunityCheckHandler(args);
			return 0;
		});
		
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
	



	DispIoType21 dispIo21;
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
};

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



