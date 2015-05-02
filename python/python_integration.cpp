
#include "stdafx.h"

#include <regex>

#include "python_integration.h"
#include "tio/tio.h"
#include "python_embed.h"

PythonIntegration::PythonIntegration(const string& searchPattern, const string& filenameRegexp) {
	mSearchPattern = searchPattern;
	mFilenameRegexp = filenameRegexp;
}

PythonIntegration::~PythonIntegration() {
}

void PythonIntegration::LoadScripts() {
	// Enumerate all scripts in scr that would be accessible via a script number
	// and preload them
	logger->info("Discovering Python scripts...");

	TioFileList list;
	tio_filelist_create(&list, mSearchPattern.c_str());

	regex scriptRegex(mFilenameRegexp, regex_constants::ECMAScript | regex_constants::icase);
	smatch scriptMatch;

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

bool PythonIntegration::IsValidScriptId(ScriptId scriptId) {
	return mScripts.find(scriptId) != mScripts.end();
}

int PythonIntegration::RunScript(ScriptId scriptId, EventId evt, PyObject* args) {

	ScriptRecord script;
	if (!LoadScript(scriptId, script)) {
		return 1;
	}

	auto dict = PyModule_GetDict(script.module);
	auto eventName = GetFunctionName(evt);
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

bool PythonIntegration::LoadScript(int scriptId, ScriptRecord &scriptOut) {
	auto it = mScripts.find(scriptId);
	if (it == mScripts.end()) {
		logger->error("Trying to invoke unknown script id {}", scriptId);
		return false;
	}

	auto &script = it->second;
	if (script.loadingError) {
		return false; // cannot execute a script we previously failed to load
	}

	// We have not yet loaded the Python module
	if (!script.module) {
		script.module = PyImport_ImportModule(script.moduleName.c_str());
		if (!script.module) {
			// Loading error
			logger->error("Could not load script {}.", script.filename);
			PyErr_Print();
			script.loadingError = true; // Do not try this over and over again
			return false;
		} else {
			// Add globals on demand
			auto dict = PyModule_GetDict(script.module);
			AddGlobalsOnDemand(dict);
		}
	}

	scriptOut = script;
	return true;
}


/*
	Will check if the given dict contains one of the constants and if not present,
	will import them automatically.
*/
void PythonIntegration::AddGlobalsOnDemand(PyObject* dict) {

	if (PyDict_GetItemString(dict, "stat_strength")) {
		return; // Already imported
	}

	PyDict_Merge(dict, MainModuleDict, 0);
}
