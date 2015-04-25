#include "stdafx.h"
#include "common.h"
#include "python_spell.h"
#include <spell.h>
#include "..\tio\tio.h"
#include "temple_functions.h"
#include "python_header.h"


PythonSpellSystem pySpellSystem;


class PythonSpellReplacements : TempleFix
{
	macTempleFix(Python Spell)
	{
		// macReplaceFun(100C0180, _SpellTrigger) // fucker is bugged :(
	}

} pySpellReplacements;

static struct PythonSpellSystemAddresses : AddressTable {


	//100BF0C0
	char ** spellEventNames;
	PyObject* (__cdecl * PySpellFromSpellId)(uint32_t spellId);
	void(__cdecl * SpellPacketUpdateFromPySpell)(); //usercall  PySpell * @<eax>
	LocFull * pySpellLoc;
	LocFull * pySpellLoc2;

	PythonSpellSystemAddresses()
	{
		macRebase(spellEventNames, 102CFE24)
		macRebase(PySpellFromSpellId, 100BE880)
		macRebase(pySpellLoc, 10BCABD8)
		macRebase(pySpellLoc2, 10BCABF0)
		macRebase(SpellPacketUpdateFromPySpell, 100BE2C0)
		macRebase(SpellSoundPlay, 100BF770)
	}

	uint32_t (__cdecl *SpellSoundPlay)(SpellPacketBody* spellPacketBody, PySpellEventCode pySpellEventCode);
} addresses;


char* PythonSpellSystem::GetSpellEventName(PySpellEventCode spellEventCode)
{
	return addresses.spellEventNames[spellEventCode];
}

uint32_t PythonSpellSystem::GetSpellFilename(char* filenameOut, uint32_t spellId)
{
	TioFileList tfl;
	char filePattern[0x100] = {0,};
	uint32_t spellEnum = spellSys.GetSpellEnumFromSpellId(spellId);
	_snprintf(filePattern, 0x100, "scr\\Spell%03d*.py", spellEnum);
	tio_filelist_create(&tfl, filePattern);
	if (tfl.count == 1)
	{
		_splitpath(tfl.files->name, nullptr, nullptr, filenameOut, nullptr);
		return 1;
	} 
	if (tfl.count > 1)
	{
		hooked_print_debug_message("PyScript: multiple script files with number: %d \n", spellId);
		for (int i = 0; i < tfl.count; i++)
		{
			hooked_print_debug_message("        : %s \n", &tfl.files[i]);
		}
	}
	return 0;
}

uint32_t PythonSpellSystem::SpellTrigger(uint32_t spellId, PySpellEventCode spellEventCode)
{
	uint32_t flagSthg = 0;
	char filename[0x100] = {0,};
	if (!GetSpellFilename(filename, spellId))
		return 0;
	
	char * spellEventName = GetSpellEventName(spellEventCode);
	if (!spellEventName || strlen(spellEventName) == 0)
	{
		hooked_print_debug_message("python_spell.cpp / SpellTrigger(): WARNING - spell script=(%s) is missing python trigger=(%d)\n", filename, spellEventCode);
		return 0;
	} 
	auto pytup = PyTuple_New(1);
	PySpell * pySpell = (PySpell *)PySpellFromSpellId(spellId);
	SpellPacketBody spellPktBody;
	++pySpell->ob_refcnt;
	PyTuple_SetItem(pySpell, 0, (PyObject*)pySpell);
	spellSys.GetSpellPacketBody(pySpell->spellId , &spellPktBody);
	if (spellEventCode == OnSpellEffect && spellPktBody.targetListNumItems > 0)
	{
		for (uint32_t i = 0; i < spellPktBody.targetListNumItems; i++)
		{
			objHndl tgtObj = spellPktBody.targetListHandles[i];
			if (!objects.ScriptExecute(spellPktBody.objHndCaster, tgtObj, spellId, 0, san_spell_cast, 0))
				flagSthg = 1;
		}
		if (flagSthg) return 1;

	}
	PyObject * pyscriptRet = templeFuncs.PyScript_Execute(filename, spellEventName, pytup);
	if (pyscriptRet)
	{
		PySpellLocSet(&pySpell->target_location_full);
		PySpellLoc2Set(&pySpell->target_location_full);
		SpellPacketUpdateFromPySpell(pySpell);

		pySpell->ob_refcnt--;
		if (!pySpell->ob_refcnt)
			pySpell->ob_type->tp_dealloc(pySpell);

		pyscriptRet->ob_refcnt--;
		if (!pyscriptRet->ob_refcnt)
			pyscriptRet->ob_type->tp_dealloc(pyscriptRet);

		pytup->ob_refcnt--;
		if (!pytup->ob_refcnt)
			pytup->ob_type->tp_dealloc(pytup);

		SpellSoundPlay(&spellPktBody, spellEventCode);
		return 1;
	}
	pytup->ob_refcnt--;
	if (!pytup->ob_refcnt)
	{
		pytup->ob_type->tp_dealloc(pytup);
		return 0;
	}
	return 0;
}

PyObject* PythonSpellSystem::PySpellFromSpellId(uint32_t spellId)
{
	return addresses.PySpellFromSpellId(spellId);
}

void PythonSpellSystem::PySpellLocSet(LocFull* locFull)
{
	*addresses.pySpellLoc = *locFull;
}

void PythonSpellSystem::PySpellLoc2Set(LocFull* locFull)
{
	*addresses.pySpellLoc2 = *locFull;
}

void PythonSpellSystem::SpellPacketUpdateFromPySpell(PySpell* pySpell)
{
	macAsmProl
	__asm{
		mov ecx, this;
		mov esi, addresses.SpellPacketUpdateFromPySpell;
		mov eax, pySpell;
		call esi;
	}
	macAsmEpil
}

uint32_t PythonSpellSystem::SpellSoundPlay(SpellPacketBody* spellPktBody, PySpellEventCode pySpellEvent)
{
	return addresses.SpellSoundPlay(spellPktBody, pySpellEvent);
}

uint32_t _SpellTrigger(uint32_t spellId, PySpellEventCode spellEventCode)
{
	return pySpellSystem.SpellTrigger(spellId, spellEventCode);
}