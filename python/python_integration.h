
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

	// Indicates that the integration is currently executing a script attached to an obj
	// Only if this is true do the next two properties (newsid + counters) have any actual meaning
	bool IsInObjInvocation() {
		return mInObjInvocation;
	}
	void SetInObjInvocation(bool inObjInvocation) {
		mInObjInvocation = inObjInvocation;
		mNewSid = -1;
		memset(mCounters, 0, sizeof(mCounters));
	}

	// Four 8-bit counter values are provided for each script attachment to track some state
	int GetCounter(int idx) {
		if (idx >= 0 && idx < 4) {
			return mCounters[idx] & 0xFF;
		}
		return 0;
	}
	void SetCounter(int idx, int value) {
		if (idx >= 0 && idx < 4) {
			mCounters[idx] = value & 0xFF;
		}
	}

	// Used by a script attached to an object to replace itself with something else
	void SetNewSid(int newSid) {
		mNewSid = newSid;
	}
	int GetNewSid() {
		return mNewSid;
	}

private:
	typedef unordered_map<int, ScriptRecord> ScriptCache;
	ScriptCache mScripts;

	bool mInObjInvocation = false;
	int mNewSid = -1;
	int mCounters[4];
};

extern PythonIntegration pythonIntegration;

void RunPythonString(const char *command);