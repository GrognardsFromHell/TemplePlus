
#pragma once

#include <obj.h>

#include <pybind11/pybind11.h>

extern PyTypeObject PyObjHandleType;

// Use with PyArg_ParseTuple and a O& placeholder
BOOL ConvertObjHndl(PyObject *obj, objHndl *pHandleOut);

PyObject *PyObjHndl_Create(objHndl handle);
PyObject *PyObjHndl_CreateNull();
objHndl PyObjHndl_AsObjHndl(PyObject *obj);
bool PyObjHndl_Check(PyObject *obj);

void init_objhndl_class(pybind11::module &module);
