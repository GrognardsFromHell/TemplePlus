#include "api.h"

#include <infrastructure/stringutil.h>
#include <temple/dll.h>
#include <temple/vfs.h>
#include <particles/instances.h>

#include "external.h"

using namespace temple;

std::string lastError;

TempleDll::TempleDll(const std::wstring &installationDir, IDirect3DDevice9Ex* device)
	: renderingDevice(device, 1024, 1024),
	mdfFactory(renderingDevice),
	aasConfig(CreateAasConfig()),
	aasFactory(aasConfig),
	shapeRenderer2d(renderingDevice),
	shapeRenderer3d(renderingDevice),
	aasRenderer(aasFactory, renderingDevice, shapeRenderer2d, shapeRenderer3d, mdfFactory),
	renderManager(renderingDevice, aasFactory, aasRenderer) {

	particles::IPartSysExternal::SetCurrent(&EditorExternal::GetInstance());

}

TempleDll::~TempleDll() {
}

temple::AasConfig TempleDll::CreateAasConfig() {
	AasConfig config;
	return config;
}

const char *TempleDll_GetLastError() {
	return lastError.c_str();
}

TempleDll* TempleDll_Load(const wchar_t* installationDir, 
	const wchar_t* tpDataPath,
	IDirect3DDevice9Ex* device) {

	// Make sure it has a trailing path separator
	std::wstring installDirStr(installationDir);
	if (!installDirStr.empty() && installDirStr.back() != L'\\') {
		installDirStr += L"\\";
	}
	SetCurrentDirectoryW(installDirStr.c_str());

	auto& dll = temple::Dll::GetInstance();
	dll.Load(installDirStr);

	auto tioVfs = std::make_unique<temple::TioVfs>();

	// Add the archives we expect in the install dir
	tioVfs->AddPath("ToEE1.dat");
	tioVfs->AddPath("ToEE2.dat");
	tioVfs->AddPath("ToEE3.dat");
	tioVfs->AddPath("ToEE4.dat");
	tioVfs->AddPath(".\\data");
	auto cTpDataPath(ucs2_to_local(tpDataPath));
	tioVfs->AddPath(cTpDataPath.c_str());

	vfs.reset(tioVfs.release());

	// Init the subsystems we need
	try {
		return new TempleDll(installDirStr, device);
	} catch (std::exception &e) {
		lastError = e.what();
		return nullptr;
	}
}

void TempleDll_Unload(TempleDll* templeDll) {
	delete templeDll;

	auto& dll = temple::Dll::GetInstance();
	dll.Unload();
}
