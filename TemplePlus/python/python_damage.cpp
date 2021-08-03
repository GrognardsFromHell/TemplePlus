
#include "stdafx.h"
#include "python_bonus.h"
#include <structmember.h>
#include "../damage.h"
#include "../common.h"
#include "../feat.h"
#include "python_dice.h"
#include "../dice.h"
#include "python_damage.h"


struct PyDamagePacket {
	PyObject_HEAD;
	DamagePacket *dam;
};

PyObject *PyDamagePacket_Repr(PyObject *obj) {
	auto self = (PyDamagePacket*)obj;
	string text;

	text = fmt::format("DamagePacket[ Dice: {}]", self->dam->diceCount);

	return PyString_FromString(text.c_str());
}

PyObject *PyDamagePacket_Clone(PyObject *obj, PyObject *args) {
	auto self = (PyDamagePacket*)obj;
	auto result = PyObject_New(PyDamagePacket, obj->ob_type);
	result->dam = self->dam;
	return (PyObject*)result;
}

PyObject *PyDamagePacket_AddDice(PyObject *obj, PyObject *args) {
	auto self = (PyDamagePacket*)obj;


	DamageType damageType = DamageType::Unspecified;
	Dice dice;
	D20ActionType actionType = D20A_NONE;
	int damageMesLine = 103; // unknown
	const char* damDescription = nullptr;
	if (!PyArg_ParseTuple(args, "O&|iis:PyDamagePacket.add_dice", &ConvertDice, &dice, &damageType, &damageMesLine, &damDescription)) {
		return PyInt_FromLong(0);
	}


	auto result = self->dam->AddDamageDice(dice.ToPacked(), damageType, damageMesLine, damDescription );
	return PyInt_FromLong(result);
}



int PyDamagePacket_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	auto self = (PyDamagePacket*)obj;
	self->dam = nullptr;
	
	return 0;
}

PyObject *PyDamagePacket_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyDamagePacket, &PyDamagePacketType);
	return (PyObject*)self;
}

static PyMemberDef PyDamagePacket_Members[] = {
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef PyDamagePacket_Methods[] = {
	{ "clone", PyDamagePacket_Clone, METH_VARARGS, NULL },
	{ "add_dice", PyDamagePacket_AddDice, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PyDamagePacketType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyDamagePacket",                  /*tp_name*/
	sizeof(PyDamagePacket*),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyDamagePacket_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	PyDamagePacket_Methods,            /* tp_methods */
	PyDamagePacket_Members,            /* tp_members */
	0,                   	   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyDamagePacket_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyDamagePacket_New,                /* tp_new */
};

PyObject* PyDamagePacket_FromDamagePacket(const DamagePacket& dam) {
	auto self = PyObject_New(PyDamagePacket, &PyDamagePacketType);
	self->dam = (DamagePacket*)&dam;
	return (PyObject*)self;
}

bool ConvertDamagePacket(PyObject* obj, DamagePacket **pDamPkt) {
	if (obj->ob_type != &PyDamagePacketType) {
		PyErr_SetString(PyExc_TypeError, "Expected a PyDamagePacket object.");
		return false;
	}

	auto pyDamagePacket = (PyDamagePacket*)obj;
	*pDamPkt = pyDamagePacket->dam;
	return true;
}
