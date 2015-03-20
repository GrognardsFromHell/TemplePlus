#include "stdafx.h"
#include "gamesystems.h"
#include "fixes.h"
#include "temple_functions.h"

/*
FieldDefs *fieldDefTable = (FieldDefs*)0x10B3D7D8;
for (auto field : fieldDefTable->fields) {
LOG(info) << "FIELD!";
}
*/

typedef bool (__cdecl *GameSystemInit)(GameSystemConf* conf);
typedef void (__cdecl *GameSystemReset)();
typedef bool (__cdecl *GameSystemModuleLoad)();
typedef void (__cdecl *GameSystemModuleUnload)();
typedef void (__cdecl *GameSystemExit)();
typedef void (__cdecl *GameSystemAdvanceTime)(time_t time);
typedef bool (__cdecl *GameSystemSave)(TioFile file);
typedef bool (__cdecl *GameSystemLoad)(GameSystemSaveFile* saveFile);
typedef void (__cdecl *GameSystemResetBuffers)(RebuildBufferInfo* rebuildInfo);

struct GameSystem {
	const char* name;
	GameSystemInit init;
	GameSystemReset reset;
	GameSystemModuleLoad moduleLoad;
	GameSystemModuleUnload moduleUnload;
	GameSystemExit exit;
	GameSystemAdvanceTime advanceTime;
	uint32_t field1c;
	GameSystemSave save;
	GameSystemLoad load;
	GameSystemResetBuffers resetBuffers;
	uint32_t loadscreenMesIdx;
};

struct GameSystems {
	GameSystem systems[61];
};

static GlobalStruct<GameSystems, 0x102AB368> gameSystems;
static GameSystem orgFirstSystem;
static GameSystem orgLastSystem;

static vector<std::function<void(GameSystemConf*)>> beforeInitCallbacks;
static vector<std::function<void(GameSystemConf*)>> afterInitCallbacks;
static vector<std::function<void()>> beforeResetCallbacks;
static vector<std::function<void()>> afterResetCallbacks;
static vector<std::function<void()>> beforeModuleLoadCallbacks;
static vector<std::function<void()>> afterModuleLoadCallbacks;
static vector<std::function<void()>> beforeModuleUnloadCallbacks;
static vector<std::function<void()>> afterModuleUnloadCallbacks;
static vector<std::function<void()>> beforeExitCallbacks;
static vector<std::function<void()>> afterExitCallbacks;
static vector<std::function<void(time_t)>> beforeAdvanceTimeCallbacks;
static vector<std::function<void(time_t)>> afterAdvanceTimeCallbacks;
static vector<std::function<void(TioFile)>> beforeSaveCallbacks;
static vector<std::function<void(TioFile)>> afterSaveCallbacks;
static vector<std::function<void(GameSystemSaveFile*)>> beforeLoadCallbacks;
static vector<std::function<void(GameSystemSaveFile*)>> afterLoadCallbacks;
static vector<std::function<void(RebuildBufferInfo*)>> beforeResizeBuffersCallbacks;
static vector<std::function<void(RebuildBufferInfo*)>> afterResizeBuffersCallbacks;

struct BeforeCallbacks {
	static bool __cdecl Init(GameSystemConf* conf) {
		LOG(info) << "EVENT: Before Init";

		for (auto& callback : beforeInitCallbacks) {
			callback(conf);
		}

		if (orgFirstSystem.init) {
			return orgFirstSystem.init(conf);
		}
		return true;
	}

	static void __cdecl Reset() {
		LOG(info) << "EVENT: Before Reset";

		for (auto& callback : beforeResetCallbacks) {
			callback();
		}

		if (orgFirstSystem.reset) {
			orgFirstSystem.reset();
		}
	}

	static bool __cdecl ModuleLoad() {
		LOG(info) << "EVENT: Before Module Load";

		for (auto& callback : beforeModuleLoadCallbacks) {
			callback();
		}

		if (orgFirstSystem.moduleLoad) {
			return orgFirstSystem.moduleLoad();
		}
		return true;
	}

	static void __cdecl ModuleUnload() {
		LOG(info) << "EVENT: Before Module Unload";

		for (auto& callback : beforeModuleUnloadCallbacks) {
			callback();
		}

		if (orgFirstSystem.moduleUnload) {
			orgFirstSystem.moduleUnload();
		}
	}

	static void __cdecl Exit() {
		LOG(info) << "EVENT: Before Exit";

		for (auto& callback : beforeExitCallbacks) {
			callback();
		}

		if (orgFirstSystem.exit) {
			orgFirstSystem.exit();
		}
	}

	static void __cdecl AdvanceTime(time_t time) {
		// This spams the log a lot
		// LOG(info) << "EVENT: Before Advance Time";
		for (auto& callback : beforeAdvanceTimeCallbacks) {
			callback(time);
		}

		if (orgFirstSystem.advanceTime) {
			orgFirstSystem.advanceTime(time);
		}
	}

	static bool __cdecl Save(TioFile file) {
		LOG(info) << "EVENT: Before Save";

		for (auto& callback : beforeSaveCallbacks) {
			callback(file);
		}

		if (orgFirstSystem.save) {
			return orgFirstSystem.save(file);
		}
		return true;
	}

	static bool __cdecl Load(GameSystemSaveFile* file) {
		LOG(info) << "EVENT: Before Load";

		for (auto& callback : beforeLoadCallbacks) {
			callback(file);
		}

		if (orgFirstSystem.load) {
			return orgFirstSystem.load(file);
		}
		return true;
	}

	static void __cdecl ResetBuffers(RebuildBufferInfo* info) {
		LOG(info) << "EVENT: Before Reset Buffers";

		for (auto& callback : beforeResizeBuffersCallbacks) {
			callback(info);
		}

		if (orgFirstSystem.resetBuffers) {
			orgFirstSystem.resetBuffers(info);
		}
	}
};

struct AfterCallbacks {
	static bool __cdecl Init(GameSystemConf* conf) {
		bool result = true;
		if (orgFirstSystem.init) {
			result = orgFirstSystem.init(conf);
		}

		LOG(info) << "EVENT: After Init";

		for (auto& callback : afterInitCallbacks) {
			callback(conf);
		}

		/*
		
int NumberOfSetBits(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
		struct FieldDef {
			int protoPropIndex;
			int field4;
			int PropBitmap_idx1;
			uint32_t PropBitmask;
			int PropBitmap_idx2;
			uint32_t IsStoredInPropCollection;
			uint32_t FieldTypeCode;
		};

		struct FieldDefs {
			FieldDef fields[430];
		};

		const char **typeNames = (const char**)0x102CD7F8;
		const char **fieldNames = (const char**)0x102CD840;
		FieldDefs *fieldDefTable = *(FieldDefs**)0x10B3D7D8;
		string typeNamesHeader;
		for (int j = 0; j <= 16; ++j) {
			typeNamesHeader.append(";").append(typeNames[j]);
		}

		LOG(info) << "fieldname;proto idx;field4;bitmapidx1;bitmask;bitmapidx2;stored_in_prop_collection;field_type" << typeNamesHeader;
		int i = 0;
		for (auto field : fieldDefTable->fields) {
			string typeSupport;
			for (int j = 0; j <= 16; ++j) {
				if (templeFuncs.DoesTypeSupportField(j, i)) {
					typeSupport.append(";X");
				} else {
					typeSupport.append(";");
				}
			}

			LOG(info)
				<< fieldNames[i++] << ";"
				<< field.protoPropIndex << ";" << field.field4 << ";" << field.PropBitmap_idx1
				<< ";" << format("0x%x") % field.PropBitmask << ";" << field.PropBitmap_idx2
				<< ";" << field.IsStoredInPropCollection << ";" << field.FieldTypeCode << typeSupport;
		}*/

		return result;
	}

	static void __cdecl Reset() {
		if (orgFirstSystem.reset) {
			orgFirstSystem.reset();
		}

		LOG(info) << "EVENT: After Reset";
		for (auto& callback : afterResetCallbacks) {
			callback();
		}
	}

	static bool __cdecl ModuleLoad() {
		bool result = true;
		if (orgFirstSystem.moduleLoad) {
			result = orgFirstSystem.moduleLoad();
		}
		LOG(info) << "EVENT: After Module Load";
		for (auto& callback : afterModuleLoadCallbacks) {
			callback();
		}
		return result;
	}

	static void __cdecl ModuleUnload() {
		if (orgFirstSystem.moduleUnload) {
			orgFirstSystem.moduleUnload();
		}
		LOG(info) << "EVENT: After Module Unload";
		for (auto& callback : afterModuleUnloadCallbacks) {
			callback();
		}
	}

	static void __cdecl Exit() {
		if (orgFirstSystem.exit) {
			orgFirstSystem.exit();
		}
		LOG(info) << "EVENT: After Exit";
		for (auto& callback : afterExitCallbacks) {
			callback();
		}
	}

	static void __cdecl AdvanceTime(time_t time) {
		if (orgFirstSystem.advanceTime) {
			orgFirstSystem.advanceTime(time);
		}
		// Logspam
		// LOG(info) << "EVENT: After Advance Time";
		for (auto& callback : afterAdvanceTimeCallbacks) {
			callback(time);
		}
	}

	static bool __cdecl Save(TioFile file) {
		bool result = true;
		if (orgFirstSystem.save) {
			result = orgFirstSystem.save(file);
		}
		LOG(info) << "EVENT: After Save";
		for (auto& callback : afterSaveCallbacks) {
			callback(file);
		}
		return result;
	}

	static bool __cdecl Load(GameSystemSaveFile* file) {
		bool result = true;
		if (orgFirstSystem.load) {
			result = orgFirstSystem.load(file);
		}
		LOG(info) << "EVENT: After Load";
		for (auto& callback : afterLoadCallbacks) {
			callback(file);
		}
		return result;
	}

	static void __cdecl ResetBuffers(RebuildBufferInfo* info) {
		if (orgFirstSystem.resetBuffers) {
			orgFirstSystem.resetBuffers(info);
		}
		LOG(info) << "EVENT: After Reset Buffers";
		for (auto& callback : afterResizeBuffersCallbacks) {
			callback(info);
		}
	}
};

/*
	Overrides the first and last game system so we can provide before/after hooks for all events.
*/
class GameSystemHookInitializer : TempleFix {
public:
	const char* name() override {
		return "Gamesystem Hooks";
	}

	void apply() override {
		// Override all functions for "before" callbacks
		auto& firstSystem = gameSystems->systems[0];
		orgFirstSystem = firstSystem;
		firstSystem.init = &BeforeCallbacks::Init;
		firstSystem.reset = &BeforeCallbacks::Reset;
		firstSystem.moduleLoad = &BeforeCallbacks::ModuleLoad;
		firstSystem.moduleUnload = &BeforeCallbacks::ModuleUnload;
		firstSystem.exit = &BeforeCallbacks::Exit;
		firstSystem.advanceTime = &BeforeCallbacks::AdvanceTime;
		firstSystem.save = &BeforeCallbacks::Save;
		firstSystem.load = &BeforeCallbacks::Load;
		firstSystem.resetBuffers = &BeforeCallbacks::ResetBuffers;

		// Override all functions for "before" callbacks
		auto& lastSystem = gameSystems->systems[60];
		orgLastSystem = lastSystem;
		lastSystem.init = &AfterCallbacks::Init;
		lastSystem.reset = &AfterCallbacks::Reset;
		lastSystem.moduleLoad = &AfterCallbacks::ModuleLoad;
		lastSystem.moduleUnload = &AfterCallbacks::ModuleUnload;
		lastSystem.exit = &AfterCallbacks::Exit;
		lastSystem.advanceTime = &AfterCallbacks::AdvanceTime;
		lastSystem.save = &AfterCallbacks::Save;
		lastSystem.load = &AfterCallbacks::Load;
		lastSystem.resetBuffers = &AfterCallbacks::ResetBuffers;
	}

} gameSystemHookInitializer;

void GameSystemHooks::AddInitHook(std::function<void(GameSystemConf*)> callback, bool before) {
	if (before) {
		beforeInitCallbacks.push_back(callback);
	} else {
		afterInitCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddResetHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeResetCallbacks.push_back(callback);
	} else {
		afterResetCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddModuleLoadHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeModuleLoadCallbacks.push_back(callback);
	} else {
		afterModuleLoadCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddModuleUnloadHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeModuleUnloadCallbacks.push_back(callback);
	} else {
		afterModuleUnloadCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddExitHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeExitCallbacks.push_back(callback);
	} else {
		afterExitCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddAdvanceTimeHook(std::function<void(time_t)> callback, bool before) {
	if (before) {
		beforeAdvanceTimeCallbacks.push_back(callback);
	} else {
		afterAdvanceTimeCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddSaveHook(std::function<void(TioFile)> callback, bool before) {
	if (before) {
		beforeSaveCallbacks.push_back(callback);
	} else {
		afterSaveCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddLoadHook(std::function<void(GameSystemSaveFile*)> callback, bool before) {
	if (before) {
		beforeLoadCallbacks.push_back(callback);
	} else {
		afterLoadCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddResizeBuffersHook(std::function<void(RebuildBufferInfo*)> callback, bool before) {
	if (before) {
		beforeResizeBuffersCallbacks.push_back(callback);
	} else {
		afterResizeBuffersCallbacks.push_back(callback);
	}
}
