
#include "stdafx.h"
#include "python_globalflags.h"
#include <util/addresses.h>
#include "../quest.h"

const int QuestCount = 3200; // see script_init

static struct QuestAddresses : AddressTable {
	int **globalFlags;

	bool Get(int flagIdx) {
		int idx = flagIdx / 32;
		uint32_t bit = flagIdx % 32;
		int flagWord = (*globalFlags)[idx];
		return (flagWord & (1 << bit)) != 0;
	}
	void Set(int flagIdx, bool value) {
		int idx = flagIdx / 32;
		int bit = flagIdx % 32;
		int &flagWord = (*globalFlags)[idx];
		uint32_t mask = (1 << bit);
		if (value) {
			flagWord |= mask;
		} else {
			flagWord &= ~mask;
		}
	}

	QuestAddresses() {
		rebase(globalFlags, 0x103073B8);
	}
} addresses;

struct PyQuest {
	PyObject_HEAD;
	int idx;
};

static PyObject *PyQuest_GetState(PyObject *obj, void *) {
	auto self = (PyQuest*)obj;
	return PyInt_FromLong((int) quests.GetState(self->idx));
}

static int PyQuest_SetState(PyObject *obj, PyObject *value, void *) {
	auto self = (PyQuest*)obj;

	if (!PyInt_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "Quest state must be an integer.");
		return -1;
	}

	auto state = (QuestState) PyInt_AsLong(value);
	quests.SetState(self->idx, state);
	return 0;
}

static PyGetSetDef PyQuestGetSet[] = {
	{ "state", PyQuest_GetState, PyQuest_SetState, NULL }
};

static PyObject *PyQuest_Unbotch(PyObject *obj, PyObject *args) {
	auto self = (PyQuest*)obj;
	auto newState = quests.Unbotch(self->idx);
	return PyInt_FromLong((int)newState);
}

static PyMethodDef PyQuestMethods[] = {
	{ "unbotch", PyQuest_Unbotch, METH_VARARGS, NULL }
};

static PyTypeObject PyQuestType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyQuest", /*tp_name*/
	sizeof(PyQuest), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyObject_Del, /*tp_dealloc*/
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
	PyQuestMethods, /* tp_methods */
	0, /* tp_members */
	PyQuestGetSet, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

PyObject *PyQuest_Create(int idx) {
	auto self = PyObject_NEW(PyQuest, &PyQuestType);
	self->idx = idx;
	return (PyObject*) self;
}

static PyObject *PyQuests_GetItem(PyObject*, Py_ssize_t index) {
	if (index < 0 || index >= QuestCount) {
		PyErr_Format(PyExc_IndexError, "Quest index %d is out of range.", index);
		return 0;
	}
	return PyQuest_Create(index);
}

static PySequenceMethods PyQuestsSequence = {
	0,
	0,
	0,
	PyQuests_GetItem,
	0,
	0,
	0,
};

static PyTypeObject PyQuestsType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyQuests", /*tp_name*/
	sizeof(PyObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	&PyQuestsSequence, /*tp_as_sequence*/
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

PyObject* PyQuests_Create() {
	if (PyType_Ready(&PyQuestsType)) {
		return 0;
	}

	return PyObject_New(PyObject, &PyQuestsType);
}
