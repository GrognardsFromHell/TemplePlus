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