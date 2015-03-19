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

struct GameSystemConf {
	bool editor;
	int width;
	int height;
	int field_c;
	int field_10;
	int field_14;
};

struct TioFile {
	const char* filename;
	uint32_t flags;
	FILE fileHandle;
	uint32_t field_c;
	uint32_t field_10;
	uint32_t field_14;
	uint32_t field_18;
	uint32_t field_1c;
	uint32_t field_20;
	uint32_t field_24;
	uint32_t field_28;
	uint32_t field_2c;
};

struct GameSystemSaveFile {
	uint32_t saveVersion;
	TioFile* file;
};

struct RebuildBufferInfo {
	uint32_t gameConfField_c;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t width;
	uint32_t height;
	uint32_t unk3;
	uint32_t unk4;
	uint32_t width2;
	uint32_t height2;
};

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

struct BeforeCallbacks {
	static bool __cdecl Init(GameSystemConf* conf) {
		LOG(info) << "EVENT: Before Init";

		if (orgFirstSystem.init) {
			return orgFirstSystem.init(conf);
		}
		return true;
	}

	static void __cdecl Reset() {
		LOG(info) << "EVENT: Before Reset";

		if (orgFirstSystem.reset) {
			orgFirstSystem.reset();
		}
	}

	static bool __cdecl ModuleLoad() {
		LOG(info) << "EVENT: Before Module Load";

		if (orgFirstSystem.moduleLoad) {
			return orgFirstSystem.moduleLoad();
		}
		return true;
	}

	static void __cdecl ModuleUnload() {
		LOG(info) << "EVENT: Before Module Unload";
		if (orgFirstSystem.moduleUnload) {
			orgFirstSystem.moduleUnload();
		}
	}

	static void __cdecl Exit() {
		LOG(info) << "EVENT: Before Exit";
		if (orgFirstSystem.exit) {
			orgFirstSystem.exit();
		}
	}

	static void __cdecl AdvanceTime(time_t time) {
		// This spams the log a lot
		// LOG(info) << "EVENT: Before Advance Time";
		if (orgFirstSystem.advanceTime) {
			orgFirstSystem.advanceTime(time);
		}
	}

	static bool __cdecl Save(TioFile file) {
		LOG(info) << "EVENT: Before Save";
		
		if (orgFirstSystem.save) {
			return orgFirstSystem.save(file);
		}
		return true;
	}

	static bool __cdecl Load(GameSystemSaveFile* file) {
		LOG(info) << "EVENT: Before Load";
		
		if (orgFirstSystem.load) {
			return orgFirstSystem.load(file);
		}
		return true;
	}

	static void __cdecl ResetBuffers(RebuildBufferInfo* info) {
		LOG(info) << "EVENT: Before Reset Buffers";
		if (orgFirstSystem.resetBuffers) {
			orgFirstSystem.resetBuffers(info);
		}
	}
};

int NumberOfSetBits(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

struct AfterCallbacks {
	static bool __cdecl Init(GameSystemConf* conf) {
		bool result = true;
		if (orgFirstSystem.init) {
			result = orgFirstSystem.init(conf);
		}

		LOG(info) << "EVENT: After Init";

		/*struct FieldDef {
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
	}

	static bool __cdecl ModuleLoad() {
		bool result = true;
		if (orgFirstSystem.moduleLoad) {
			result = orgFirstSystem.moduleLoad();
		}
		LOG(info) << "EVENT: After Module Load";
		return result;
	}

	static void __cdecl ModuleUnload() {
		if (orgFirstSystem.moduleUnload) {
			orgFirstSystem.moduleUnload();
		}
		LOG(info) << "EVENT: After Module Unload";
	}

	static void __cdecl Exit() {
		if (orgFirstSystem.exit) {
			orgFirstSystem.exit();
		}
		LOG(info) << "EVENT: After Module Exit";
	}

	static void __cdecl AdvanceTime(time_t time) {
		if (orgFirstSystem.advanceTime) {
			orgFirstSystem.advanceTime(time);
		}
		// Logspam
		// LOG(info) << "EVENT: After Advance Time";
	}

	static bool __cdecl Save(TioFile file) {
		bool result = true;
		if (orgFirstSystem.save) {
			result = orgFirstSystem.save(file);
		}
		LOG(info) << "EVENT: After Save";
		return result;
	}

	static bool __cdecl Load(GameSystemSaveFile* file) {
		bool result = true;
		if (orgFirstSystem.load) {
			result = orgFirstSystem.load(file);
		}
		LOG(info) << "EVENT: After Load";
		return result;
	}

	static void __cdecl ResetBuffers(RebuildBufferInfo* info) {
		if (orgFirstSystem.resetBuffers) {
			orgFirstSystem.resetBuffers(info);
		}
		LOG(info) << "EVENT: After Reset Buffers";
	}
};

/*
	Overrides the first and last game system so we can provide before/after hooks for all events.
*/
class GameSystemHook : TempleFix {
public:
	const char* name() override {
		return "Gamesystem Hooks";
	}

	void apply() override {
		// Override all functions for "before" callbacks
		auto &firstSystem = gameSystems->systems[0];
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
		auto &lastSystem = gameSystems->systems[60];
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

} gameSystemHook;
