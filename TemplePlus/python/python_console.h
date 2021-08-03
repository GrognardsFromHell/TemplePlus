
#pragma once

class PyConsole {
public:
	PyConsole();
	~PyConsole();

	void Exec(const std::string &command);
private:
	PyObject *mLocals = nullptr;
	PyObject *mMainModule = nullptr;
};
