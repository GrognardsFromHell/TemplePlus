
#include "stdafx.h"
#include <temple/dll.h>
#include "gamesystems/timeevents.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/mapsystem.h"
#include <config/config.h>
#include <description.h>
#include <objlist.h>

#include "animgoals/anim.h"
#include <gamesystems\legacy.h>
#include "legacysystems.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "combat.h"
#include "graphics/mapterrain.h"
#include "objfade.h"
#include "ui/ui_dialog.h"
#include <tig/tig_timer.h>

/*
Internal system specification used by the time event system
*/
struct TimeEventSystemSpec {
	char name[20];
	int isPersistent;
	int objrefFlags;
	GameClockType clockType;
	void(__cdecl *eventExpired)();
	void(__cdecl *eventRemoved)();
	BOOL (__cdecl* paramValidator)(TimeEventListEntry* timeEvtListEntry); // in principle checks if it's ok to serialize parameter to savegame file (but in practice the callback is always null)
};

// Lives @ 0x102BD900
struct TimeEventSystems {
	TimeEventType systems[38];
};

// 0x102BDF98 [4]  one for each timer type
struct TimeEventFlagPacket {
	int field0;
	int objRef;
	int field8;
	int fieldc;
	int field10;
};

#pragma region Time Event Systems

using LegacyExpireFunc = BOOL(const TimeEvent*);

#pragma region Expire Funcs
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

	if (temple::GetRef<int>(0x10AA83D4))
	{
		static auto actualGameCallback = temple::GetPointer<LegacyExpireFunc>(0x10173830);
		return actualGameCallback(event);
	}

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

	if (temple::GetRef<int>(0x10AA83D4)){
		static auto actualGameCallback = temple::GetPointer<LegacyExpireFunc>(0x101739C0);
		return actualGameCallback(event);
	}

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

	if (temple::GetRef<int>(0x10AA83D4)) {
		static auto actualGameCallback = temple::GetPointer<LegacyExpireFunc>(0x101119B0);
		return actualGameCallback(event);
	}
	
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
	// calls 0x10025050 which does some sector lighting updates mainly
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
static BOOL RemoveObjfade(const TimeEvent* event) {
	static auto callback = temple::GetPointer<LegacyExpireFunc>(0x1004C570);
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
#pragma endregion

#pragma region System Specs
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
		RemoveObjfade,
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
#pragma endregion
const TimeEventTypeSpec& GetTimeEventTypeSpec(TimeEventType type) {
	return sTimeEventTypeSpecs[(size_t)type];
}

#pragma endregion


class TimeEventHooks : public TempleFix
{
public: 
	static BOOL TimeEventListEntryAdd(TimeEventListEntry * evt);
	static BOOL TimeEventReadFromFile(TioFile * file, TimeEvent * evtOut);

	void apply() override 
	{

		replaceFunction(0x100607E0, TimeEventListEntryAdd);

		static int (*orgTimeEventValidate)(TimeEventListEntry* evt, int flag) = 
			replaceFunction<int (__cdecl)(TimeEventListEntry*, int)>(0x10060430, [](TimeEventListEntry* evt, int isLoadingMap){
			
			return (BOOL)evt->IsValid(isLoadingMap);
			/*int result = orgTimeEventValidate(evt, isLoadingMap);
			if (!result){
				logger->debug("Failed to validate time event. Event system {}, param0 {}", (int)evt->evt.system, evt->evt.params[0].int32);
			}
			return result;*/
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
			auto handle = evt->params[1].handle;
			auto id = evt->params[0].int32;
			
			if (!handle){
				return FALSE;
			}

			return gameSystems->GetObjFade().TimeEventExpired(evt);
			// return orgObjfadeTimeeventExpires(evt);
		});

		// fix for Encumbered Complain on null handle crash (I think it happens if you encumber an NPC and leave the area / send him off from the party?)
		static int (*orgEncumberedComplainTimeeventExpires)(TimeEvent* ) = replaceFunction<int(__cdecl)(TimeEvent*)>(0x10037DF0, [](TimeEvent*evt)	{
			if (!evt->params[0].handle)	{
				return TRUE;
			}
			return orgEncumberedComplainTimeeventExpires(evt);
		});


		//static void (*orgAdvanceTime)(uint32_t ) = replaceFunction<void (__cdecl)(uint32_t)>(0x100620C0, [](uint32_t newTimeMs){
		//	// obsolete, replaced
		//	auto &timePlayed = temple::GetRef<GameTime>(0x10AA83B8);
		//	auto &gameTimeElapsed = temple::GetRef<GameTime>(0x10AA83C0);
		//	auto &animTime = temple::GetRef<GameTime>(0x10AA83C8);
		//	
		//	auto & lastPingMs = temple::GetRef<uint32_t>(0x10AA83D0);
		//	auto timeDeltaMs = newTimeMs - lastPingMs;
		//	
		//	// limit time step to 250ms
		//	if (timeDeltaMs > 250){
		//		timeDeltaMs = 250;
		//	}

		//	// update last ping
		//	lastPingMs = newTimeMs; 

		//	// mark that we are currently in time event advance time, so that new time events get appended in a special list (see TimeEventSystem::ScheduleInternal)
		//	auto isInAdvanceTime = temple::GetRef<BOOL>(0x10AA83DC);
		//	isInAdvanceTime = TRUE;



		//	// advance the time played
		//	
		//	timePlayed.timeInMs += timeDeltaMs;

		//	if (timePlayed.timeInMs > 86400000){
		//		timePlayed.timeInDays += timePlayed.timeInMs / 86400000;
		//		timePlayed.timeInMs = timePlayed.timeInMs % 86400000;
		//	}

		//	// advanced time elapsed and anim time
		//	auto timeEventUnk10AA83D8 = temple::GetRef<int>(0x10AA83D8); // something related to UI

		//	
		//	if (!timeEventUnk10AA83D8){

		//		if (!uiSystems->GetDlg().IsActive() && !combatSys.isCombatActive()){
		//				
		//			
		//			gameTimeElapsed.timeInMs += timeDeltaMs;

		//			if (gameTimeElapsed.timeInMs > 86400000) {
		//				gameTimeElapsed.timeInDays += gameTimeElapsed.timeInMs / 86400000;
		//				gameTimeElapsed.timeInMs = gameTimeElapsed.timeInMs % 86400000;
		//			}

		//			auto updateDaylight = temple::GetRef<void(__cdecl)()>(0x100A75E0);
		//			updateDaylight();
		//			gameSystems->GetTerrain().UpdateDayNight();

		//		}
		//	}

		//	
		//	if (!timeEventUnk10AA83D8 || uiSystems->GetDlg().IsActive() ){
		//		
		//		animTime.timeInMs += timeDeltaMs;

		//		if (animTime.timeInMs > 86400000) {
		//			animTime.timeInDays += animTime.timeInMs / 86400000;
		//			animTime.timeInMs = animTime.timeInMs % 86400000;
		//		}
		//	}

		//	// expire events whose time has come (executing their expired callback)

		//	auto expiredCount = 0;
		//	TimeEventListEntry lastValid;
		//	for (auto clockType = 0; clockType < (int)GameClockType::ClockTypeCount; clockType++){
		//		GameTime * time;
		//		switch ((GameClockType)clockType){
		//		case GameClockType::RealTime:
		//			time = &timePlayed; break;
		//		case GameClockType::GameTime:
		//			time = &gameTimeElapsed;
		//			break;
		//		case GameClockType::GameTimeAnims:
		//		default:
		//			time = &animTime; 
		//			break;
		//		}

		//		TimeEventListEntry **evtListEntry = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73FC)[clockType];
		//		while (*evtListEntry){
		//			auto node = *evtListEntry;
		//			auto nextNode = node->nextEvent;

		//			if (node->evt.time.timeInDays > time->timeInDays
		//				|| (node->evt.time.timeInDays >= time->timeInDays
		//					&& node->evt.time.timeInMs > time->timeInMs)) {
		//				break;
		//			}

		//			// Expire event
		//			*evtListEntry = nextNode;

		//			auto sysSpec = GetTimeEventTypeSpec(node->evt.system);

		//			if (node->IsValid(0)){
		//				lastValid = *node;
		//				sysSpec.expiredCallback(&node->evt);
		//			}

		//			if (sysSpec.removedCallback){
		//				sysSpec.removedCallback(&node->evt);
		//			}
		//			free(node);

		//			expiredCount++;
		//			if (expiredCount >= 500){
		//				logger->error("TimeEvent::AdvanceTime: Suspected Infinite Loop Caugt: Last Type: {}", (int)node->evt.system);
		//				expiredCount = 0;
		//				break;
		//			}

		//		}
		//	}



		//	// unmark isInAdvanceTime
		//	isInAdvanceTime = FALSE;



		//	// append events that were added during this function call
		//	for (auto clockType = 0; clockType < (int)GameClockType::ClockTypeCount; clockType++){
		//		TimeEventListEntry** evtListInAdvanceTime = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73E8)[clockType];
		//		
		//		while (*evtListInAdvanceTime){
		//			auto node = *evtListInAdvanceTime;
		//			auto nextNode = node->nextEvent;
		//			
		//			if (node->ObjHandlesValid()){
		//				gameSystems->GetTimeEvent().TimeEventListEntryAdd(node);
		//			}
		//			else{
		//				free(node);
		//			}

		//			*evtListInAdvanceTime = nextNode;
		//		}
		//	}

		//	// return orgAdvanceTime(timeMsec);
		//});

		/*static int (*orgTimeEventSchedule)(TimeEvent*, GameTime*, GameTime*, GameTime*) = replaceFunction<int(__cdecl)(TimeEvent*, GameTime*, GameTime*, GameTime*)>(0x10060720, [](TimeEvent* evt, GameTime* timeDelta, GameTime* timeAbsolute, GameTime* timeResultOut)
		{
			if (evt->system == TimeEventType::Search)
			{
				auto asdf = 1;
			}
			return orgTimeEventSchedule(evt, timeDelta, timeAbsolute, timeResultOut);
		});*/

		

		static BOOL(__cdecl*orgExpireLock)(TimeEvent*) = replaceFunction<BOOL(TimeEvent*)>(0x10021230, [](TimeEvent* evt) {
			if (!evt->params[0].handle) // fix for crash with null handle
			{
				logger->debug("Caught ExpireLock event with null handle!");
				return TRUE;
			}
			return orgExpireLock(evt);
		});


		static BOOL(__cdecl*orgExpireAiDelay)(TimeEvent*) = replaceFunction<BOOL(TimeEvent*)>(0x100584B0, [](TimeEvent* evt) {
			if (!evt->params[0].handle){ // fix for crash with null handle
				logger->debug("Caught ExpireAiDelay event with null handle!");
				return TRUE;
			}
			return orgExpireAiDelay(evt);
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

	if (!file)
		return false;


	auto &gameTimePlayed = temple::GetRef<GameTime>(0x10AA83B8);
	auto &gameTimeElapsed = temple::GetRef<GameTime>(0x10AA83C0);
	auto &gameTimeAnim = temple::GetRef<GameTime>(0x10AA83C8);
	
	if (!tio_fwrite(&gameTimePlayed, sizeof(GameTime), 1, file) 
		|| !tio_fwrite(&gameTimeElapsed, sizeof(GameTime), 1, file)
		|| !tio_fwrite(&gameTimeAnim, sizeof(GameTime), 1, file))
		return false;
	

	TimeEventListEntry ** evtList;
	uint64_t filePos = 0i64, filePos2 = 0i64;
	for (auto clockType = 0; clockType <= (int)GameClockType::GameTimeAnims; clockType++) {
		evtList = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73FC)[(int)clockType];
		auto count = 0;
		
		tio_fgetpos(file, &filePos);
		if (!tio_fwrite(&count, sizeof(int), 1, file))
			return false;

		while (*evtList) {
			auto listNode = *evtList;
			auto &sysSpec = sTimeEventTypeSpecs[(int)listNode->evt.system];
			evtList = &(listNode->nextEvent);
			if (sysSpec.persistent) {
				if (!TimeEventParamSerializer(file, sysSpec, listNode))
					return false;
				count++;
			}
		}
		// update the event count in the beginning
		tio_fgetpos(file, &filePos2);
		tio_fsetpos(file, &filePos);
		tio_fwrite(&count, sizeof(int), 1, file);
		tio_fsetpos(file, &filePos2);
	}
	



	return true;



	/*auto save = temple::GetPointer<int(TioFile*)>(0x10061840);
	return save(file) == 1;*/
}
bool TimeEventSystem::LoadGame(GameSystemSaveFile* saveFile) {

	auto count = 0;
	auto file = saveFile->file;
	if (!file)
		return false;

	auto &gameTimePlayed = temple::GetRef<GameTime>(0x10AA83B8);
	auto &gameTimeElapsed = temple::GetRef<GameTime>(0x10AA83C0);

	if (!tio_fread(&gameTimePlayed, sizeof(GameTime),1, file) || !tio_fread(&gameTimeElapsed, sizeof(GameTime), 1, file))
		return false;

	auto updateDaylight = temple::GetRef<void(__cdecl)()>(0x100A75E0);
	updateDaylight();
	auto mapGroundToggleDaynight = temple::GetRef<void(__cdecl)()>(0x1002D290);
	mapGroundToggleDaynight();

	auto &gameTimeAnim = temple::GetRef<GameTime>(0x10AA83C8);
	if (!tio_fread(&gameTimeAnim, sizeof(GameTime), 1, file))
		return false;
	
	for (auto clockType = 0; clockType <= (int)GameClockType::GameTimeAnims; clockType++) {

		if (!tio_fread(&count, sizeof(int), 1, file) == 1)
			return false;

		for (auto i = 0; i < count; i++) {
			TimeEvent evt;
			if (!TimeEventReadFromFile(file, &evt))
				return false;
			if ((int)evt.system >(int)TimeEventType::PythonRealtime)
				return false;
			auto timeEvtIsEditor = temple::GetRef<int>(0x10AA73F4);
			if (timeEvtIsEditor)
				return false;
			auto evtListEntry = new TimeEventListEntry;
			evtListEntry->evt = evt;
			evtListEntry->nextEvent = nullptr;
			if (evt.system == TimeEventType::Lock) {
				auto dummy = 1;
			}
			if (!TimeEventListEntryAdd(evtListEntry))
				return false;
		}

	}
	

	return true;



	/*auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10061f90);
	return load(saveFile) == 1;*/
}
void TimeEventSystem::AdvanceTime(uint32_t newTimeMs) {
	//auto advanceTime = temple::GetPointer<void(uint32_t)>(0x100620c0);

	auto &timePlayed = temple::GetRef<GameTime>(0x10AA83B8);
	auto &gameTimeElapsed = temple::GetRef<GameTime>(0x10AA83C0);
	auto &animTime = temple::GetRef<GameTime>(0x10AA83C8);

	auto & lastPingMs = temple::GetRef<uint32_t>(0x10AA83D0);

	auto timeDeltaMs = newTimeMs - lastPingMs;

	// limit time step to 250ms
	if (timeDeltaMs > 250) {
		timeDeltaMs = 250;
	}
	auto expiredCount = 0;

	//advanceTime(newTimeMs);
	//return;


	// update last ping
	lastPingMs = newTimeMs;

	// mark that we are currently in time event advance time, so that new time events get appended in a special list (see TimeEventSystem::ScheduleInternal)
	auto &isInAdvanceTime = temple::GetRef<BOOL>(0x10AA83DC);
	isInAdvanceTime = TRUE;


	// advance the time played

	timePlayed.timeInMs += timeDeltaMs;

	if (timePlayed.timeInMs > 86400000) {
		timePlayed.timeInDays += timePlayed.timeInMs / 86400000;
		timePlayed.timeInMs = timePlayed.timeInMs % 86400000;
	}

	// advanced time elapsed and anim time
	auto &timeAdvanceBlockerCount = temple::GetRef<int>(0x10AA83D8); // count of windows that block fidget animations / time advance

	
	if (!timeAdvanceBlockerCount) {

		if (!uiSystems->GetDlg().IsActive() && !combatSys.isCombatActive()) {


			gameTimeElapsed.timeInMs += timeDeltaMs;

			if (gameTimeElapsed.timeInMs > 86400000) {
				gameTimeElapsed.timeInDays += gameTimeElapsed.timeInMs / 86400000;
				gameTimeElapsed.timeInMs = gameTimeElapsed.timeInMs % 86400000;
			}

			auto updateDaylight = temple::GetRef<void(__cdecl)()>(0x100A75E0);
			updateDaylight();
			gameSystems->GetTerrain().UpdateDayNight();

		}
	}
	combatSys.forceEndedCombatNow = false; // reset this

	if (!timeAdvanceBlockerCount || uiSystems->GetDlg().IsActive()) {

		animTime.timeInMs += timeDeltaMs;

		if (animTime.timeInMs > 86400000) {
			animTime.timeInDays += animTime.timeInMs / 86400000;
			animTime.timeInMs = animTime.timeInMs % 86400000;
		}
	}

	// expire events whose time has come (executing their expired callback)

	
	TimeEventListEntry lastValid;
	expiredCount = 0;
	for (auto clockType = 0; clockType < (int)GameClockType::ClockTypeCount; clockType++) {
		GameTime * time;
		switch ((GameClockType)clockType) {
		case GameClockType::RealTime:
			time = &timePlayed; break;
		case GameClockType::GameTime:
			time = &gameTimeElapsed;
			break;
		case GameClockType::GameTimeAnims:
		default:
			time = &animTime;
			break;
		}

		TimeEventListEntry **evtListEntry = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73FC)[clockType];
		auto node = *evtListEntry;
		while (node){
			
			if (node->evt.time.timeInDays > time->timeInDays
				|| (node->evt.time.timeInDays >= time->timeInDays
					&& node->evt.time.timeInMs > time->timeInMs)) {
				break;
			}
			auto nextNode = node->nextEvent;

			*evtListEntry = nextNode;

			// Expire event
			auto sysSpec = GetTimeEventTypeSpec(node->evt.system);
			auto now = TigGetSystemTime();
			if (node->IsValid(0)) {
				lastValid = *node;
				sysSpec.expiredCallback(&node->evt);
			}

			if (sysSpec.removedCallback) {
				sysSpec.removedCallback(&node->evt);
			}
			/*if (TigElapsedSystemTime(now) > 100) {
				logger->trace("Slow callback detected on system {}",(int) node->evt.system);
			}*/
			auto evtSystemId = node->evt.system;
			free(node);

			expiredCount++;
			if (expiredCount >= 500) {
				logger->error("TimeEvent::AdvanceTime: Suspected Infinite Loop Caugt: Last Type: {}", (int)evtSystemId);
				expiredCount = 0;
				break;
			}

			node = *evtListEntry;
		}
	}
	/*if (expiredCount)
		logger->debug("Expired {} events", expiredCount);*/


	// unmark isInAdvanceTime
	isInAdvanceTime = FALSE;



	// append events that were added during this function call
	for (auto clockType = 0; clockType < (int)GameClockType::ClockTypeCount; clockType++) {
		TimeEventListEntry** evtListInAdvanceTime = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73E8)[clockType];

		while (*evtListInAdvanceTime) {
			auto node = *evtListInAdvanceTime;
			auto nextNode = node->nextEvent;

			if (node->ObjHandlesValid()) {
				gameSystems->GetTimeEvent().TimeEventListEntryAdd(node);
			}
			else {
				free(node);
			}

			*evtListInAdvanceTime = nextNode;
		}
	}

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

/* 0x1005FC60 */
GameTime TimeEventSystem::GetAnimTime()
{
	static auto GameTime_GetAnim = temple::GetPointer<uint64_t()>(0x1005FC60);
	return GameTime_GetAnim();
}

bool TimeEventSystem::IsDaytime() {
	static auto Is_Daytime = temple::GetPointer<BOOL()>(0x100600e0);
	return Is_Daytime() == TRUE;
}

void TimeEventSystem::GameTimeAdd(const GameTime & advanceBy) {
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

void TimeEventSystem::PushDisableAdvance(){
	static auto call = temple::GetPointer<void()>(0x100603F0);
	call();
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

bool TimeEventSystem::Schedule(TimeEvent * evt, const GameTime * delay, const GameTime * baseTime, GameTime * triggerTimeOut, const char * sourceFile, int sourceLine){

	if (!evt)
		return false;

	if ((int)evt->system > (int)TimeEventType::TimeEventSystemCount)
		return false;


	GameTime time;
	auto clockType = sTimeEventTypeSpecs[(int)evt->system].clock;

	if (baseTime && (baseTime->timeInDays ||baseTime->timeInMs)){
		time.timeInDays = baseTime->timeInDays;
		time.timeInMs = baseTime->timeInMs;
	} 
	else{
		if (clockType == GameClockType::RealTime){
			auto &gameTimePlayed = temple::GetRef<GameTime>(0x10AA83B8);
			time = gameTimePlayed;
		}
		else if (clockType == GameClockType::GameTime){
			auto &gameTimeElapsed = temple::GetRef<GameTime>(0x10AA83C0);
			time = gameTimeElapsed;
		}
		else if (clockType == GameClockType::GameTimeAnims){
			auto &gameTimeAnim = temple::GetRef<GameTime>(0x10AA83C8);
			time = gameTimeAnim;
		}
	}

	time.timeInDays += delay->timeInDays;
	time.timeInMs   += delay->timeInMs;
	if (time.timeInMs >= 86400000){ // 1 or more days in msec
		time.timeInDays += time.timeInMs / 86400000;
		time.timeInMs %= 86400000;
	}

	return  ScheduleInternal(&time, evt, triggerTimeOut);
	//using ScheduleFn = BOOL(TimeEvent* createArgs, const GameTime *time, const GameTime *curTime, GameTime *pTriggerTimeOut, const char *sourceFile, int sourceLine);
	//static auto timeevent_add_ex = temple::GetPointer<ScheduleFn>(0x10060720);

	//return timeevent_add_ex(evt, delay, baseTime, triggerTimeOut, sourceFile, sourceLine) == TRUE;
}

bool TimeEventSystem::ScheduleInternal(GameTime * time, TimeEvent * evt, GameTime * triggerTimeOut){
	

	if (!evt)
		return false;

	if ((int)evt->system > (int)TimeEventType::TimeEventSystemCount)
		return false;

	auto timeEvtIsEditor = temple::GetRef<int>(0x10AA73F4);
	if (timeEvtIsEditor)
		return false;

	if (time->timeInMs==0){
		time->timeInMs = 1;
	}
	auto &sysSpec = sTimeEventTypeSpecs[(int)evt->system];

	evt->time = *time;

	auto newEntry = new TimeEventListEntry;
	if (!newEntry) exit(1);
	// build the new entry
	newEntry->evt = *evt;
	// store object references
	for (auto i = 0; i < 4; i++) {
		if (sysSpec.argTypes[i] != TimeEventArgType::Object) {
			newEntry->objects[i].guid.subtype = ObjectIdKind::Null;
			continue;
		}
		newEntry->objects[i] = FrozenObjRef::Freeze(evt->params[i].handle);
	}
	
	TimeEventListEntry**  evtList = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73FC)[(int)sysSpec.clock];
	
	auto isSpecialScheduling = temple::GetRef<BOOL>(0x10AA83E0);
	if (IsInAdvanceTime() && !isSpecialScheduling){
		evtList = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73E8)[(int)sysSpec.clock];
	}

	// insert event to the list (sorting from earliest to latest)
	while (*evtList){
		auto node = *evtList;
		if (node->evt.time.timeInDays > evt->time.timeInDays
			|| (node->evt.time.timeInDays >= evt->time.timeInDays) 
			&& node->evt.time.timeInMs >= evt->time.timeInMs){
			break;
		}
		evtList = &node->nextEvent;
	}
	newEntry->nextEvent = *evtList;
	*evtList = newEntry;

	
	
	if (triggerTimeOut){
		*triggerTimeOut = evt->time;
	}

	return true;
}

BOOL TimeEventSystem::TimeEventListEntryAdd(TimeEventListEntry * evt){
	if (!evt)
		return FALSE;

	auto subSys = evt->evt.system;
	auto &sysSpec = GetTimeEventTypeSpec(subSys);
	auto clockType = sysSpec.clock;



	TimeEventListEntry ** evtList;
	auto isAdvancingTime = temple::GetRef<int>(0x10AA83DC);
	if (isAdvancingTime)
		evtList = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73E8)[(int)clockType];
	else
		evtList = &temple::GetRef<TimeEventListEntry*[]>(0x10AA73FC)[(int)clockType];

	// find where to insert the event
	auto listNode = *evtList;
	while (listNode) {
		if (listNode->evt.time.timeInDays > evt->evt.time.timeInDays
			|| listNode->evt.time.timeInDays >= evt->evt.time.timeInDays
			&& listNode->evt.time.timeInMs >= evt->evt.time.timeInMs)
			break;
		evtList = &listNode->nextEvent;
		listNode = listNode->nextEvent;
	}

	evt->nextEvent = *evtList;

	// convert obj handles to persistable references
	for (auto i = 0; i < 4; i++) {
		if (sysSpec.argTypes[i] == TimeEventArgType::Object) {
			evt->objects[i] = FrozenObjRef::Freeze(evt->evt.params[i].handle);
		} else {
			evt->objects[i].guid.subtype = ObjectIdKind::Null;
		}
	}

	*evtList = evt;

	return TRUE;
}

BOOL TimeEventSystem::TimeEventReadFromFile(TioFile * file, TimeEvent * evtOut){

	if (!file)
		return FALSE;

	if (!tio_fread(&evtOut->time, sizeof(GameTime), 1, file)
		|| !tio_fread(&evtOut->system, sizeof(int), 1, file))
		return FALSE;

	auto subSys = evtOut->system;
	auto &sysSpec = sTimeEventTypeSpecs[(int)subSys];
	objHndl handle;

	for (auto i = 0; i < 4; i++) {
		auto par = &evtOut->params[i];
		switch (sysSpec.argTypes[i]) {
		case TimeEventArgType::Int:
			if (!tio_fread(par, sizeof(int), 1, file))
				return FALSE;
			break;
		case TimeEventArgType::Float:
			if (!tio_fread(par, sizeof(float), 1, file))
				return FALSE;
			break;
		case TimeEventArgType::Location:
			if (!tio_fread(par, sizeof(locXY), 1, file))
				return FALSE;
			break;
		case TimeEventArgType::Object:
			if (evtOut->system == TimeEventType::Lock) {
				auto dmmy = 1;
			}
			if (!FrozenObjRef::Load(&handle, nullptr, file)) {
				return FALSE;
			}
			par->handle = handle;
			break;
		case TimeEventArgType::PythonObject:
			if (!temple::GetRef<BOOL(__cdecl)(void*, TioFile*)>(0x100AD7C0)(par, file))
				return FALSE;
			break;
		case TimeEventArgType::None:
			break;
		default:
			logger->error("Undefined parameter type!");
			break;
		}
	}

	return TRUE;
}

BOOL TimeEventSystem::TimeEventParamSerializer(TioFile * file, const TimeEventTypeSpec & sysSpec, TimeEventListEntry * listEntry){

	if (!file)
		return FALSE;
	if (!tio_fwrite(&listEntry->evt.time, sizeof(GameTime), 1, file)
		|| !tio_fwrite(&listEntry->evt.system, sizeof(TimeEventType), 1, file))
		return FALSE;

	for (auto i = 0; i < 4; i++) {
		auto &evtArg = listEntry->evt.params[i];
		auto objInfo = &listEntry->objects[i];
		switch (sysSpec.argTypes[i]) {
			case TimeEventArgType::Int:
			case TimeEventArgType::Float:
				if (!tio_fwrite(&listEntry->evt.params[i], sizeof(int), 1, file))
					return FALSE;
				break;
			case TimeEventArgType::Location:
				if (!tio_fwrite(&listEntry->evt.params[i].location, sizeof(locXY), 1, file))
					return FALSE;
				break;
			case TimeEventArgType::Object:
				if (listEntry->evt.system == TimeEventType::Lock) {
					auto dummy = 1;
				}
				if (!FrozenObjRef::Save(listEntry->evt.params[i].handle, objInfo, file) )
					return FALSE;
				break;
			case TimeEventArgType::PythonObject:
				temple::GetRef<void(__cdecl)(PyObject*, TioFile *)>(0x100AD600)(listEntry->evt.params[i].pyobj, file);
				break;
			case TimeEventArgType::None:
				break;
			default:
				logger->error("Undefined time event arg type");
				break;
		}
	}

	return TRUE;
}

bool TimeEventSystem::IsInAdvanceTime()
{
	return temple::GetRef<BOOL>(0x10AA83DC)!=0;
}

BOOL TimeEventHooks::TimeEventListEntryAdd(TimeEventListEntry * evt) {
	return gameSystems->GetTimeEvent().TimeEventListEntryAdd(evt);
}

BOOL TimeEventHooks::TimeEventReadFromFile(TioFile * file, TimeEvent * evtOut){
	return gameSystems->GetTimeEvent().TimeEventReadFromFile(file, evtOut);
}

bool TimeEventListEntry::ObjHandlesValid(){
	auto sysSpec = sTimeEventTypeSpecs[(int)this->evt.system];

	for (auto i=0; i < 4; i++){
		if (sysSpec.argTypes[i] != TimeEventArgType::Object)
			continue;
		if (!objSystem->IsValidHandle(this->evt.params[i].handle))
			return false;
	}

	return true;
}

bool TimeEventListEntry::IsValid(int isLoadingMap){

	auto evt = this;

	auto handle = objHndl::null;

	for (auto i = 0; i < 4; i++) {
		auto objKind = evt->objects[i].guid.subtype;
		auto isNull = evt->objects[i].guid.subtype == ObjectIdKind::Null;
		if ((uint16_t)objKind >= 4 && (uint16_t)objKind < 0xFFFE) {
			logger->debug("Illegal object Kind caught in TimeEventValidate");
		}

		auto &parVal = evt->evt.params[i];
		handle = parVal.handle;
		if (isNull) {
			// auto hasValidHandleAnyway = false;
			if (sTimeEventTypeSpecs[(int)evt->evt.system].argTypes[i] == TimeEventArgType::Object) {
				evt->evt.params[i].handle = objHndl::null;

				if (handle) {
					logger->warn("Non-null handle for ObjectIdKind::Null in TimeEventValidate: {}", handle);
					/*if (objSystem->IsValidHandle(handle)){
					hasValidHandleAnyway = true;
					evt->evt.params[i].handle = handle;
					}*/
				}
			}
			continue;
		}

		if (sTimeEventTypeSpecs[(int)evt->evt.system].argTypes[i] == TimeEventArgType::Object && !handle) {
			logger->debug("Null object handle for GUID {}", evt->objects[i].guid);
		}

		if (handle || isLoadingMap) {
			if (gameSystems->GetMap().IsClearingMap()) {
				return false;
			}
			if (objSystem->IsValidHandle(handle)) {
				if (!handle || objSystem->GetObject(handle)->GetFlags() & OF_DESTROYED) {
					handle = objHndl::null;
					if (evt->evt.system == TimeEventType::ObjFade) {
						auto dummy = 1;
					}
					logger->debug("Destroyed object caught in TimeEvent IsValidHandle");
					return false;
				}
				continue;
			}

			if (FrozenObjRef::Unfreeze(evt->objects[i], &handle)) {
				evt->evt.params[i].handle = handle;
				if (evt->evt.system == TimeEventType::Lock) {
					auto dummy = 1;
				}
				if (!handle || objSystem->GetObject(handle)->GetFlags() & OF_DESTROYED) {
					// logger->debug("Destroyed object caught in validateRecovery");
					return false;
				}
				continue;
			}
			else {
				if (evt->evt.system == TimeEventType::Lock) {
					auto dummy = 1;
				}
				auto tryAgain = objSystem->GetHandleById(evt->objects[i].guid);
				evt->evt.params[i].handle = objHndl::null;
				logger->debug("TImeEvent: Error: Object validate recovery Failed. TE-Type: {}", (int)evt->evt.system);
				return false;
			}

		}

	}

	return true;
}
