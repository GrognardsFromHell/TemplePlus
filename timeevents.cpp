
#include "stdafx.h"
#include "addresses.h"
#include "timeevents.h"
#include "fixes.h"

static struct TimeEventSystems {
	TimeEventSystem systems[38];
} *systemsTable;

static struct TimeEventFuncs : AddressTable {
	
	TimeEventFuncs() {
		rebase(systemsTable, 0x102BD900);
	}	

} timeEventFuncs;

static class Debug : TempleFix {
public:
	const char* name() override {
		return "Time Events";
	}
	void apply() override {
	}
} debug;