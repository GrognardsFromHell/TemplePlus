
#include "stdafx.h"
#include "fixes.h"
#include "gamesystems.h"
#include "graphics.h"

struct float4 {
	float x;
	float y;
	float z;
	float w;
};

static struct FogVertexData : AddressTable {
	int *vertexCount;  
	int *indexCount; 
	float4 *vertices;
	D3DCOLOR *diffuse;
	uint16_t *indices;

	FogVertexData() {
		rebase(vertexCount, 0x1080FA80);
		rebase(indexCount, 0x108EC594);
		rebase(vertices, 0x10820468);
		rebase(diffuse, 0x108244A0);
		rebase(indices, 0x108E94C8);
	}
} vertexData;

static void(__cdecl *RenderFog)();

static void __cdecl HookedRenderFog() {
	RenderFog();
}

class FogSeamsFix : public TempleFix {
public:
	const char* name() override {
		return "Fixes seams in the fog of war textures (only happens on D3D9)";
	}

	void apply() override {
		RenderFog = static_cast<void(__cdecl*)()>(replaceFunction(0x1002F180, HookedRenderFog));
	}
} fogSeamsFix;
