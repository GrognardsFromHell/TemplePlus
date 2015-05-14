
#include "stdafx.h"
#include "Python.h"
#include "osdefs.h"
#include "marshal.h"
#include "structmember.h"
#include "tio/tio.h"
#include "python_integration_obj.h"
#include <util/addresses.h>
#include <util/stringutil.h>

struct pytcout {
	PyObject_HEAD;
	int softspace;
};

PyObject *pytcout_write(PyObject *self, PyObject *args) {
	char *message;
	int messageLen;

	if (!PyArg_ParseTuple(args, "s#:PyTempleConsoleOut.write", &message, &messageLen))
		return NULL;

	static bool inConsoleAppend = false;

	if (!inConsoleAppend) {
		inConsoleAppend = true;
		auto res = pythonObjIntegration.ExecuteScript("ui.console", "append", args);
		if (!res) {
			PyErr_Print();
		}
		Py_XDECREF(res);
		inConsoleAppend = false;
	}
		
	// Dont append a new line for the logger
	string msg = message;
	msg = rtrim(msg);
	if (trim(msg).empty()) {
		Py_RETURN_NONE; // no empty lines
	}
	logger->info("Python: {}", msg);

	Py_RETURN_NONE;
}

PyObject *pytcout_flush(PyObject *self, PyObject *args) {
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "write", pytcout_write, METH_VARARGS, 0 },
	{ "flush", pytcout_flush, METH_VARARGS, 0 },
	{ NULL, NULL }   /* sentinel */
};

static PyMemberDef members[] = {
	{ "softspace", T_INT, offsetof(pytcout, softspace), 0,
	"flag indicating that a space needs to be printed; used by print" },
	{ NULL } /* Sentinel */
};

static PyTypeObject PyTempleConsoleOutType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"PyTempleConsoleOut",
	sizeof(pytcout),
	0,                                          /* tp_itemsize */
	(destructor)PyObject_Del,					/* tp_dealloc */
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
	PyObject_GenericSetAttr,                    /* tp_setattro */
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
	members,
	0
};


PyObject *PyTempleConsoleOut_New() {
	return PyObject_New(PyObject, &PyTempleConsoleOutType);
}
