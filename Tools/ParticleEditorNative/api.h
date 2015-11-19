#pragma once

#include <string>
#include <platform/d3d.h>

#define API __declspec(dllexport)

#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <infrastructure/meshes.h>
#include <temple/meshes.h>
#include <temple/aasrenderer.h>
#include <particles/render.h>

struct TempleDll;

namespace particles {
	class PartSys;
}

#include <temple/meshes.h>

struct TempleDll {
	TempleDll(const std::wstring &installationDir, IDirect3DDevice9Ex* device);
	~TempleDll();

	static temple::AasConfig CreateAasConfig();

	gfx::RenderingDevice renderingDevice;
	gfx::MdfMaterialFactory mdfFactory;
	temple::AasConfig aasConfig;
	temple::AasAnimatedModelFactory aasFactory;
	temple::AasRenderer aasRenderer;
	particles::ParticleRendererManager renderManager;
	std::unique_ptr<particles::PartSys> partSys;
	
};

extern "C" {

	API TempleDll* TempleDll_Load(const wchar_t* dllPath,
		const wchar_t *tpDataPath,
		IDirect3DDevice9Ex* device);
	API void TempleDll_Unload(TempleDll*);
	API const char *TempleDll_GetLastError();

	API bool ParticleSystem_RenderVideo(TempleDll* dll,
	                                    D3DCOLOR background,
	                                    const wchar_t* outputFile,
	                                    int fps);


	API gfx::AnimatedModelPtr* AnimatedModel_FromFiles(TempleDll* dll,
	                                                   const wchar_t* skmFilename,
	                                                   const wchar_t* skaFilename);

	API void AnimatedModel_Render(gfx::AnimatedModelPtr* handle, IDirect3DDevice9*, float w, float h, float scale);

	API void AnimatedModel_AdvanceTime(gfx::AnimatedModelPtr* handle, float time);

	API void AnimatedModel_Free(gfx::AnimatedModelPtr* handle);

}
