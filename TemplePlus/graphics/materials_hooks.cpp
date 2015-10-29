#include "stdafx.h"
#include "materials.h"
#include "util/fixes.h"

#include <d3dx9math.h>

#include "mdfrenderer.h"
#include "graphics.h"

/*
	This structure is allocated by the caller on the stack and
	we will it with the callbacks and userdata pointer for the
	shader.
*/

using vector4 = D3DXVECTOR4;
using vector2 = D3DXVECTOR2;
using PrivateShaderData = MdfRenderMaterial*;
using LegacyRenderFunc = void(int vertexCount,
                              vector4* pos,
                              vector4* normal,
                              D3DCOLOR* diffuse,
                              vector2* uv,
                              int primCount,
                              uint16_t* indices,
                              PrivateShaderData shaderData);

using LegacyGetPrimaryTextureIdFunc = int(PrivateShaderData shaderData,
                                          int* textureIdOut);

using LegacyColorFunc = int(PrivateShaderData shaderData,
                            const char* type,
                            D3DCOLOR* colorOut);

#pragma pack(push, 1)
struct LegacyShader {
	LegacyRenderFunc* renderFunc;
	int field4;
	LegacyGetPrimaryTextureIdFunc* getTextureIdFunc;
	LegacyColorFunc* getColorFunc;
	LegacyColorFunc* setColorFunc;
	PrivateShaderData privateShaderData;
};
#pragma pack(pop)

static class MaterialsHooks : TempleFix {
public:
	const char* name() override {
		return "Replaces the old material system";
	}

	void apply() override;

	static std::shared_ptr<gfx::Material> sLastMaterial;

	static int LoadShader(int shaderId, LegacyShader* shaderOut);
	static int RegisterShader(const char *filename, int *shaderIdOut);
	static int RegisterReplacementMaterial(int specialMatIdx, const char *filename, int* shaderIdOut);

	static void RenderShader(
		int vertexCount,
		vector4* pos,
		vector4* normal,
		D3DCOLOR* diffuse,
		vector2* uv,
		int primCount,
		uint16_t* indices,
		PrivateShaderData shaderData
	);
	static int GetPrimaryTexture(
		PrivateShaderData shaderData,
		int* textureIdOut
	);
	static int GetColor(
		PrivateShaderData shaderData,
		const char* type,
		D3DCOLOR* colorOut
	);
	static int SetColor(
		PrivateShaderData shaderData,
		const char* type,
		D3DCOLOR* colorOut
	);
} fix;

std::shared_ptr<gfx::Material> MaterialsHooks::sLastMaterial;

void MaterialsHooks::apply() {
	replaceFunction(0x101E20C0, LoadShader);
	replaceFunction(0x101E2160, RegisterShader);
	replaceFunction(0x101E3410, RegisterReplacementMaterial);
}

int MaterialsHooks::LoadShader(int shaderId, LegacyShader* shaderOut) {

	// Shader has to have been registered earlier
	auto mdfFactory = static_cast<MdfMaterialFactory*>(gfx::gMdfMaterialFactory);
	auto material = mdfFactory->GetById(shaderId);

	if (!material->IsValid()) {
		logger->error("Legacy shader with id {} wasn't found.", shaderId);
		return 17;
	}

	// Keep a reference around
	sLastMaterial = material;

	// Wire the legacy shader up with delegates from this class
	shaderOut->renderFunc = &RenderShader;
	shaderOut->getTextureIdFunc = &GetPrimaryTexture;
	shaderOut->setColorFunc = &SetColor;
	shaderOut->getColorFunc = &GetColor;
	// Pointer to the gfx::Material
	shaderOut->privateShaderData = static_cast<MdfRenderMaterial*>(sLastMaterial.get()); 

	return 0;
}

int MaterialsHooks::RegisterShader(const char* filename, int* shaderIdOut) {

	auto material = gfx::gMdfMaterialFactory->LoadMaterial(filename);
	if (!material->IsValid()) {
		return 17;
	}

	auto mdfMaterial(std::static_pointer_cast<MdfRenderMaterial>(material));
	*shaderIdOut = mdfMaterial->GetId();
	return 0;

}

int MaterialsHooks::RegisterReplacementMaterial(int specialMatIdx, const char* filename, int* shaderIdOut) {

	int shaderId;
	if (RegisterShader(filename, &shaderId) != 0) {
		*shaderIdOut = -1;
		return 17;
	}
	
	// Transform the shader id back to a special material id
	*shaderIdOut = 0x80000000 | (specialMatIdx << 26) | shaderId;
	return 0;

}

void MaterialsHooks::RenderShader(int vertexCount,
                                  vector4* pos,
                                  vector4* normal,
                                  D3DCOLOR* diffuse,
                                  vector2* uv,
                                  int primCount,
                                  uint16_t* indices,
                                  PrivateShaderData shaderData) {

	MdfRenderer renderer(*graphics);
	renderer.Render(shaderData,
		vertexCount,
		pos,
		normal,
		diffuse,
		uv,
		primCount,
		indices);

}

int MaterialsHooks::GetPrimaryTexture(PrivateShaderData shaderData,
                                      int* textureIdOut) {

	auto texture = shaderData->GetPrimaryTexture();
	if (texture->IsValid()) {
		*textureIdOut = texture->GetId();
		return 0;
	}

	*textureIdOut = 0;
	// This actually differs from vanilla, where 0 is returned even
	// for errors.
	return 17;

}

int MaterialsHooks::GetColor(PrivateShaderData shaderData,
                             const char* type,
                             D3DCOLOR* colorOut) {
	return 0; // TODO
}

int MaterialsHooks::SetColor(PrivateShaderData shaderData,
                             const char* type,
                             D3DCOLOR* colorOut) {
	return 0; // TODO
}

void ClearMaterialsHooksState() {
	MaterialsHooks::sLastMaterial.reset();
}
