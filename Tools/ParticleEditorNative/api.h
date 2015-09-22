#pragma once

#include <string>
#include <d3d9.h>

#define API __declspec(dllexport)

class particles::PartSys;

extern "C" {
	API bool ParticleSystem_RenderVideo(IDirect3DDevice9* device,
	                                    particles::PartSys* sys,
	                                    D3DCOLOR background,
	                                    const wchar_t* outputFile,
	                                    int fps);
}

void InitRenderStates(IDirect3DDevice9 *device, float w, float h, float scale);
