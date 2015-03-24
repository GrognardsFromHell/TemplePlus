
#pragma once

#include "addresses.h"
#include "tio.h"

struct GameSystemConf {
	bool editor;
	int width;
	int height;
	int bufferstuffIdx;
	int field_10;
	int renderfunc;
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

struct GameSystemFuncs : AddressTable {

	void NewInit(const GameSystemConf &conf);
	bool (__cdecl *Init)(GameSystemConf *conf);
	bool (__cdecl *LoadModule)(const char *name);
	void (__cdecl *UnloadModule)();
	void (__cdecl *Shutdown)();

	GameSystemFuncs() {
		rebase(Init, 0x10004C40);
		rebase(LoadModule, 0x10005480);
		rebase(UnloadModule, 0x10004280);
		rebase(Shutdown, 0x10001F30);
	}
};

extern GameSystemFuncs gameSystemFuncs;

// Register hooks for game system events
class GameSystemHooks {
public:
	static void AddInitHook(std::function<void(const GameSystemConf*)> callback, bool before = false);
	static void AddResetHook(std::function<void()> callback, bool before = false);
	static void AddModuleLoadHook(std::function<void()> callback, bool before = false);
	static void AddModuleUnloadHook(std::function<void()> callback, bool before = false);
	static void AddExitHook(std::function<void()> callback, bool before = false);
	static void AddAdvanceTimeHook(std::function<void(uint32_t)> callback, bool before = false);
	static void AddSaveHook(std::function<void(TioFile)> callback, bool before = false);
	static void AddLoadHook(std::function<void(GameSystemSaveFile*)> callback, bool before = false);
	static void AddResizeBuffersHook(std::function<void(RebuildBufferInfo*)> callback, bool before = false);
};
