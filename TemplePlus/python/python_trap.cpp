
#include "stdafx.h"
#include "python_trap.h"
#include "python_object.h"
#include "python_dice.h"
#include "../traps.h"

struct PyTrapDamage {
	PyObject_HEAD;
	TrapDamage damage;
};

static PyObject* PyTrapDamage_GetDamage(PyObject *obj, void*) {
	auto self = (PyTrapDamage*)obj;
	Dice dice = Dice::FromPacked(self->damage.packedDice);
	return PyDice_FromDice(dice);
}

static PyObject* PyTrapDamage_GetType(PyObject *obj, void*) {
	auto self = (PyTrapDamage*)obj;
	return PyInt_FromLong((int) self->damage.type);
}

static PyGetSetDef PyTrapDamage_GetSet[] = {
	{ (char*) "damage", PyTrapDamage_GetDamage, NULL, NULL },
	{ (char*) "type", PyTrapDamage_GetType, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyTypeObject PyTrapDamageType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PyTrapDamage", /*tp_name*/
	sizeof(PyTrapDamage), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
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
	PyTrapDamage_GetSet, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

static PyObject *PyTrapDamage_Create(TrapDamage trapDamage) {
	auto self = PyObject_New(PyTrapDamage, &PyTrapDamageType);
	self->damage = trapDamage;
	return (PyObject*)self;
}

struct PyTrap {
	PyObject_HEAD;
	objHndl handle;
};

static PyObject *PyTrap_Repr(PyObject *obj) {
	auto self = (PyTrap*)obj;
	
	auto trap = traps.GetByObj(self->handle);
	if (!trap) {
		return PyString_FromString("Trap(INVALID)");
	}

	auto triggerName = pythonObjIntegration.GetEventName(trap->trigger);

	std::string result = fmt::format("Trap({},{},{},{},{})", trap->name, triggerName, trap->partSysName, trap->replaceWith, "xx");
	return PyString_FromString(result.c_str());
}

static PyObject *PyTrap_Attack(PyObject*, PyObject *args) {
	objHndl target;
	int attackBonus, criticalHitRange, ranged;
	if (!PyArg_ParseTuple(args, "O&iii:trap.attack", &ConvertObjHndl, &target, &attackBonus, &criticalHitRange, &ranged)) {
		return 0;
	}

	auto caf = traps.Attack(target, attackBonus, criticalHitRange, ranged);
	return PyInt_FromLong(caf);
}

static PyMethodDef PyTrap_Methods[] = {
	{ "attack", PyTrap_Attack, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyObject *PyTrap_GetObj(PyObject *obj, void*) {
	auto self = (PyTrap*)obj;
	return PyObjHndl_Create(self->handle);
}

static const Trap* GetTrap(PyObject *obj) {
	auto self = (PyTrap*)obj;
	auto trap = traps.GetByObj(self->handle);
	if (!trap) {
		PyErr_SetString(PyExc_RuntimeError, "This object no longer has a trap script attached.");
		return 0;
	}
	return trap;
}

static PyObject *PyTrap_GetId(PyObject *obj, void*) {
	auto trap = GetTrap(obj);
	if (trap) {
		return PyInt_FromLong(trap->id);
	} else {
		return 0;
	}
}

static PyObject *PyTrap_GetSan(PyObject *obj, void*) {
	auto trap = GetTrap(obj);
	if (trap) {
		return PyInt_FromLong((int) trap->trigger);
	} else {
		return 0;
	}
}

static PyObject *PyTrap_GetPartSys(PyObject *obj, void*) {
	auto trap = GetTrap(obj);
	if (trap) {
		return PyString_FromString(trap->partSysName);
	} else {
		return 0;
	}
}

static PyObject *PyTrap_GetDamage(PyObject *obj, void*) {
	auto trap = GetTrap(obj);
	if (trap) {
		auto result = PyTuple_New(trap->damageCount);
		for (int i = 0; i < trap->damageCount; ++i) {
			PyTuple_SET_ITEM(result, i, PyTrapDamage_Create(trap->damage[i]));
		}
		return result;
	} else {
		return 0;
	}
}

static PyGetSetDef PyTrap_GetSet[] = {
	{ (char*)"obj", PyTrap_GetObj, NULL, NULL },
	{ (char*)"id", PyTrap_GetId, NULL, NULL },
	{ (char*)"san", PyTrap_GetSan, NULL, NULL },
	{ (char*)"partsys", PyTrap_GetPartSys, NULL, NULL },
	{ (char*)"damage", PyTrap_GetDamage, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyTypeObject PyTrapType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PyTrap", /*tp_name*/
	sizeof(PyTrap), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	PyTrap_Repr, /*tp_repr*/
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
	PyTrap_Methods, /* tp_methods */
	0, /* tp_members */
	PyTrap_GetSet, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

PyObject* PyTrap_Create(objHndl handle) {
	if (PyType_Ready(&PyTrapType)) {
		logger->error("Unable to initialize PyTrap type");
		return 0;
	}

	auto self = PyObject_New(PyTrap, &PyTrapType);
	self->handle = handle;
	return (PyObject*)self;
}
