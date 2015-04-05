
#include "stdafx.h"
#include "obj.h"
#include "addresses.h"
#include "fixes.h"
#include "gamesystems.h"
#include "obj_fieldnames.h"

const size_t objHeaderSize = 4; // Constant
const size_t objBodySize = 168; // Passed in to Object_Tables_Init
const size_t objSize = objHeaderSize + objBodySize;
static_assert(sizeof(GameObject) == (objSize), "Object structure has the wrong size!");

// Root hashtable for all objects
struct ObjectMasterTableRow {
	GameObject objects[0x2000];
};

struct ObjectMasterTable {
	ObjectMasterTableRow *rows[256];
};

GlobalPrimitive<ObjectMasterTable*, 0x10BCAC50> objMasterTable;
GlobalPrimitive<int, 0x10BCAC4C> objMasterTableSize; // Starts at 1, Max is 256

GlobalPrimitive<uint32_t, 0x10BCAC30> objPoolSize;

const uint32_t ObjFieldDefCount = 430;

struct ObjFieldDef {
	int protoPropIndex;
	int field4;
	int bitmapIndex1;
	uint32_t bitmapMask;
	int bitmapIndex2;
	uint32_t IsStoredInPropCollection;
	uint32_t FieldTypeCode;
};

static struct ObjInternal : AddressTable {
	ObjFieldDef **fieldDefs;
	
	ObjInternal() {
		rebase(fieldDefs, 0x10B3D7D8);
	}
} objInternal;

class ObjTableDump : TempleFix {
public:
	const char* name() override {
		return "Obj Table Dump";
	}

	static void dumpObjectTables() {
		
	}

	void apply() override {
		GameSystemHooks::AddModuleLoadHook(&dumpObjectTables);
	}
} objTableDump;

Objects objects;

Objects::Objects() {
	rebase(_GetInternalFieldInt32, 0x1009E1D0);
}
