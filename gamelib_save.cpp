#include "stdafx.h"
#include "fixes.h"
#include "temple_functions.h"

//GlobalPrimitive<PyObject* (), 0x1026C2E4> pyToEEPy_BuildValue;

uint32_t Co8DataHook_save(void* retaddr , char* pString, void * p2)
{
	char a[] = "(s)";
	char b[] = "_co8init";
	char c[] = "save";
	auto dude = Py_BuildValue(a, pString);
	//auto dude = (PyObject* ())pyToEEPy_BuildValue(a, pString);
	templeFuncs.PyScript_Execute(b, c, dude);
	return 1;
};


class PersistentDataModSave : public TempleFix {
public:
	const char* name() override {
		return "SpellSlinger's Persistent Data Mod for the Savegame function)";
	}

	void apply() override {
		writeCall(0x10004865, Co8DataHook_save);
		writeHex( 0x10004865 + 5, "C3"); // write another retn
	}
} persistentDataModSave;