
#pragma once

#include "addresses.h"
#include "obj.h"

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

static GlobalPrimitive<PyListObject*, 0x10BCABD0>* pPySpellList;

struct TemplePyObjHandle : public PyObject {
	ObjectId objId;
	uint64_t objHandle;
};



struct objHndl_Plus_PartsysID{
	objHndl objHnd;
	uint32_t nPartsysID;
};

struct PySpell : public PyObject {
	uint32_t nSpellEnum;
	uint32_t nSpellEnum_Original;
	uint32_t nID;
	uint32_t field_14;
	objHndl ObjHnd_Caster;
	uint32_t nCasterPartsysID;
	uint32_t nCasterClass;
	uint32_t nCasterClass_Alt; // used for spells cast from items, and maybe domain spells too
	uint32_t field_2C;
	LocFull target_location_full;
	uint32_t nDC;
	uint32_t nSpellLevel;
	uint32_t nCasterLevel;
	uint32_t nRangeExact;
	uint32_t nDuration;
	uint32_t field_58;
	PyObject* pSpellVariables;
	uint32_t nTargetListNumItems_Copy;
	uint32_t nNumOfTargets;
	uint32_t nNumOfProjectiles;
	uint32_t field_6C;
	objHndl_Plus_PartsysID TargetList[32];

	//not a complete description yet, full size is 0x2A0
};