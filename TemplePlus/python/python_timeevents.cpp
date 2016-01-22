
#include "stdafx.h"
#include "python_timeevents.h"
#include "../gamesystems/timeevents.h"
#include "python_tio.h"
#include <util/fixes.h>
#include <tio/tio.h>

const uint32_t startSentinel = 0xADD2DECA;
const uint32_t endSentinel = 0x9BADDAD5;

static int ProcessEvent(const TimeEvent &evt) {
	auto callable = evt.params[0].pyobj;
	auto argtuple = evt.params[1].pyobj;

	// Auto-Wrap non-tuple arguments
	if (!PyTuple_Check(argtuple)) {
		auto realArgs = PyTuple_New(1);
		PyTuple_SET_ITEM(realArgs, 0, argtuple);
		argtuple = realArgs;
	}
	
	auto res = PyObject_CallObject(callable, argtuple);
	if (!res) {
		logger->error("Unable to execute callback for python time event.");
		PyErr_Print();
	} else {
		Py_DECREF(res);
	}

	Py_XDECREF(callable);
	Py_XDECREF(argtuple);
	return 1;
}

static int SerializeEvent(PyObject *obj, TioFile *file) {

	BOOL result = FALSE;

	uint32_t sentinel = startSentinel;
	if (tio_fwrite(&sentinel, 4, 1, file) != 1) {
		logger->error("Could not write python event start sentinel.");
		return 0;
	}

	if (sentinel != startSentinel) {
		logger->error("Mismatching sentinel {:x} != {:x}", sentinel, startSentinel);
		return 0;
	}

	auto pickleModule = PyImport_ImportModule("cPickle");
	auto pickleDict = PyModule_GetDict(pickleModule);

	auto dumpsFunc = PyDict_GetItemString(pickleDict, "dumps");

	if (dumpsFunc) {
		auto res = PyObject_CallFunctionObjArgs(dumpsFunc, obj, NULL);

		if (!res) {
			logger->error("Unable to pickle python arg to time event.");
			PyErr_Print();
		} else {
			char* buffer;
			Py_ssize_t bufferLen;
			PyString_AsStringAndSize(res, &buffer, &bufferLen);

			if (tio_fwrite(buffer, 1, bufferLen, file) != bufferLen) {
				logger->error("Unable to write pickled python object to game save.");
			} else {
				result = TRUE;
			}

			Py_DECREF(res);			
		}
	}

	Py_DECREF(pickleModule);

	sentinel = endSentinel;
	if (tio_fwrite(&sentinel, 4, 1, file) != 1) {
		logger->error("Could not write python event start sentinel.");
		return FALSE;
	}

	return result;

}

static int DeserializeEvent(PyObject **pOut, TioFile *file) {

	*pOut = nullptr;
	
	uint32_t sentinel;
	if (tio_fread(&sentinel, 4, 1, file) != 1) {
		logger->error("Could not read python event start sentinel.");
		return 0;
	}

	if (sentinel != startSentinel) {
		logger->error("Mismatching sentinel {:x} != {:x}", sentinel, startSentinel);
		return 0;
	}

	auto pickleModule = PyImport_ImportModule("templeplus.legacypickle");
	if (!pickleModule) {
		PyErr_Print();
		return FALSE;
	}
	auto pickleDict = PyModule_GetDict(pickleModule);

	auto loadFunc = PyDict_GetItemString(pickleDict, "load");

	if (loadFunc) {
		auto fileWrapper = PyTioFile_FromTioFile(file);
		
		auto res = PyObject_CallFunctionObjArgs(loadFunc, fileWrapper, NULL);
		*pOut = res;

		if (!res) {
			logger->error("Unable to unpickle python arg to time event.");
			PyErr_Print();
		} else {
			if (tio_fread(&sentinel, 4, 1, file) == 1) {
				if (sentinel != endSentinel) {
					logger->error("Python time event sentinel is wrong: {:x} != {:x}", sentinel, endSentinel);
					Py_XDECREF(*pOut);
					*pOut = nullptr;
				}
			} else {
				logger->error("Unable to read python time event end sentinel.");
				Py_XDECREF(*pOut);
				*pOut = nullptr;
			}			
		}

		PyTioFile_Invalidate(fileWrapper);
		Py_DECREF(fileWrapper);
	}

	Py_DECREF(pickleModule);

	return *pOut != nullptr;
}

static class PythonTimeEventFix : TempleFix {
public:
	const char* name() override {
		return "Python Timeevent Replacement";
	}
	
	void apply() override {
		replaceFunction(0x100AD560, ProcessEvent);
		replaceFunction(0x100AD600, SerializeEvent);
		replaceFunction(0x100AD7C0, DeserializeEvent);
	}
} fix;
