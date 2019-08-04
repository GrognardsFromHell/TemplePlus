
#include "stdafx.h"
#include "python_globalvars.h"
#include <temple/dll.h>

const int GlobalVarCount = 2000; // see script_init

static struct GlobalVarAddresses : temple::AddressTable {
	int **globalVars;

	GlobalVarAddresses() {
		rebase(globalVars, 0x103073B8);
	}
} addresses;

static PyObject *PyGlobalVars_GetItem(PyObject*, Py_ssize_t index) {
	if (index < 0 || index >= GlobalVarCount) {
		PyErr_Format(PyExc_IndexError, "Global variable index %d is out of range.", index);
		return 0;
	}
	return PyInt_FromLong((*addresses.globalVars)[index]);
}

static int PyGlobalVars_SetItem(PyObject *, Py_ssize_t index, PyObject *item) {
	if (index < 0 || index >= GlobalVarCount) {
		PyErr_Format(PyExc_IndexError, "Global variable index %d is out of range.", index);
		return -1;
	}
	if (! (PyInt_Check(item) || PyLong_Check(item)) ){
		PyErr_SetString(PyExc_ValueError, "Can only set integer values as global variable.");
		return -1;
	}

	auto value = PyInt_AsLong(item);
	(*addresses.globalVars)[index] = value;
	return 0;
}

static PySequenceMethods PyGlobalVarsSequence = {
	0,
	0,
	0,
	PyGlobalVars_GetItem,
	0,
	PyGlobalVars_SetItem,
	0,
};

static PyTypeObject PyGlobalVarsType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyGlobalVars", /*tp_name*/
	sizeof(PyObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	&PyGlobalVarsSequence, /*tp_as_sequence*/
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

PyObject* PyGlobalVars_Create() {
	if (PyType_Ready(&PyGlobalVarsType)) {
		return 0;
	}

	return PyObject_New(PyObject, &PyGlobalVarsType);
}
