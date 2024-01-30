
#include "stdafx.h"

#include <regex>

#include "python_integration.h"
#include "tio/tio.h"
#include "python_embed.h"
#include <infrastructure/elfhash.h>

PythonIntegration::PythonIntegration(const string& searchPattern, const string& filenameRegexp, bool isHashId) {
	mSearchPattern = searchPattern;
	mFilenameRegexp = filenameRegexp;
	mIsHashId = isHashId;
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
		if (mIsHashId)
			record.id = ElfHash::Hash(scriptMatch[2]);
		else
			record.id = stoi(scriptMatch[2]);

		if (mScripts.find(record.id) != mScripts.end()) {
			if (record.id == 522){ // ugly hack for glibness spell - damn you troika!
				if (strstr(record.filename.c_str(), "Wall of Fire") == nullptr){
					logger->error("Multiple scripts have the id {}. Skipping {}.", record.id, scriptName);
					continue;
				}
			}
			else{
				logger->error("Multiple scripts have the id {}. Skipping {}.", record.id, scriptName);
				continue;
			}
			
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
	if (PyInt_Check(resultObj) || PyLong_Check(resultObj)) {
		result = PyInt_AsLong(resultObj);
	}

	Py_DECREF(resultObj);

	return result;
}

int PythonIntegration::RunScriptDefault0(ScriptId scriptId, EventId evt, PyObject * args)
{

	ScriptRecord script;
	if (!LoadScript(scriptId, script)) {
		return 0;
	}

	auto dict = PyModule_GetDict(script.module);
	auto eventName = GetFunctionName(evt);
	auto callback = PyDict_GetItemString(dict, eventName);

	if (!callback || !PyCallable_Check(callback)) {
		/*logger->error("Script {} attached as {} is missing the corresponding function.",
			script.filename, eventName);*/
		return 0;
	}

	auto resultObj = PyObject_CallObject(callback, args);

	if (!resultObj) {
		logger->error("An error occurred while calling event {} for script {}.", eventName, script.filename);
		PyErr_Print();
		return 0;
	}

	auto result = 0;
	if (PyInt_Check(resultObj)) {
		result = PyInt_AsLong(resultObj);
	}
	Py_DECREF(resultObj);

	return result; 
}

std::string PythonIntegration::RunScriptStringResult(ScriptId scriptId, EventId evt, PyObject * args){

	ScriptRecord script;
	if (!LoadScript(scriptId, script)) {
		return fmt::format("");
	}

	auto dict = PyModule_GetDict(script.module);
	auto eventName = GetFunctionName(evt);
	auto callback = PyDict_GetItemString(dict, eventName);

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("Script {} attached as {} is missing the corresponding function.",
			script.filename, eventName);
		return fmt::format("");
	}

	auto resultObj = PyObject_CallObject(callback, args);

	if (!resultObj) {
		logger->error("An error occurred while calling event {} for script {}.", eventName, script.filename);
		PyErr_Print();
		return fmt::format("");
	}

	auto result = fmt::format("");
	if (PyString_Check(resultObj)) {
		result.append(PyString_AsString(resultObj));
	}
	Py_DECREF(resultObj);

	return result;
}

std::map<int, std::vector<int>> PythonIntegration::RunScriptMapResult(ScriptId scriptId, EventId evt, PyObject * args)
{
	ScriptRecord script;
	if (!LoadScript(scriptId, script)) {
		return std::map<int, std::vector<int>>();
	}

	auto dict = PyModule_GetDict(script.module);
	auto eventName = GetFunctionName(evt);
	auto callback = PyDict_GetItemString(dict, eventName);

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("Script {} attached as {} is missing the corresponding function.",
			script.filename, eventName);
		return std::map<int, std::vector<int>>();
	}

	auto resultObj = PyObject_CallObject(callback, args);

	if (!resultObj) {
		logger->error("An error occurred while calling event {} for script {}.", eventName, script.filename);
		PyErr_Print();
		return std::map<int, std::vector<int>>();
	}

	auto result = std::map<int, std::vector<int>>();
	if (!PyDict_Check(resultObj)) {
		return std::map<int, std::vector<int>>();
	}

	PyObject *pykey, *pyvalue;
	Py_ssize_t pos = 0;

	while (PyDict_Next(resultObj, &pos, &pykey, &pyvalue)) {
		if (!PyInt_Check(pykey)){
			logger->warn("RunScriptMapResult: Invalid python value or key");
			continue;
		}
		auto key = PyInt_AsLong(pykey);

		if (PyInt_Check(pyvalue)){
			auto value = PyInt_AsLong(pyvalue);
			result[key] = std::vector<int>({ value });
		}
		else if (PyTuple_Check(pyvalue)){
			result[key] = std::vector<int>();
			for (auto i = 0; i < PyTuple_Size(pyvalue); i++){
				auto value = PyTuple_GetItem(pyvalue, i);
				if (PyInt_Check(value))
					result[key].push_back(PyInt_AsLong(value));
				else if (PyString_Check(value))
				{
					result[key].push_back(ElfHash::Hash(PyString_AsString(value)));
				}
			}
		}
		
	}

	Py_DECREF(resultObj);
	return result;
}

std::vector<int> PythonIntegration::RunScriptVectorResult(ScriptId scriptId, EventId evt, PyObject * args)
{
	auto result = std::vector<int>();
	ScriptRecord script;
	if (!LoadScript(scriptId, script)) {
		return result;
	}

	auto dict = PyModule_GetDict(script.module);
	auto eventName = GetFunctionName(evt);
	auto callback = PyDict_GetItemString(dict, eventName);

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("Script {} attached as {} is missing the corresponding function.",
			script.filename, eventName);
		return result;
	}

	auto resultObj = PyObject_CallObject(callback, args);

	if (!resultObj) {
		logger->error("An error occurred while calling event {} for script {}.", eventName, script.filename);
		PyErr_Print();
		return result;
	}

	if (!PyList_Check(resultObj)){
		return result;
	}

	PyObject *pyvalue;
	Py_ssize_t pos = 0;

	auto listSize = PyList_Size(resultObj);
	for (auto i=0; i < listSize; i++){
		pyvalue = PyList_GetItem(resultObj, i);
		if (!PyInt_Check(pyvalue)){
			logger->warn("RunScriptVectorResult: Invalid python value");
			continue;
		}
		result.push_back(PyInt_AsLong(pyvalue));
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

	auto func = PyDict_GetItemString(MainModuleDict, "critter_is_unconscious");
	Expects(func != nullptr);

	// We have not yet loaded the Python module
	if (!script.module) {
		script.module = PyImport_ImportModule(&script.moduleName[0]);
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
	PyDict_Merge(dict, MainModuleDict, 0);
}
