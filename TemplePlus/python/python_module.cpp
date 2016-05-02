
#include "stdafx.h"
#include "python_module.h"
#include "python_game.h"
#include "python_object.h"
#include "python_dice.h"
#include "python_bonus.h"

static PyObject *Anyone(PyObject *obj, PyObject *args) {
	PyObject *targetObjs;
	PyObject *methodName;
	PyObject *methodArg;
	if (!PyArg_ParseTuple(args, "O!SO", &PyTuple_Type, &targetObjs, &methodName, &methodArg)) {
		return nullptr;
	}

	auto size = PyTuple_Size(targetObjs);
	for (ssize_t i = 0; i < size; ++i) {
		auto targetObj = PyTuple_GET_ITEM(targetObjs, i);
		auto result = PyObject_CallMethodObjArgs(targetObj, methodName, methodArg, NULL);
		if (!result) {
			PyErr_Print();
			auto methodNameStr = PyString_AsString(methodName);
			auto targetObjStrObj = PyObject_Str(targetObj);
			auto targetObjStr = PyString_AsString(targetObjStrObj);
			logger->error("Calling function {} for obj {} failed in anyone.", methodNameStr, targetObjStr);
			Py_DECREF(targetObjStrObj);
		} else {
			if (PyObject_IsTrue(result)) {
				return result;
			}
			Py_DECREF(result);
		}
	}

	Py_RETURN_FALSE;
}

static PyMethodDef pyToeeMethods[] = {
	{ "anyone", Anyone, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

void PyToeeInitModule() {

	auto module = Py_InitModule("toee", pyToeeMethods); // Borrowed ref
	auto dict = PyModule_GetDict(module); // Borrowed ref

	// The null object (pointless by the way, since it implements IsTrue)
	auto nullHandle = PyObjHndl_CreateNull();
	PyDict_SetItemString(dict, "OBJ_HANDLE_NULL", nullHandle);
	Py_DECREF(nullHandle);
	
	// The game object, which behaves more like a module, but has getter/setter based properties
	auto pyGame = PyGame_Create(); // New ref
	PyDict_SetItemString(dict, "game", pyGame);
	Py_DECREF(pyGame);

	if (PyType_Ready(&PyDiceType)) {
		PyErr_Print();
	}
	if (PyType_Ready(&PyObjHandleType)) {
		PyErr_Print();
	}
	if (PyType_Ready(&PyBonusListType)) {
		PyErr_Print();
	}

	// This is critical for unpickling object handles stored in timed events
	PyDict_SetItemString(dict, "PyObjHandle", (PyObject*) &PyObjHandleType);
	PyDict_SetItemString(dict, "dice_new", (PyObject*) &PyDiceType);
	PyDict_SetItemString(dict, "PyBonusList", (PyObject*)&PyBonusListType);

	// Copy all constants into toee for legacy support
	auto constantsMod = PyImport_ImportModule("templeplus.constants"); // New ref
	if (!constantsMod) {
		throw TempleException("TemplePlus Python module templeplus.constants is missing.");
	}
	PyDict_Merge(dict, PyModule_GetDict(constantsMod), 0);
	Py_DECREF(constantsMod);
	
}
