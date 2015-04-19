
#pragma once

struct ScriptRecord {
	int id;
	string moduleName;
	string filename;
	PyObject *module = nullptr; // If not null, owns the ref
	bool loadingError = false;
};

enum class ScriptEvent : uint32_t {
	Examine = 0,
	Use,
	Destroy,
	Unlock,
	Get,
	Drop,
	Throw,
	Hit,
	Miss,
	Dialog,
	FirstHeartbeat,
	CatchingThiefPc,
	Dying,
	EnterCombat,
	ExitCombat,
	StartCombat,
	EndCombat,
	BuyObject,
	Resurrect,
	Heartbeat,
	LeaderKilling,
	InsertItem,
	WillKos,
	TakingDamage,
	WieldOn,
	WieldOff,
	CritterHits,
	NewSector,
	RemoveItem,
	LeaderSleeping,
	Bust,
	DialogOverride,
	Transfer,
	CaughtThief,
	CriticalHit,
	CriticalMiss,
	Join,
	Disband,
	NewMap,
	Trap,
	TrueSeeing,
	SpellCast,
	UnlockAttempt
};

/*
	Integration points between the ToEE engine and the Python scripting system.
*/
class PythonIntegration {
public:

	void LoadScripts();
	void UnloadScripts();

	bool IsValidScriptId(int scriptId);
	void RunString(const char *command);
	int RunScript(int scriptId, ScriptEvent evt, PyObject *args);

private:
	typedef unordered_map<int, ScriptRecord> ScriptCache;
	ScriptCache mScripts;
};

extern PythonIntegration pythonIntegration;

void RunPythonString(const char *command);