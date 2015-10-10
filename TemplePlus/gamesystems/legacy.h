
#pragma once

#include <temple/dll.h>
#include <game_config.h>
#include <tig/tig_mes.h>
#include <tig/tig_texture.h>

#include "tio/tio.h"

#pragma pack(push, 1)

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
#pragma pack(pop)

class Graphics;

struct GameSystemConf;

struct GameSystemFuncs : temple::AddressTable {
	bool(__cdecl *Init)(GameSystemConf *conf);
	bool(__cdecl *LoadModule)(const char *name);
	void(__cdecl *UnloadModule)();
	void(__cdecl *Shutdown)();

	GameSystemFuncs() {
		rebase(Init, 0x10004C40);
		rebase(LoadModule, 0x10005480);
		rebase(UnloadModule, 0x10004280);
		rebase(Shutdown, 0x10001F30);
	}
};

struct GameSystemInitTable : temple::AddressTable {

	// Called when config setting for game difficulty changes
	GameConfigChangedCallback DifficultyChanged;

	// Used by module load to determine whether the module has already been loaded
	bool *moduleLoaded;

	// These functions are used to propagate initial config values to the corresponding subsystems
	void(__cdecl *SetDrawHp)(bool drawHp);
	void(__cdecl *SetEndTurnDefault)(int endTurnDefault);
	void(__cdecl *SetEndTurnTime)(int endTurnTime);
	void(__cdecl *SetConcurrentTurnbased)(int value);
	void(__cdecl *SetClothFrameSkip)(int value);
	void(__cdecl *SetEnvMapping)(int value);
	void(__cdecl *SetParticleFidelity)(float value); // Value is clamped to [0,1]

	void(__cdecl *DestroyPlayerObject)();
	void(__cdecl *EndGame)();

	// These two callbacks are basically used by create buffers/free buffers to callback into the game layer (above tig layer)
	void(*GameCreateVideoBuffers)();
	void(*GameFreeVideoBuffers)();
	void(*GameDisableDrawingForce)();
	void(*GameEnableDrawing)();
	void(*CreateTownmapFogBuffer)();
	void(*ReleaseTownmapFogBuffer)();
	void(*ReleaseShadowMapBuffer)();
	void(*CreateShadowMapBuffer)();

	bool(__cdecl *TigWindowBufferstuffCreate)(int *bufferStuffIdx);
	void(__cdecl *TigWindowBufferstuffFree)(int bufferStuffIdx);

	// Initializes several random tables, vectors and shuffled int lists as well as the lightning material.
	void(__cdecl *InitPfxLightning)();

	// Indicates that the game was not installed partially and the CD is not needed for dat files
	bool *fullInstall;

	// This is set to true if the language is "en". Purpose is yet unknown other than it being read by tig_font
	bool *fontIsEnglish;

	// Global copy of the game system config, mostly read by game systems themselves
	GameSystemConf *gameSystemConf;

	// Handle to the id->filename table for meshes
	MesHandle *meshesMes;

	// State relating to the ironman mode
	bool *ironmanFlag;
	char **ironmanSaveGame;

	// The exact purpose of the scratchbuffer is unknown at this point
	TigBuffer **scratchBuffer;

	// It's not even safe to assume these rects belong to the scratchbuffer
	TigRect *scratchBufferRect;
	TigRect *scratchBufferRectExtended;

	/*
	Stores the last timestamp when the game systems were processed.
	Sadly, this is used from somewhere deep in the obj system.
	*/
	uint32_t *lastAdvanceTime;

	GameSystemInitTable() {
		rebase(DifficultyChanged, 0x10003770);
		rebase(SetDrawHp, 0x1002B910);
		rebase(SetEndTurnDefault, 0x10092280);
		rebase(SetEndTurnTime, 0x100922A0);
		rebase(SetConcurrentTurnbased, 0x10092260);
		rebase(SetParticleFidelity, 0x101E7810);
		rebase(SetClothFrameSkip, 0x102629C0);
		rebase(SetEnvMapping, 0x101E0A20);
		rebase(InitPfxLightning, 0x10087220);
		rebase(DestroyPlayerObject, 0x1006EEF0);
		rebase(EndGame, 0x1014E160);

		rebase(moduleLoaded, 0x10307054);
		rebase(fullInstall, 0x10306F48);
		rebase(fontIsEnglish, 0x10EF2E5C);
		rebase(gameSystemConf, 0x103072A0);
		rebase(meshesMes, 0x10307168);
		rebase(ironmanFlag, 0x103072B8);
		rebase(ironmanSaveGame, 0x103072C0);
		rebase(scratchBuffer, 0x103072DC);
		rebase(scratchBufferRect, 0x1030727C);
		rebase(scratchBufferRectExtended, 0x10307290);
		rebase(lastAdvanceTime, 0x11E726AC);

		rebase(GameCreateVideoBuffers, 0x10001370);
		rebase(GameFreeVideoBuffers, 0x100013A0);
		rebase(GameDisableDrawingForce, 0x100027D0);
		rebase(CreateTownmapFogBuffer, 0x1002C220);
		rebase(ReleaseTownmapFogBuffer, 0x1002C270);
		rebase(CreateShadowMapBuffer, 0x1001D390);
		rebase(ReleaseShadowMapBuffer, 0x1001D4B0);
		rebase(TigWindowBufferstuffCreate, 0x10113EB0);
		rebase(TigWindowBufferstuffFree, 0x101DF2C0);

	}
};


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

struct LegacyGameSystems {
	GameSystem systems[gameSystemCount];
};

extern temple::GlobalStruct<LegacyGameSystems, 0x102AB368> legacyGameSystems;
extern GameSystemFuncs gameSystemFuncs;
extern GameSystemInitTable gameSystemInitTable;
