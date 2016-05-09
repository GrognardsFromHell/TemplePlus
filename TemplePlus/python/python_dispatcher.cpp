
#include "stdafx.h"
#include "python_bonus.h"
#include <structmember.h>
#include "../damage.h"
#include "../common.h"
#include "../feat.h"
#include "python_dice.h"
#include "../dice.h"
#include "../dispatcher.h"
#include "../common.h"
#include <infrastructure/elfhash.h>
#include "python_object.h"
#include "python_dispatcher.h"

#undef HAVE_ROUND
#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j){
	return i + j;
}

PYBIND11_PLUGIN(tp_dispatcher){
	py::module m("tp_dispatcher", "Temple+ Dispatcher module, used for creating modifier extensions.");

	m.def("add", &add);
	
	return m.ptr();
}



struct PyModifier {
	PyObject_HEAD;
	CondNode *cond;
};

struct PyModifierSpec{
	PyObject_HEAD;
	CondStructNew condSpec;
	int condId;
	char condName[512];
};

struct PyDispatchEventObject {
	PyObject_HEAD;
	DispIO *evtObj;
};

struct PyDispatchEventObjectD20Signal : PyDispatchEventObject {

};

struct PyEventArgs {
	PyObject_HEAD;
	DispatcherCallbackArgs args;
};



PyObject* PyEventArgs_Create(DispatcherCallbackArgs args) {
	auto result = PyObject_New(PyEventArgs, &PyEventArgsType);
	result->args= args;
	return (PyObject*)result;
}

PyObject* PyDispatchEventObject_Create(DispIO* dispIo) {
	auto result = PyObject_New(PyDispatchEventObject, &PyDispatchEventObjectType);
	result->evtObj = dispIo;
	return (PyObject*)result;
}



/******************************************************************************
* PyModifierSpec
******************************************************************************/
#pragma region PyModifierSpec


PyObject *PyModifierSpec_Repr(PyObject *obj) {
	auto self = (PyModifierSpec*)obj;
	string text;

	text = format("Modifier Spec: {}", self->condSpec.condName);

	return PyString_FromString(text.c_str());
}



int PyModifierSpec_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	auto self = (PyModifierSpec*)obj;
	CondStructNew condSpecNew;
	self->condSpec = condSpecNew;
	self->condId = 0;
	memset(self->condName, 0, sizeof self->condName);
	return 0;
}

PyObject *PyModifierSpec_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyModifierSpec, &PyModifierSpecType);
	return (PyObject*)self;
}


int PyModHookWrapper(DispatcherCallbackArgs args){
	auto callback = (PyObject*)args.GetData1();

	
	auto dispPyArgs = PyTuple_New(2);
	PyTuple_SET_ITEM(dispPyArgs, 0, PyObjHndl_Create(args.objHndCaller));
	PyTuple_SET_ITEM(dispPyArgs, 1, PyEventArgs_Create(args));

	//PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(combatant));
	auto resultObj = PyObject_CallObject(callback, dispPyArgs);

	return 0;
}




PyObject *PyModifierSpec_AddHook(PyObject *obj, PyObject *args){
	auto self = (PyModifierSpec*)obj;

	enum_disp_type dispType = dispType0;
	D20DispatcherKey dispKey = DK_NONE;
	PyObject *callback = nullptr;

	if (!PyArg_ParseTuple(args, "iiO:PyModifierSpec.__add_hook__", &dispType, &dispKey, &callback)) {
		return PyInt_FromLong(0);
	}

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("PyModifierSpec {} attached with a bad hook.", self->condSpec.condName);
		return PyInt_FromLong(0);
	}


	self->condSpec.AddHook(dispType, dispKey, PyModHookWrapper, (uint32_t)callback, 0);
	return PyInt_FromLong(1);
};


PyObject *PyModifierSpec_RegisterMod(PyObject *obj, PyObject *args) {
	auto self = (PyModifierSpec*)obj;

	const char *name;
	int numArgs = 0, preventDup = 1;

	if (!PyArg_ParseTuple(args, "sii:PyModifierSpec.__register_mod__", &name, &numArgs, &preventDup)) {
		return PyInt_FromLong(0);
	}

	if (!name)
		return PyInt_FromLong(0);

	auto strSize = strlen(name);
	if (strSize > 512)
		strSize = 511;

	self->condSpec.numArgs = numArgs;
	self->condSpec.condName = self->condName;
	memcpy(self->condName, name, strSize);
	self->condId = ElfHash::Hash(name);

	self->condSpec.Register();
	return PyInt_FromLong(1);
};


PyObject *PyModifierSpec_AddHookPreventDup(PyObject *obj, PyObject *args) {
	// todo
	return PyInt_FromLong(1);
};

static PyMemberDef PyModifierSpec_Members[] = {
	{ "num_args", T_INT, offsetof(PyModifierSpec, condSpec.numArgs), 0, NULL },
	{ "num_hooks", T_INT, offsetof(PyModifierSpec, condSpec.numHooks), 0, NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef PyModifierSpec_Methods[] = {
	{ "__add_hook__", PyModifierSpec_AddHook, METH_VARARGS, NULL },
	{ "__add_hook_prevent_dup__", PyModifierSpec_AddHookPreventDup, METH_VARARGS, NULL },
	{ "__register_mod__", PyModifierSpec_RegisterMod, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PyModifierSpecType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyModifierSpec",                  /*tp_name*/
	sizeof(CondStructNew)+sizeof(int),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyModifierSpec_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	PyModifierSpec_Methods,            /* tp_methods */
	PyModifierSpec_Members,            /* tp_members */
	0,                   	   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyModifierSpec_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyModifierSpec_New,                /* tp_new */
};




PyObject* PyModifierSpec_FromCondStructNew(const CondStructNew& condSpec) {
	auto self = PyObject_New(PyModifierSpec, &PyModifierSpecType);
	self->condSpec = condSpec;
	self->condId = ElfHash::Hash(condSpec.condName);
	return (PyObject*)self;
}

bool ConvertDamagePacket(PyObject* obj, CondStructNew **pCondStructNew) {
	if (obj->ob_type != &PyModifierSpecType) {
		PyErr_SetString(PyExc_TypeError, "Expected a PyModifierSpec object.");
		return false;
	}

	auto pyMod = (PyModifierSpec*)obj;
	*pCondStructNew = &pyMod->condSpec;
	return true;
}

#pragma endregion




/******************************************************************************
* PyEventArgs
******************************************************************************/

#pragma region PyEventArgs

PyObject *PyEventArgs_Repr(PyObject *obj) {
	auto self = (PyEventArgs*)obj;
	string text;
	text = format("Event Args [{}][{}]", self->args.dispType, self->args.dispKey);
	return PyString_FromString(text.c_str());
}

int PyEventArgs_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	auto self = (PyEventArgs*)obj;
	memset(&self->args, 0, sizeof self->args);
	return 0;
}

PyObject *PyEventArgs_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyEventArgs, &PyEventArgsType);
	memset(&self->args, 0, sizeof self->args);
	return (PyObject*)self;
}



PyObject* PyEventArgs_GetEventObject(PyObject *obj, void *) {
	auto self = (PyEventArgs*)obj;
	switch (self->args.dispKey) {
		
	}
	return PyDispatchEventObject_Create(self->args.dispIO);
}

PyObject* PyEventArgs_GetAttachee(PyObject *obj, void *) {
	auto self = (PyEventArgs*)obj;
	switch (self->args.dispKey) {

	}
	return PyObjHndl_Create(self->args.objHndCaller);
}

static PyGetSetDef PyEventArgsGetSet[] = {
	{ "event_obj", PyEventArgs_GetEventObject, NULL, NULL },
	{ "attachee", PyEventArgs_GetAttachee, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyMethodDef PyEventArgs_Methods[] = {
	//{ "__add_hook__", PyModifierSpec_AddHook, METH_VARARGS, NULL },
	//{ "__add_hook_prevent_dup__", PyModifierSpec_AddHookPreventDup, METH_VARARGS, NULL },
	//{ "__register_mod__", PyModifierSpec_RegisterMod, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PyEventArgsType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyEventArgs",                  /*tp_name*/
	sizeof(CondStructNew) + sizeof(int),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyEventArgs_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	PyEventArgs_Methods,            /* tp_methods */
	0,						   /* tp_members */
	PyEventArgsGetSet,     	   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyEventArgs_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyEventArgs_New,                /* tp_new */
};


#pragma endregion

/******************************************************************************
* PyDispatchEventObject
******************************************************************************/

#pragma region PyDispatchEventObject

PyObject *PyDispatchEventObject_Repr(PyObject *obj) {
	auto self = (PyDispatchEventObject*)obj;
	string text;
	text = format("EventObject [{}]", self->evtObj->dispIOType);
	return PyString_FromString(text.c_str());
}

int PyDispatchEventObject_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	// not sure how relevant this is
	return 0;
}

PyObject *PyDispatchEventObject_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyDispatchEventObject, &PyDispatchEventObjectType);
	// not sure how relevant this is
	return (PyObject*)self;
}


//
//static PyGetSetDef PyDispatchEventObject_GetSet[] = {
//	{ "event_obj", PyEventArgs_GetEventObject, NULL, NULL },
//	{ NULL, NULL, NULL, NULL }
//};

//static PyMethodDef PyEventArgs_Methods[] = {
//	{ NULL, NULL, NULL, NULL }
//};

PyTypeObject PyDispatchEventObjectType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyDispatchEventObject",                  /*tp_name*/
	sizeof(PyDispatchEventObject),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyDispatchEventObject_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	0,            /* tp_methods */
	0,						   /* tp_members */
	0,     					   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyDispatchEventObject_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyDispatchEventObject_New,                /* tp_new */
};


#pragma endregion
