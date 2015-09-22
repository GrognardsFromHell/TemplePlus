
#include "stdafx.h"
#include <temple/dll.h>
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

string TimeEvents::FormatTime(const GameTime& time) {
	char buffer[256];
	_FormatTime(time, buffer);
	return buffer;
}

TimeEvents::TimeEvents() {
	rebase(systemsTable, 0x102BD900);
	rebase(_Schedule, 0x10060720);
	rebase(_AdvanceTime, 0x10060C90);
	rebase(_AddTime, 0x10062390);
	rebase(_GetTime, 0x1005FC90);
	rebase(_FormatTime, 0x10061310);
	rebase(_IsDaytime, 0x100600E0);
}
