
#pragma once

#include "python_integration.h"

enum class ObjScriptEvent : uint32_t {
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

class PythonObjIntegration : public PythonIntegration {
public:

	PythonObjIntegration();

	/*
	Spell ID is only used for script event san_cast_spell
	*/
	int ExecuteObjectScript(objHndl triggerer, objHndl attachee, int spellId, ObjScriptEvent evt);
	int ExecuteObjectScript(objHndl triggerer, objHndl attachee, ObjScriptEvent evt);
	
	void RunAnimFrameScript(const char *command);
		
	PyObject *ExecuteScript(const char *moduleName, const char *functionName, PyObject *args);
	
	PyObject *ExecuteScript(const char *moduleName, const char *functionName);

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
	void SetCounterContext(objHndl attachee, int scriptId, ObjScriptEvent evt) {
		mCounterObj = attachee;
		mCounterScriptId = scriptId;
		mCounterEvent = evt;
	}
	objHndl GetCounterObj() {
		return mCounterObj;
	}
	ObjScriptEvent GetCounterEvent() {
		return mCounterEvent;
	}

	// Used by a script attached to an object to replace itself with something else
	void SetNewSid(int newSid) {
		mNewSid = newSid;
	}
	int GetNewSid() {
		return mNewSid;
	}

	string GetEventName(ObjScriptEvent evt) {
		return GetFunctionName((EventId)evt);
	}

protected:
	const char* GetFunctionName(EventId evt) override;

private:

	bool mInObjInvocation = false;
	int mNewSid = -1;
	objHndl mAnimatedObj;

	// Used to track which counters are manipulated
	objHndl mCounterObj;
	ObjScriptEvent mCounterEvent;
	int mCounterScriptId;
};

extern PythonObjIntegration pythonObjIntegration;
