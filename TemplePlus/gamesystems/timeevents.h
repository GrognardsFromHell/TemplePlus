#pragma once

#include "gametime.h"
#include "gamesystem.h"

struct GameSystemConf;

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
		I suspect the 0x10AA83D8 is an end of turn flag
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
enum class TimeEventType : uint32_t {
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

enum class TimeEventArgType {
	None,
	Int,
	Float,
	Object,
	PythonObject,
	Location
};

/**
 * Contains the specification of how a type of time event is to be handled.
 */
struct TimeEvent;
struct TimeEventTypeSpec {
	GameClockType clock; // Which clock is used for events of this type
	std::function<BOOL(const TimeEvent* event)> expiredCallback; // Called when an event of this type expires
	std::function<BOOL(const TimeEvent* event)> removedCallback; // Called whenever an event is freed (even if not expired)
	bool persistent; // Events of this type are saved to the savegame
	std::array<TimeEventArgType, 4> argTypes; // The types of the arguments stored in the time event

	TimeEventTypeSpec(GameClockType clock,
		std::function<BOOL(const TimeEvent* event)> expiredCallback,
		std::function<BOOL(const TimeEvent* event)> removedCallback,
		bool persistent,
		TimeEventArgType arg1 = TimeEventArgType::None,
		TimeEventArgType arg2 = TimeEventArgType::None,
		TimeEventArgType arg3 = TimeEventArgType::None,
		TimeEventArgType arg4 = TimeEventArgType::None
		) : clock(clock), expiredCallback(expiredCallback), removedCallback(removedCallback), persistent(persistent), argTypes({arg1, arg2, arg3, arg4}) {
	}
};

// Get the argument types for a specific type of time event
const TimeEventTypeSpec& GetTimeEventTypeSpec(TimeEventType type);

union TimeEventArg {
	int32_t int32;
	float float32;
	objHndl handle;
	PyObject *pyobj;
	LocAndOffsets location;
};

struct TimeEventObjInfo {
	ObjectId guid;
	locXY location;
	int mapNumber;
	int padding;
};

struct TimeEvent {
	GameTime time;
	TimeEventType system;
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

#pragma pack(pop)

class TimeEventSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "TimeEvent";
	TimeEventSystem(const GameSystemConf &config);
	~TimeEventSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;

	void LoadForCurrentMap();
	void ClearForMapClose();

	void Schedule(TimeEvent &evt, uint32_t delayInMs);

	void ScheduleAbsolute(TimeEvent &evt, const GameTime &baseTime, uint32_t delayInMs);

	/**
	 * Schedule an event for immediate execution the next time the simulation is run.
	 */
	void ScheduleNow(TimeEvent &evt);

	GameTime GetTime();

	bool IsDaytime();

	void AdvanceTime(const GameTime &advanceBy);

	// This odd, at a glance it seems to do the same as the previous, but if more than a day is passed
	// additional dispatcher functions are called for the party...???
	void AddTime(int timeInMs);

	string FormatTime(const GameTime &time);

	/**
	 * Removes all time events of the given system type without calling their expiry function.
	 */
	void RemoveAll(TimeEventType type);

private:
	/*
	Adds a timed event to be executed later.
	- evt is the event to add. It is copied. It's time may be adjusted though.
	- delay is the delay from the current time (based on the time event system being used) to the trigger
	- baseTime is optional and allows the current clock to be specified explicitly. This overrides the queue clock.
	- triggerTime is an optional pointer that will be filled with the absolute, calculated trigger time.
	- sourceFile and sourceLine are not used.
	*/
	bool Schedule(TimeEvent *evt, const GameTime *delay, const GameTime *baseTime, GameTime *triggerTime, const char *sourceFile, int sourceLine);
};
