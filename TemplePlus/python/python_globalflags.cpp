
#include "stdafx.h"
#include "python_globalflags.h"
#include <temple/dll.h>

#include "gamesystems/gamesystems.h"
#include "gamesystems/scripting.h"

const int GlobalFlagCount = 3200; // see script_init

static PyObject *PyGlobalFlags_GetItem(PyObject*, Py_ssize_t index) {
	if (index < 0 || index >= GlobalFlagCount) {
		PyErr_Format(PyExc_IndexError, "Global flag index %d is out of range.", index);
		return 0;
	}
	return PyInt_FromLong(gameSystems->GetScript().GetGlobalFlag(index) ? 1 : 0);
}

static int PyGlobalFlags_SetItem(PyObject *, Py_ssize_t index, PyObject *item) {
	if (index < 0 || index >= GlobalFlagCount) {
		PyErr_Format(PyExc_IndexError, "Global flag index %d is out of range.", index);
		return -1;
	}
	auto val = PyObject_IsTrue(item) != 0;
	gameSystems->GetScript().SetGlobalFlag(index, val);
	return 0;
}

static PySequenceMethods PyGlobalFlagsSequence = {
	0,
	0,
	0,
	PyGlobalFlags_GetItem,
	0,
	PyGlobalFlags_SetItem,
	0,
};

static PyTypeObject PyGlobalFlagsType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyGlobalFlags", /*tp_name*/
	sizeof(PyObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	&PyGlobalFlagsSequence, /*tp_as_sequence*/
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

PyObject* PyGlobalFlags_Create() {
	if (PyType_Ready(&PyGlobalFlagsType)) {
		return 0;
	}

	return PyObject_New(PyObject, &PyGlobalFlagsType);
}
