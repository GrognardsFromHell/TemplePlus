
#include "stdafx.h"
#include "config.h"
#include <infrastructure/INI.h>

TemplePlusConfig config;

typedef INI <string, string, string> ini_t;

static ini_t ini("TemplePlus.ini", false);

class ConfigSetting {
public:
	template<class T> using Setter = function<void(T)>;
	template<class T> using Getter = function<T()>;
	
	static ConfigSetting String(const char *option, Setter<string> setter, Getter<string> getter, const char *description = nullptr) {
		return ConfigSetting(option, setter, getter, description);
	}

	static ConfigSetting WString(const char *option, Setter<wstring> setter, Getter<wstring> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			return ucs2_to_utf8(getter());
		};
		auto setterAdapter = [=](string value) {
			setter(utf8_to_ucs2(value));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}
	static ConfigSetting StringList(const char *option, char sep, Setter<const vector<string>&> setter, Getter<vector<string>> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			string result;
			for (auto &str : getter()) {
				if (!result.empty()) {
					result += sep;
				}
				result += str;
			}
			return result;
		};
		auto setterAdapter = [=](string x) {
			vector<string> result;
			stringstream ss(x);
			string token;
			while (getline(ss, token, sep)) {
				if (!token.empty()) {
					result.push_back(token);
				}
			}
			setter(result);
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting StringSet(const char *option, char sep, Setter<const unordered_set<string>&> setter, Getter<unordered_set<string>> getter, const char *description = nullptr) {

		auto getAdapter = [=] {
			string result;
			for (auto &str : getter()) {
				if (!result.empty()) {
					result += sep;
				}
				result += str;
			}
			return result;
		};

		auto setAdapter = [=](string x) {
			unordered_set<string> result;
			stringstream ss(x);
			string token;
			while (getline(ss, token, sep)) {
				if (!token.empty()) {
					result.insert(token);
				}
			}
			setter(result);
		};

		return ConfigSetting(option, setAdapter, getAdapter, description);
	}

	template<typename T>
	static ConfigSetting Set(const char *option, char sep, Setter<const unordered_set<T>&> setter, Getter<unordered_set<T>> getter, const map<T, string> & toS, const map<string, T> & fromS) {
		auto getAdapter = [=] {
			string result;
			for (auto &item : getter()) {
				auto mstr = toS.find(item);
				if (mstr == toS.end()) continue;

				if (!result.empty()) {
					result += sep;
				}
				result += mstr->second;
			}
			return result;
		};

		auto setAdapter = [=](string x) {
			unordered_set<T> result;
			stringstream ss(x);
			string token;
			while (getline(ss, token, sep)) {
				if (token.empty()) continue;

				auto mitem = fromS.find(tolower(token));
				if (mitem == fromS.end()) continue;

				result.insert(mitem->second);
			}
			setter(result);
		};

		return ConfigSetting(option, setAdapter, getAdapter, nullptr);
	}

	static ConfigSetting Bool(const char *option, Setter<bool> setter, Getter<bool> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			return getter() ? "true" : "false";
		};
		auto setterAdapter = [=] (string x) {
			setter(x == "true" || x == "1");
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Int(const char *option, Setter<int> setter, Getter<int> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			return to_string(getter());
		};
		auto setterAdapter = [=](string x) {
			setter(stoi(x));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Double(const char *option, Setter<double> setter, Getter<double> getter, const char *description = nullptr) {
		auto getterAdapter = [=] {
			return to_string(getter());
		};
		auto setterAdapter = [=](string x) {
			setter(stod(x));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	const char *option() const {
		return mOption;
	}

	Setter<string> setter() const {
		return mSetter;
	}

	Getter<string> getter() const {
		return mGetter;
	}

private:
	ConfigSetting(const char *option, 
		Setter<string> setter, 
		Getter<string> getter,
		const char *comment)
		: mOption(option), mComment(comment), mSetter(setter), mGetter(getter) {
	}

	const char *mOption;
	const char *mComment;
	Setter<string> mSetter;
	Getter<string> mGetter;
};

static std::map<string, PnPSource> nameToSource = {
	{ "phb", PnPSource::PHB },
	{ "toee", PnPSource::ToEE },
	{ "spellcompendium", PnPSource::SpellCompendium },
	{ "phb2", PnPSource::PHB2 },
	{ "homebrew", PnPSource::Homebrew },
	{ "co8", PnPSource::Co8 }
};

static std::map<PnPSource, string> sourceToName = {
	{ PnPSource::PHB, "phb" },
	{ PnPSource::ToEE, "toee" },
	{ PnPSource::SpellCompendium, "spellcompendium" },
	{ PnPSource::PHB2, "phb2" },
	{ PnPSource::Homebrew, "homebrew" },
	{ PnPSource::Co8, "co8" }
};

#define CONF_STRING(FIELDNAME) ConfigSetting::String(#FIELDNAME, [] (string val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_WSTRING(FIELDNAME) ConfigSetting::WString(#FIELDNAME, [] (wstring val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_STRING_LIST(FIELDNAME, SEP) ConfigSetting::StringList(#FIELDNAME, SEP, [] (const vector<string> &val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_INT(FIELDNAME) ConfigSetting::Int(#FIELDNAME, [] (int val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_BOOL(FIELDNAME) ConfigSetting::Bool(#FIELDNAME, [] (bool val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_DOUBLE(FIELDNAME) ConfigSetting::Double(#FIELDNAME, [] (double val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_STRING_SET(FIELDNAME, SEP) ConfigSetting::StringSet(#FIELDNAME, SEP, [](const std::unordered_set<string> &val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_SET(T, FIELDNAME, SEP, TOS, FROMS) ConfigSetting::Set<T>(#FIELDNAME, SEP, [](const std::unordered_set<T> &val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; }, TOS, FROMS)

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
	CONF_BOOL(debugObjects),

	CONF_STRING(defaultModule),
	CONF_INT(windowWidth),
	CONF_INT(windowHeight),
	CONF_DOUBLE(dmGuiScale),
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
	CONF_BOOL(showHitChances),
	CONF_BOOL(dumpFullMemory),

	CONF_BOOL(newClasses),
	CONF_BOOL(newRaces),
	CONF_BOOL(metamagicStacking),
	CONF_BOOL(monstrousRaces),
	CONF_BOOL(forgottenRealmsRaces),
	CONF_BOOL(laxRules),
	CONF_BOOL(stricterRulesEnforcement),
	CONF_BOOL(preferPoisonSpecFile),
	CONF_BOOL(disableReachWeaponDonut),
	CONF_BOOL(highlightContainers),
	CONF_BOOL(disableAlignmentRestrictions),
	CONF_BOOL(disableCraftingSpellReqs),
	CONF_BOOL(disableMulticlassXpPenalty),
	CONF_BOOL(disableDoorRelocking),
	CONF_BOOL(dialogueUseBestSkillLevel),

	CONF_BOOL(showTargetingCirclesInFogOfWar),
	CONF_BOOL(nonCoreMaterials),
	CONF_SET(PnPSource, nonCoreSources, ';', sourceToName, nameToSource),
	CONF_BOOL(tolerantNpcs),
	CONF_STRING(fogOfWar),
	CONF_DOUBLE(speedupFactor),
	CONF_BOOL(fastSneakAnim),
	CONF_BOOL(disableScreenShake),

	CONF_BOOL(alertAiThroughDoors),
	CONF_BOOL(preferUse5FootStep),
	CONF_BOOL(extendedSpellDescriptions),
	CONF_INT(walkDistanceFt),
	CONF_BOOL(newAnimSystem),
	CONF_BOOL(upscaleLinearFiltering),
	CONF_BOOL(disableChooseRandomSpell_RegardInvulnerableStatus),
	CONF_BOOL(wildShapeUsableItems),
	CONF_INT(npcStatBoost),
	CONF_BOOL(disableReachWeaponDonut)
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

// Calculates a source for a spell based on its numbering, as a default.
PnPSource DefaultSpellSource(int spellEnum)
{
	if (spellEnum <= 568)
		return PnPSource::PHB;
	else if (spellEnum <= 802)
		return PnPSource::ToEE;
	else
		return PnPSource::Homebrew;
}

