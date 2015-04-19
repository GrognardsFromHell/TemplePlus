
#pragma once

#include <obj.h>

extern PyTypeObject PyObjHandleType;

// Use with PyArg_ParseTuple and a O& placeholder
bool ConvertObjHndl(PyObject *obj, objHndl *pHandleOut);

PyObject *PyObjHndl_Create(objHndl handle);
PyObject *PyObjHndl_CreateNull();
objHndl PyObjHndl_AsObjHndl(PyObject *obj);
bool PyObjHndl_Check(PyObject *obj);
