
#include "stdafx.h"
#include "Python.h"
#include "osdefs.h"
#include "marshal.h"
#include "tio/tio.h"

PyObject *pytcout_write(PyObject *self, PyObject *args) {
	char *message;

	if (!PyArg_ParseTuple(args, "s:PyTempleConsoleOut.write", &message))
		return NULL;

	logger->info("Python: {}", message);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef methods[] = {
	{ "write", pytcout_write, METH_VARARGS, 0 },
	{ NULL, NULL }   /* sentinel */
};

static PyTypeObject PyTempleConsoleOutType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"PyTempleConsoleOut",
	sizeof(PyObject),
	0,                                          /* tp_itemsize */
	(destructor)PyObject_Free,					/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_compare */
	0,											/* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	PyObject_GenericGetAttr,                    /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,							/* tp_flags */
	0,											/* tp_doc */
	0,											/* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	methods,                        /* tp_methods */
	0,
};


PyObject *PyTempleConsoleOut_New() {
	return PyObject_New(PyObject, &PyTempleConsoleOutType);
}
