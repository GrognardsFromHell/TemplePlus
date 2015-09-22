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

PyObject* PyObjScripts_Create(objHndl handle) {
	auto self = PyObject_New(PyObjScripts, &PyObjScriptsType);
	self->handle = handle;
	return (PyObject*)self;
}
