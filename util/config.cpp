
#include "stdafx.h"
#include "config.h"
#include "../dependencies/feather-ini/INI.h"

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

	static ConfigSetting StringList(const char *option, char sep, Setter<const vector<string>&> setter, Getter<vector<string>> getter, const char *description = nullptr) {
		Getter<string> getterAdapter = [=] {
			string result;
			for (auto &str : getter()) {
				if (!result.empty()) {
					result += sep;
				}
				result += str;
			}
			return result;
		};
		Setter<string> setterAdapter = [=](string x) {
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

	static ConfigSetting Bool(const char *option, Setter<bool> setter, Getter<bool> getter, const char *description = nullptr) {
		Getter<string> getterAdapter = [=] {
			return getter() ? "true" : "false";
		};
		Setter<string> setterAdapter = [=] (string x) {
			setter(x == "true" || x == "1");
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Int(const char *option, Setter<int> setter, Getter<int> getter, const char *description = nullptr) {
		Getter<string> getterAdapter = [=] {
			return to_string(getter());
		};
		Setter<string> setterAdapter = [=](string x) {
			setter(stoi(x));
		};

		return ConfigSetting(option, setterAdapter, getterAdapter, description);
	}

	static ConfigSetting Double(const char *option, Setter<double> setter, Getter<double> getter, const char *description = nullptr) {
		Getter<string> getterAdapter = [=] {
			return to_string(getter());
		};
		Setter<string> setterAdapter = [=](string x) {
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

#define CONF_STRING(FIELDNAME) ConfigSetting::String(#FIELDNAME, [] (string val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_STRING_LIST(FIELDNAME, SEP) ConfigSetting::StringList(#FIELDNAME, SEP, [] (const vector<string> &val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_INT(FIELDNAME) ConfigSetting::Int(#FIELDNAME, [] (int val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_BOOL(FIELDNAME) ConfigSetting::Bool(#FIELDNAME, [] (bool val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })
#define CONF_DOUBLE(FIELDNAME) ConfigSetting::Double(#FIELDNAME, [] (double val) { config.FIELDNAME = val; }, [] { return config.FIELDNAME; })

static ConfigSetting configSettings[] = {
	CONF_BOOL(skipIntro),
	CONF_BOOL(skipLegal),
	CONF_BOOL(engineEnhancements),
	//CONF_BOOL(editor),
	CONF_BOOL(useDirect3d9Ex),
	CONF_BOOL(windowed),
	CONF_BOOL(noSound),
	CONF_BOOL(featPrereqWarnings),
	CONF_BOOL(spellAlreadyKnownWarnings),
	CONF_BOOL(NPCsLevelLikePCs),
	//CONF_DOUBLE(randomEncounterExperienceFactor),
	CONF_BOOL(newFeatureTestMode),
	CONF_STRING(defaultModule),
	CONF_INT(windowWidth),
	CONF_INT(windowHeight),
	CONF_INT(renderWidth),
	CONF_INT(renderHeight),
	CONF_BOOL(debugMessageEnable),
	CONF_BOOL(lockCursor),
	CONF_BOOL(showExactHPforNPCs),
	CONF_STRING_LIST(additionalTioPaths, ';'),
	CONF_INT(pointBuyPoints),
	CONF_INT(maxPCs),
	CONF_BOOL(maxPCsFlexible),
	CONF_BOOL(usingCo8)
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

}

void TemplePlusConfig::Save() {
	ini.clear();
	ini.create("TemplePlus");

	for (const auto &setting : configSettings) {
		ini.set(setting.option(), setting.getter()());
	}

	ini.save();
}
