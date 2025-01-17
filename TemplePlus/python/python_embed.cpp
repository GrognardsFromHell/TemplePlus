
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
#include "python_module.h"
#include "python_dispatcher.h"

#include "../gamesystems/gamesystems.h"
#include "python_integration_class_spec.h"
#include "python_integration_d20_action.h"
#include "python_integration_feat.h"

#include <pybind11/embed.h>
#include "python_integration_race.h"
#include <python/python_spell.h>

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

static bool __cdecl PythonInit(GameSystemConf *conf) {

	static char* sArgv = "";

	Py_OptimizeFlag++;
	Py_VerboseFlag++;
	Py_NoSiteFlag++;
	Py_SetProgramName("TemplePlus.exe");

	Py_Initialize();

	PySys_SetArgv(0, &sArgv);
	
	PySys_SetObject("stderr", PyTempleConsoleOut_New());
	PySys_SetObject("stdout", PyTempleConsoleOut_New());
		
	PyTempleImporter_Install();
	PyToeeInitModule();
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

	//Always bring in the toee module for convenience
	console->Exec("import toee");

	// don't forget PyTempleImporter_Install (when adding new python integrations, for example...)
	pythonObjIntegration.LoadScripts();
	pySpellIntegration.LoadScripts();
	pythonRaceIntegration.LoadScripts();
	pythonClassIntegration.LoadScripts();
	pythonD20ActionIntegration.LoadScripts(); 
	pyFeatIntegration.LoadScripts();
	
	// tpModifiers is imported in conditions.cpp since it needs the condition hashtable

	return true;
}

static BOOL __cdecl PythonSaveGame(TioFile *file) {
	return PyGame_Save(file) ? TRUE : FALSE;
}

static BOOL __cdecl PythonLoadGame(GameSystemSaveFile *file) {
	return PyGame_Load(file) ? TRUE : FALSE;
}

static void __cdecl PythonReset() {
	PyGame_Reset();
	PySpell_Reset();
}

static void __cdecl PythonExit() {
	pythonObjIntegration.UnloadScripts();
	pySpellIntegration.UnloadScripts();
	pythonRaceIntegration.UnloadScripts();
	pythonClassIntegration.UnloadScripts();
	pythonD20ActionIntegration.UnloadScripts();

	Py_XDECREF(MainModuleDict);
	Py_XDECREF(MainModule);

	PyGame_Exit();

	Py_Finalize();
}

static void breakIt() {
	_CrtDbgBreak();
}

// Python Script Extensions
static class PythonEngineReplacement : public TempleFix {
public:
	void apply() override {
		// Overwrite all imports from pytoee22.dll with our break function to catch all errors
		uint32_t firstImportAt = 0x1026C294;
		uint32_t lastImportAt = 0x1026C418;
		uint32_t newCallTo = reinterpret_cast<uint32_t>(&breakIt);
		for (auto importAddr = firstImportAt; importAddr <= lastImportAt; importAddr += 4) {
			write(importAddr, &newCallTo, 4);
		}

		replaceFunction(0x100AC960, PythonSaveGame);
		replaceFunction(0x100AC920, PythonLoadGame);
		replaceFunction(0x100ADDD0, PythonReset);
		replaceFunction(0x100AD4C0, PythonExit);
		replaceFunction(0x100ADA30, PythonInit);
	}

} pythonEngineReplacement;
