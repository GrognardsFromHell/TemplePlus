
#include "stdafx.h"
#include <gamesystems.h>
#include <util/fixes.h>
#include "python_debug.h"
#include "python_importer.h"
#include "python_consoleout.h"
#include "tio/tio.h"

extern "C" {
	#include "ioredir.h"
	IoRedir *ioredir;
}

static void init_ioredir();

static struct PythonInitInternal : AddressTable {

	// Writes to the console window
	size_t (__cdecl *PythonConsoleWrite)(const void* ptr, size_t size, size_t count, void* handle);

	PythonInitInternal() {
		rebase(PythonConsoleWrite, 0x101DFE90);
	}	
} pythonInitInternal;

static bool __cdecl script_init_python(GameSystemConf *conf) {

	ioredir = new IoRedir;
	ioredir->clearerr = (ior_clearerr)tio_clearerr;
	ioredir->feof = (ior_feof)tio_feof;
	ioredir->fclose = (ior_fclose)tio_fclose;
	ioredir->fflush = (ior_fflush)tio_fflush;
	ioredir->fgets = (ior_fgets)tio_fgets;
	ioredir->fopen = (ior_fopen)tio_fopen;
	ioredir->ferror = (ior_ferror)tio_ferror;
	ioredir->fprintf = (ior_fprintf)tio_fprintf;
	ioredir->fputc = (ior_fputc)tio_fputc;
	ioredir->fgetpos = (ior_fgetpos)tio_fgetpos;
	ioredir->fread = (ior_fread)tio_fread;
	ioredir->fseek = (ior_fseek)tio_fseek;
	ioredir->open_exclusive = (ior_fopen_exclusive)tio_fopen_exclusive;
	ioredir->fstat = (ior_fstat)tio_fstat;
	ioredir->ftell = (ior_ftell)tio_ftell;
	ioredir->fputs = (ior_fputs)tio_fputs;
	ioredir->fgetc = (ior_fgetc)tio_fgetc;
	ioredir->isatty = (ior_isatty)tio_isatty;
	ioredir->fsetpos = (ior_fsetpos)tio_fsetpos;
	ioredir->setvbuf = (ior_setvbuf)tio_setvbuf;
	ioredir->tmpfile = (ior_tmpfile)tio_tmpfile;
	ioredir->fwrite = (ior_fwrite)tio_fwrite;
	ioredir->vfprintf = (ior_vfprintf)tio_vfprintf;
	ioredir->rewind = (ior_rewind)tio_rewind;
	ioredir->from_stdio = (ior_from_stdio)tio_file_from_stdio;
	ioredir->ungetc = (ior_ungetc)tio_ungetc;

	TioFileFuncs fileFuncs;
	fileFuncs.write = pythonInitInternal.PythonConsoleWrite;

	ioredir->custom_stderr = tio_file_from_funcs(&fileFuncs, nullptr);
	ioredir->custom_stdin = tio_file_from_stdio(stdin);
	ioredir->custom_stdout = tio_file_from_funcs(&fileFuncs, nullptr);

	Py_OptimizeFlag++;
	Py_VerboseFlag++;
	Py_NoSiteFlag++;
	Py_SetProgramName("TemplePlus.exe");
	Py_Initialize();
	
	PySys_SetObject("stderr", PyTempleConsoleOut_New());
	PySys_SetObject("stdout", PyTempleConsoleOut_New());
		
	PySys_SetPath("tio://lib/;tio://scr/");

	PyTempleImporter_Install();
	
	auto m = PyImport_ImportModule("site");
	if (!m) {
		auto err = PyErr_Occurred();
		if (err) {
			PyObject *excType, *excValue, *excTraceback;
			PyErr_Fetch(&excType, &excValue, &excTraceback);

			char *pStrErrorMessage = PyString_AsString(excValue);
			logger->error("{}", pStrErrorMessage);
			
			PyErr_Restore(excType, excValue, excTraceback);

			PyErr_Print();
		}

		logger->error("Unable to import Python site module!");
	}
	Py_XDECREF(m);

	return true;
}

static void init_ioredir() {
	
}

class PythonExtensions : public TempleFix {
public:
	const char* name() override {
		return "Python Script Extensions";
	}

	void apply() override {
		replaceFunction(0x100ADA30, &script_init_python);
	}

} pythonReplacement;
