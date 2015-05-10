#include "stdafx.h"
#include <util/addresses.h>

extern "C" {
#include "ctypes/ffi.h"
#include "ctypes/ctypes.h"
}

unordered_map<uint32_t, PyCFuncPtrObject*> hookTable;

static const char* GetMinhookError(MH_STATUS status);

PyObject* PyHooks_ReplaceFunction(PyObject*, PyObject* args) {
	uint32_t address;
	PyCFuncPtrObject* funcPtr;
	if (!PyArg_ParseTuple(args, "iO!:_hooks.replace_func", &address, &PyCFuncPtr_Type, &funcPtr)) {
		return 0;
	}

	// Clear previous hook
	auto it = hookTable.find(address);
	if (it != hookTable.end()) {
		// TODO: Restore using MH?
		Py_DECREF(it->second);
		hookTable.erase(it);
	}

	auto target = reinterpret_cast<LPVOID>(address);
	LPVOID oldFunc;
	auto status = MH_CreateHook(target, funcPtr->thunk->pcl_exec, &oldFunc);
	if (status != MH_OK) {
		PyErr_SetString(PyExc_RuntimeError, GetMinhookError(status));
		return 0;
	}

	status = MH_EnableHook(target);
	if (status != MH_OK) {
		if (MH_RemoveHook(target) != MH_OK) {
			logger->error("Unable to remove previously created hook in error handler");
		}
		PyErr_SetString(PyExc_RuntimeError, GetMinhookError(status));
		return 0;
	}

	Py_INCREF(funcPtr); // We need some form of cleanup i'd guess
	hookTable[address] = funcPtr;

	// Create a function pointer from the old address
	auto resArgs = Py_BuildValue("(i)", oldFunc);
	auto result = funcPtr->ob_type->tp_new(funcPtr->ob_type, resArgs, NULL);
	Py_DECREF(resArgs);

	return result;
}

PyObject* PyHooks_RestoreFunction(PyObject*, PyObject* args) {
	Py_RETURN_NONE;
}

PyObject* PyHooks_Rebase(PyObject*, PyObject* args) {
	uint32_t address;
	if (!PyArg_ParseTuple(args, "i:_hooks.rebase", &address)) {
		return 0;
	}

	if (address <= 0x10000000 || address >= 0x20000000) {
		PyErr_Format(PyExc_ValueError, "address must be between 0x10000000 and 0x20000000 (both exclusive). Was: 0x%x", address);
		return 0;
	}

	address = reinterpret_cast<uint32_t>(temple_address(address));
	return PyInt_FromSize_t(address);
}

static PyMethodDef HookMethods[] = {
	{"replace_func", PyHooks_ReplaceFunction, METH_VARARGS, NULL},
	{"restore_func", PyHooks_RestoreFunction, METH_VARARGS, NULL},
	{"rebase", PyHooks_Rebase, METH_VARARGS, NULL},
	{NULL, NULL, NULL, NULL}
};

PyMODINIT_FUNC init_hooks() {
	Py_InitModule("_hooks", HookMethods);
}

static const char* GetMinhookError(MH_STATUS status) {
	switch (status) {
	case MH_OK:
		return "OK";
	case MH_ERROR_ALREADY_INITIALIZED:
		return "MinHook is already initialized.";
	case MH_ERROR_NOT_INITIALIZED:
		return "MinHook is not initialized yet, or already uninitialized.";

	case MH_ERROR_ALREADY_CREATED:
		return "The hook for the specified target function is already created.";

	case MH_ERROR_NOT_CREATED:
		return "The hook for the specified target function is not created yet.";

	case MH_ERROR_ENABLED:
		return "The hook for the specified target function is already enabled.";

	case MH_ERROR_DISABLED:
		return "The hook for the specified target function is not enabled yet, or already disabled.";

	case MH_ERROR_NOT_EXECUTABLE:
		return "The specified pointer is invalid. It points the address of non-allocated and/or non-executable region.";

	case MH_ERROR_UNSUPPORTED_FUNCTION:
		return "The specified target function cannot be hooked.";

	case MH_ERROR_MEMORY_ALLOC:
		return "Failed to allocate memory";

	case MH_ERROR_MEMORY_PROTECT:
		return "Failed to change the memory protection.";

	case MH_ERROR_MODULE_NOT_FOUND:
		return "The specified module is not loaded.";

	case MH_ERROR_FUNCTION_NOT_FOUND:
		return "The specified function is not found.";
	default:
		return "Unknown Error";
	}

}
