
#include "stdafx.h"
#include "python_debug.h"
#include "../condition.h"

/*
	Dumps all conditions from the global hashtable to a Wiki article.
*/
PyObject *PyDebug_DumpConds() {
	
	auto cap = conds.mCondStructHashtable->numItems;
	auto result = PyList_New(cap);

	for (size_t i = 0; i < cap; ++i) {
		auto idx = conds.mCondStructHashtable->idxArray[i];
		auto data = conds.mCondStructHashtable->dataArray[idx];
		
		auto hooks = PyList_New(0);
		auto hook = data->subDispDefs;
		while (hook->dispCallback) {
			auto v = Py_BuildValue("IIIII", hook->dispCallback, hook->data1, hook->data2, hook->dispType, hook->dispKey);
			PyList_Append(hooks, v);
			hook++;
		}

		auto c = Py_BuildValue("sIO", data->condName, data->numArgs, hooks);
		Py_DecRef(hooks);
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

static PyMethodDef PyDebug_Methods[] = {
	{ "dump_conds", (PyCFunction) PyDebug_DumpConds, METH_NOARGS, NULL },
	{ NULL, }
};

void PyDebug_Init() {
	Py_InitModule("debug", PyDebug_Methods);
}
