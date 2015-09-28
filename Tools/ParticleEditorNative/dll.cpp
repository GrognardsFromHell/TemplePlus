#include "api.h"

#include <temple/dll.h>
#include <temple/vfs.h>

TempleDll::TempleDll(const std::wstring& installDir) {

	auto tioVfs = std::make_unique<temple::TioVfs>();

	// Add the archives we expect in the install dir

	vfs.reset(tioVfs.release());
	
}

TempleDll::~TempleDll() {
}

TempleDll* TempleDll_Load(const wchar_t* installationDir) {
	auto& dll = temple::Dll::GetInstance();
	dll.Load(installationDir);

	// Init the subsystems we need
	return new TempleDll(installationDir);
}

void TempleDll_Unload(TempleDll* templeDll) {
	delete templeDll;

	auto& dll = temple::Dll::GetInstance();
	dll.Unload();
}
