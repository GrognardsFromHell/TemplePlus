
#include "stdafx.h"
#include "addresses.h"
#include "timeevents.h"

/*
Internal system specification used by the time event system
*/
struct TimeEventSystemSpec {
	char name[20];
	int field_14;
	int field_18;
	GameClockType clockType;
	void(__cdecl *eventExpired)();
	void(__cdecl *eventExpiredAlways)();
	int field_28;
};

static struct TimeEventSystems {
	TimeEventSystem systems[38];
} *systemsTable;

TimeEvents timeEvents;

TimeEvents::TimeEvents() {
	rebase(systemsTable, 0x102BD900);
	rebase(_Schedule, 0x10060720);
}
