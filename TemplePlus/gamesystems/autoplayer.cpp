#include "stdafx.h"
#include "autoplayer.h"
#include "python/python_integration_obj.h"
#include <gametime.h>
#include <python/python_time.h>

AutoplayerSystem::AutoplayerSystem()
{
}

AutoplayerSystem::~AutoplayerSystem()
{
}

void AutoplayerSystem::Reset()
{
	auto args = Py_BuildValue("()");
	pythonObjIntegration.ExecuteScript("templeplus.autoplayer", "reset", args);
	Py_DECREF(args);
}

//bool AutoplayerSystem::SaveGame(TioFile* file)
//{
//	return true;
//}
//
//bool AutoplayerSystem::LoadGame(GameSystemSaveFile* saveFile)
//{
//	return true;
//}

void AutoplayerSystem::AdvanceTime(uint32_t newTimeMs)
{
	constexpr int MIN_TIME_DELTA_MS = 100; // minimum time between firing time events script calls
	
	auto& timePlayed = temple::GetRef<GameTime>(0x10AA83B8);
	auto& gameTimeElapsed = temple::GetRef<GameTime>(0x10AA83C0);
	auto& animTime = temple::GetRef<GameTime>(0x10AA83C8);

	static uint32_t lastPingMs = 0;
	

	auto timeDeltaMs = newTimeMs - lastPingMs;

	// limit time step to 250ms
	if (timeDeltaMs > 250) {
		timeDeltaMs = 250;
	}
	auto isEventTime = timeDeltaMs > MIN_TIME_DELTA_MS;
	
	// advanced time elapsed and anim time
	//auto& timeAdvanceBlockerCount = temple::GetRef<int>(0x10AA83D8); // count of windows that block fidget animations / time advance

	// expire events whose time has come (executing their expired callback)
	if (isEventTime)
	{
		// update last ping
		lastPingMs = newTimeMs;

		GameTime* time = &timePlayed;
		
		//auto now = TigGetSystemTime(); // for debug
		
		/*if (TigElapsedSystemTime(now) > 100) {
			logger->trace("Slow callback detected on system {}",(int) node->evt.system);
		}*/
		auto pytime = PyTimeStamp_Create(*time);
		auto args = Py_BuildValue("(O)", pytime);
		pythonObjIntegration.ExecuteScript("templeplus.autoplayer", "advance_time", args);
		Py_DECREF(pytime);
		Py_DECREF(args);
		
	}
	
}

const std::string& AutoplayerSystem::GetName() const
{
	static std::string name("Autoplayer");
	return name;
}
