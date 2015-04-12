
#include "stdafx.h"
#include "python_game.h"
#include <util/addresses.h>

static struct PyGameAddresses : AddressTable {
	
	int* partyAlignment;
	int *sid;
	int *newSid;
	
	void (__cdecl *SetPartyAlignment)(int alignment);
	int (__cdecl *GetPartyAlignment)();

	void(__cdecl *SetStoryState)(int alignment);
	int(__cdecl *GetStoryState)();
	
	PyGameAddresses() {
		rebase(partyAlignment, 0x1080ABA4);
		rebase(sid, 0x10BD2DA4);
		rebase(newSid, 0x102F43F8);

		rebase(SetPartyAlignment, 0x1002B720);
		rebase(GetPartyAlignment, 0x1002B730);

		rebase(SetStoryState, 0x10006A30);
		rebase(GetStoryState, 0x10006A20);
	}
} pyGameAddresses;

#define PY_INT_GETTER_PTR(expr) ((getter) [] (PyObject*,void*) { return PyInt_FromLong(*(expr)); })
#define PY_INT_SETTER_PTR(expr) ((setter) [] (PyObject*,PyObject*val,void*) { *(expr) = PyInt_AS_LONG(val); return 0; })

#define PY_INT_GETTER(method) ((getter) [] (PyObject*,void*) { return PyInt_FromLong(method()); })
#define PY_INT_SETTER(method) ((setter) [] (PyObject*,PyObject*val,void*) { method(PyInt_AS_LONG(val)); return 0; })

#define PY_INT_PROP_PTR(name, ptrExpr, doc) { name, PY_INT_GETTER_PTR((ptrExpr)), PY_INT_SETTER_PTR(ptrExpr), doc, NULL }
#define PY_INT_PROP(name, getMeth, setMeth, doc) { name, PY_INT_GETTER(getMeth), PY_INT_SETTER(setMeth), doc, NULL }

static PyGetSetDef pyGameGetSetDefs[] = {
	PY_INT_PROP("party_alignment", pyGameAddresses.GetPartyAlignment, pyGameAddresses.SetPartyAlignment, NULL),
	PY_INT_PROP("story_state", pyGameAddresses.GetStoryState, pyGameAddresses.SetStoryState, NULL),
	{ "sid", PY_INT_GETTER_PTR(pyGameAddresses.sid), NULL, NULL },
	PY_INT_PROP_PTR("new_sid", pyGameAddresses.sid, NULL)
};

