
#include "stdafx.h"
#include <util/fixes.h>
#include "python_debug.h"
#include "python_importer.h"
#include "python_debug.h"
#include "python_game.h"
#include "python_time.h"
#include "python_consoleout.h"
#include "python_integration_obj.h"
#include "python_cheats.h"
#include "python_integration_spells.h"
#include "python_console.h"
#include "tio/tio.h"
#include "tig/tig_startup.h"
#include "tig/tig_console.h"
#include <set>
#include <string>
#include <functional>
#include "python_dispatcher.h"

#include "../gamesystems/gamesystems.h"
#include "python_integration_class_spec.h"
#include "python_integration_d20_action.h"
#include "python_integration_feat.h"

#include <pybind11/embed.h>

#include "python_embed.h"
#include "python_object.h"
#include "python_spell.h"
#include "python_trap.h"

namespace py = pybind11;

static PyObject *MainModule;
PyObject *MainModuleDict;
static PyConsole *console;

void PythonPrepareGlobalNamespace() {
	// Pre-Import anything from toee into globals
	auto toeeModule = PyImport_ImportModule("toee");
	auto toeeDict = PyModule_GetDict(toeeModule);
	if (PyDict_Merge(MainModuleDict, toeeDict, FALSE) == -1) {
		logger->error("Unable to import toee.* into __main__");
		PyErr_Print();
	}
	Py_DECREF(toeeModule);
	
	auto utilsModule = PyImport_ImportModule("utilities");
	auto utilsDict = PyModule_GetDict(utilsModule);
	if (PyDict_Merge(MainModuleDict, utilsDict, FALSE) == -1) {
		logger->error("Unable to import utilities.* into __main__");
		PyErr_Print();
	}
	Py_DECREF(utilsModule);

	auto cheats = PyCheats_Create();
	PyDict_SetItemString(MainModuleDict, "cheats", cheats);
	Py_DECREF(cheats);
}

PythonScripting::PythonScripting()
{

	static char* sArgv = "";

	Py_OptimizeFlag++;
	Py_VerboseFlag++;
	Py_NoSiteFlag++;
	Py_SetProgramName("TemplePlus.exe");

	py::initialize_interpreter();

	PySys_SetArgv(0, &sArgv);

	PySys_SetObject("stderr", PyTempleConsoleOut_New());
	PySys_SetObject("stdout", PyTempleConsoleOut_New());

	PyTempleImporter_Install();
	PyDebug_Init();

	MainModule = PyImport_ImportModule("__main__");
	MainModuleDict = PyModule_GetDict(MainModule);
	Py_INCREF(MainModuleDict); // "GLOBALS"

	PythonPrepareGlobalNamespace();

	auto m = PyImport_ImportModule("site");
	if (!m) {
		PyErr_Print();
		logger->error("Unable to import Python site module!");
	}
	Py_XDECREF(m);

	console = new PyConsole;
	tig->GetConsole().SetCommandInterpreter(std::bind(&PyConsole::Exec, console, std::placeholders::_1));

	// don't forget PyTempleImporter_Install (when adding new python integrations, for example...)
	pythonObjIntegration.LoadScripts();
	pySpellIntegration.LoadScripts();
	pythonClassIntegration.LoadScripts();
	pythonD20ActionIntegration.LoadScripts();
	pyFeatIntegration.LoadScripts();

	// tpModifiers is imported in conditions.cpp since it needs the condition hashtable

}

PythonScripting::~PythonScripting()
{
	pythonObjIntegration.UnloadScripts();
	pySpellIntegration.UnloadScripts();
	pythonClassIntegration.UnloadScripts();
	pythonD20ActionIntegration.UnloadScripts();

	Py_XDECREF(MainModuleDict);
	Py_XDECREF(MainModule);

	PyGame_Exit();

	py::finalize_interpreter();
}

void PythonScripting::Reset() {
	PyGame_Reset();
}

bool PythonScripting::SaveGame(TioFile *file) {
	return PyGame_Save(file);
}

bool PythonScripting::LoadGame(GameSystemSaveFile * save_file)
{
	return PyGame_Load(save_file);
}

bool PythonScripting::IsPythonScriptId(int script_id)
{
	return pythonObjIntegration.IsValidScriptId(script_id);
}

int PythonScripting::InvokePythonScript(const ScriptInvocation & invocation)
{

	// This seems to be primarily used by the CounterArray
	pythonObjIntegration.SetCounterContext(invocation.attachee, invocation.script->scriptId, invocation.event);
	pythonObjIntegration.SetInObjInvocation(true);

	PyObject* args;
	auto triggerer = PyObjHndl_Create(invocation.triggerer);

	// Expected arguments depend on the event type
	if (invocation.event == ObjScriptEvent::SpellCast) {
		auto attachee = PyObjHndl_Create(invocation.attachee);
		auto spell = PySpell_Create(invocation.spell_id);
		args = Py_BuildValue("OOO", attachee, triggerer, spell);
		Py_DECREF(spell);
		Py_DECREF(attachee);
	}
	else if (invocation.event == ObjScriptEvent::Trap) {
		auto attachee = PyTrap_Create(invocation.attachee);
		args = Py_BuildValue("OO", attachee, triggerer);
		Py_DECREF(attachee);
	}
	else {
		if (invocation.event == ObjScriptEvent::FirstHeartbeat) {
			int dumy = 1;
		}
		auto attachee = PyObjHndl_Create(invocation.attachee);
		args = Py_BuildValue("OO", attachee, triggerer);
		Py_DECREF(attachee);
	}

	auto result = pythonObjIntegration.RunScript(invocation.script->scriptId,
		(PythonIntegration::EventId) invocation.event,
		args);

	Py_DECREF(args);

	auto newSid = pythonObjIntegration.GetNewSid();
	if (newSid != -1 && newSid != invocation.script->scriptId) {
		invocation.script->scriptId = newSid;
	}
	pythonObjIntegration.SetInObjInvocation(false);

	return result;
}

// Python Script Extensions
static class PythonEngineReplacement : public TempleFix {
public:

	static void breakIt() {
		throw TempleException("A function has been called that was stubbed out because it was replaced by TemplePlus. This is a TemplePlus bug.");
	}

	void apply() override {
		// Overwrite all imports from pytoee22.dll with our break function to catch all errors
		uint32_t firstImportAt = 0x1026C294;
		uint32_t lastImportAt = 0x1026C418;
		uint32_t newCallTo = reinterpret_cast<uint32_t>(&breakIt);
		for (auto importAddr = firstImportAt; importAddr <= lastImportAt; importAddr += 4) {
			write(importAddr, &newCallTo, 4);
		}

		replaceFunction(0x100AC960, breakIt);
		replaceFunction(0x100AC920, breakIt);
		replaceFunction(0x100ADDD0, breakIt);
		replaceFunction(0x100AD4C0, breakIt);
		replaceFunction(0x100ADA30, breakIt);
	}

} pythonEngineReplacement;
