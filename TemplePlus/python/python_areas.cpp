
#include "stdafx.h"
#include "python_globalvars.h"
#include <temple/dll.h>
#include <obj.h>
#include "../party.h"

static struct AreaAddresses : temple::AddressTable {
	void(__cdecl *AddKnownArea)(objHndl pc, int araId);
	bool(__cdecl *IsAreaKnown)(objHndl pc, int araId);

	AreaAddresses() {

		rebase(AddKnownArea, 0x1006EA00);
		rebase(IsAreaKnown, 0x1006E9B0);
	}
} addresses;

static PyObject *PyAreas_GetItem(PyObject*, Py_ssize_t index) {
	auto leader = party.GetLeader();
	return PyInt_FromLong(addresses.IsAreaKnown(leader, index));
}

static int PyAreas_SetItem(PyObject *, Py_ssize_t index, PyObject *item) {	
	if (PyObject_IsTrue(item)) {
		auto leader = party.GetLeader();
		addresses.AddKnownArea(leader, index);
	}
	return 0;
}

static PySequenceMethods PyAreasSequence = {
	0,
	0,
	0,
	PyAreas_GetItem,
	0,
	PyAreas_SetItem,
	0,
};

static PyTypeObject PyAreasType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyAreas", /*tp_name*/
	sizeof(PyObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	&PyAreasSequence, /*tp_as_sequence*/
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

PyObject* PyAreas_Create() {
	if (PyType_Ready(&PyAreasType)) {
		return 0;
	}

	return PyObject_New(PyObject, &PyAreasType);
}
