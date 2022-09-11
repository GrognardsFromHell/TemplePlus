#include "stdafx.h"

#include "python_spell.h"
#include "python_object.h"
#include "../common.h"
#include "../spell.h"
#include "../radialmenu.h"
#include "../location.h"
#include <ai.h>
#include <party.h>
#include <critter.h>
#include <combat.h>
#include <ui/ui.h>
#include <condition.h>
#include "animgoals/anim.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include <ui/ui_picker.h>
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"

#include <pybind11/embed.h>
namespace py = pybind11;

struct PySpell;
static PyObject *PySpellTargets_Create(PySpell *spell);

struct PySpellTarget {
	objHndl obj;
	int partSysId;
	int pad;
};

struct PySpell {
	PyObject_HEAD;
	uint32_t spellEnum;
	uint32_t spellEnumOriginal;
	uint32_t spellId;
	uint32_t field_14;
	objHndl caster;
	uint32_t casterPartSysId;
	uint32_t casterClass;
	uint32_t casterClassAlt; // used for spells cast from items, and maybe domain spells too
	uint32_t field_2C;
	LocFull targetLocation;
	uint32_t dc;
	uint32_t spellLevel;
	uint32_t casterLevel;
	uint32_t rangeExact;
	uint32_t duration;
	uint32_t durationRemaining;
	uint32_t orgtargetCount;
	int targetCount;
	uint32_t projectileCount;
	uint32_t field_6C;
	PySpellTarget targets[32];
	objHndl projectiles[5];
	MetaMagicData metaMagic;
};

struct PySpellStore {
	PyObject_HEAD;
	SpellStoreData spellData;
};


const int testSizeofPyspell = sizeof PySpell; // should be 680 (due to HEAD_EXTRA is +8 relative to toee)

// Contains all active spells
typedef unordered_map<int, PySpell*> ActiveSpellMap;
static ActiveSpellMap activeSpells;

/******************************************************************************
* PySpell
******************************************************************************/
#pragma region PySpell

void PySpell_Reset() {
	for (auto it = activeSpells.begin(); it != activeSpells.end();) {
		logger->error("Undisposed PySpell caught: ID={} enum={}. Removing from Python ActiveSpellsMap activeSpells.", it->second->spellId, it->second->spellEnum);
		it = activeSpells.erase(it);
	}
}

static PyObject* PySpell_Repr(PyObject* obj) {
	auto self = (PySpell*)obj;
	return PyString_FromFormat("Spell(%d)", self->spellEnum);
}

static void PySpell_Del(PyObject* obj) {
	auto self = (PySpell*)obj;

	// Remove from list of active packets
	auto it = activeSpells.find(self->spellId);
	if (it != activeSpells.end()) {
		activeSpells.erase(it);
	}

	PyObject_Del(obj);
}

// This is effectively a static method...
static PyObject *PySpell_SpellEnd(PyObject*, PyObject *args) {
	int spellId, endDespiteTargetList = 0;
	if (!PyArg_ParseTuple(args, "i|i:pyspell.spell_end", &spellId, &endDespiteTargetList)) {
		return 0;
	}
	spellSys.SpellEnd(spellId, endDespiteTargetList);
	Py_RETURN_NONE;
}

static PyObject *PySpell_SpellRemove(PyObject *obj, PyObject *args) {
	int unk1;
	if (!PyArg_ParseTuple(args, "i:pyspell.spell_remove", &unk1)) {
		return 0;
	}
	spellSys.SpellRemove(unk1);
	Py_RETURN_NONE;
}

enum TargetSortCriteria {
	OBJ_HANDLE = 0,
	HIT_DICE = 1,
	HIT_DICE_THEN_DIST = 2,
	DIST = 3,
	DIST_FROM_CASTER = 4
};

static PyObject *PySpell_SpellTargetListSort(PyObject *obj, PyObject *args) {
	auto self = (PySpell*)obj;
	TargetSortCriteria criteria;
	BOOL descending;
	if (!PyArg_ParseTuple(args, "ii:pyspell.spell_target_list_sort", &criteria, &descending)) {
		return 0;
	}
	
	// Copy them over for much easier sorting using STL
	vector<PySpellTarget> targets(self->targetCount);
	for (int i = 0; i < self->targetCount; ++i) {
		targets[i] = self->targets[i];
	}

	switch (criteria) {
		// This seems pretty useless
	case OBJ_HANDLE:
		sort(targets.begin(), targets.end(), [=] (const PySpellTarget &a, const PySpellTarget &b) {
			if (!descending) {
				return a.obj.handle < b.obj.handle;
			} else {
				return a.obj.handle >= b.obj.handle;
			}
		});
		break;
	case HIT_DICE: 
		sort(targets.begin(), targets.end(), [=](const PySpellTarget &a, const PySpellTarget &b) {
			auto diceA = objects.GetHitDiceNum(a.obj);
			auto diceB = objects.GetHitDiceNum(b.obj);
			if (!descending) {
				return diceA < diceB;
			} else {
				return diceA >= diceB;
			}
		});
		break;
	case HIT_DICE_THEN_DIST: 
		sort(targets.begin(), targets.end(), [=](const PySpellTarget &a, const PySpellTarget &b) {
			auto diceA = objects.GetHitDiceNum(a.obj);
			auto diceB = objects.GetHitDiceNum(b.obj);
			if (diceA != diceB) {
				if (!descending) {
					return diceA < diceB;
				} else {
					return diceA >= diceB;
				}
			}

			auto distA = locSys.DistanceToLoc(a.obj, self->targetLocation.location);
			auto distB = locSys.DistanceToLoc(b.obj, self->targetLocation.location);
			if (!descending) {
				return distA < distB;
			} else {
				return distA >= distB;
			}
		});
		break;
	case DIST: 
		sort(targets.begin(), targets.end(), [=](const PySpellTarget &a, const PySpellTarget &b) {
			auto distA = locSys.DistanceToLoc(a.obj, self->targetLocation.location);
			auto distB = locSys.DistanceToLoc(b.obj, self->targetLocation.location);
			if (!descending) {
				return distA < distB;
			} else {
				return distA >= distB;
			}
		});
		break;
	case DIST_FROM_CASTER:
		sort(targets.begin(), targets.end(), [=](const PySpellTarget &a, const PySpellTarget &b) {
			// In vanilla, this still just sorts between the target loc and obj. It's fixed here
			auto distA = locSys.DistanceToObj(a.obj, self->caster);
			auto distB = locSys.DistanceToObj(b.obj, self->caster);
			if (!descending) {
				return distA < distB;
			}
			else {
				return distA >= distB;
			}
		});
		break;
	default:
		logger->warn("Unknown sorting used for sorting the target list.");
		break;
	}

	PySpell_UpdatePacket(obj);
	Py_RETURN_NONE;
}

// This is just used to communicate with the python script, 
// no internal meaning other than that
enum class RadialMenuSetting : int {
	Min = 1,
	Max = 2,
	Actual = 3
};

static PyObject *PySpell_SpellGetMenuArg(PyObject*, PyObject *args) {
	RadialMenuSetting setting;
	if (!PyArg_ParseTuple(args, "i:pyspell.spell_get_menu_arg", &setting)) {
		return 0;
	}
	
	int result;
	auto &lastSelected = radialMenus.GetLastSelected();
	switch (setting) {
	case RadialMenuSetting::Min:
		result = lastSelected.minArg;
		break;
	case RadialMenuSetting::Max: 
		result = lastSelected.maxArg;
		break;
	case RadialMenuSetting::Actual: 
		result = lastSelected.actualArg;
		break;
	default:
		Py_RETURN_NONE;
	}

	return PyInt_FromLong(result);
}


static PyObject* PySpell_RollDiceWithMetaMagic(PyObject* obj, PyObject* args) {
	int numDice;
	int numSides;
	int modifier;

	if (!PyArg_ParseTuple(args, "iii:pyspell.roll_dice_with_metamagic", &numDice, &numSides, &modifier)) {
		return 0;
	}

	//Should move the actual work into py spell
	auto self = (PySpell*)obj;

	int result = spellSys.RollWithMetamagic(self->spellId, numDice, numSides, modifier);

	return PyInt_FromLong(result);
}



static PyObject *PySpell_SpellGetPickerEndPoint(PyObject*, PyObject *args) {
	
	auto wallEndPt = uiPicker.GetWallEndPoint();
	py::object pyLoc = py::cast(wallEndPt);
	pyLoc.inc_ref();
	return pyLoc.ptr();
}



static PyObject *PySpell_IsObjectSelected(PyObject *obj, PyObject *) {
	auto self = (PySpell*)obj;
	SpellPacketBody body;
	spellSys.GetSpellPacketBody(self->spellId, &body);
	auto result = (body.pickerResult.flags & PRF_HAS_SELECTED_OBJECT) == PRF_HAS_SELECTED_OBJECT;
	return PyInt_FromLong(result);
}

static PyObject *PySpell_SummonMonsters(PyObject *obj, PyObject *args) {
	auto self = (PySpell*)obj;

	int isAiFollower;
	int protoId = 17000;
	if (!PyArg_ParseTuple(args, "i|i:pyspell.summon_monsters", &isAiFollower, &protoId)) {
		return 0;
	}

	if (self->targetCount >= 32) {
		PyErr_SetString(PyExc_RuntimeError, "Cannot add to the target list since there are already 32 targets.");
		return 0;
	}

	auto protoHandle = objects.GetProtoHandle(protoId);

	auto loc = self->targetLocation.location.location;
	auto newHandle = objects.Create(protoHandle, loc);

	if (!newHandle) {
		logger->error("Unable to create object with proto id {}", protoId);
		Py_RETURN_NONE;
	}

	objects.AiForceSpreadOut(newHandle);

	if (!critterSys.AddFollower(newHandle, self->caster, 1, isAiFollower != 0)) {
		logger->error("Unable to add new critter as a follower");
		objects.Destroy(newHandle);
		Py_RETURN_NONE;
	}
	
	combatSys.AddToInitiative(newHandle);
	auto casterIni = combatSys.GetInitiative(self->caster);
	combatSys.SetInitiative(newHandle, casterIni);

	uiSystems->GetCombat().Update();
	uiSystems->GetParty().Update();

	conds.AddTo(newHandle, "sp-Summoned", { (int)self->spellId, (int) self->duration, 0 });
	conds.AddTo(newHandle, "Timed-Disappear", { (int) self->spellId, (int)self->duration, 0 });
	
	// Add to the target list
	self->targets[self->targetCount].obj = newHandle;
	self->targets[self->targetCount].partSysId = 0;
	self->targetCount++;

	// Make NPC summoned monsters attack the party
	if (objects.IsNPC(self->caster) && !party.IsInParty(self->caster)) {

		// add the summoner's faction
		

		auto factionArr = gameSystems->GetObj().GetObject(self->caster)->GetInt32Array(obj_f_npc_faction);
		int numFactions = factionArr.GetSize();

		for (auto i = 0; i < numFactions; i++){
			auto newFac = factionArr[i];
			if (!newFac)
				continue;
			if (!objects.factions.FactionHas(newHandle, newFac)) {
				objects.factions.FactionAdd(newHandle, newFac);
			}
		}

		

		for (size_t i = 0; i < party.GroupListGetLen(); ++i) {
			auto partyMember = party.GroupListGetMemberN(i);
			critterSys.Attack(partyMember, newHandle, 1, 2);
		}

	}
	/* If PC summon - strip all factions */
	else if (party.IsInParty(self->caster)) {
		auto summonedObj = objSystem->GetObject(newHandle);
		summonedObj->ClearArray(obj_f_npc_faction);
	}
	gameSystems->GetAnim().Interrupt(newHandle, AGP_HIGHEST);	
	
	PySpell_UpdatePacket((PyObject*) self);
	return PyInt_FromLong(1);
}

static PyMethodDef PySpellMethods[] = {
	{ "spell_end", PySpell_SpellEnd, METH_VARARGS, NULL },
	{ "spell_remove", PySpell_SpellRemove, METH_VARARGS, NULL },
	{ "spell_target_list_sort", PySpell_SpellTargetListSort, METH_VARARGS, NULL },
	{ "spell_get_menu_arg", PySpell_SpellGetMenuArg, METH_VARARGS, NULL },
	{ "spell_get_picker_end_point", PySpell_SpellGetPickerEndPoint, METH_VARARGS, NULL },
	{ "is_object_selected", PySpell_IsObjectSelected, METH_VARARGS, NULL },
	{ "summon_monsters", PySpell_SummonMonsters, METH_VARARGS, NULL },
	{ "roll_dice_with_metamagic", PySpell_RollDiceWithMetaMagic, METH_VARARGS, NULL },
	{NULL, NULL, NULL, NULL}
};




static PyObject* PySpell_GetBeginRoundObj(PyObject* obj, void*) {
	return PyObjHndl_Create(spellSys.mSpellBeginRoundObj);
}


static PyObject* PySpell_GetCaster(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyObjHndl_Create(self->caster);
}

static int PySpell_SetCaster(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyObjHndl_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Caster must be an object handle");
		return -1;
	}
	self->caster = PyObjHndl_AsObjHndl(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetCasterClass(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->casterClass);
}

static PyObject* PySpell_GetSpellLevel(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->spellLevel);
}

static PyObject* PySpell_GetRangeExact(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->rangeExact);
}

static int PySpell_SetRangeExact(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Range exact must be an int");
		return -1;
	}
	self->rangeExact = PyInt_AsLong(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetTargetLoc(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyLong_FromLongLong(self->targetLocation.location.location.ToField());
}

static int PySpell_SetTargetLoc(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyLong_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Duration must be a long");
		return -1;
	}
	self->targetLocation.location.location = locXY::fromField(PyLong_AsLongLong(value));
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetTargetLocOffX(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyFloat_FromDouble(self->targetLocation.location.off_x);
}

static PyObject* PySpell_GetTargetLocOffY(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyFloat_FromDouble(self->targetLocation.location.off_y);
}

static PyObject* PySpell_GetTargetLocOffZ(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyFloat_FromDouble(self->targetLocation.off_z);
}

static PyObject* PySpell_GetTargetLocFull(PyObject* obj, void*) {
	auto self = (PySpell*)obj;

	py::object blyat = py::cast(self->targetLocation.location);
	blyat.inc_ref();
	return blyat.ptr();
}



static PyObject* PySpell_GetCasterLevel(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->casterLevel);
}

static int PySpell_SetCasterLevel(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Caster level can only be an integer");
		return -1;
	}
	self->casterLevel = PyInt_AsLong(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetDC(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->dc);
}

static int PySpell_SetDC(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "DC can only be an integer");
		return -1;
	}
	self->dc = PyInt_AsLong(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetId(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->spellId);
}

static PyObject* PySpell_GetDuration(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->duration);
}

static PyObject* PySpell_GetDurationRemain(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->durationRemaining);
}


static int PySpell_SetDuration(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Duration can only be an integer");
		return -1;
	}
	auto duration = PyInt_AsLong(value);
	if (self->metaMagic.metaMagicExtendSpellCount)
		duration = duration * (1 + self->metaMagic.metaMagicExtendSpellCount); // crappy but that's how it is in vanilla, and all the scripts are written with this in mind
	self->duration = duration;
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetNumOfTargets(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->targetCount);
}

static int PySpell_SetNumOfTargets(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Number of targets can only be an integer");
		return -1;		
	}
	self->targetCount = PyInt_AsLong(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetNumOfProjectiles(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->projectileCount);
}

static int PySpell_SetNumOfProjectiles(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Number of projectiles can only be an integer");
		return -1;
	}
	self->projectileCount = PyInt_AsLong(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetCasterPartsysId(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->casterPartSysId);
}

static int PySpell_SetCasterPartsysId(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpell*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Caster partsys id can only be an integer");
		return -1;
	}
	self->casterPartSysId = PyInt_AsLong(value);
	PySpell_UpdatePacket(obj);
	return 0;
}

static PyObject* PySpell_GetTargetList(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PySpellTargets_Create(self);
}

static PyObject* PySpell_GetSpellRadius(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	auto spellEntry = spellEntryRegistry.get(self->spellEnum);
	return PyInt_FromLong(spellEntry->radiusTarget);
}

static PyObject* PySpell_GetSpell(PyObject* obj, void*) {
	auto self = (PySpell*)obj;
	return PyInt_FromLong(self->spellEnum);
}

static PyGetSetDef PySpellGetSet[] = {
	{ "begin_round_obj", PySpell_GetBeginRoundObj, NULL, NULL },
	{"caster", PySpell_GetCaster, PySpell_SetCaster, NULL},
	{"caster_class", PySpell_GetCasterClass, NULL, NULL},
	{"spell_level", PySpell_GetSpellLevel, NULL, NULL},
	{"range_exact", PySpell_GetRangeExact, PySpell_SetRangeExact, NULL},
	{"target_loc", PySpell_GetTargetLoc, PySpell_SetTargetLoc, NULL},
	{"target_loc_off_x", PySpell_GetTargetLocOffX, NULL, NULL},
	{"target_loc_off_y", PySpell_GetTargetLocOffY, NULL, NULL},
	{"target_loc_off_z", PySpell_GetTargetLocOffZ, NULL, NULL},
	{"target_loc_full", PySpell_GetTargetLocFull, NULL, NULL },
	{"caster_level", PySpell_GetCasterLevel, PySpell_SetCasterLevel, NULL},
	{"dc", PySpell_GetDC, PySpell_SetDC, NULL},
	{"id", PySpell_GetId, NULL, NULL},
	{"duration", PySpell_GetDuration, PySpell_SetDuration, NULL},
	{ "duration_remaining", PySpell_GetDurationRemain, NULL, NULL },
	{"num_of_targets", PySpell_GetNumOfTargets, PySpell_SetNumOfTargets, NULL},
	{"num_of_projectiles", PySpell_GetNumOfProjectiles, PySpell_SetNumOfProjectiles, NULL},
	{"caster_partsys_id", PySpell_GetCasterPartsysId, PySpell_SetCasterPartsysId, NULL},
	{"target_list", PySpell_GetTargetList, NULL, NULL},
	{"spell_radius", PySpell_GetSpellRadius, NULL, NULL},
	{"spell", PySpell_GetSpell, NULL, NULL},
	{NULL, NULL, NULL, NULL}
};

static PyTypeObject PySpellType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PySpell", /*tp_name*/
	sizeof(PySpell), /*tp_basicsize*/
	0, /*tp_itemsize*/
	PySpell_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	PySpell_Repr, /*tp_repr*/
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PySpellMethods, /* tp_methods */
	0, /* tp_members */
	PySpellGetSet, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

static void PySpell_UpdateFromPacket(PySpell* self, const SpellPacketBody& spell) {
	self->spellEnum = spell.spellEnum;
	self->spellEnumOriginal = spell.spellEnumOriginal;
	self->caster = spell.caster;
	self->casterPartSysId = spell.casterPartsysId;

	// TODO: check for correctness
	if (spell.spellClass & 0x80) {
		self->casterClass = spell.spellClass & 0x7F;
	} else {
		self->casterClassAlt = spell.spellClass & 0x7F;
	}

	self->spellLevel = spell.spellKnownSlotLevel;
	self->casterLevel = spell.casterLevel;
	self->rangeExact = spell.spellRange;
	self->dc = spell.dc;
	self->duration = spell.duration;
	self->durationRemaining = spell.durationRemaining;
	self->rangeExact = spell.spellRange;
	self->orgtargetCount = spell.orgTargetCount;
	self->targetCount = spell.targetCount;
	self->projectileCount = spell.projectileCount;
	*((uint32_t*)&self->metaMagic) = spell.metaMagicData;
	self->targetLocation = spell.aoeCenter;
	for (int i = 0; i < 5; i++)
		self->projectiles[i] = spell.projectiles[i];

	for (int i = 0; i < 32; ++i) {
		self->targets[i].obj = spell.targetListHandles[i];
		self->targets[i].partSysId = spell.targetListPartsysIds[i];
	}

}

PyObject* PySpell_Create(int spellId) {
	if (PyType_Ready(&PySpellType)) {
		return 0;
	}

	// Return an already created and still active spell packet if possible
	auto it = activeSpells.find(spellId);
	if (it != activeSpells.end()) {
		auto result = it->second;
		Py_INCREF(result);
		return (PyObject*) result;
	}

	SpellPacketBody spell;
	if (!spellSys.GetSpellPacketBody(spellId, &spell)) {
		Py_RETURN_NONE;
	}

	auto self = PyObject_NEW(PySpell, &PySpellType);
	self->spellId = spellId;

	PySpell_UpdateFromPacket(self, spell);

	// We do not incref here since we remove this entry in the obj's destructor
	activeSpells[spellId] = self;

	return (PyObject*)self;
}

/*
	If any python spell for the given spell id is still active, update its properties from
	the spell packet.
*/
void PySpell_Update(int spellId) {
	auto it = activeSpells.find(spellId);

	if (it == activeSpells.end()) {
		return;
	}

	auto self = it->second;
	SpellPacketBody spell;
	if (!spellSys.GetSpellPacketBody(spellId, &spell)) {
		return;
	}

	PySpell_UpdateFromPacket(self, spell);
}

void PySpell_UpdatePacket(PyObject* pySpell) {
	auto self = (PySpell*)pySpell;

	SpellPacketBody spell;
	spellSys.GetSpellPacketBody(self->spellId, &spell);

	spell.caster = self->caster;
	spell.casterPartsysId = self->casterPartSysId;
	spell.dc = self->dc;
	spell.orgTargetCount = self->targetCount;
	spell.projectileCount = self->projectileCount;
	spell.metaMagicData = *((uint32_t*)&self->metaMagic);
	spell.aoeCenter = self->targetLocation;

	for (int i = 0; i < 5; ++i) {
		spell.projectiles[i] = self->projectiles[i];
	}

	spell.casterLevel = self->casterLevel;
	spell.targetCount = self->targetCount;
	spell.duration = self->duration;
	if (spell.durationRemaining <= 0)
		spell.durationRemaining = self->duration;
	spell.spellRange = self->rangeExact;

	for (int i = 0; i < self->targetCount; ++i) {
		spell.targetListHandles[i] = self->targets[i].obj;
		spell.targetListPartsysIds[i] = self->targets[i].partSysId;
	}
	spell.UpdateSpellsCastRegistry();
}

objHndl PySpell_GetTargetHandle(PyObject* spell, int targetIdx) {
	auto self = (PySpell*)spell;
	return self->targets[targetIdx].obj;
}

#pragma endregion

/******************************************************************************
* Spell Target
******************************************************************************/

#pragma region Spell Target

static PyObject* PySpellTargets_Repr(PyObject* obj);

struct PySpellTargetsEntry {
	PyObject_HEAD;
	PySpell *spell;
	int targetIdx;
};

static void PySpellTargetsEntry_Del(PyObject *obj) {
	auto self = (PySpellTargetsEntry*)obj;
	Py_DECREF(self->spell);
	PyObject_Del(obj);
}

static PyObject* PySpellTargetsEntry_Repr(PyObject *obj) {
	auto self = (PySpellTargetsEntry*)obj;
	auto prefix = PyString_FromFormat("PyTargetArrayItem[index = %d],parent:", self->targetIdx);
	auto suffix = PySpellTargets_Repr(obj);
	PyString_ConcatAndDel(&prefix, suffix);
	return prefix;
}

static PyObject* PySpellTargetsEntry_GetObj(PyObject *obj, void*) {
	auto self = (PySpellTargetsEntry*)obj;
	return PyObjHndl_Create(self->spell->targets[self->targetIdx].obj);
}

static int PySpellTargetsEntry_SetObj(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpellTargetsEntry*)obj;
	if (PyObjHndl_Check(value)) {
		self->spell->targets[self->targetIdx].obj = PyObjHndl_AsObjHndl(value);
		PySpell_UpdatePacket((PyObject*) self->spell);
		return 0;
	}
	PyErr_SetString(PyExc_TypeError, "obj can only be set to an object handle.");
	return -1;
}

static PyObject* PySpellTargetsEntry_GetPartsysId(PyObject *obj, void*) {
	auto self = (PySpellTargetsEntry*)obj;
	return PyInt_FromLong(self->spell->targets[self->targetIdx].partSysId);
}

static int PySpellTargetsEntry_SetPartsysId(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpellTargetsEntry*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "The particle system id can only be set to an integer.");
		return -1;
	}
	self->spell->targets[self->targetIdx].partSysId = PyInt_AsLong(value);
	PySpell_UpdatePacket((PyObject*) self->spell);
	return 0;
}

static PyGetSetDef PySpellTargetsEntryGetSet[] = {
	{ "obj", PySpellTargetsEntry_GetObj, PySpellTargetsEntry_SetObj, NULL },
	{ "partsys_id", PySpellTargetsEntry_GetPartsysId, PySpellTargetsEntry_SetPartsysId, NULL },
	{NULL, NULL, NULL, NULL}
};

static PyTypeObject PySpellTargetsEntryType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PySpellTargetsEntry", /*tp_name*/
	sizeof(PySpellTargetsEntry), /*tp_basicsize*/
	0, /*tp_itemsize*/
	PySpellTargetsEntry_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	PySpellTargetsEntry_Repr, /*tp_repr*/
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
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
	PySpellTargetsEntryGetSet, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

static PyObject *PySpellTargetsEntry_Create(PySpell *spell, int targetIdx) {
	if (targetIdx < 0 || targetIdx >= spell->targetCount) {
		PyErr_Format(PyExc_IndexError, "Target array index %d is out of range.", targetIdx);
		return 0;
	}
	auto result = PyObject_New(PySpellTargetsEntry, &PySpellTargetsEntryType);
	result->spell = spell;
	Py_INCREF(spell);
	result->targetIdx = targetIdx;
	return (PyObject*) result;
}

#pragma endregion

/******************************************************************************
 * Spell Target Array
 ******************************************************************************/
#pragma region Spell Target Array
struct PySpellTargets {
	PyObject_HEAD;
	PySpell *spell;
};

static void PySpellTargets_Del(PyObject *obj) {
	auto self = (PySpellTargets*)obj;
	Py_DECREF(self->spell);
	PyObject_Del(obj);
}

static PyObject* PySpellTargets_Repr(PyObject* obj) {
	auto self = (PySpellTargets*)obj;
	auto spell = self->spell;

	if (spell->targetCount <= 0) {
		return PyString_FromString("No targets");
	}

	auto text = format("number of targets: ({}) - ", spell->targetCount);

	for (auto i = 0; i < spell->targetCount; ++i) {
		auto targetObj = spell->targets[i].obj;
		if (obj) {
			auto displayName = objects.GetDisplayName(targetObj, objHndl::null);
			text.append(format("{}[{}({})]", i, displayName, targetObj));
		} else {
			text.append(format("{}[OBJ_HANDLE_NULL], ", i));
		}
	}

	return PyString_FromString(text.c_str());
}

static bool PySpell_RemoveTarget(PySpell *spell, objHndl handle) {
	// Iterate over all targets, removing the ones that are equal to the target we should remove
	auto changed = false;

	for (auto i = 0; i < spell->targetCount; ++i) {
		if (spell->targets[i].obj == handle) {
			auto remaining = spell->targetCount - i - 1;
			if (remaining > 0) {
				memcpy(&spell->targets[i], &spell->targets[i + 1], sizeof(PySpellTarget) * remaining);
			}
			--spell->targetCount;
			changed = true;
		}
	}

	return changed;
}

static PyObject* PySpellTargets_RemoveTarget(PyObject* obj, PyObject* args) {
	auto self = (PySpellTargets*)obj;
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:pyspelltargets.remove_target", &ConvertObjHndl, &target)) {
		return 0;
	}
	
	auto spell = self->spell;
	if (PySpell_RemoveTarget(spell, target)) {
		PySpell_UpdatePacket((PyObject*)self->spell);
	}
	Py_RETURN_NONE;
}

static PyObject* PySpellTargets_RemoveList(PyObject* obj, PyObject* args) {
	auto self = (PySpellTargets*)obj;
	PyObject *list;
	if (!PyArg_ParseTuple(args, "O!:pyspelltargets.remove_target", &PyList_Type, &list)) {
		return 0;
	}

	bool changed = false;
	for (int i = 0; i < PyList_GET_SIZE(list); ++i) {
		auto objObj = PyList_GET_ITEM(list, i);
		if (!PyObjHndl_Check(objObj)) {
			continue;
		}
		auto handle = PyObjHndl_AsObjHndl(objObj);
		if (PySpell_RemoveTarget(self->spell, handle)) {
			changed = true;
		}
	}
	
	if (changed) {
		PySpell_UpdatePacket((PyObject*)self->spell);
	}
	Py_RETURN_NONE;
}

static PyMethodDef PySpellTargetsMethods[] = {
	{ "remove_target", PySpellTargets_RemoveTarget, METH_VARARGS, NULL },
	{ "remove_list", PySpellTargets_RemoveList, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

Py_ssize_t PySpellTargets_Len(PyObject *obj) {
	auto self = (PySpellTargets*)obj;
	return self->spell->targetCount;
}

static PyObject *PySpellTargets_GetItem(PyObject *obj, Py_ssize_t index) {
	auto self = (PySpellTargets*) obj;
	return PySpellTargetsEntry_Create(self->spell, index);
}

static PySequenceMethods PySpellTargetsSequence = {
	PySpellTargets_Len,
	0,
	0,
	PySpellTargets_GetItem,
	0,
	0,
	0,
};

static PyTypeObject PySpellTargetsType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PySpellTargets", /*tp_name*/
	sizeof(PySpellTargets), /*tp_basicsize*/
	0, /*tp_itemsize*/
	PySpellTargets_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	PySpellTargets_Repr, /*tp_repr*/
	0, /*tp_as_number*/
	&PySpellTargetsSequence, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PySpellTargetsMethods, /* tp_methods */
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

PyObject *PySpellTargets_Create(PySpell *spell) {
	auto self = PyObject_New(PySpellTargets, &PySpellTargetsType);
	self->spell = spell;
	Py_INCREF(spell);
	return (PyObject*)self;
}

BOOL ConvertTargetArray(PyObject* obj, PySpell** pySpellOut) {
	if (obj->ob_type != &PySpellTargetsType) {
		PyErr_SetString(PyExc_TypeError, "Expected spell target array.");
		return FALSE;
	}

	*pySpellOut = ((PySpellTargets*)obj)->spell;
	return TRUE;
}
#pragma endregion


/******************************************************************************
* Spell Store
******************************************************************************/

#pragma region Spell Store
BOOL ConvertSpellStore(PyObject * obj, SpellStoreData * pSpellStoreOut)
{
	if (obj == Py_None) {
		pSpellStoreOut->spellEnum = 0;
		pSpellStoreOut->spellLevel = 0;
		pSpellStoreOut->classCode = 0;
		return TRUE;
	}

	if (obj->ob_type != &PySpellStoreType) {
		PyErr_SetString(PyExc_TypeError, "Expected PySpellStore.");
		return FALSE;
	}

	*pSpellStoreOut = ((PySpellStore*)obj)->spellData;
	return TRUE;
}


static int PySpellStore_Init(PyObject* obj, PyObject* args, PyObject* kwargs) {
	auto self = (PySpellStore*)obj;

	if (!PyArg_ParseTuple(args, "|iii:PySpellStore", &self->spellData.spellEnum, &self->spellData.classCode, &self->spellData.spellLevel)) {
		return -1;
	}

	return 0;
}

static PyObject* PySpellStore_New(PyTypeObject*, PyObject*, PyObject*) {
	auto self = PyObject_New(PySpellStore, &PySpellStoreType);
	self->spellData = SpellStoreData();
	return (PyObject*)self;
}

PyObject* PySpellStore_Create(const SpellStoreData& spellData){
	if (PyType_Ready(&PySpellStoreType)) {
		return 0;
	}

	auto self = PyObject_NEW(PySpellStore, &PySpellStoreType);
	self->spellData = spellData;

	return (PyObject*)self;
}

SpellStoreData PySpellStore_AsSpellStore(PyObject * obj){
	auto self = (PySpellStore*)obj;
	return self->spellData;
}

static void PySpellStore_Del(PyObject *obj) {
	PyObject_Del(obj);
}

static PyObject* PySpellStore_Repr(PyObject* obj) {
	auto self = (PySpellStore*)obj;
	return PyString_FromFormat("PySpellStore %s (enum=%d, class=%d, level=%d)", spellSys.GetSpellName(self->spellData.spellEnum), self->spellData.spellEnum, self->spellData.classCode, self->spellData.spellLevel);
}


static PyObject* PySpellStore_IsNaturallyCast(PyObject* obj, PyObject* args) {
	auto self = (PySpellStore*)obj;
	if (!self->spellData.spellEnum)
		return PyInt_FromLong(0);

	if (spellSys.isDomainSpell(self->spellData.classCode))
		return PyInt_FromLong(0);

	return PyInt_FromLong(d20ClassSys.IsNaturalCastingClass(spellSys.GetCastingClass(self->spellData.classCode)));

}

static PyObject* PySpellStore_IsAreaSpell(PyObject* obj, PyObject* args) {
	auto self = (PySpellStore*)obj;
	if (!self->spellData.spellEnum)
		return PyInt_FromLong(0);

	SpellEntry spEntry(self->spellData.spellEnum);
	if (!spEntry.spellEnum){
		return PyInt_FromLong(0);
	}

	return PyInt_FromLong(spEntry.IsBaseModeTarget(UiPickerType::Area));
}

static PyObject* PySpellStore_IsModeTarget(PyObject* obj, PyObject* args) {
	auto self = (PySpellStore*)obj;
	if (!self->spellData.spellEnum)
		return PyInt_FromLong(0);

	SpellEntry spEntry(self->spellData.spellEnum);
	if (!spEntry.spellEnum) {
		return PyInt_FromLong(0);
	}

	int modeTarget;
	if (!PyArg_ParseTuple(args, "i:PySpellStore.is_mode_target", &modeTarget)) {
		return PyInt_FromLong(0);
	}

	return PyInt_FromLong(spEntry.IsBaseModeTarget((UiPickerType)modeTarget));
}

static PyObject* PySpellStore_IsUsedUp(PyObject* obj, PyObject* args) {
	auto self = (PySpellStore*)obj;
	if (!self->spellData.spellEnum)
		return PyInt_FromLong(0);

	return PyInt_FromLong(self->spellData.spellStoreState.usedUp & 1);
}

static PyMethodDef PySpellStoreMethods[] = {
	{"is_area_spell", PySpellStore_IsAreaSpell, METH_VARARGS, NULL },
	{"is_mode_target", PySpellStore_IsModeTarget, METH_VARARGS, NULL },
	{"is_naturally_cast", PySpellStore_IsNaturallyCast, METH_VARARGS, NULL },
	{"is_used_up", PySpellStore_IsUsedUp, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};



static PyObject* PySpellStore_GetEnum(PyObject* obj, void*) {
	auto self = (PySpellStore*)obj;
	return PyInt_FromLong(self->spellData.spellEnum);
}

static int PySpellStore_SetEnum(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpellStore*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "spell_enum can only be an integer");
		return -1;
	}
	self->spellData.spellEnum = PyInt_AsLong(value);
	return 0;
}

static PyObject* PySpellStore_GetLevel(PyObject* obj, void*) {
	auto self = (PySpellStore*)obj;
	return PyInt_FromLong(self->spellData.spellLevel);
}

static int PySpellStore_SetLevel(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpellStore*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "spell_level can only be an integer");
		return -1;
	}
	self->spellData.spellLevel = PyInt_AsLong(value);
	return 0;
}

static PyObject* PySpellStore_GetClass(PyObject* obj, void*) {
	auto self = (PySpellStore*)obj;
	return PyInt_FromLong(self->spellData.classCode);
}

static PyObject* PySpellStore_GetCasterClass(PyObject* obj, void*) {
	auto self = (PySpellStore*)obj;
	return PyInt_FromLong(spellSys.GetCastingClass(self->spellData.classCode));
}

static int PySpellStore_SetClass(PyObject *obj, PyObject *value, void*) {
	auto self = (PySpellStore*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "spell_class can only be an integer");
		return -1;
	}
	self->spellData.classCode = PyInt_AsLong(value);
	return 0;
}


static PyObject* PySpellStore_GetName(PyObject* obj, void*) {
	auto self = (PySpellStore*)obj;
	return PyString_FromString(spellSys.GetSpellName(self->spellData.spellEnum));
}
static PyGetSetDef PySpellStoreGetSet[] = {
	{ "spell_enum", PySpellStore_GetEnum, PySpellStore_SetEnum, NULL },
	{ "spell_level", PySpellStore_GetLevel, PySpellStore_SetLevel, NULL },
	{ "spell_class", PySpellStore_GetClass, PySpellStore_SetClass, NULL },
	{ "caster_class", PySpellStore_GetCasterClass, NULL, NULL },
	{ "spell_name", PySpellStore_GetName, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PySpellStoreType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PySpellStore", /*tp_name*/
	sizeof(PySpellStore), /*tp_basicsize*/
	0, /*tp_itemsize*/
	PySpellStore_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	PySpellStore_Repr, /*tp_repr*/
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PySpellStoreMethods, /* tp_methods */
	0, /* tp_members */
	PySpellStoreGetSet, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	PySpellStore_Init, /* tp_init */
	0, /* tp_alloc */
	PySpellStore_New, /* tp_new */
};

#pragma endregion