
#pragma once

class PyConsole {
public:
	PyConsole();
	~PyConsole();

	void Exec(const string &command);
private:
	PyObject *mLocals = nullptr;
	PyObject *mMainModule = nullptr;
};
