
#pragma once

/*
	Utility class to extend ToEE's Python with new global functionality.
*/
class PythonGlobalExtension {
public:
	PythonGlobalExtension();
	virtual ~PythonGlobalExtension();

	virtual void extend(PyObject *globals) = 0;
	
	static void installExtensions();
private:
	static vector<PythonGlobalExtension*> &extensions();
};

