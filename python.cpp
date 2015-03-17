
#include "stdafx.h"
#include "fixes.h"
#include "addresses.h"
#include "dependencies/python-2.2/Python.h"
#include "temple_functions.h""

/*
	Define all type objects used by ToEE.
*/
// ObjHandle
	static GlobalStruct<PyTypeObject, 0x102CF3B8> pyObjHandleType;
	static getattrfunc pyObjHandleTypeGetAttr; // Original getattr of pyObjHandleType
	static setattrfunc pyObjHandleTypeSetAttr; // Original setattr of pyObjHandleType
	static GlobalStruct<PyMethodDef, 0x102CE9A8> pyObjHandleMethods;
	//PyCFunction a[]; // this causes errors during compile!
	

struct ObjectId {
	uint16_t subtype;
	GUID guid;
};

struct TemplePyObjHandle : public PyObject {
	ObjectId objId;
	uint64_t objHandle;
};

PyObject* __cdecl  pyObjHandleType_getAttrNew(TemplePyObjHandle *obj, char *name) {
	LOG(info) << "Tried getting property: " << name;
	if (!strcmp(name, "co8rocks")) {
		return PyString_FromString("IT SURE DOES!");
	}

	if (!strcmpi(name, "ObjHandle")) {
		uint64_t objhnd = obj->objHandle; // this is ok
		PyObject* PyObj_ObjHnd = PyLong_FromLongLong(objhnd);
		PyObject* PyReturn = PyTuple_New(2);
		PyTuple_SetItem(PyReturn, 0, PyObj_ObjHnd);
		PyTuple_SetItem(PyReturn, 1, PyObj_ObjHnd);
		return  PyReturn; // it returns a PyTuple with wrong number! number look like memory addresses
	}



	if (!strcmpi(name, "substitute_inventory")) {
		ObjHndl ObjSubsInv = templeFuncs.Obj_Get_Substitute_Inventory(obj->objHandle);
		return templeFuncs.PyObj_From_ObjHnd(ObjSubsInv);
	}

	if (!strcmpi(name, "obj_get_field_64")) {
		
		return 0; 
	}

	return pyObjHandleTypeGetAttr(obj, name);
}


int __cdecl  pyObjHandleType_setAttrNew(TemplePyObjHandle *obj, char *name, TemplePyObjHandle *obj2) {
	LOG(info) << "Tried setting property: " << name;
	if (!strcmp(name, "co8rocks")) {
		return 0;
	}

	if (!strcmp(name, "substitute_inventory")) {


		if (obj2 != nullptr)  {
			if (obj->ob_type == obj2->ob_type){
				templeFuncs.Obj_Set_Field_ObjHnd(obj->objHandle, 365, obj2->objHandle);
			}
			
		}
		
		return 0;
	}


	return pyObjHandleTypeSetAttr(obj, name, obj2);
}

class PythonExtensions : public TempleFix {
public:
	const char* name() override {
		return "Python Script Extensions";
	}
	void apply() override;
} pythonExtension;

void PythonExtensions::apply() {

	// Hook the getattr function of obj handles
	pyObjHandleTypeGetAttr = pyObjHandleType->tp_getattr;
	pyObjHandleType->tp_getattr = (getattrfunc) pyObjHandleType_getAttrNew;

	pyObjHandleTypeSetAttr = pyObjHandleType->tp_setattr;
	pyObjHandleType->tp_setattr = (setattrfunc) pyObjHandleType_setAttrNew;


	//a[0] = pyObjHandleMethods->ml_meth;
	
}
