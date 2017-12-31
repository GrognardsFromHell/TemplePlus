
#pragma once 

struct TioFile;
struct GameSystemSaveFile;
struct ScriptInvocation;

class PythonScripting {
public:
	PythonScripting();
	~PythonScripting();

	void Reset();
	bool SaveGame(TioFile *file);
	bool LoadGame(GameSystemSaveFile *save_file);


	bool IsPythonScriptId(int script_id);
	int InvokePythonScript(const ScriptInvocation &invocation);
};

extern PyObject *MainModuleDict;
