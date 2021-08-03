﻿
#include "stdafx.h"
#include "config.h"
#include <infrastructure/INI.h>

TemplePlusConfig config;

typedef INI <std::string, std::string, std::string> ini_t;

static ini_t ini("TemplePlus.ini", false);

class ConfigSetting {
public:
    template<class T> using Setter = std::function<void(T)>;
    template<class T> using Getter = std::function<T()>;
	
	static ConfigSetting String(const char *option, Setter<std::string> setter, Getter<std::string> getter, const char *description = nullptr) {
		return ConfigSetting(option, setter, getter, description);
	}

	static ConfigSetting WString(const char *option, Setter<std::wstring> setter, Getter<std::wstring> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			return ucs2_to_utf8(getter());
		};
		auto setterAdapter = [=](std::string value) {
			setter(utf8_to_ucs2(value));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}
	static ConfigSetting StringList(const char *option, char sep, Setter<const std::vector<std::string>&> setter, Getter<std::vector<std::string>> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
		    std::string result;
			for (auto &str : getter()) {
				if (!result.empty()) {
					result += sep;
				}
				result += str;
			}
			return result;
		};
		auto setterAdapter = [=](std::string x) {
		    std::vector<std::string> result;
		    std::stringstream ss(x);
		    std::string token;
		    while (std::getline(ss, token, sep)) {
				if (!token.empty()) {
					result.push_back(token);
				}
			}
			setter(result);
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Bool(const char *option, Setter<bool> setter, Getter<bool> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			return getter() ? "true" : "false";
		};
		auto setterAdapter = [=] (std::string x) {
			setter(x == "true" || x == "1");
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Int(const char *option, Setter<int> setter, Getter<int> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
		    return std::to_string(getter());
		};
		auto setterAdapter = [=](std::string x) {
			setter(stoi(x));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Double(const char *option, Setter<double> setter, Getter<double> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
		    return std::to_string(getter());
		};
		auto setterAdapter = [=](std::string x) {
			setter(stod(x));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	const char *option() const {
		return mOption;
	}

	Setter<std::string> setter() const {
		return mSetter;
	}

	Getter<std::string> getter() const {
		return mGetter;
	}

private:
	ConfigSetting(const char *option, 
		Setter<std::string> setter,
		Getter<std::string> getter,
		const char *comment)
		: mOption(option), mComment(comment), mSetter(setter), mGetter(getter) {
	}

	const char *mOption;
	const char *mComment;
	Setter<std::string> mSetter;
	Getter<std::string> mGetter;
};

#define CONF_STRING(FIELDNAME) ConfigSetting::String(#FIELDNAME, [] (std::string val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_WSTRING(FIELDNAME) ConfigSetting::WString(#FIELDNAME, [] (std::wstring val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_STRING_LIST(FIELDNAME, SEP) ConfigSetting::StringList(#FIELDNAME, SEP, [] (const std::vector<std::string> &val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_INT(FIELDNAME) ConfigSetting::Int(#FIELDNAME, [] (int val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_BOOL(FIELDNAME) ConfigSetting::Bool(#FIELDNAME, [] (bool val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_DOUBLE(FIELDNAME) ConfigSetting::Double(#FIELDNAME, [] (double val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })

static ConfigSetting configSettings[] = {
	CONF_BOOL(skipIntro),
	CONF_BOOL(skipLegal),
	//CONF_BOOL(engineEnhancements),
	//CONF_BOOL(editor),
	CONF_BOOL(windowed),
	CONF_BOOL(antialiasing),
	CONF_BOOL(softShadows),
	CONF_BOOL(noSound),
	CONF_BOOL(featPrereqWarnings),
	CONF_BOOL(spellAlreadyKnownWarnings),
	CONF_BOOL(NPCsLevelLikePCs),
	//CONF_DOUBLE(randomEncounterExperienceFactor),
	CONF_BOOL(newFeatureTestMode),
	CONF_BOOL(pathfindingDebugMode),
	CONF_STRING(defaultModule),
	CONF_INT(windowWidth),
	CONF_INT(windowHeight),
	CONF_BOOL(enlargeDialogFonts),
	CONF_BOOL(windowedLockCursor),
	CONF_BOOL(dungeonMaster),
	CONF_INT(renderWidth),
	CONF_INT(renderHeight),
	CONF_BOOL(debugMessageEnable),
	CONF_INT(logLevel),
	CONF_BOOL(lockCursor),
	CONF_BOOL(showExactHPforNPCs),
	CONF_STRING_LIST(additionalTioPaths, ';'),
	CONF_INT(pointBuyPoints),
	CONF_INT(maxPCs),
	CONF_BOOL(maxPCsFlexible),
	CONF_INT(maxLevel),
	CONF_WSTRING(toeeDir),
	CONF_INT(sectorCacheSize),
	CONF_INT(screenshotQuality),
	CONF_BOOL(debugPartSys),
	CONF_STRING(hpOnLevelup),
	CONF_STRING(HpForNPCHd),
	CONF_BOOL(maxHpForNpcHitdice),
	CONF_BOOL(autoUpdate),
	CONF_BOOL(allowXpOverflow),
	CONF_BOOL(slowerLevelling),
	CONF_BOOL(d3dDebug),
	CONF_INT(displayAdapter),
	CONF_INT(msaaSamples),
	CONF_INT(msaaQuality),
	CONF_BOOL(showNpcStats),
	CONF_BOOL(newClasses),
	CONF_BOOL(newRaces),
	CONF_BOOL(metamagicStacking),
	CONF_BOOL(monstrousRaces),
	CONF_BOOL(forgottenRealmsRaces),
	CONF_BOOL(laxRules),
	CONF_BOOL(stricterRulesEnforcement),
	CONF_BOOL(disableAlignmentRestrictions),
	CONF_BOOL(disableCraftingSpellReqs),
	CONF_BOOL(disableMulticlassXpPenalty),
	CONF_BOOL(disableDoorRelocking),
	CONF_BOOL(showTargetingCirclesInFogOfWar),
	CONF_BOOL(nonCoreMaterials),
	CONF_BOOL(tolerantNpcs),
	CONF_STRING(fogOfWar),
	CONF_DOUBLE(speedupFactor),
	CONF_BOOL(fastSneakAnim),
	CONF_BOOL(alertAiThroughDoors),
	CONF_BOOL(preferUse5FootStep),
	CONF_BOOL(extendedSpellDescriptions),
	CONF_INT(walkDistanceFt),
	CONF_BOOL(newAnimSystem),
	CONF_BOOL(upscaleLinearFiltering),
	CONF_BOOL(disableChooseRandomSpell_RegardInvulnerableStatus),
	CONF_BOOL(wildShapeUsableItems)
};

void TemplePlusConfig::Load() {

	ini.parse();

	if (ini.select("TemplePlus")) {
		
		for (const auto &setting : configSettings) {
			auto def = setting.getter()();
			auto val = ini.get(setting.option(), def);
			setting.setter()(val);
		}

	}

	if (ini.select("Vanilla")) {

		for (auto& entry : *ini.current) {
			SetVanillaString(entry.first, entry.second);
		}

	}
}

void TemplePlusConfig::Save() {
	ini.clear();
	ini.create("TemplePlus");

	for (const auto &setting : configSettings) {
		ini.set(setting.option(), setting.getter()());
	}

	ini.create("Vanilla");

	for (auto& entry : vanillaSettings) {
		ini.set(entry.first, entry.second.value);
	}

	ini.save();
}

std::string TemplePlusConfig::GetPath() {
	return ini.filename;
}

void TemplePlusConfig::SetPath(const std::string& path) {
	ini.filename = path;
}

void TemplePlusConfig::AddVanillaSetting(const std::string& name, 
										 const std::string& defaultValue, 
								         ConfigChangedCallback changeCallback) {
	auto nameLower(tolower(name));
	
	auto it = vanillaSettings.find(nameLower);
	if (it == vanillaSettings.end()) {
		vanillaSettings[nameLower].value = defaultValue;
		vanillaSettings[nameLower].callback = changeCallback;
	} else {
		vanillaSettings[nameLower].callback = changeCallback;
	}
}

void TemplePlusConfig::RemoveVanillaCallback(const std::string & name)
{
	auto nameLower(tolower(name));

	auto it = vanillaSettings.find(nameLower);
	if (it != vanillaSettings.end()) {
		it->second.callback = nullptr;
	}
}

int TemplePlusConfig::GetVanillaInt(const std::string & name) const {
	auto nameLower(tolower(name));

	auto it = vanillaSettings.find(nameLower);
	if (it == vanillaSettings.end()) {
		return 0;
	} else {
		return std::stoi(it->second.value);
	}
}

std::string TemplePlusConfig::GetVanillaString(const std::string & name) const {
	static std::string sEmptySetting;
	auto nameLower(tolower(name));

	auto it = vanillaSettings.find(nameLower);
	if (it == vanillaSettings.end()) {
		return sEmptySetting;
	} else {
		return it->second.value;
	}
}

void TemplePlusConfig::SetVanillaString(const std::string & name, const std::string & value)
{
	auto nameLower(tolower(name));

	auto it = vanillaSettings.find(nameLower);
	if (it != vanillaSettings.end()) {
		it->second.value = value;
		if (it->second.callback) {
			it->second.callback();
		}
	} else {
		AddVanillaSetting(name, value);
	}
}

void TemplePlusConfig::SetVanillaInt(const std::string & name, int value)
{
	SetVanillaString(name, std::to_string(value));
}
