
#include "stdafx.h"

#include <regex>

#include <util/fixes.h>
#include <obj.h>
#include "python_integration.h"
#include "python_object.h"
#include "tio/tio.h"

PythonIntegration pythonIntegration;

// Python name of a script event
static const char *GetScriptEventFunction(ScriptEvent evt);

static struct IntegrationAddresses : AddressTable {
	int *sleepStatus;

	/*
		spellId and unk1 actually seem to be a pair that can also be an obj hndl but this only seems to be used
		for the legacy scripting system.
		unk2 seems to be entirely unused.
	*/
	int (__cdecl *ExecuteObjectScript)(objHndl triggerer, objHndl attachee, int spellId, int unk1, ScriptEvent evt, int unk2);

	IntegrationAddresses() {
		rebase(sleepStatus, 0x109DD854);
		rebase(ExecuteObjectScript, 0x10025D60);
	}
} addresses;

static PyObject *ExecuteScript(const char *moduleName, const char *functionName, PyObject* args) {

	auto module = PyImport_ImportModule(moduleName); // New ref
	if (!module) {
		logger->error("Unable to find Python module {}", moduleName);
		Py_RETURN_NONE;
	}
	
	auto dict = PyModule_GetDict(module); // Borrowed ref
	auto callback = PyDict_GetItemString(dict, functionName); // Borrowed ref

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("Python module {} is missing callback {}.", moduleName, functionName);
		Py_DECREF(module);
		Py_RETURN_NONE;
	}

	auto result = PyObject_CallObject(callback, args);

	if (!result) {
		PyErr_Print();
		logger->error("Failed to invoke function {} in Python module {}.", functionName, moduleName);
		Py_DECREF(module);
		Py_RETURN_NONE;
	}
	
	Py_DECREF(module);
	return result;
}

static PyObject *ExecuteScript(const char *moduleName, const char *functionName) {
	auto args = PyTuple_New(0);
	auto result = ExecuteScript(moduleName, functionName, args);
	Py_DECREF(args);
	return result;
}

/*
	Calls into random_encounter.can_sleep to update how the party 
	can rest in the current area.
*/
void UpdateSleepStatus() {
	auto result = ExecuteScript("random_encounter", "can_sleep");
	if (result) {
		*addresses.sleepStatus = PyInt_AsLong(result);		
		Py_DECREF(result);
	}
}

/*
	Calls into random_encounter.encounter_create.
*/
static void __cdecl RandomEncounterCreate(void *arg) {
	// TODO

}

/*
	Calls into random_encounter.encounter_exists.
*/
static bool __cdecl RandomEncounterExists(void *, void*) {
	// TODO
	return false;
}

/*
	Calls into rumor_control.rumor_given_out
*/
static void __cdecl RumorGivenOut(int rumorIdx) {
	
}

/*
	Calls into rumor_control.find_rumor.
	Quote from the python function:
	# this function returns the message line (0, 10, 20, etc) of a
	# rumor to be told to the PC in question, or -1 if no rumor is
	# available
*/
static int __cdecl RumorFind(objHndl pc, objHndl npc) {
	return -1;
}

/*
	Calls pc_start.pc_start
*/
static void PcStart(objHndl pc) {
	
}

/*
	Calls the actual python spell script.
*/
static int SpellTrigger(int spellPacketId, int eventType) {
	return 0;
}

/*
	Calls the python spell with more args?
*/
static int SpellTrigger2(int spellPacketId, int eventType, objHndl handle, int someNumber) {
	return 0;
}

/*
	Calls into py00116Tolub brawl_end directly
*/
static void BrawlResult(int) {
	
}

static int Co8Load(const char *str) {
	return 0;
}

static int Co8Save(const char *str) {
	return 0;
}

/*
	Checks whether the given number is a Python script by
	loading and unloading it.
	This seems highly inefficient and should really be 
	handled differently.
*/
static bool IsPythonScript(int scriptNumber) {
	return pythonIntegration.IsValidScriptId(scriptNumber);
}

struct ObjScriptInvocation {
	ObjectScript *script;
	int field4;
	objHndl triggerer;
	objHndl attachee;
	int spellId;
	int arg4;
	ScriptEvent evt;
};

static int RunPythonObjScript(ObjScriptInvocation *invoc) {

	// This seemsprimarily used by the CounterArray
	// set_script_context(invoc->scriptIdxPtr->scriptNumber, invoc->attachee, invoc->eventId);
	pythonIntegration.SetInObjInvocation(true);

	PyObject *args;
	PyObject *triggerer = PyObjHndl_Create(invoc->triggerer);

	// Expected arguments depend on the event type
	if (invoc->evt == ScriptEvent::SpellCast) {
		auto attachee = PyObjHndl_Create(invoc->attachee);
		args = Py_BuildValue("OOO", attachee, triggerer, Py_None); // TODO: Spells
		Py_DECREF(attachee);
	} else if (invoc->evt == ScriptEvent::Trap) {
		auto attachee = PyObjHndl_Create(invoc->attachee); // TODO: Traps
		args = Py_BuildValue("OO", attachee, triggerer); 
		Py_DECREF(attachee);
	} else {
		auto attachee = PyObjHndl_Create(invoc->attachee);
		args = Py_BuildValue("OO", attachee, triggerer);
		Py_DECREF(attachee);
	}

	auto result = pythonIntegration.RunScript(invoc->script->scriptId,
		invoc->evt,
		args);

	Py_DECREF(args);

	auto newSid = pythonIntegration.GetNewSid();
	if (newSid != -1 && newSid != invoc->script->scriptId) {
		invoc->script->scriptId = newSid;
	}
	pythonIntegration.SetInObjInvocation(false);

	return result;
}

void RunAnimFramePythonScript(const char *command) {
	pythonIntegration.RunAnimFrameScript(command);
}

void SetAnimatedObject(objHndl handle) {
	pythonIntegration.SetAnimatedObject(handle);
}

class PythonScriptIntegration : public TempleFix {
public:
	const char* name() override {
		return "Python Script Integration Extensions";
	}

	void apply() override {
		replaceFunction(0x10045850, UpdateSleepStatus);
		replaceFunction(0x10046030, RandomEncounterCreate);
		replaceFunction(0x100461E0, RandomEncounterExists);
		replaceFunction(0x1005FA00, RumorGivenOut);
		replaceFunction(0x1005FB70, RumorFind);
		replaceFunction(0x1006D190, PcStart);
		replaceFunction(0x100C0180, SpellTrigger);
		replaceFunction(0x100C0390, SpellTrigger2);
		replaceFunction(0x100EBE20, BrawlResult);
		replaceFunction(0x11EB6940, Co8Save);
		replaceFunction(0x11EB6966, Co8Load);
		replaceFunction(0x100AE1E0, IsPythonScript);
		replaceFunction(0x100AE210, RunPythonObjScript);
		replaceFunction(0x100AEDA0, SetAnimatedObject);
	}

} pythonIntegrationReplacement;

void PythonIntegration::LoadScripts() {
	// Enumerate all scripts in scr that would be accessible via a script number
	// and preload them
	logger->info("Discovering Python scripts...");

	TioFileList list;
	tio_filelist_create(&list, "scr\\Py*.py");
	
	regex scriptRegex("(py(\\d{5}).*)\\.py", regex_constants::ECMAScript | regex_constants::icase);
	smatch scriptMatch;

	char ** sanNames = reinterpret_cast<char**>(0x102AC3A0);
	while (*sanNames) {
		logger->info("{}", *sanNames);
		sanNames++;
	}

	for (auto i = 0; i < list.count; ++i) {
		auto &file = list.files[i];
		string scriptName = file.name; // This is only the filename (no directory)

		// Is it a proper script file?
		if (!regex_match(scriptName, scriptMatch, scriptRegex)) {
			logger->debug("Skipping {} because it is not a script file.", scriptName);
			continue;
		}

		ScriptRecord record;
		record.filename = scriptName;
		record.moduleName = scriptMatch[1];
		record.id = stoi(scriptMatch[2]);

		if (mScripts.find(record.id) != mScripts.end()) {
			logger->error("Multiple scripts have the id {}. Skipping {}.", record.id, scriptName);
			continue;
		}
		
		logger->trace("Discovered {} (ID: {})", record.moduleName, record.id);
		mScripts[record.id] = record;
	}

	tio_filelist_destroy(&list);
}

void PythonIntegration::UnloadScripts() {
	for (auto &entry : mScripts) {
		Py_XDECREF(entry.second.module);
	}
	mScripts.clear();
}

int PythonIntegration::ExecuteObjectScript(objHndl triggerer, objHndl attachee, int spellId, ScriptEvent evt) {
	return addresses.ExecuteObjectScript(triggerer, attachee, spellId, 0, evt, 0);
}

int PythonIntegration::ExecuteObjectScript(objHndl triggerer, objHndl attachee, ScriptEvent evt) {
	return ExecuteObjectScript(triggerer, attachee, 0, evt);
}

bool PythonIntegration::IsValidScriptId(int scriptId) {
	return mScripts.find(scriptId) != mScripts.end();
}

void PythonIntegration::RunAnimFrameScript(const char* command) {
	logger->info("Running Python command {}", command);

	auto mainModule = PyImport_ImportModule("__main__");
	auto mainDict = PyModule_GetDict(mainModule);
	auto locals = PyDict_New();

	// Put the anim obj into the locals
	auto animObj = PyObjHndl_Create(mAnimatedObj);
	PyDict_SetItemString(locals, "anim_obj", animObj);
	Py_DECREF(animObj);

	auto result = PyRun_String(command, Py_single_input, mainDict, locals);

	Py_DECREF(mainModule);
	Py_DECREF(locals);

	if (!result) {
		PyErr_Print();
	} else {
		Py_DECREF(result);
	}
}

int PythonIntegration::RunScript(int scriptId, ScriptEvent evt, PyObject* args) {
	auto it = mScripts.find(scriptId);
	if (it == mScripts.end()) {
		logger->error("Trying to invoke unknown script id {}", scriptId);
		return 1;
	}

	auto &script = it->second;
	if (script.loadingError) {
		return 1; // cannot execute a script we previously failed to load
	}

	// We have not yet loaded the Python module
	if (!script.module) {
		script.module = PyImport_ImportModule(script.moduleName.c_str());
		if (!script.module) {
			// Loading error
			logger->error("Could not load script {}.", script.filename);
			PyErr_Print();
			script.loadingError = true; // Do not try this over and over again
			return 1;
		}
	}

	auto dict = PyModule_GetDict(script.module);
	auto eventName = GetScriptEventFunction(evt);
	auto callback = PyDict_GetItemString(dict, eventName);

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("Script {} attached as {} is missing the corresponding function.", 
			script.filename, eventName);
		return 1;
	}

	auto resultObj = PyObject_CallObject(callback, args);

	if (!resultObj) {
		logger->error("An error occurred while calling event {} for script {}.", eventName, script.filename);
		PyErr_Print();
		return 1;
	}

	auto result = -1;
	if (PyInt_Check(resultObj)) {
		result = PyInt_AsLong(resultObj);
	}
	Py_DECREF(resultObj);

	return result;
}

static const char *scriptEventFunctions[] = {
	"san_examine",
	"san_use",
	"san_destroy",
	"san_unlock",
	"san_get",
	"san_drop",
	"san_throw",
	"san_hit",
	"san_miss",
	"san_dialog",
	"san_first_heartbeat",
	"san_catching_thief_pc",
	"san_dying",
	"san_enter_combat",
	"san_exit_combat",
	"san_start_combat",
	"san_end_combat",
	"san_buy_object",
	"san_resurrect",
	"san_heartbeat",
	"san_leader_killing",
	"san_insert_item",
	"san_will_kos",
	"san_taking_damage",
	"san_wield_on",
	"san_wield_off",
	"san_critter_hits",
	"san_new_sector",
	"san_remove_item",
	"san_leader_sleeping",
	"san_bust",
	"san_dialog_override",
	"san_transfer",
	"san_caught_thief",
	"san_critical_hit",
	"san_critical_miss",
	"san_join",
	"san_disband",
	"san_new_map",
	"san_trap",
	"san_true_seeing",
	"san_spell_cast",
	"san_unlock_attempt"
};

static const char *GetScriptEventFunction(ScriptEvent evt) {
	return scriptEventFunctions[(uint32_t)evt];
}
