
#include "stdafx.h"

#include "python_spell.h"
#include "../common.h"
#include "../spell.h"

struct PySpellTarget {
	objHndl obj;
	int partSysId;
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
	uint32_t field_58;
	PyObject* spellVariables;
	uint32_t targetCountCopy;
	int targetCount;
	uint32_t projectileCount;
	uint32_t field_6C;
	PySpellTarget targets[32];
	SpellPacketBody *spellPacketBodies[8];
	MetaMagicData metaMagic;
	int field_270;
	int field_274;
};

// Contains all active spells
typedef unordered_map<int, PySpell*> ActiveSpellMap;
static ActiveSpellMap activeSpells;

static PyObject *PySpell_Repr(PyObject *obj) {
	auto self = (PySpell*)obj;
	return PyString_FromFormat("Spell(%d)", self->spellEnum);
}

static void PySpell_Del(PyObject *obj) {
	auto self = (PySpell*)obj;
	Py_DECREF(self->spellVariables);

	// Remove from list of active packets
	auto it = activeSpells.find(self->spellId);
	if (it != activeSpells.end()) {
		activeSpells.erase(it);
	}

	PyObject_Del(obj);
}

static PyMethodDef PySpellMethods[] = {
	{ NULL, NULL, NULL, NULL }
};

static PyGetSetDef PySpellGetSet[] = {
	{ NULL, NULL, NULL, NULL }
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

static void PySpell_UpdateFromPacket(PySpell *self, const SpellPacketBody &spell) {
	self->spellEnum = spell.spellEnum;
	self->spellEnumOriginal = spell.spellEnumOriginal;
	self->caster = spell.objHndCaster;
	self->casterPartSysId = spell.casterPartsysId;

	// TODO: check for correctness
	if (spell.casterClassCode & 0x80) {
		self->casterClass = spell.casterClassCode & 0x7F;
	}
	else {
		self->casterClassAlt = spell.casterClassCode & 0x7F;
	}

	// I think we can replace this with something better
	/*pyspell_targetloc2.LocAndOff_XY.locx = spell.locFull.location.location.locx;
	pyspell_targetloc1.LocAndOff_XY.locx = spell.locFull.LocAndOff_XY.locx;
	pyspell_targetloc2.LocAndOff_XY.off_x = spell.locFull.LocAndOff_XY.off_x;
	pyspell_targetloc2.LocAndOff_XY.locy = spell.locFull.LocAndOff_XY.locy;
	pyspell_targetloc1.LocAndOff_XY.locy = spell.locFull.LocAndOff_XY.locy;
	pyspell_targetloc1.LocAndOff_XY.off_x = spell.locFull.LocAndOff_XY.off_x;
	pyspell_targetloc2.LocAndOff_XY.off_y = spell.locFull.LocAndOff_XY.off_y;
	pyspell_targetloc1.LocAndOff_XY.off_y = spell.locFull.LocAndOff_XY.off_y;
	pyspell_targetloc2.off_z = spell.locFull.off_z;
	pyspell_targetloc1.off_z = spell.locFull.off_z;*/

	self->spellLevel = spell.spellKnownSlotLevel;
	self->casterLevel = spell.baseCasterLevel;
	self->rangeExact = spell.spellRange;
	self->dc = spell.spellDC;
	self->duration = spell.spellDuration;
	self->field_58 = spell.field_ACC;
	self->rangeExact = spell.spellRange;
	self->targetCountCopy = spell.targetListNumItemsCopy;
	self->targetCount = spell.targetListNumItems;
	self->projectileCount = spell.numProjectiles;
	*((uint32_t*)&self->metaMagic) = spell.metaMagicData;
	self->targetLocation = spell.locFull;
	self->field_270 = spell.field_9C8;
	self->field_274 = spell.field_9CC;
	self->spellPacketBodies[0] = spell.spellPktBods[0];
	self->spellPacketBodies[1] = spell.spellPktBods[1];
	self->spellPacketBodies[2] = spell.spellPktBods[2];
	self->spellPacketBodies[3] = spell.spellPktBods[3];
	self->spellPacketBodies[4] = spell.spellPktBods[4];
	self->spellPacketBodies[5] = spell.spellPktBods[5];
	self->spellPacketBodies[6] = spell.spellPktBods[6];
	self->spellPacketBodies[7] = spell.spellPktBods[7];

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
	self->spellVariables = PyList_New(0);

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

	spell.objHndCaster = self->caster;
	spell.casterPartsysId = self->casterPartSysId;
	spell.spellDC = self->dc;
	spell.targetListNumItemsCopy = self->targetCount;	
	spell.numProjectiles = self->projectileCount;	
	spell.metaMagicData = *((uint32_t*)&self->metaMagic);	
	spell.locFull = self->targetLocation;	
	spell.field_9C8 = self->field_270;	
	spell.field_9CC = self->field_274;

	for (int i = 0; i < 8; ++i) {
		spell.spellPktBods[i] = self->spellPacketBodies[i];
	}
	
	spell.baseCasterLevel = self->casterLevel;
	spell.targetListNumItems = self->targetCount;
	spell.spellDuration = self->duration;
	if (spell.field_ACC <= 0)
		spell.field_ACC = self->duration;
	spell.spellRange = self->rangeExact;

	for (int i = 0; i < self->targetCount; ++i) {
		spell.targetListHandles[i] = self->targets[i].obj;
		spell.targetListPartsysIds[i] = self->targets[i].partSysId;
	}

	spellSys.UpdateSpellPacket(spell);
}
