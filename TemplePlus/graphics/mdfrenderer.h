#pragma once

#include "renderer.h"

namespace gfx{
	class Texture;
	enum class MdfTextureBlendType : uint32_t;
	struct MdfGeneralMaterialSampler;
}

class Graphics;
class MdfRenderMaterial;

struct D3DXVECTOR2;
struct D3DXVECTOR4;

class MdfRenderer {
public:
	explicit MdfRenderer(Graphics &g);
	~MdfRenderer();

	void Render(MdfRenderMaterial* material,
	            int vertexCount,
	            D3DXVECTOR4* pos,
	            D3DXVECTOR4* normal,
	            D3DCOLOR* diffuse,
	            D3DXVECTOR2* uv,
	            int primCount,
	            uint16_t* indices);

private:

	void RenderGeneral(MdfRenderMaterial* material, int vertexCount, D3DXVECTOR4* pos, D3DXVECTOR4* normal, DWORD* diffuse, D3DXVECTOR2* uv, int primCount, uint16_t* indices);

	void SetTextureTransform(int sampler, const gfx::Texture* texture);
	
	void SetTextureBlending(int sampler, gfx::MdfTextureBlendType blendType);

	D3DXVECTOR4* RecalcNormals(
		int vertexCount,
		D3DXVECTOR4* pos,
		D3DXVECTOR4* normal,
		int primCount,
		uint16_t* indices
		);

	void RenderTextured(MdfRenderMaterial* material,
		int vertexCount,
		D3DXVECTOR4* pos,
		D3DXVECTOR4* normal,
		D3DCOLOR* diffuse,
		D3DXVECTOR2* uv,
		int primCount,
		uint16_t* indices);

	D3DXVECTOR2* GenerateUVs(
		int sampler,
		const gfx::MdfGeneralMaterialSampler &mdfSampler,
		int vertexCount,
		D3DXVECTOR4* normals,
		D3DXVECTOR2 *uv
		);

	D3DCOLOR* GenerateDiffuse(
		D3DCOLOR materialDiffuse, 
		int vertexCount, 
		D3DCOLOR *diffuse
		);
	
	D3DCOLOR MultiplyColors(D3DCOLOR a, D3DCOLOR b);

	struct Resources;	
	Graphics& mGraphics;
	Renderer mRenderer;
	std::unique_ptr<Resources> mResources;
	float mTextureAnimTime = 0.0f;
};
