#pragma once

#include <string>
#include <d3d9.h>

#define API __declspec(dllexport)

#include <infrastructure/meshes.h>

struct TempleDll;

namespace particles {
	class PartSys;
}

#include <temple/meshes.h>

struct TempleDll {

	explicit TempleDll(const std::wstring &installationDir);
	~TempleDll();

	temple::AasAnimatedModelFactory modelFactory;
	
};

extern "C" {

	API TempleDll* TempleDll_Load(const wchar_t* dllPath);
	API void TempleDll_Unload(TempleDll*);

	API bool ParticleSystem_RenderVideo(IDirect3DDevice9* device,
	                                    particles::PartSys* sys,
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

void InitRenderStates(IDirect3DDevice9* device, float w, float h, float scale);
