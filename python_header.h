/*
Define all type objects used by ToEE.
*/
// ObjHandle
static GlobalStruct<PyTypeObject, 0x102CF3B8> pyObjHandleType;
static getattrfunc pyObjHandleTypeGetAttr; // Original getattr of pyObjHandleType
static setattrfunc pyObjHandleTypeSetAttr; // Original setattr of pyObjHandleType
static GlobalStruct<PyMethodDef, 0x102CE9A8> pyObjHandleMethods;

static GlobalStruct<PyTypeObject, 0x102CFBC0> pySpellType;
static getattrfunc pySpellTypeGetAttr; // Original getattr of pySpellType

//PyCFunction a[]; // this causes errors during compile!


struct ObjectId {
	uint16_t subtype;
	GUID guid;
};

struct LocXY{
	uint32_t locx;
	uint32_t locy;
};

struct Loc_And_Offsets{
	LocXY location;
	float off_x;
	float off_y;
};

struct LocFull {
	Loc_And_Offsets location;
	float off_z;
};

struct TemplePyObjHandle : public PyObject {
	ObjectId objId;
	uint64_t objHandle;
};



struct PySpell : public PyObject {
	uint32_t nSpellEnum;
	uint32_t nSpellEnum_Original;
	uint32_t nID;
	uint32_t field_14;
	ObjHndl ObjHnd_Caster;
	uint32_t nCasterPartsysID;
	uint32_t nCasterClass;
	uint32_t nCasterClass_DomainSpell;
	uint32_t field_2C;
	LocFull target_location_full;
	uint32_t nDC;
	uint32_t nSpellLevel;
	uint32_t nCasterLevel;
	uint32_t nRangeExact;
	uint32_t nDuration;
	uint32_t field_58;
	PyObject* pSpellVariables;
	uint32_t field_60;
	uint32_t bNumOfTargets;
	uint32_t nNumOfProjectiles;
	//not a complete description yet, full size is 0x2A0
};