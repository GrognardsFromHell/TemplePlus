
#include "stdafx.h"
#include "python_bonus.h"
#include <structmember.h>
#include "../bonus.h"
#include "../common.h"
#include "../feat.h"


struct PyBonusList {
	PyObject_HEAD;
	BonusList *bonlist;
};

PyObject *PyBonusList_Repr(PyObject *obj) {
	auto self = (PyBonusList*)obj;
	string text;
	
	text = fmt::format("BonustList[{}]", self->bonlist->bonCount);
		
	return PyString_FromString(text.c_str());
}

PyObject *PyBonusList_Clone(PyObject *obj, PyObject *args) {
	auto self = (PyBonusList*)obj;
	auto result = PyObject_New(PyBonusList, obj->ob_type);
	result->bonlist = self->bonlist;
	return (PyObject*)result;
}

PyObject *PyBonusList_GetSum(PyObject *obj, PyObject *args) {
	auto self = (PyBonusList*)obj;
	auto result = self->bonlist->GetEffectiveBonusSum();
	return PyInt_FromLong(result);
}

PyObject *PyBonusList_AddBonus(PyObject *obj, PyObject *args) {
	auto self = (PyBonusList*)obj;


	int value, bonType = 0, bonMesLine = 101; // bonus type 0 is generic and stacking; 101 is "Misc."
	if (!PyArg_ParseTuple(args, "i|ii:PyBonusList_AddBonus", &value, &bonType, &bonMesLine)) {
		return PyInt_FromLong(0);
	}

	auto result = self->bonlist->AddBonus(value, bonType, bonMesLine);
	return PyInt_FromLong(result);
}

PyObject *PyBonusList_AddCap(PyObject *obj, PyObject *args) {
	auto self = (PyBonusList*)obj;


	int value, capType = 0, bonMesLine = 101; // type 0 is generic; line 101 is for "Misc."
	if (!PyArg_ParseTuple(args, "i|ii:PyBonusList_AddCap", &value, &capType, &bonMesLine)) {
		return PyInt_FromLong(0);
	}

	auto result = self->bonlist->AddCap(capType, value, bonMesLine);
	return PyInt_FromLong(result);
}

PyObject *PyBonusList_AddFromFeat(PyObject *obj, PyObject *args) {
	auto self = (PyBonusList*)obj;

	// todo support for new feats
	int value, bonType = 0, bonMesLine = 101, featEnum =0; // bonus type 0 is generic and stacking; 101 is "Misc."
	if (!PyArg_ParseTuple(args, "i|iii:PyBonusList.add_from_feat", &value, &bonType, &bonMesLine, &featEnum)) {
		return PyInt_FromLong(0);
	}

	if (featEnum < 0)
		return PyInt_FromLong(0);

	if (featEnum >= NUM_FEATS)
		return PyInt_FromLong(0);

	auto result = self->bonlist->AddBonusFromFeat(value, bonType, bonMesLine, (feat_enums)featEnum);
	return PyInt_FromLong(result);
}

int PyBonusList_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	auto self = (PyBonusList*)obj;
	self->bonlist = nullptr;
	
	return 0;
}

PyObject *PyBonusList_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyBonusList, &PyBonusListType);
	return (PyObject*)self;
}

static PyMemberDef PyBonusList_Members[] = {
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef PyBonusList_Methods[] = {
	{ "clone", PyBonusList_Clone, METH_VARARGS, NULL },
	{ "get_sum", PyBonusList_GetSum, METH_VARARGS, NULL },
	{ "add", PyBonusList_AddBonus, METH_VARARGS, NULL },
	{ "add_cap", PyBonusList_AddCap, METH_VARARGS, NULL },
	{ "add_from_feat", PyBonusList_AddFromFeat, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PyBonusListType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyBonusList",                  /*tp_name*/
	sizeof(PyBonusList*),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyBonusList_Repr,               /*tp_repr*/
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
	PyBonusList_Methods,            /* tp_methods */
	PyBonusList_Members,            /* tp_members */
	0,                   	   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyBonusList_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyBonusList_New,                /* tp_new */
};

PyObject* PyBonusList_FromBonusList(const BonusList& bonlist ){
	auto self = PyObject_New(PyBonusList, &PyBonusListType);
	self->bonlist = (BonusList*)&bonlist;
	return (PyObject*)self;
}

bool ConvertBonusList(PyObject* obj, BonusList **pDiceOut) {
	if (obj->ob_type != &PyBonusListType) {
		PyErr_SetString(PyExc_TypeError, "Expected a PyBonusList object.");
		return false;
	}

	auto pyBonusList = (PyBonusList*)obj;
	*pDiceOut = pyBonusList->bonlist;
	return true;
}
