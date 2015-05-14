
#include "stdafx.h"
#include "python_console.h"
#include "python_consoleout.h"

PyConsole::PyConsole() {
	mMainModule = PyImport_AddModule("__main__");
	mLocals = PyDict_New();
}

PyConsole::~PyConsole() {
	Py_DECREF(mMainModule);
	Py_DECREF(mLocals);
}

void PyConsole::Exec(const string& command) {

	auto globals = PyModule_GetDict(mMainModule);
	auto result = PyRun_String(command.c_str(), Py_single_input, globals, mLocals);

	if (!result) {
		PyErr_Print();
	} else {
		Py_DECREF(result);
	}
}
