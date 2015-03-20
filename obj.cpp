
#include "stdafx.h"
#include "obj.h"
#include "addresses.h"
#include "fixes.h"
#include "gamesystems.h"

const size_t objHeaderSize = 4; // Constant
const size_t objBodySize = 168; // Passed in to Object_Tables_Init
const size_t objSize = objHeaderSize + objBodySize;
static_assert(sizeof(GameObject) == objSize, "Object structure has the wrong size!");

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

class ObjTableDump : TempleFix {
public:
	const char* name() override {
		return "Obj Table Dump";
	}

	static void dumpObjectTables() {

		/*LOG(info) << "Object pool size: " << objPoolSize;
		
		ObjectMasterTable *table = objMasterTable;
		for (int i = 0; i < objMasterTableSize; ++i) {
			auto row = table->rows[i];

			for (auto &obj : row->objects) {
				LOG(info) << "obj!";
			}
		}*/

	}

	void apply() override {
		GameSystemHooks::AddModuleLoadHook(&dumpObjectTables);
	}
} objTableDump;