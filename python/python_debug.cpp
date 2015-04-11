
#include "stdafx.h"
#include "python_debug.h"

vector<PythonDebugFunc*> &PythonDebugFunc::registry() {
	static vector<PythonDebugFunc*> *reg = nullptr;
	if (!reg) {
		reg = new vector<PythonDebugFunc*>();
	}
	return *reg;
}
