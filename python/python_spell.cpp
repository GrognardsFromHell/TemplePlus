#include "stdafx.h"
#include "common.h"
#include "python_spell.h"


PythonSpellSystem pySpellSystem;


class PythonSpellReplacements : TempleFix
{
	macTempleFix(Python Spell)
	{
		//macReplaceFun(100C0180, spell_trigger)
	}

} pySpellReplacements;

static struct PythonSpellSystemAddresses : AddressTable {

	PythonSpellSystemAddresses()
	{
		
	}
} addresses;

