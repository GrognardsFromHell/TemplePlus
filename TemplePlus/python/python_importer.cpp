
#include "stdafx.h"
#include "Python.h"
#include "osdefs.h"
#include "tio/tio.h"
#include "python_importer.h"

enum ImporterFileType {
	IFT_NONE = 0,
	IFT_SOURCE = 1,
	IFT_BYTECODE = 2,
	IFT_PACKAGE = 4
};

struct PyTempleImporterSearchOrder {
	string suffix;
	int type;
};

struct ModuleInfo {
	bool found = false;
	bool package = false;
	uint32_t lastModified = 0; // Applies to source
	string sourcePath;
	string compiledPath;
	string packagePath; // Path to directory without __init__.py suffix if package=true
};

class PyTempleImporter {
public:
	PyTempleImporter();

	// "Finder" interface
	static PyObject *FindModule(PyObject *self, PyObject* args);

	// "Loader" interface
	static PyObject *LoadModule(PyObject *self, PyObject* args);

	void CreateSearchOrder();

	ModuleInfo GetModuleInfo(const string &fullname);

	static PyObject *ReadData(const string &path);

	static PyTempleImporter *instance;

	PyObject *mFinder;
	vector<PyTempleImporterSearchOrder> mSearchOrder;
	vector<string> mSearchPath;
};

PyTempleImporter* PyTempleImporter::instance = nullptr;

static PyMethodDef pyTempleImporterMethods[] = {
	{ "find_module", &PyTempleImporter::FindModule, METH_VARARGS, 0 },	
	{ "load_module", &PyTempleImporter::LoadModule, METH_VARARGS, 0 },	
	{ NULL, NULL }   /* sentinel */
};

static PyTypeObject PyTempleImporterType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"PyTempleImporter",
	sizeof(PyObject),
	0,                                          /* tp_itemsize */
	(destructor)PyObject_Free,					/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_compare */
	0,											/* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	PyObject_GenericGetAttr,                    /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,							/* tp_flags */
	0,											/* tp_doc */
	0,											/* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	pyTempleImporterMethods,
	0
};

PyTempleImporter::PyTempleImporter() {
	if (PyType_Ready(&PyTempleImporterType) < 0)
		throw TempleException("Python type has not yet been registered");

	CreateSearchOrder();

	mFinder = PyObject_New(PyObject, &PyTempleImporterType);

	// Insert into python meta_path system
	auto path_hooks = PySys_GetObject("meta_path");
	PyList_Insert(path_hooks, 0, mFinder);
}

PyObject* PyTempleImporter::FindModule(PyObject* self, PyObject* args) {
	char *fullname;
	PyObject *path = nullptr;

	if (!PyArg_ParseTuple(args, "s|O:PyTempleImporter.FindModule", &fullname, &path)) {
		return nullptr;
	}

	auto moduleInfo = instance->GetModuleInfo(fullname);
	if (moduleInfo.found) {
		Py_INCREF(self);
		return self;
	}

	// Returning none here means we cannot handle the module
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* PyTempleImporter::LoadModule(PyObject* self, PyObject* args) {

	char *fullname;
	if (!PyArg_ParseTuple(args, "s:PyTempleImporter.LoadModule", &fullname))
		return nullptr;

	auto moduleInfo = instance->GetModuleInfo(fullname);
	if (!moduleInfo.found) {
		// If it was found before, why not now?
		return nullptr;
	}

	// Try loading compiled code first
	if (moduleInfo.sourcePath.empty()) {
		throw TempleException("Bytecode not supported yet");
	}
	auto data = ReadData(moduleInfo.sourcePath);
	auto code = Py_CompileString(PyString_AsString(data), moduleInfo.sourcePath.c_str(), Py_file_input);

	if (!code) {
		return nullptr; // Compilation error
	}

	auto module = PyImport_AddModule(fullname);	
	if (!module) {
		Py_DECREF(code);
		return nullptr;
	}

	auto moduleDict = PyModule_GetDict(module);
	if (PyDict_SetItemString(moduleDict, "__loader__", self) != 0) {
		Py_DECREF(code);
		Py_DECREF(module);
		return nullptr;
	}

	if (moduleInfo.package) {
		PyObject *packagePath = PyString_FromString(moduleInfo.packagePath.c_str());
		PyObject *packagePathList = Py_BuildValue("[O]", packagePath);
		Py_DECREF(packagePath);
		if (!packagePathList) {
			Py_DECREF(code);
			Py_DECREF(module);
			return nullptr;
		}
		auto err = PyDict_SetItemString(moduleDict, "__path__", packagePathList);
		Py_DECREF(packagePathList);
		if (err != 0) {
			Py_DECREF(code);
			Py_DECREF(module);
			return nullptr;
		}
	}

	module = PyImport_ExecCodeModuleEx(fullname, code, const_cast<char*>(moduleInfo.sourcePath.c_str()));
	Py_DECREF(code);
	
	return module;
}

void PyTempleImporter::CreateSearchOrder() {

	/* zip_searchorder defines how we search for a module in the Zip
	archive: we first search for a package __init__, then for
	non-package .pyc, .pyo and .py entries. The .pyc and .pyo entries
	are swapped by initzipimport() if we run in optimized mode. Also,
	'/' is replaced by SEP there. */
	mSearchOrder.push_back({ SEP + string("__init__.pyc"), IFT_PACKAGE | IFT_BYTECODE });
	mSearchOrder.push_back({ SEP + string("__init__.pyo"), IFT_PACKAGE | IFT_BYTECODE });
	mSearchOrder.push_back({ SEP + string("__init__.py"), IFT_PACKAGE | IFT_SOURCE });
	mSearchOrder.push_back({ ".pyc", IFT_BYTECODE });
	mSearchOrder.push_back({ ".pyo", IFT_BYTECODE });
	mSearchOrder.push_back({ ".py", IFT_SOURCE });
	mSearchOrder.push_back({ "", IFT_NONE });

	if (Py_OptimizeFlag) {
		swap(mSearchOrder[0], mSearchOrder[1]);
		swap(mSearchOrder[3], mSearchOrder[4]);
	}
}

ModuleInfo PyTempleImporter::GetModuleInfo(const string& fullname) {

	ModuleInfo result;

	string relPath = fullname;
	for (auto &ch : relPath) {
		if (ch == '.') {
			ch = SEP;
		}
	}
	
	for (auto prefix : mSearchPath) {
		for (auto entry : mSearchOrder) {

			// Is this an entry for sth we already found?
			if (result.found) {				
				if ((entry.type & IFT_BYTECODE) && !result.compiledPath.empty()) {
					// Search order entry is for bytecode, which we already found
					continue;
				} else if (!(entry.type & IFT_BYTECODE) && !result.sourcePath.empty()) {
					// Search order entry is for source code, which we already found
					continue;
				}
			}

			auto path = format("{}{}{}", prefix, relPath, entry.suffix);

			TioFile *fh;
			if ((fh = tio_fopen(path.c_str(), "rb")) != nullptr) {
				result.found = true;
				result.package = (entry.type & IFT_PACKAGE) != 0;
				if (result.package) {
					result.packagePath = format("{}{}{}", prefix, relPath, SEP);
				}

				if (entry.type & IFT_BYTECODE) {
					result.compiledPath = path;
				} else {
					result.sourcePath = path;
				
					// Try getting the mtime for the source
					TioFileListFile fileInfo;
					if (!tio_fstat(fh, &fileInfo)) {
						result.lastModified = fileInfo.lastModified;
					}
				}

				tio_fclose(fh);
			}
		}

		if (result.found) {
			for (size_t i = 0; i < result.packagePath.size(); ++i) {
				if (result.packagePath[i] == '\\') {
					result.packagePath[i] = '/';	
				}
			}
			for (size_t i = 0; i < result.sourcePath.size(); ++i) {
				if (result.sourcePath[i] == '\\') {
					result.sourcePath[i] = '/';	
				}	
			}
			
			return result;
		}
	}

	return result;

}

PyObject* PyTempleImporter::ReadData(const string& path) {
	auto fh = tio_fopen(path.c_str(), "rb");

	if (!fh) {
		PyErr_SetFromErrnoWithFilename(PyExc_IOError, path.c_str());
		return nullptr;
	}

	auto len = tio_filelength(fh);

	auto rawData = PyString_FromStringAndSize(nullptr, len + 1);
	if (rawData == nullptr) {
		tio_fclose(fh);
		return nullptr;
	}

	auto buf = PyString_AsString(rawData);

	if (tio_fread(buf, 1, len, fh) != len) {
		tio_fclose(fh);
		PyErr_SetString(PyExc_IOError,
			"PyTempleImporter: can't read data");
		Py_DECREF(rawData);
		return nullptr;
	}

	tio_fclose(fh);

	buf[len] = '\0';

	return rawData;
}

void PyTempleImporter_Install() {
	assert(!PyTempleImporter::instance);
	PyTempleImporter::instance = new PyTempleImporter;
	PyTempleImporter::instance->mSearchPath.push_back("python-lib/");
	PyTempleImporter::instance->mSearchPath.push_back("templeplus/lib/");
	PyTempleImporter::instance->mSearchPath.push_back("scr/");
	PyTempleImporter::instance->mSearchPath.push_back("rules/races/");
	PyTempleImporter::instance->mSearchPath.push_back("rules/");
	PyTempleImporter::instance->mSearchPath.push_back("rules/char_class/");
	PyTempleImporter::instance->mSearchPath.push_back("rules/d20_actions/");
	PyTempleImporter::instance->mSearchPath.push_back("scr/feats/");
}

void PyTempleImporter_Uninstall() {
	delete PyTempleImporter::instance;
	PyTempleImporter::instance = nullptr;
}
