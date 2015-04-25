
#include "stdafx.h"
#include "python_counters.h"
#include "python_integration.h"
#include <util/addresses.h>

struct PyCounters {
	PyObject_HEAD;
	objHndl handle;
	ScriptEvent evt;
};

static PyObject *PyCounters_GetItem(PyObject *obj, Py_ssize_t index) {
	auto self = (PyCounters*)obj;

	auto attachment = objects.GetScriptAttachment(self->handle, (int)self->evt);
	int val;
	if (attachment.scriptId) {
		val = (attachment.counters >> (index * 8)) & 0xFF;
	} else {
		val = 0;
	}
	return PyInt_FromLong(val);
}

static int PyCounters_SetItem(PyObject *obj, Py_ssize_t index, PyObject *item) {
	auto self = (PyCounters*) obj;
	auto attachment = objects.GetScriptAttachment(self->handle, (int)self->evt);
	auto value = PyInt_AsLong(item);
	if (attachment.scriptId) {
		// Clear the current value
		int mask = 0xFF << (index * 8);
		attachment.counters &= ~mask;
		attachment.counters |= (value & 0xFF) << (index * 8);
		objects.SetScriptAttachment(self->handle, index, attachment);
	}
	return 0;
}

static PySequenceMethods PyCountersSequence = {
	0,
	0,
	0,
	PyCounters_GetItem,
	0,
	PyCounters_SetItem,
	0,
};

static PyTypeObject PyCountersType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyCounters", /*tp_name*/
	sizeof(PyCounters), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	&PyCountersSequence, /*tp_as_sequence*/
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

PyObject* PyCounters_Create() {
	if (PyType_Ready(&PyCountersType)) {
		return 0;
	}

	auto result = PyObject_New(PyCounters, &PyCountersType);
	result->handle = pythonIntegration.GetCounterObj();
	result->evt = pythonIntegration.GetCounterEvent();
	return (PyObject*) result;
}
