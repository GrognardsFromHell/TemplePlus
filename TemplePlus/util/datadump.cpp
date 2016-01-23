
#include "stdafx.h"

#include <infrastructure/json11.hpp>
#include <temple/dll.h>
using namespace json11;
#include <fstream>
#include "gamesystems/timeevents.h"

#pragma pack(push, 1)
struct MapGameSys {
	const char *name;
	int32_t init;
	int32_t reset;
	int32_t modLoad;
	int32_t modUnload;
	int32_t shutdown;
	int32_t advanceTime;
	int32_t field1c;
	int32_t saveGame;
	int32_t loadGame;
	int32_t field28;
	int32_t mapClose;
	int32_t mapReset;
	int loadScreenMes;
};

/*
Internal system specification used by the time event system
*/
struct TimeEventSystemSpec {
	char name[20];
	BOOL persistent;
	uint32_t args;
	GameClockType clockType;
	void(__cdecl *eventExpired)();
	void(__cdecl *eventExpiredAlways)();
	int field_28;
};
#pragma pack(pop)

namespace dump {
	void DumpMapSystems() {
		MapGameSys* gameSys = temple::GetPointer<MapGameSys>(0x102BEEB0);

		std::vector<Json> systems;
		for (int i = 0; i < 18; ++i) {
			auto& sys = gameSys[i];
			systems.push_back(Json::object{
				{ "name", sys.name },
				{ "init", sys.init },
				{ "reset", sys.reset },
				{ "moduleLoad", sys.modLoad },
				{ "moduleUnload", sys.modUnload },
				{ "exit", sys.shutdown },
				{ "advanceTime", sys.advanceTime },
				{ "field1c", sys.field1c },
				{ "save", sys.saveGame },
				{ "load", sys.loadGame },
				{ "mapClose", sys.mapClose },
				{ "resetBuffers", sys.mapReset },
			});
		}
		std::ofstream dump("mapsys.json");
		dump << json11::Json(systems).dump();
		dump.close();

	}

	static uint32_t arg1Flags[]{1, 2, 4, 0x1000, 0x10000};
	static uint32_t arg2Flags[]{8, 0x10, 0x20, 0x2000, 0x20000};
	static uint32_t arg3Flags[]{0x40, 0x80, 0x100, 0x4000, 0x40000};
	static uint32_t arg4Flags[]{0x200, 0x400, 0x800, 0x8000, 0x80000};

	static void AddArgs(uint32_t argFlags, uint32_t *argMasks, std::vector<std::string> &args) {
		static const char *argTypes[] = { "TimeEventArgType::Int", 
			"TimeEventArgType::Object", 
			"TimeEventArgType::Location", 
			"TimeEventArgType::Float", 
			"TimeEventArgType::PythonObject" };
		bool result = false;
		for (size_t i = 0; i < 5; ++i) {
			if (argFlags & argMasks[i]) {
				args.push_back(argTypes[i]);
				result = true;
			}
		}
		if (!result) {
			args.push_back("");
		}
	}

	void DumpTimeEventSystems() {
		auto timerSystem = temple::GetPointer<TimeEventSystemSpec>(0x102BD900);

		std::vector<Json> systems;
		for (int i = 0; i < 38; ++i) {
			auto& sys = timerSystem[i];

			std::vector<std::string> args;
			AddArgs(sys.args, arg1Flags, args);
			AddArgs(sys.args, arg2Flags, args);
			AddArgs(sys.args, arg3Flags, args);
			AddArgs(sys.args, arg4Flags, args);
			while (!args.empty() && args.back().empty()) {
				args.erase(args.end() - 1);
			}

			const char *clockType = "unknown";
			switch (sys.clockType) {
			case GameClockType::RealTime: 
				clockType = "GameClockType::RealTime";
				break;
			case GameClockType::GameTime: 
				clockType = "GameClockType::GameTime";
				break;
			case GameClockType::GameTimeAnims:
				clockType = "GameClockType::GameTimeAnims";
				break;
			default: 
				break;
			}

			auto sysObj = Json::object{
				{ "name", sys.name },
				{ "persistent", sys.persistent == 1 },
				{ "args", args },
				{ "clock", clockType },
				{ "expiredFunc", (int32_t)sys.eventExpired }
			};
			if (sys.eventExpiredAlways) {
				sysObj["removedFunc"] = (int32_t)sys.eventExpiredAlways;
			}

			systems.push_back(sysObj);
		}
		std::ofstream dump("timeeventsystems.json");
		dump << json11::Json(systems).dump();
		dump.close();

	}

}

