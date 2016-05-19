#pragma once

#include <string>
#include <platform/d3d.h>

#define API __declspec(dllexport)

#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/shaperenderer2d.h>
#include <graphics/shaperenderer3d.h>
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
	TempleDll(const std::wstring &installationDir);
	~TempleDll();

	static temple::AasConfig CreateAasConfig();

	gfx::RenderingDevice renderingDevice;
	gfx::MdfMaterialFactory mdfFactory;
	temple::AasConfig aasConfig;
	gfx::ShapeRenderer2d shapeRenderer2d;
	gfx::ShapeRenderer3d shapeRenderer3d;
	temple::AasAnimatedModelFactory aasFactory;
	temple::AasRenderer aasRenderer;
	particles::ParticleRendererManager renderManager;
	std::unique_ptr<particles::PartSys> partSys;
	gfx::AnimatedModelPtr currentModel;
	gfx::AnimatedModelParams animParams;
	std::unique_ptr<class EditorExternal> external;

	gfx::RenderTargetTexturePtr renderTarget;
	gfx::RenderTargetDepthStencilPtr renderTargetDepth;

	void SetRenderTarget(IUnknown *surface);
	
};

extern "C" {

	API TempleDll* TempleDll_Load(const wchar_t* dllPath, const wchar_t *tpDataPath);
	API void TempleDll_Unload(TempleDll*);
	API const char *TempleDll_GetLastError();
	API void TempleDll_Flush(TempleDll*);
	API void TempleDll_SetScale(TempleDll*, float);
	API void TempleDll_SetRenderTarget(TempleDll*, IUnknown*);
	API void TempleDll_CenterOn(TempleDll*, float x, float y, float z);

	API bool ParticleSystem_RenderVideo(TempleDll* dll,
	                                    XMCOLOR background,
	                                    const wchar_t* outputFile,
	                                    int fps);


	API gfx::AnimatedModelPtr* AnimatedModel_FromFiles(TempleDll* dll,
	                                                   const wchar_t* skmFilename,
	                                                   const wchar_t* skaFilename);

	API void AnimatedModel_Render(TempleDll* dll, gfx::AnimatedModelPtr* handle);

	API void AnimatedModel_AdvanceTime(gfx::AnimatedModelPtr* handle, float time);

	API void AnimatedModel_Free(gfx::AnimatedModelPtr* handle);

}
