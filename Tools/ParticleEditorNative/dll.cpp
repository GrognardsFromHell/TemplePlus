#include "api.h"

#include <infrastructure/stringutil.h>
#include <temple/dll.h>
#include <temple/vfs.h>
#include <graphics/dynamictexture.h>
#include <particles/instances.h>

#include "external.h"

using namespace temple;

std::string lastError;

TempleDll::TempleDll(const std::wstring &installationDir)
	: renderingDevice(nullptr, 0, true),
	camera(std::make_shared<gfx::WorldCamera>()),
	mdfFactory(renderingDevice),
	aasConfig(CreateAasConfig()),
	aasFactory(aasConfig),
	shapeRenderer2d(renderingDevice),
	shapeRenderer3d(renderingDevice),
	aasRenderer(aasFactory, renderingDevice, shapeRenderer2d, shapeRenderer3d, mdfFactory),
	renderManager(renderingDevice, aasFactory, aasRenderer) {

	renderingDevice.SetCurrentCamera(camera);

	animParams.rotation = XMConvertToRadians(135.0f);

	external = std::make_unique<EditorExternal>(*this);
	particles::IPartSysExternal::SetCurrent(external.get());

}

TempleDll::~TempleDll() {
}

temple::AasConfig TempleDll::CreateAasConfig() {
	AasConfig config;
	return config;
}

void TempleDll::SetRenderTarget(IUnknown * surface)
{
	renderTarget = renderingDevice.CreateRenderTargetForSharedSurface(surface);

	auto &size = renderTarget->GetSize();

	// Check render target -> free it
	if (renderTargetDepth) {
		auto &depthSize = renderTargetDepth->GetSize();
		if (depthSize.width != size.width || depthSize.height != size.height) {
			renderTargetDepth.reset();
		}
	}

	if (!renderTargetDepth) {
		renderTargetDepth = renderingDevice.CreateRenderTargetDepthStencil(size.width, size.height);
	}

	renderingDevice.PushRenderTarget(renderTarget, renderTargetDepth);
	renderingDevice.ClearCurrentColorTarget(XMCOLOR(0, 0, 0, 0));
	renderingDevice.ClearCurrentDepthTarget();
}

const char *TempleDll_GetLastError() {
	return lastError.c_str();
}

TempleDll* TempleDll_Load(const wchar_t* installationDir, 
	const wchar_t* tpDataPath) {

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
		return new TempleDll(installDirStr);
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

void TempleDll_Flush(TempleDll *templeDll) {
	templeDll->renderingDevice.Flush();
}

void TempleDll_SetRenderTarget(TempleDll *templeDll, IUnknown *surface) {
	if (templeDll->renderTarget) {
		templeDll->renderingDevice.PopRenderTarget();
	}
	templeDll->SetRenderTarget(surface);
}

void TempleDll_CenterOn(TempleDll *dll, float x, float y, float z)
{
	dll->camera->CenterOn(x, y, z);
}

void TempleDll_SetScale(TempleDll *templeDll, float scale) {
	templeDll->camera->SetScale(scale);
}
