
#include "stdafx.h"
#include "common.h"
#include "fixes.h"
#include "pythonglobal.h"
#include "python_header.h"
#include "obj.h"
#include "d20.h"


void PyPerformTouchAttack_PatchedCallToHitProcessing(D20Action * pd20A, D20Action d20A, uint32_t savedesi, uint32_t retaddr, PyObject * pyObjCaller, PyObject * pyTupleArgs);

class SpellConditionFixes : public TempleFix {
public:
	const char* name() override {
		return "Spell Condition Fixes (for buggy spell effects)";
	}

	void VampiricTouchFix();
	
	void apply() override {
		
		VampiricTouchFix();
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
};

void PyPerformTouchAttack_PatchedCallToHitProcessing( D20Action * pd20A, D20Action d20A, uint32_t savedesi, uint32_t retaddr, PyObject * pyObjCaller, PyObject * pyTupleArgs)
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
	

	d20.ToHitProc(pd20A);
	return;
	
}