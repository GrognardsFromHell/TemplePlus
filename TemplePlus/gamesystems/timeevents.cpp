
#include "stdafx.h"
#include <temple/dll.h>
#include "gamesystems/timeevents.h"
#include "gamesystems/gamesystems.h"
#include <config/config.h>
#include <description.h>

#include "anim.h"

/*
Internal system specification used by the time event system
*/
struct TimeEventSystemSpec {
	char name[20];
	int field_14;
	int field_18;
	GameClockType clockType;
	void(__cdecl *eventExpired)();
	void(__cdecl *eventRemoved)();
	int field_28;
};

// Lives @ 0x102BD900
struct TimeEventSystems {
	TimeEventType systems[38];
};

#pragma region Time Event Systems

using LegacyExpireFunc = BOOL(const TimeEvent*);

static BOOL ExpireDebug(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10061250);
	return callback(event);
}

static BOOL ExpireAnimEvent(const TimeEvent* event)
{
	return gameSystems->GetAnim().ProcessAnimEvent(event);
	//return TRUE;
}

static BOOL ExpireBkgAnim(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireFidgetAnim(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100144c0);
	return callback(event);
}

static BOOL ExpireScript(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1000bea0);
	return callback(event);
}

static BOOL ExpirePythonScript(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100ad560);
	return callback(event);
}

static BOOL ExpirePoison(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireNormalHealing(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1007efb0);
	return callback(event);
}

static BOOL ExpireSubdualHealing(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1007eca0);
	return callback(event);
}

static BOOL ExpireAging(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireAI(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1005f090);
	return callback(event);
}

static BOOL ExpireAIDelay(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100584b0);
	return callback(event);
}

static BOOL ExpireCombat(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireTBCombat(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireAmbientLighting(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireWorldMap(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireSleeping(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireClock(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireNPCWaitHere(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100579a0);
	return callback(event);
}

static BOOL ExpireMainMenu(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireLight(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100a8010);
	return callback(event);
}

static BOOL ExpireLock(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10021230);
	return callback(event);
}

static BOOL ExpireNPCRespawn(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1006deb0);
	return callback(event);
}

static BOOL ExpireDecayDeadBodies(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1007f230);
	return callback(event);
}

static BOOL ExpireItemDecay(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireCombatFocusWipe(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10080510);
	return callback(event);
}

static BOOL ExpireFade(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10051c10);
	return callback(event);
}

static BOOL ExpireGFadeControl(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireTeleported(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10025250);
	return callback(event);
}

static BOOL ExpireSceneryRespawn(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x101f5850);
	return callback(event);
}

static BOOL ExpireRandomEncounters(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1009a610);
	return callback(event);
}

static BOOL ExpireObjfade(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1004c490);
	return callback(event);
}

static BOOL ExpireActionQueue(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100151f0);
	return callback(event);
}

static BOOL ExpireSearch(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10046f00);
	return callback(event);
}

static BOOL ExpireIntgameTurnbased(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1009a880);
	return callback(event);
}

static BOOL ExpirePythonDialog(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100acc60);
	return callback(event);
}

static BOOL ExpireEncumberedComplain(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x10037df0);
	return callback(event);
}

static BOOL ExpirePythonRealtime(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x100ad560);
	return callback(event);
}


static const TimeEventTypeSpec sTimeEventTypeSpecs[] = {
	// Debug
	TimeEventTypeSpec(
	GameClockType::RealTime,
		ExpireDebug,
		nullptr,
		false,
		TimeEventArgType::Int,
		TimeEventArgType::Int
		),

	// Anim
	TimeEventTypeSpec(
		GameClockType::GameTimeAnims,
		ExpireAnimEvent,
		nullptr,
		true,
		TimeEventArgType::Int
		),

	// Bkg Anim
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireBkgAnim,
		nullptr,
		false,
		TimeEventArgType::Int,
		TimeEventArgType::Int,
		TimeEventArgType::Int
		),

	// Fidget Anim
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireFidgetAnim,
		nullptr,
		false
		),

	// Script
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireScript,
		nullptr,
		true,
		TimeEventArgType::Int,
		TimeEventArgType::Int,
		TimeEventArgType::Object,
		TimeEventArgType::Object
		),

	// PythonScript
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpirePythonScript,
		nullptr,
		true,
		TimeEventArgType::PythonObject,
		TimeEventArgType::PythonObject
		),

	// Poison
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpirePoison,
		nullptr,
		true,
		TimeEventArgType::Int,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Normal Healing
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireNormalHealing,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Subdual Healing
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireSubdualHealing,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Aging
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireAging,
		nullptr,
		true
		),

	// AI
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireAI,
		nullptr,
		false,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// AI Delay
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireAIDelay,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// Combat
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireCombat,
		nullptr,
		true
		),

	// TB Combat
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireTBCombat,
		nullptr,
		true
		),

	// Ambient Lighting
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireAmbientLighting,
		nullptr,
		true
		),

	// WorldMap
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireWorldMap,
		nullptr,
		true
		),

	// Sleeping
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireSleeping,
		nullptr,
		false,
		TimeEventArgType::Int
		),

	// Clock
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireClock,
		nullptr,
		true
		),

	// NPC Wait Here
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireNPCWaitHere,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// MainMenu
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireMainMenu,
		nullptr,
		false,
		TimeEventArgType::Int
		),

	// Light
	TimeEventTypeSpec(
		GameClockType::GameTimeAnims,
		ExpireLight,
		nullptr,
		false,
		TimeEventArgType::Int,
		TimeEventArgType::Int
		),

	// Lock
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireLock,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// NPC Respawn
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireNPCRespawn,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// Decay Dead Bodies
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireDecayDeadBodies,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Item Decay
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireItemDecay,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Combat-Focus Wipe
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireCombatFocusWipe,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Fade
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireFade,
		nullptr,
		true,
		TimeEventArgType::Int,
		TimeEventArgType::Int,
		TimeEventArgType::Float,
		TimeEventArgType::Int
		),

	// GFadeControl
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireGFadeControl,
		nullptr,
		true
		),

	// Teleported
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireTeleported,
		nullptr,
		false,
		TimeEventArgType::Object
		),

	// Scenery Respawn
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireSceneryRespawn,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// Random Encounters
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireRandomEncounters,
		nullptr,
		true
		),

	// objfade
	TimeEventTypeSpec(
		GameClockType::GameTimeAnims,
		ExpireObjfade,
		nullptr,
		true,
		TimeEventArgType::Int,
		TimeEventArgType::Object
		),

	// Action Queue
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireActionQueue,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// Search
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireSearch,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// intgame_turnbased
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpireIntgameTurnbased,
		nullptr,
		false,
		TimeEventArgType::Int,
		TimeEventArgType::Int
		),

	// python_dialog
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpirePythonDialog,
		nullptr,
		true,
		TimeEventArgType::Object,
		TimeEventArgType::Object,
		TimeEventArgType::Int
		),

	// encumbered complain
	TimeEventTypeSpec(
		GameClockType::GameTime,
		ExpireEncumberedComplain,
		nullptr,
		true,
		TimeEventArgType::Object
		),

	// PythonRealtime
	TimeEventTypeSpec(
		GameClockType::RealTime,
		ExpirePythonRealtime,
		nullptr,
		true,
		TimeEventArgType::PythonObject,
		TimeEventArgType::PythonObject
		),

};

const TimeEventTypeSpec& GetTimeEventTypeSpec(TimeEventType type) {
	return sTimeEventTypeSpecs[(size_t)type];
}

#pragma endregion


class TimeEventHooks : public TempleFix
{
public: 
	void apply() override 
	{
		static int (*orgTimeEventValidate)(TimeEventListEntry* evt, int flag) = replaceFunction<int (__cdecl)(TimeEventListEntry*, int)>(0x10060430, [](TimeEventListEntry* evt, int flag)
		{
			/*if (evt->evt.system == TimeEventType::ObjFade)
			{
				int dummy = 1;
				if (config.newFeatureTestMode)
				{
					if (evt->objects[1].guid.subtype == ObjectIdKind::Null)
					{
						int dsummy = 1;
					}
					logger->debug("TimeEventValidate: ObjFade evt id {} for obj {}", evt->evt.params[0].int32, description.getDisplayName(evt->evt.params[1].handle));
				}
			} */
			int result = orgTimeEventValidate(evt, flag);
			if (!result)
			{
				logger->debug("Failed to validate time event. Event system {}, param0 {}", (int)evt->evt.system, evt->evt.params[0].int32);
			}
			return result;
		});

		static void (*orgTransparencySet)(objHndl , int) = replaceFunction<void(__cdecl)(objHndl, int)>(0x10020060, [](objHndl obj, int flag)
		{
			if (!obj)
			{
				int dummy = 1;
			} else
				orgTransparencySet(obj, flag);
		});

		static int (*orgGetOpacity)(objHndl) = replaceFunction<int(__cdecl)(objHndl)>(0x10020180, [](objHndl obj)
		{
			if (!obj)
			{
				int dummy = 1;
			}
			return orgGetOpacity(obj);
		});


		static int (*orgObjfadeTimeeventExpires)(TimeEvent* ) = replaceFunction<int(__cdecl)(TimeEvent*)>(0x1004C490, [](TimeEvent*evt)
		{
			auto param1 = evt->params[1];
			auto param0 = evt->params[0];
			if (!param1.handle)
			{
				int dummy = 1;
			}
			return orgObjfadeTimeeventExpires(evt);
		});

		// fix for Encumbered Complain on null handle crash (I think it happens if you encumber an NPC and leave the area / send him off from the party?)
		static int (*orgEncumberedComplainTimeeventExpires)(TimeEvent* ) = replaceFunction<int(__cdecl)(TimeEvent*)>(0x10037DF0, [](TimeEvent*evt)	{
			if (!evt->params[0].handle)	{
				return 1;
			}
				

			return orgEncumberedComplainTimeeventExpires(evt);
		});


		static int (*orgAdvanceTime)(int ) = replaceFunction<int (__cdecl)(int)>(0x100620C0, [](int timeMsec)
		{
			return orgAdvanceTime(timeMsec);
		});

		static int (*orgTimeEventSchedule)(TimeEvent*, GameTime*, GameTime*, GameTime*) = replaceFunction<int(__cdecl)(TimeEvent*, GameTime*, GameTime*, GameTime*)>(0x10060720, [](TimeEvent* evt, GameTime* timeDelta, GameTime* timeAbsolute, GameTime* timeResultOut)
		{
			/*if (evt->system == TimeEventType::ObjFade)
			{
				if (config.newFeatureTestMode)
				{
					logger->debug("TimeEventSchedule: ObjFade evt id {} for obj {}", evt->params[0].int32, description.getDisplayName(evt->params[1].handle));
				}
			}*/
			return orgTimeEventSchedule(evt, timeDelta, timeAbsolute, timeResultOut);
		});

		static void(__cdecl*orgExpireLock)(TimeEvent*) = replaceFunction<void(TimeEvent*)>(0x10021230, [](TimeEvent* evt) {
			if (!evt->params[0].handle) // fix for crash with null handle
				return;

			return orgExpireLock(evt);
		});
	}
} hooks;

//*****************************************************************************
//* TimeEvent
//*****************************************************************************

TimeEventSystem::TimeEventSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100616a0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system TimeEvent");
	}
}
TimeEventSystem::~TimeEventSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10061820);
	shutdown();
}
void TimeEventSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100617a0);
	reset();
}
bool TimeEventSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x10061840);
	return save(file) == 1;
}
bool TimeEventSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10061f90);
	return load(saveFile) == 1;
}
void TimeEventSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x100620c0);
	advanceTime(time);
}
const std::string &TimeEventSystem::GetName() const {
	static std::string name("TimeEvent");
	return name;
}

void TimeEventSystem::LoadForCurrentMap()
{
	static auto loadForCurrentMap = temple::GetPointer<void()>(0x10061D10);
	loadForCurrentMap();
}

void TimeEventSystem::ClearForMapClose()
{
	static auto clearForMapClose = temple::GetPointer<void()>(0x10061A50);
	clearForMapClose();
}

BOOL TimeEventSystem::Schedule(TimeEvent & evt, uint32_t delayInMs, GameTime *triggerTimeOut)
{
	GameTime delay(0, delayInMs);
	return Schedule(&evt, &delay, nullptr, triggerTimeOut, nullptr, 0);
}

BOOL TimeEventSystem::ScheduleAbsolute(TimeEvent & evt, const GameTime & baseTime, uint32_t delayInMs, GameTime *triggerTimeOut) {
	GameTime delay(0, delayInMs);
	return Schedule(&evt, &delay, &baseTime, triggerTimeOut, nullptr, 0);
}

BOOL TimeEventSystem::ScheduleNow(TimeEvent & evt)
{
	static auto timeevent_add_special = temple::GetPointer<BOOL(TimeEvent *createArgs)>(0x10062340);
	return timeevent_add_special(&evt);
}

GameTime TimeEventSystem::GetTime() {
	static auto GameTime_Get = temple::GetPointer<uint64_t()>(0x1005fc90);
	return GameTime_Get();
}

bool TimeEventSystem::IsDaytime() {
	static auto Is_Daytime = temple::GetPointer<BOOL()>(0x100600e0);
	return Is_Daytime() == TRUE;
}

void TimeEventSystem::AdvanceTime(const GameTime & advanceBy) {
	static auto GameTime_Advance = temple::GetPointer<BOOL(const GameTime*)>(0x10060c90);
	GameTime_Advance(&advanceBy);
}


// This odd, at a glance it seems to do the same as the previous, but if more than a day is passed
// additional dispatcher functions are called for the party...???
void TimeEventSystem::AddTime(int timeInMs) {
	static auto GameTime_Add = temple::GetPointer<BOOL(int)>(0x10062390);
	GameTime_Add(timeInMs);
}

std::string TimeEventSystem::FormatTime(const GameTime& time) {

	static auto GameTime_Format = temple::GetPointer<void(const GameTime*, char *)>(0x10061310);

	char buffer[256];
	GameTime_Format(&time, buffer);
	return buffer;
}

void TimeEventSystem::RemoveAll(TimeEventType type) {
	static auto timeevent_remove_all = temple::GetPointer<signed int(int systemType)>(0x10060970);
	timeevent_remove_all((int)type);
}

void TimeEventSystem::Remove(TimeEventType type, Predicate predicate)
{
	static std::function<bool(const TimeEvent&)> sPredicate;
	sPredicate = predicate;

	using LegacyPredicate = BOOL(*)(const TimeEvent&);
	static auto TimeEventsRemove = temple::GetPointer<BOOL(TimeEventType, LegacyPredicate)>(0x10060a40);

	TimeEventsRemove(type, [](const TimeEvent &evt) {
		return sPredicate(evt) ? TRUE : FALSE;
	});

	sPredicate = nullptr;
}

bool TimeEventSystem::Schedule(TimeEvent * evt, const GameTime * delay, const GameTime * baseTime, GameTime * triggerTimeOut, const char * sourceFile, int sourceLine)
{
	using ScheduleFn = BOOL(TimeEvent* createArgs, const GameTime *time, const GameTime *curTime, GameTime *pTriggerTimeOut, const char *sourceFile, int sourceLine);
	static auto timeevent_add_ex = temple::GetPointer<ScheduleFn>(0x10060720);

	return timeevent_add_ex(evt, delay, baseTime, triggerTimeOut, sourceFile, sourceLine) == TRUE;
}
