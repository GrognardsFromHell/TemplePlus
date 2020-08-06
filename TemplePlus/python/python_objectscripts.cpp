#include "stdafx.h"
#include "python_objectscripts.h"
#include "python_support.h"

struct PyObjScripts {
	PyObject_HEAD;
	objHndl handle;
};

PyObject* PyObjScripts_GetItem(PyObject* obj, Py_ssize_t idx) {
	auto self = (PyObjScripts*)obj;
	auto scriptId = objects.GetScript(self->handle, idx);
	return PyInt_FromLong(scriptId);
}

int PyObjScripts_SetItem(PyObject* obj, Py_ssize_t idx, PyObject* value) {
	auto self = (PyObjScripts*)obj;
	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Script ids need to be integers.");
		return -1;
	}	
	auto scriptId = PyInt_AsLong(value);
	objects.SetScript(self->handle, idx, scriptId);
	return 0;
}

PySequenceMethods PyObjScriptsSequence = {
	0,
	0,
	0,
	PyObjScripts_GetItem,
	0,
	PyObjScripts_SetItem,
	0,
	0,
	0,
	0
};

static PyObject* PyObjScripts_Counter_Get(PyObject* obj, PyObject* args) {
	auto self = (PyObjScripts*)obj;
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int counter_index, evt;
	if (!PyArg_ParseTuple(args, "ii:PyObjScripts.counter_get", &evt, &counter_index)) {
		return 0;
	}
	auto attachment = objects.GetScriptAttachment(self->handle, evt);
	int val;
	if (attachment.scriptId) {
		val = (attachment.counters >> (counter_index * 8)) & 0xFF;
	}
	else {
		val = 0;
	}
	return PyInt_FromLong(val);
}

static PyObject* PyObjScripts_Counter_Set(PyObject* obj, PyObject* args) {
	auto self = (PyObjScripts*)obj;
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int counter_index, evt, value;
	if (!PyArg_ParseTuple(args, "iii:PyObjScripts.counter_set", &evt, &counter_index, &value)) {
		return 0;
	}
	auto attachment = objects.GetScriptAttachment(self->handle, evt);
	if (attachment.scriptId) {
		// Clear the current value
		int mask = 0xFF << (counter_index * 8);
		attachment.counters &= ~mask;
		attachment.counters |= (value & 0xFF) << (counter_index * 8);
		objects.SetScriptAttachment(self->handle, evt, attachment);
	}
	Py_RETURN_NONE;
}

static PyMethodDef PyObjScriptsMethods[] = {
	{ "counter_get", PyObjScripts_Counter_Get, METH_VARARGS, NULL },
	{ "counter_set", PyObjScripts_Counter_Set, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};


PyTypeObject PyObjScriptsType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyObjScripts", /*tp_name*/
	sizeof(PyObjScripts), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	&PyObjScriptsSequence, /*tp_as_sequence*/
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
	PyObjScriptsMethods, /* tp_methods */
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

PyObject* PyObjScripts_Create(objHndl handle) {
	auto self = PyObject_New(PyObjScripts, &PyObjScriptsType);
	self->handle = handle;
	return (PyObject*)self;
}
