
#include "stdafx.h"
#include "pythonglobal.h"
#include <temple/dll.h>

static struct TemplePythonData : temple::AddressTable {

	PyObject **globals;

	TemplePythonData() {
		rebase(globals, 0x10BCA764);
	}

} templePythonData;

PythonGlobalExtension::PythonGlobalExtension() {
	extensions().push_back(this);
}

PythonGlobalExtension::~PythonGlobalExtension() {
}

void PythonGlobalExtension::installExtensions() {

	// Get the python global object
	// TODO re-enable later
	/*auto globals = *templePythonData.globals;
	auto list = extensions();

	logger->info("Applying {} Python global extensions.", list.size());

	for (auto extension : list) {
		extension->extend(globals);
	}*/

}

std::vector<PythonGlobalExtension*>& PythonGlobalExtension::extensions() {
    static auto extensions = new std::vector<PythonGlobalExtension*>;
	return *extensions;
}
