#pragma once

#include "temple_functions.h"

#pragma pack(push, 1)

/*
	There are three game time clocks that advance under different conditions.
*/
enum class GameClockType : uint32_t {
	/*
		Always increased every timestep.
	*/
	RealTime = 0,
	/*
		Only advances while out of combat, [0x10AA83D8] is 0 and the dialog UI is not visible.
	*/
	GameTime,
	/*
		Advances as long as the dialog UI is not visible, but not while the
		game is otherwise paused.
	*/
	GameTimeAnims
};

/*
	The time event system that are available and can handle events.
*/
enum class TimeEventSystem : uint32_t {
	Debug = 0,
	Anim = 1,
	BkgAnim = 2,
	FidgetAnim = 3,
	Script = 4,
	PythonScript = 5,
	Poison = 6,
	NormalHealing = 7,
	SubdualHealing = 8,
	Aging = 9,
	AI = 10,
	AIDelay = 11,
	Combat = 12,
	TBCombat = 13,
	AmbientLighting = 14,
	WorldMap = 15,
	Sleeping = 16,
	Clock = 17,
	NPCWaitHere = 18,
	MainMenu = 19,
	Light = 20,
	Lock = 21,
	NPCRespawn = 22,
	DecayDeadBodies = 23,
	ItemDecay = 24,
	CombatFocusWipe = 25,
	Fade = 26,
	GFadeControl = 27,
	Teleported = 28,
	SceneryRespawn = 29,
	RandomEncounters = 30,
	ObjFade = 31,
	ActionQueue = 32,
	Search = 33,
	IntgameTurnbased = 34,
	PythonDialog = 35,
	EncumberedComplain = 36,
	PythonRealtime = 37
};

struct GameTime {
	int timeInDays = 0;
	int timeInMs = 0;

	GameTime() {
	}

	GameTime(int days, int ms) : timeInDays(days), timeInMs(ms) {
	}
};

struct TimeEventArg {
	int field0 = 0;
	int field1 = 0;
	int field2 = 0;
	int field3 = 0;
};

struct TimeEventObjInfo {
	ObjectId guid;
	locXY location;
	int mapNumber;
	int padding;
};

struct TimeEvent {
	GameTime time;
	TimeEventSystem system;
	int padding = 0;
	TimeEventArg params[4];
};

struct TimeEventListEntry {
	TimeEvent evt;
	// Keeps track of objs referenced in the time event
	TimeEventObjInfo objects[4];
	// Linked list ptr to next entry
	TimeEventListEntry* nextEvent;
};

extern struct TimeEvents : AddressTable {

	void Schedule(const TimeEvent &evt, uint32_t delayInMs) {
		GameTime delay(0, delayInMs);
		_Schedule(&evt, &delay, nullptr, nullptr, nullptr, 0);
	}

	void ScheduleAbsolute(const TimeEvent &evt, const GameTime &baseTime, uint32_t delayInMs) {
		GameTime delay(0, delayInMs);
		_Schedule(&evt, &delay, &baseTime, nullptr, nullptr, 0);
	}
	
	TimeEvents();

private:
	/*
		Adds a timed event to be executed later.
		- evt is the event to add. It is copied.
		- delay is the delay from the current time (based on the time event system being used) to the trigger	
		- baseTime is optional and allows the current clock to be specified explicitly. This overrides the queue clock.
		- triggerTime is an optional pointer that will be filled with the absolute, calculated trigger time.
		- sourceFile and sourceLine are not used.
	*/
	bool (__cdecl *_Schedule)(const TimeEvent *evt, const GameTime *delay, const GameTime *baseTime, GameTime *triggerTime, const char *sourceFile, int sourceLine);
} timeEvents;

#pragma pack(pop)
