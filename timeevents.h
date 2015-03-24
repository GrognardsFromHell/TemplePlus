
#pragma once

#pragma pack(push, 1)
struct TimeEventSystem {
	char name[20];
	int field_14;
	int field_18;
	int queueType;
	void (__cdecl *eventExpired)();
	void (__cdecl *callback)();
	int field_28;

};
#pragma pack(pop)

