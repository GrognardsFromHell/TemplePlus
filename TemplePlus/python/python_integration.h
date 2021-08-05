
#pragma once

#include "../obj.h"

/*
	Describes a script used in an event handler.
*/
struct ScriptRecord {
	int id;
	std::string moduleName;
	std::string filename;
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

	PythonIntegration(const std::string &searchPattern, const std::string &filenameRegexp, bool hashId = false);
	virtual ~PythonIntegration();

	void LoadScripts();
	void UnloadScripts();

	bool IsValidScriptId(ScriptId scriptId);

	int RunScript(ScriptId scriptId, EventId eventId, PyObject *args);
	int RunScriptDefault0(ScriptId scriptId, EventId eventId, PyObject *args);
	std::string RunScriptStringResult(ScriptId scriptId, EventId eventId, PyObject *args);
	std::map<int, std::vector<int>> RunScriptMapResult(ScriptId scriptId, EventId eventId, PyObject* args);
	std::vector<int> RunScriptVectorResult(ScriptId scriptId, EventId eventId, PyObject* args);
	
	/*
		Gets a loaded instance of a script module or null if loading failed.
		Returns a borrowed ref
	*/
	bool LoadScript(ScriptId scriptId, ScriptRecord &scriptOut);

protected:
	virtual const char *GetFunctionName(EventId eventId) = 0;
	void AddGlobalsOnDemand(PyObject* dict);
	typedef std::unordered_map<int, ScriptRecord> ScriptCache;
	ScriptCache mScripts;
private:
	
	bool mIsHashId;
	std::string mSearchPattern;
	std::string mFilenameRegexp;
};


