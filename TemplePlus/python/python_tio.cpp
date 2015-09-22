
#include "stdafx.h"
#include "python_tio.h"

struct PyTioFile {
	PyObject_HEAD;
	TioFile *file;
};

static inline TioFile *GetTioFile(PyObject *obj) {
	return ((PyTioFile*)obj)->file;
}

static PyObject *PyTioFile_Read(PyObject *obj, PyObject *countObj) {
	auto file = GetTioFile(obj);

	int size = PyInt_AsLong(countObj);

	if (size < 0) {
		PyErr_SetString(PyExc_ValueError, "Cannot read with negative size.");
		return 0;
	}

	vector<char> buffer(size);
	int readSize = tio_fread(buffer.data(), 1, buffer.size(), file);
	
	return PyString_FromStringAndSize(buffer.data(), readSize);
}

static PyObject *PyTioFile_Readline(PyObject *obj, PyObject *) {
	auto file = GetTioFile(obj);

	vector<char> result;
	char b;
	while (tio_fread(&b, 1, 1, file) == 1) {
		result.push_back(b);
		if (b == '\n') {
			break;
		}
	}

	return PyString_FromStringAndSize(result.data(), result.size());

}

static PyMethodDef PyTioFileMethods[] = {
	{ "read", PyTioFile_Read, METH_O, NULL },
	{ "readline", PyTioFile_Readline, METH_NOARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyTypeObject PyTioFileType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PyTioFile", /*tp_name*/
	sizeof(PyTioFile), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
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
	PyTioFileMethods , /* tp_methods */
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

PyObject* PyTioFile_FromTioFile(TioFile* file) {
	auto self = PyObject_NEW(PyTioFile, &PyTioFileType);
	self->file = file;
	return (PyObject*) self;
}

void PyTioFile_Invalidate(PyObject* pyTioFile) {
	((PyTioFile*)pyTioFile)->file = nullptr;
}
