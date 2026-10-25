#pragma once
#include <cstdint>
#include "config/config.h"

/*
	Debug-only object handle validator (ini: debugObjHandles=false by default).
	Hunts the rare inventory corruption (null / swapped / dangling items in inventories).

	- Keeps its own master registry of every live object handle, fed by ObjRegistry::Add /
	  Remove / Clear, plus a set of recently freed handles (to tell "dangling" from "garbage").
	- Validates inventories when they are written (InsertAtLocation, ForceRemove), read
	  (InventorySystem::GetInventory) and after every save load (full sweep).
	- Mismatches go to the log + anomalyNotices ("objhandle" kind) with call site and object ids.
	When the switch is off every hook is a single bool test.
*/
typedef uint64_t objHndl_t;

class ObjHandleValidator {
public:
	static bool Enabled() { return config.debugObjHandles; }
	void OnAdd(uint64_t handle);
	void OnRemove(uint64_t handle);
	void OnClear();
	// Checks the inventory array of a container/critter. Returns number of problems found.
	int CheckInventory(uint64_t parent, const char *site, void *caller = nullptr);
	int CheckAll(const char *site);
};

extern ObjHandleValidator objHandleValidator;
