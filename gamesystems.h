
#pragma once

#include "util/addresses.h"
#include "tio/tio.h"

struct GameSystemConf {
	bool editor;
	int width;
	int height;
	int bufferstuffIdx;
	void *field_10;
	void *renderfunc;
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

	/*
		Called during the main loop.
	*/
	void AdvanceTime();

	/*
		Used to notify game systems about an updated screen size.
	*/
	void ResizeScreen(int w, int h);

	bool (__cdecl *Init)(GameSystemConf *conf);
	bool (__cdecl *LoadModule)(const char *name);
	void (__cdecl *UnloadModule)();
	void (__cdecl *Shutdown)();

	/*
		Makes a savegame.
	*/
	bool SaveGame(const string &filename, const string &displayName);

	/*
		Loads a game.
	*/
	bool LoadGame(const string &filename);

	/*
		Ends the game, resets the game systems and returns to the main menu.
	*/
	void EndGame();

	/*
		Call this before loading a game. Use not yet known.
		TODO I do NOT think this is used, should be checked. Seems like leftovers from even before arcanum
	*/
	void DestroyPlayerObject();

	GameSystemFuncs() {
		rebase(Init, 0x10004C40);
		rebase(LoadModule, 0x10005480);
		rebase(UnloadModule, 0x10004280);
		rebase(Shutdown, 0x10001F30);
	}
};

extern GameSystemFuncs gameSystemFuncs;

void GameSystemsRender();
