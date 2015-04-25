
#pragma once

#include "../obj.h"

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
	Dialog = 9,
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

	/*
		Spell ID is only used for script event san_cast_spell
	*/
	int ExecuteObjectScript(objHndl triggerer, objHndl attachee, int spellId, ScriptEvent event);
	int ExecuteObjectScript(objHndl triggerer, objHndl attachee, ScriptEvent event);

	bool IsValidScriptId(int scriptId);
	void RunAnimFrameScript(const char *command);
	int RunScript(int scriptId, ScriptEvent evt, PyObject *args);

	/*
		Gets a loaded instance of a script module or null if loading failed.
		Returns a borrowed ref
	*/
	bool LoadScript(int scriptId, ScriptRecord &scriptOut);

	/*
		Set the object that is being animated. All calls to RunString will 
		have this object in their local variables.
	*/
	void SetAnimatedObject(objHndl handle) {
		mAnimatedObj = handle;
	}

	// Indicates that the integration is currently executing a script attached to an obj
	// Only if this is true do the next two properties (newsid + counters) have any actual meaning
	bool IsInObjInvocation() {
		return mInObjInvocation;
	}
	void SetInObjInvocation(bool inObjInvocation) {
		mInObjInvocation = inObjInvocation;
		mNewSid = -1;
	}

	/*
		Sets the context for the script counter manipulation.
	*/
	void SetCounterContext(objHndl attachee, int scriptId, ScriptEvent evt) {
		mCounterObj = attachee;
		mCounterScriptId = scriptId;
		mCounterEvent = evt;
	}

	// Four 8-bit counter values are provided for each script attachment to track some state
	int GetCounter(int idx);
	void SetCounter(int idx, int value);

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
	objHndl mAnimatedObj;

	// Used to track which counters are manipulated
	objHndl mCounterObj;
	ScriptEvent mCounterEvent;
	int mCounterScriptId;
};

extern PythonIntegration pythonIntegration;

void RunAnimFramePythonScript(const char *command);
