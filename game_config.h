
#pragma once

#include <temple/dll.h>

struct GameConfigEntry;
typedef void(__cdecl *GameConfigChangedCallback)();

struct GameConfig {
	// Filename of config file
	const char *filename;

	// Pointer to head of linked list for all config entries
	GameConfigEntry *entries;

	// Indicates that the config has been changed
	bool changed;
};
struct GameConfigEntry {
	// Name of the config setting
	const char *key;
	// It's current value
	const char *value;
	// Callback to be called when the value changes (optional)
	GameConfigChangedCallback changeCallback;
	// Pointer to next entry in linked list (optional)
	GameConfigEntry *next;
};

/*
	These functions relate to toee.cfg parsing and setting default values for it, etc.
*/
struct GameConfigFuncs : temple::AddressTable {
	// Sets entries to 0 and filename to the given filename.
	void Init(const char *filename) {
		_Init(gameConfig.ptr(), filename);
	}

	/*
	Adds a supported setting to the game configuration and specifies a callback that will be called when the value changes.
	Should the setting already exist, only the callback is modified.
	*/
	void AddSetting(const char *key, const char *value, GameConfigChangedCallback changedCallback = nullptr) {
		_AddSetting(gameConfig.ptr(), key, value, changedCallback);
	}

	/*
	Gets the string value of the given config key.
	Returns nullptr if the config key does not exist.
	*/
	const char * GetString(const char *key) {
		return _GetString(gameConfig.ptr(), key);
	}

	/*
	Sets the integer configuration value of the given key.
	*/
	void SetString(const char *key, const char *value) {
		_SetString(gameConfig.ptr(), key, value);
	}

	/*
	Gets the integer configuration value of the given key.
	If the key does not exist, it returns 0.
	*/
	int GetInt(const char *key) {
		return _GetInt(gameConfig.ptr(), key);
	}

	/*
	Sets the integer configuration value of the given key.
	*/
	void SetInt(const char *key, int value) {
		_SetInt(gameConfig.ptr(), key, value);
	}

	/*
	Loads the config file. The filename is specified in config->filename. If the file doesn't exist,
	nothing happens.
	*/
	void Load() {
		_Load(gameConfig.ptr());
	}

	/*
	Saves the configuration file to the filename specified in the config.
	*/
	void Save() {
		_Save(gameConfig.ptr());
	}

	GameConfigFuncs() {
		rebase(_Init, 0x10086E30);
		rebase(_Load, 0x100870F0);
		rebase(_Save, 0x10086EA0);
		rebase(_AddSetting, 0x10086FA0);
		rebase(_GetInt, 0x100871F0);
		rebase(_GetString, 0x100870B0);
		rebase(_SetInt, 0x100871C0);
		rebase(_SetString, 0x10087000);
	}

	// Global config struct used by ToEE
	temple::GlobalStruct<GameConfig, 0x11E726A0> gameConfig;

private:
	void(__cdecl *_Init)(GameConfig *config, const char *filename);
	void(__cdecl *_AddSetting)(GameConfig *config,
		const char *key,
		const char *defaultValue,
		GameConfigChangedCallback changedCallback);
	const char *(__cdecl *_GetString)(const GameConfig *config, const char *key);
	void(__cdecl *_SetString)(GameConfig *config, const char *key, const char *value);
	int(__cdecl *_GetInt)(const GameConfig *config, const char *key);
	void(__cdecl *_SetInt)(GameConfig *config, const char *key, int value);
	void(__cdecl *_Load)(GameConfig *config);
	void(__cdecl *_Save)(GameConfig *config);

};

extern struct GameConfigFuncs gameConfigFuncs;
