
#pragma once

#define PY_INT_GETTER_PTR(expr) ((getter) [] (PyObject*,void*) { return PyInt_FromLong(*(expr)); })
#define PY_INT_SETTER_PTR(expr) ((setter) [] (PyObject*,PyObject*val,void*) { *(expr) = PyInt_AS_LONG(val); return 0; })

#define PY_INT_GETTER(method) ((getter) [] (PyObject*,void*) { return PyInt_FromLong(method()); })
#define PY_INT_SETTER(method) ((setter) [] (PyObject*,PyObject*val,void*) { method(PyInt_AS_LONG(val)); return 0; })

#define PY_STR_GETTER(method) ((getter) [] (PyObject*,void*) { return PyString_FromString(method().c_str()); })
#define PY_STR_SETTER(method) ((setter) [] (PyObject*,PyObject*val,void*) { method(PyString_AsString(val)); return 0; })

#define PY_INT_PROP_PTR(name, ptrExpr, doc) { name, PY_INT_GETTER_PTR((ptrExpr)), PY_INT_SETTER_PTR(ptrExpr), doc, NULL }
// Read only integer property using a pointer
#define PY_INT_PROP_PTR_RO(name, ptrExpr, doc) { name, PY_INT_GETTER_PTR((ptrExpr)), NULL, doc, NULL }
#define PY_INT_PROP(name, getMeth, setMeth, doc) { name, PY_INT_GETTER(getMeth), PY_INT_SETTER(setMeth), doc, NULL }
// Read only integer property using a getter
#define PY_INT_PROP_RO(name, getMeth, doc) { name, PY_INT_GETTER(getMeth), NULL, doc, NULL }

#define PY_STR_PROP(name, getMeth, setMeth, doc) { name, PY_STR_GETTER(getMeth), PY_STR_SETTER(setMeth), doc, NULL }
// Read only str property using a getter
#define PY_STR_PROP_RO(name, getMeth, doc) { name, PY_STR_GETTER(getMeth), NULL, doc, NULL }

inline bool GetFloatLenient(PyObject *value, float &valueOut) {
	if (PyInt_Check(value)) {
		valueOut = (float)PyInt_AsLong(value);
		return true;
	}
	if (PyFloat_Check(value)) {
		valueOut = (float)PyFloat_AsDouble(value);
		return true;
	}
	PyErr_SetString(PyExc_TypeError, "property can only be set to a floating point number or integer.");
	return false;
}

inline bool GetInt(PyObject *value, int &valueOut) {
	if (PyInt_Check(value)) {
		valueOut = PyInt_AsLong(value);
		return true;
	}
	PyErr_SetString(PyExc_TypeError, "property can only be set to an integer.");
	return false;
}

