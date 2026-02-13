#include "stdafx.h"
#include "python_integration_obj.h"
#include "python_object.h"
#include "python_trap.h"
#include "python_embed.h"
#include "python_spell.h"
#include <dialog.h>
#include <critter.h>
#include <util/fixes.h>
#include <infrastructure/mesparser.h>
#include <gamesystems/objects/objsystem.h>
#include <config/config.h>

PythonObjIntegration pythonObjIntegration;

static struct IntegrationAddresses : temple::AddressTable {
	/*
	spellId and unk1 actually seem to be a pair that can also be an obj hndl but this only seems to be used
	for the legacy scripting system.
	unk2 seems to be entirely unused.
	*/
	int (__cdecl *ExecuteObjectScript)(objHndl triggerer, objHndl attachee, int spellId, int unk1, ObjScriptEvent evt, int unk2);

	IntegrationAddresses() {
		rebase(ExecuteObjectScript, 0x10025D60);
	}
} addresses;

/*
Calls into rumor_control.rumor_given_out
*/
static void __cdecl RumorGivenOut(int rumorIdx) {
	logger->debug("Rumor given out: {}", rumorIdx);

	auto args = Py_BuildValue("(i)", 10 * rumorIdx);
	auto result = pythonObjIntegration.ExecuteScript("rumor_control", "rumor_given_out", args);
	Py_DECREF(result);
	Py_DECREF(args);
}

/*
Calls into rumor_control.find_rumor.
Quote from the python function:
# this function returns the message line (0, 10, 20, etc) of a
# rumor to be told to the PC in question, or -1 if no rumor is
# available
*/
static int __cdecl RumorFind(objHndl pc, objHndl npc) {

	auto args = PyTuple_New(2);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(pc));
	PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(npc));

	auto result = pythonObjIntegration.ExecuteScript("rumor_control", "find_rumor", args);
	auto rumorId = -1;

	if (PyInt_Check(result)) {
		rumorId = PyInt_AsLong(result);
		if (rumorId != -1) {
			rumorId /= 10;
		}
	}
	Py_DECREF(result);
	Py_DECREF(args);

	logger->debug("Rumor found: {}", rumorId);
	return rumorId;
}

/*
Calls pc_start.pc_start
*/
static void PcStart(objHndl pc) {
	inventory.Clear(pc, FALSE);
	
	// This checks that the PC has at least one level in any of the classes
	int classIndex = 0;
	for (auto classEnum : d20ClassSys.baseClassEnums) {
		auto stat = static_cast<Stat>(classEnum);

		if (objects.StatLevelGet(pc, stat) > 0) break;

		classIndex++;
	}

	// couldn't find class
	if (classIndex > d20ClassSys.baseClassEnums.size()) return;

	// offset by 1 for tutorial inventory
	if (classIndex >= VANILLA_NUM_CLASSES) classIndex++;

	try {
		auto content(MesFile::ParseFile("rules\\start_equipment.mes"));
		
		auto key = classIndex;

		// Modify for "small" races
		auto race = critterSys.GetRace(pc);
		if (race == race_halfling || race == race_gnome) {
			key += 100;
		}

		auto it = content.find(key);
		if (it != content.end()) {
			auto protoIds = split(it->second, ' ', true);
			for (auto protoIdStr : protoIds) {
				auto protoId = stoi(protoIdStr);
				auto protoHandle = objSystem->GetProtoHandle(protoId);
				if (!protoHandle) { // fixes crash issue in Terra Arcanum Co8 3.0.4 start_equipment.mes
					logger->error("PCStart: invalid proto specified: {}", protoId); 
					continue;
				}
					
				critterSys.GiveItem(pc, protoId);
			}
		}

		auto args = PyTuple_New(1);
		PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(pc));

		auto result = pythonObjIntegration.ExecuteScript("pc_start", "pc_start", args);
		Py_DECREF(result);
		Py_DECREF(args);
	} catch (TempleException& e) {
		logger->warn("Unable to load starting equipment: {}", e.what());
	}

	inventory.WieldBestAll(pc, objHndl::null);
}

/*
Checks whether the given number is a Python script by
loading and unloading it.
This seems highly inefficient and should really be
handled differently.
*/
static BOOL IsPythonScript(int scriptNumber) {
	return pythonObjIntegration.IsValidScriptId(scriptNumber);
}

struct ObjScriptInvocation {
	ObjectScript* script;
	int field4;
	objHndl triggerer;
	objHndl attachee;
	int spellId;
	int arg4;
	ObjScriptEvent evt;
};

static int RunPythonObjScript(ObjScriptInvocation* invoc) {

	// This seems to be primarily used by the CounterArray
	pythonObjIntegration.SetCounterContext(invoc->attachee, invoc->script->scriptId, invoc->evt);
	pythonObjIntegration.SetInObjInvocation(true);

	PyObject* args;
	auto triggerer = PyObjHndl_Create(invoc->triggerer);

	// Expected arguments depend on the event type
	if (invoc->evt == ObjScriptEvent::SpellCast) {
		auto attachee = PyObjHndl_Create(invoc->attachee);
		auto spell = PySpell_Create(invoc->spellId);
		args = Py_BuildValue("OOO", attachee, triggerer, spell);
		Py_DECREF(spell);
		Py_DECREF(attachee);
	} else if (invoc->evt == ObjScriptEvent::Trap) {
		auto attachee = PyTrap_Create(invoc->attachee);
		args = Py_BuildValue("OO", attachee, triggerer);
		Py_DECREF(attachee);
	} else {
		if (invoc->evt == ObjScriptEvent::FirstHeartbeat){
			logger->trace("Running first heartbeat script for {}", invoc->attachee);
		}
		auto attachee = PyObjHndl_Create(invoc->attachee);
		args = Py_BuildValue("OO", attachee, triggerer);
		Py_DECREF(attachee);
	}

	auto result = pythonObjIntegration.RunScript(invoc->script->scriptId,
	                                             (PythonIntegration::EventId) invoc->evt,
	                                             args);
	
	Py_DECREF(args);

	auto newSid = pythonObjIntegration.GetNewSid();
	if (newSid != -1 && newSid != invoc->script->scriptId) {
		invoc->script->scriptId = newSid;
	}
	pythonObjIntegration.SetInObjInvocation(false);

	return result;
}

static void SetAnimatedObject(objHndl handle) {
	pythonObjIntegration.SetAnimatedObject(handle);
}

/*
Set up the locals dictionary for executing dialog guards and actions.
*/
static PyObject* CreateDialogLocals(const DialogState* dialog, int pickedLine) {

	auto locals = PyDict_New();
	auto pcObj = PyObjHndl_Create(dialog->pc);
	PyDict_SetItemString(locals, "pc", pcObj);
	Py_DECREF(pcObj);
	auto npcObj = PyObjHndl_Create(dialog->npc);
	PyDict_SetItemString(locals, "npc", npcObj);
	Py_DECREF(npcObj);
	auto lineObj = PyInt_FromLong(pickedLine);
	PyDict_SetItemString(locals, "picked_line", lineObj);
	Py_DECREF(lineObj);

	return locals;
}

static void RunDialogAction(const char* actionString, DialogState* dialog, int pickedLine) {
	logger->debug("Running dialog actions '{}'", actionString);

	auto commands = split(actionString, ';', true);

	// Set script context so counters will work
	pythonObjIntegration.SetCounterContext(dialog->npc, dialog->dialogScriptId, ObjScriptEvent::Dialog);

	/*
	We use the globals defined by the python script @ dialog->dialogScriptId
	*/
	ScriptRecord script;
	if (!pythonObjIntegration.LoadScript(dialog->dialogScriptId, script)) {
		logger->error("Cannot load script id {} to run dialog guard {}", dialog->dialogScriptId, actionString);
		return;
	}
	
	auto globalsDict = PyModule_GetDict(script.module);
	auto locals = CreateDialogLocals(dialog, pickedLine);
	if (pythonObjIntegration.GetPickerObj()) {
		PyDict_SetItemString(locals, "picker_obj", pythonObjIntegration.GetPickerObj());
	}

	for (const auto& command : commands) {
		logger->debug("Running dialog action '{}'", command);

		auto result = PyRun_String(command.c_str(), Py_single_input, globalsDict, locals);

		if (!result) {
			PyErr_Print();
		} else {
			Py_DECREF(result);
		}
	}

	Py_DECREF(locals);
}

static BOOL RunDialogGuard(const char* expression, DialogState* dialog, int pickedLine) {
	logger->debug("Running dialog guard {} for node {}, npc text {}, in script {}", expression, dialog->lineNumber, dialog->npcLineText, dialog->dialogScriptId);
	if (pickedLine) logger->debug("Picked Line is {}", pickedLine);

	// Set script context so counters will work
	pythonObjIntegration.SetCounterContext(dialog->npc, dialog->dialogScriptId, ObjScriptEvent::Dialog);

	/*
	We use the globals defined by the python script @ dialog->dialogScriptId
	*/
	ScriptRecord script;
	if (!pythonObjIntegration.LoadScript(dialog->dialogScriptId, script)) {
		logger->error("Cannot load script id {} to run dialog guard {}", dialog->dialogScriptId, expression);
		return false;
	}

	/*
	Set up the locals dictionary for the script execution
	*/
	auto locals = CreateDialogLocals(dialog, pickedLine);

	// Skip whitespace at the start of the expression
	while (isspace(expression[0])) {
		expression++;
	}

	pythonObjIntegration.SetInDialogGuard(true);

	auto globalsDict = PyModule_GetDict(script.module);
	auto result = PyRun_String(expression, Py_eval_input, globalsDict, locals);
	Py_DECREF(locals);
	pythonObjIntegration.SetInDialogGuard(false);

	if (!result) {
		PyErr_Print();
		return false;
	}

	int guardResult = PyObject_IsTrue(result);
	Py_DECREF(result);

	logger->debug("Dialog guard result is {}", guardResult);
	return guardResult == 1;
}

// Python Script Integration Extensions (Objects)
static class PythonObjIntegrationFix : public TempleFix {
public:
	void apply() override {
		replaceFunction(0x1005FA00, RumorGivenOut);
		replaceFunction(0x1005FB70, RumorFind);
		replaceFunction(0x1006D190, PcStart);
		replaceFunction(0x100AE1E0, IsPythonScript);
		replaceFunction(0x100AE210, RunPythonObjScript);
		replaceFunction(0x100AEDA0, SetAnimatedObject);
		replaceFunction(0x100AE7A0, RunDialogAction);
		replaceFunction(0x100AE3F0, RunDialogGuard);
	}

} fix;

PythonObjIntegration::PythonObjIntegration() : PythonIntegration("scr\\Py*.py", "(py(\\d{5}).*)\\.py") {
}

int PythonObjIntegration::ExecuteObjectScript(objHndl triggerer, objHndl attachee, int spellId, ObjScriptEvent evt) {
	return addresses.ExecuteObjectScript(triggerer, attachee, spellId, 0, evt, 0);
}

int PythonObjIntegration::ExecuteObjectScript(objHndl triggerer, objHndl attachee, ObjScriptEvent evt) {
	return ExecuteObjectScript(triggerer, attachee, 0, evt);
}


void PythonObjIntegration::RunAnimFrameScript(const std::string &command) {
	logger->trace("Running Python command {}", command);

	if (config.disableScreenShake) {
		if (strstr(command.c_str(), "game.shake")) {
			return;
		}
	}

	auto locals = PyDict_New();

	// Put the anim obj into the locals
	auto animObj = PyObjHndl_Create(mAnimatedObj);
	PyDict_SetItemString(locals, "anim_obj", animObj);
	Py_DECREF(animObj);

	auto result = PyRun_String(command.c_str(), Py_eval_input, MainModuleDict, locals);

	Py_DECREF(locals);

	if (!result) {
		PyErr_Print();
	} else {
		Py_DECREF(result);
	}
}

PyObject* PythonObjIntegration::ExecuteScript(const char* moduleName, const char* functionName) {
	auto args = PyTuple_New(0);
	auto result = ExecuteScript(moduleName, functionName, args);
	Py_DECREF(args);
	return result;
}

PyObject* PythonObjIntegration::ExecuteScript(const char* moduleName, const char* functionName, PyObject* args) {
	auto module = PyImport_ImportModule(moduleName); // New ref
	if (!module) {
	/*auto locals = PyDict_New();
	auto module = PyImport_ImportModuleEx(const_cast<char*>(moduleName), MainModuleDict, locals, 0); // New ref
	Py_DECREF(locals);
	if (!module) {		*/
		logger->error("Unable to find Python module {}", moduleName);
		Py_RETURN_NONE;
	}

	auto dict = PyModule_GetDict(module); // Borrowed ref
	AddGlobalsOnDemand(dict);

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

void PythonObjIntegration::SetPickerObj(PyObject *obj) {
	if (mPickerObj) {
		Py_DECREF(mPickerObj);
	}
	if (obj) {
		Py_INCREF(obj);
	}
	mPickerObj = obj;
}

PyObject *PythonObjIntegration::GetPickerObj() {
	return mPickerObj;
}

static const char* scriptEventFunctions[] = {
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

const char* PythonObjIntegration::GetFunctionName(EventId evt) {
	return scriptEventFunctions[evt];
}
