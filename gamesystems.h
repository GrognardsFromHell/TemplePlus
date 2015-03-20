
#pragma once

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

// Register hooks for game system events
class GameSystemHooks {
public:
	static void AddInitHook(std::function<void(GameSystemConf*)> callback, bool before = false);
	static void AddResetHook(std::function<void()> callback, bool before = false);
	static void AddModuleLoadHook(std::function<void()> callback, bool before = false);
	static void AddModuleUnloadHook(std::function<void()> callback, bool before = false);
	static void AddExitHook(std::function<void()> callback, bool before = false);
	static void AddAdvanceTimeHook(std::function<void(time_t)> callback, bool before = false);
	static void AddSaveHook(std::function<void(TioFile)> callback, bool before = false);
	static void AddLoadHook(std::function<void(GameSystemSaveFile*)> callback, bool before = false);
	static void AddResizeBuffersHook(std::function<void(RebuildBufferInfo*)> callback, bool before = false);
};
