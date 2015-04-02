#include "stdafx.h"
#include "fixes.h"
#include "temple_functions.h"

uint32_t Co8DataHook_load(void* retaddr_hookee, char* pString, void * p2)
{
	char a[] = "(s)";
	char b[] = "_co8init";
	char c[] = "load";
	auto dude = Py_BuildValue(a, pString);
	templeFuncs.PyScript_Execute(b, c, dude);
	return 1;
};

class PersistentDataModLoad : public TempleFix {
public:
	const char* name() override {
		return "SpellSlinger's Persistent Data Mod for the Loadgame function)";
	}

	void apply() override {
		writeCall(0x10002D29, Co8DataHook_load);
		writeHex(0x10002D29 + 5, "C3"); // write a retn
	}
} persistentDataModLoad;
