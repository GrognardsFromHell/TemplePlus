
#include "stdafx.h"

#include <temple/dll.h>
#include "util/fixes.h"
#include "config.h"

struct GameConfig;
using GameConfigChangedCallback = void(*)();

static class ConfigHooks : public TempleFix {

	void apply() override;

	static void Init(GameConfig *config, const char *filename);
	static void AddSetting(GameConfig *config,
		const char *key,
		const char *defaultValue,
		GameConfigChangedCallback changedCallback);
	static const char *GetString(const GameConfig *config, const char *key);
	static void SetString(GameConfig *config, const char *key, const char *value);
	static int GetInt(const GameConfig *config, const char *key);
	static void SetInt(GameConfig *config, const char *key, int value);
	static void Load(GameConfig *config);
	static void Save(GameConfig *config);

} hooks;

void ConfigHooks::apply() {
	replaceFunction(0x10086E30, Init);
	replaceFunction(0x100870F0, Load);
	replaceFunction(0x10086EA0, Save);
	replaceFunction(0x10086FA0, AddSetting);
	replaceFunction(0x100871F0, GetInt);
	replaceFunction(0x100870B0, GetString);
	replaceFunction(0x100871C0, SetInt);
	replaceFunction(0x10087000, SetString);
}

void ConfigHooks::Init(GameConfig* config, const char* filename) {
}

void ConfigHooks::AddSetting(GameConfig*, const char * key, const char * defaultValue, GameConfigChangedCallback changedCallback)
{
	config.AddVanillaSetting(key, defaultValue, changedCallback);
}

const char * ConfigHooks::GetString(const GameConfig*, const char * key)
{	
	static std::string sSetting;

	sSetting = config.GetVanillaString(key);
	return sSetting.c_str();
}

void ConfigHooks::SetString(GameConfig*, const char * key, const char * value)
{
	config.SetVanillaString(key, value);
	config.Save();
}

int ConfigHooks::GetInt(const GameConfig*, const char * key)
{
	return config.GetVanillaInt(key);
}

void ConfigHooks::SetInt(GameConfig*, const char * key, int value)
{
	config.SetVanillaInt(key, value);
	config.Save();
}

void ConfigHooks::Load(GameConfig*) {
}

void ConfigHooks::Save(GameConfig*) {
}
