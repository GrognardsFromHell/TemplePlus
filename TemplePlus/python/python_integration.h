
#pragma once

#include "../obj.h"

/*
	Describes a script used in an event handler.
*/
struct ScriptRecord {
	int id;
	string moduleName;
	string filename;
	PyObject *module = nullptr; // If not null, owns the ref
	bool loadingError = false;
};

/*
	Base for integration points between the ToEE engine and the Python scripting system.
*/
class PythonIntegration {
public:
	typedef int ScriptId;
	typedef int EventId;

	PythonIntegration(const string &searchPattern, const string &filenameRegexp);
	virtual ~PythonIntegration();

	void LoadScripts();
	void UnloadScripts();

	bool IsValidScriptId(ScriptId scriptId);

	int RunScript(ScriptId scriptId, EventId eventId, PyObject *args);

	/*
		Gets a loaded instance of a script module or null if loading failed.
		Returns a borrowed ref
	*/
	bool LoadScript(ScriptId scriptId, ScriptRecord &scriptOut);

protected:
	virtual const char *GetFunctionName(EventId eventId) = 0;
	void AddGlobalsOnDemand(PyObject* dict);

private:
	typedef unordered_map<int, ScriptRecord> ScriptCache;
	ScriptCache mScripts;
	string mSearchPattern;
	string mFilenameRegexp;
};

void RunAnimFramePythonScript(const char *command);
