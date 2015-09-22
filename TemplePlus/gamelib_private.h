
#pragma once

#include "gamesystems.h"

typedef bool(__cdecl *GameSystemInit)(const GameSystemConf* conf);
typedef void(__cdecl *GameSystemReset)();
typedef bool(__cdecl *GameSystemModuleLoad)();
typedef void(__cdecl *GameSystemModuleUnload)();
typedef void(__cdecl *GameSystemExit)();
typedef void(__cdecl *GameSystemAdvanceTime)(uint32_t time);
typedef bool(__cdecl *GameSystemSave)(TioFile *file);
typedef bool(__cdecl *GameSystemLoad)(GameSystemSaveFile* saveFile);
typedef void(__cdecl *GameSystemResetBuffers)(RebuildBufferInfo* rebuildInfo);

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

const uint32_t gameSystemCount = 61;

struct GameSystems {
	GameSystem systems[gameSystemCount];
};

extern temple::GlobalStruct<GameSystems, 0x102AB368> gameSystems;
