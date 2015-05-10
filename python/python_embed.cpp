
#include "stdafx.h"
#include <gamesystems.h>
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
#include "python_hooks.h"
#include "tio/tio.h"
#include <set>
#include "python_module.h"

static struct PythonInitInternal : AddressTable {

	// Writes to the console window
	size_t (__cdecl *PythonConsoleWrite)(const void* ptr, size_t size, size_t count, void* handle);

	void(__cdecl *TigConsoleSetCommandInterpreter)(void(__cdecl*)(const char*));

	PythonInitInternal() {
		rebase(PythonConsoleWrite, 0x101DFE90);
		rebase(TigConsoleSetCommandInterpreter, 0x101DF820);
	}	
} pythonInitInternal;

static PyObject *MainModule;
PyObject *MainModuleDict;
static PyConsole *console;

// Wrapper for PyConsole::Exec
static void __cdecl PyConsole_Exec(const char *str) {
	if (console) {
		console->Exec(str);
	}
}

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
	auto utilsDict = PyModule_GetDict(toeeModule);
	if (PyDict_Merge(MainModuleDict, utilsDict, FALSE) == -1) {
		logger->error("Unable to import utilities.* into __main__");
		PyErr_Print();
	}
	Py_DECREF(utilsModule);

	auto cheats = PyCheats_Create();
	PyDict_SetItemString(MainModuleDict, "cheats", cheats);
	Py_DECREF(cheats);
}

// Forward declare the init functions of the two librocket modules
extern "C" {
	void init_rocketcore();
	void init_rocketcontrols();
}

static bool __cdecl PythonInit(GameSystemConf *conf) {

	Py_OptimizeFlag++;
	Py_VerboseFlag++;
	Py_NoSiteFlag++;
	PyImport_AppendInittab("_hooks", init_hooks);
	Py_SetProgramName("TemplePlus.exe");
	Py_Initialize();
	
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
	pythonInitInternal.TigConsoleSetCommandInterpreter(PyConsole_Exec);

	pythonObjIntegration.LoadScripts();
	pythonSpellIntegration.LoadScripts();

	init_rocketcore();
	init_rocketcontrols();

	return true;
}

static bool __cdecl PythonSaveGame(TioFile *file) {
	return PyGame_Save(file);
}

static bool __cdecl PythonLoadGame(GameSystemSaveFile *file) {
	return PyGame_Load(file);
}

static void __cdecl PythonReset() {
	PyGame_Reset();
}

static void __cdecl PythonExit() {
	pythonObjIntegration.UnloadScripts();
	pythonSpellIntegration.UnloadScripts();

	Py_XDECREF(MainModuleDict);
	Py_XDECREF(MainModule);

	PyGame_Exit();

	Py_Finalize();
}

static void breakIt() {
	_CrtDbgBreak();
}

static class PythonEngineReplacement : public TempleFix {
public:
	const char* name() override {
		return "Python Script Extensions";
	}

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
