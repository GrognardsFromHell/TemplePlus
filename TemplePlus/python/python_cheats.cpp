
#include "stdafx.h"
#include "python_cheats.h"
#include <temple/dll.h>
#include <util/fixes.h>
#include <common.h>
#include <party.h>
#include <obj.h>
#include <critter.h>
#include <d20_level.h>
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>

// Returns 1 if it can handle the command
typedef int (__cdecl *CheatFn)(const char *command);
typedef void (__cdecl *CheatSetStateFn)(const char *command, int state);

const int cheatCount = 40;

#pragma pack(push, 1)
struct Cheat {
	char *name;
	int unk1;
	CheatSetStateFn setState;
	CheatFn handler;
};
#pragma pack(pop)

static struct PythonCheatsAddresses : temple::AddressTable {
	Cheat *cheats;

	PythonCheatsAddresses() {
		rebase(cheats, 0x102B02E8);
	}
} addresses;

static PyObject *PyCheats_GetAttr(PyObject*, char *name) {
	
	if (!_stricmp(name, "critical")) {
		auto alwaysCrit = temple::GetRef<int>(0x10BCA8B0);
		return PyInt_FromLong(alwaysCrit);
	}
	
	for (int i = 0; i < cheatCount; ++i) {
		auto cheat = addresses.cheats[i];
		if (!_stricmp(name, cheat.name) && cheat.handler) {
			return PyInt_FromLong(cheat.handler(name));
		}
	}
	PyErr_Format(PyExc_AttributeError, "Unknown attribute: %s", name);
	return 0;
}

static int PyCheats_SetAttr(PyObject*, char *name, PyObject *value) {
	auto intVal = PyInt_AsLong(value);

	if (!_stricmp(name, "critical")) {
		auto &alwaysCrit = temple::GetRef<int>(0x10BCA8B0);
		auto &alwaysCritPfx = temple::GetRef<int>(0x10300D10);
		alwaysCrit = intVal;
		alwaysCritPfx = intVal;
		logger->info("Cheats: critical set to {}", intVal);
		return 0;
	}

	for (int i = 0; i < cheatCount; ++i) {
		auto cheat = addresses.cheats[i];
		if (!_stricmp(name, cheat.name) && cheat.setState) {
			cheat.setState(name, intVal);
			return 0;
		}
	}

	PyErr_Format(PyExc_AttributeError, "Unknown attribute: %s", name);
	return -1;
}

static PyTypeObject PyCheatsType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PyCheats", /*tp_name*/
	sizeof(PyObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	PyCheats_GetAttr, /*tp_getattr*/
	PyCheats_SetAttr, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	0, /*tp_getattro*/
	0, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	0, /* tp_methods */
	0, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

PyObject* PyCheats_Create() {
	return PyObject_New(PyObject, &PyCheatsType);
}


class CheatHooks : TempleFix
{ // using this to prevent players from spawning invalid protos (and more importantly, not crashing the game when I make console typos!!!)
public: 
	static char * ParseString(char * in, char* out);

	void apply() override 
	{

		// give
		replaceFunction<int(__cdecl)(char*)>(0x100492D0, [](char* consoleStr)
		{
			char cStrCopy[256];
			auto v1 = ParseString(consoleStr, cStrCopy);
			ParseString(v1, cStrCopy);

			int protoNum = 5001; // DO NOT USE arrow
			sscanf(cStrCopy, "%d", &protoNum);

			if (protoNum < 1000){
				logger->warn("Invalid proto for give command: {}", protoNum);
				return 0;
			}
				

			auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoNum);
			if (!protoHandle)
				return 0;

			auto leader = party.GetConsciousPartyLeader();
			auto loc = objects.GetLocation(leader);



			auto handleNew = gameSystems->GetObj().CreateObject(protoHandle, loc);
			if (handleNew){
				if (!inventory.SetItemParent(handleNew, leader, ItemInsertFlags::IIF_None))
				{
					objects.Destroy(handleNew);
					return 0;
				}
				auto obj = gameSystems->GetObj().GetObject(handleNew);
				obj->SetItemFlag(OIF_IDENTIFIED, 1);
			}
			
			return 1;
		});

		// create
		replaceFunction<int(__cdecl)(char*)>(0x100496D0, [](char* consoleStr)
		{
			char cStrCopy[256];
			auto v1 = ParseString(consoleStr, cStrCopy);
			ParseString(v1, cStrCopy);

			int protoNum = 5001; // DO NOT USE arrow
			sscanf(cStrCopy, "%d", &protoNum);
			auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoNum);
			if (!protoHandle)
				return 0;

			auto leader = party.GetConsciousPartyLeader();
			auto loc = objects.GetLocation(leader);



			auto handleNew = gameSystems->GetObj().CreateObject(protoHandle, loc);
			if (handleNew) {
				critterSys.GenerateHp(handleNew);	
			}
			auto& consoleNewlyCreatedObj = temple::GetRef<objHndl>(0x10AA31B8);
			consoleNewlyCreatedObj = handleNew;
			return 1;
		});

		// levelup
		replaceFunction<int(__cdecl)(char*)>(0x100495B0, [](char* consoleStr)
		{
			for (auto i = 0u; i < party.GroupListGetLen(); i++){
				auto handle =  party.GroupListGetMemberN(i);
				if (!party.ObjIsAIFollower(handle) && !d20LevelSys.CanLevelup(handle)){
					auto obj = gameSystems->GetObj().GetObject(handle);

					auto curLvl = critterSys.GetEffectiveLevel(handle);
					auto xpReq = d20LevelSys.GetXpRequireForLevel(curLvl + 1);

					auto curXp = obj->GetInt32(obj_f_critter_experience);
					if ((int)xpReq > curXp)
						critterSys.AwardXp(handle, xpReq - curXp);
				}
			}
			return 1;
		});
	}
} hooks;

char* CheatHooks::ParseString(char*in, char* out)
{
	int i = 0;
	auto chr = *in;

	// advance to first non-whitespace / terminator
	while(chr && chr == ' '){
		chr = *++in;
	}

	if (*in == '"'){
		while (*in && *in != '"'){
			out[i] = *in;
			in++;
			i++;
		}
	} 
	else
	{
		while (*in){
			if (*in == ' ')
				break;
			out[i++] = *in;
			in++;
		}
	}
	out[i] = 0;
	return in;
}
