#include "stdafx.h"

#include <infrastructure/renderstates.h>

#include "mdfrenderer.h"
#include "graphics.h"
#include "materials.h"

struct MdfRenderer::Resources : ResourceListener {
	Resources(Graphics& g) : mRegistration(g, this) {
	}

	void CreateResources(Graphics&) override {

	}

	void FreeResources(Graphics&) override {

	}

	ResourceListenerRegistration mRegistration;

};

MdfRenderer::MdfRenderer(Graphics& g)
	: mGraphics(g), mRenderer(g), mResources(std::make_unique<Resources>(g)) {
}

MdfRenderer::~MdfRenderer() = default;

void MdfRenderer::RenderGeneral(MdfRenderMaterial* material, int vertexCount, D3DXVECTOR4* pos, D3DXVECTOR4* normal, DWORD* diffuse, D3DXVECTOR2* uv, int primCount, uint16_t* indices) {

	auto mdfMaterial = static_cast<const gfx::MdfGeneralMaterial*>(material->GetSpec());

	std::array<D3DXVECTOR2*, 4> uvs{
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	if (mdfMaterial->zFillOnly) {
		renderStates->SetColorWriteEnable(false, false, false, false);
	}
	if (mdfMaterial->disableZ) {
		renderStates->SetZEnable(false);
	}
	if (mdfMaterial->colorFillOnly) {
		renderStates->SetZWriteEnable(false);
	}

	// Some materials request the normals to be recalculated
	if (mdfMaterial->recalculateNormals) {
		normal = RecalcNormals(
			vertexCount,
			pos,
			normal,
			primCount,
			indices
		);
	}

	auto textures = material->GetTextures();
	for (auto sampler = 0; sampler < 4; ++sampler) {

		auto texture = textures[sampler];

		if (!texture) {
			renderStates->SetTexture(sampler, nullptr);
			renderStates->SetTextureColorOp(sampler, D3DTOP_DISABLE);
			renderStates->SetTextureAlphaOp(sampler, D3DTOP_DISABLE);
			uvs[sampler] = nullptr;
			continue;
		}

		if (mdfMaterial->linearFiltering) {
			renderStates->SetTextureMipFilter(sampler, D3DTEXF_LINEAR);
			renderStates->SetTextureMinFilter(sampler, D3DTEXF_LINEAR);
			renderStates->SetTextureMagFilter(sampler, D3DTEXF_LINEAR);
		}

		SetTextureTransform(sampler, texture.get());
		renderStates->SetTexture(sampler, texture->GetDeviceTexture());

		auto& mdfSampler = mdfMaterial->samplers[sampler];

		SetTextureBlending(sampler, mdfSampler.blendType);

		renderStates->SetTextureAddressU(sampler, D3DTADDRESS_WRAP);
		renderStates->SetTextureAddressV(sampler, D3DTADDRESS_WRAP);

		uvs[sampler] = GenerateUVs(sampler, mdfSampler, vertexCount, normal, uv);
	}

	renderStates->SetColorVertex(true);

	D3DMATERIAL9 lightingMaterial{
		{1.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0, 0, 0, 1.0f},
		{0, 0, 0, 0},
		mdfMaterial->specularPower
	};

	// Fun fact: Doesn't actually seem to support gloss maps ...
	auto glossmap = material->GetGlossMap();
	if (!glossmap) {
		auto red = (mdfMaterial->specular >> 16) & 0xFF;
		auto green = (mdfMaterial->specular >> 8) & 0xFF;
		auto blue = mdfMaterial->specular & 0xFF;
		lightingMaterial.Specular.r = red / 255.f;
		lightingMaterial.Specular.g = green / 255.f;
		lightingMaterial.Specular.b = blue / 255.f;
	}

	auto device = mGraphics.device();
	D3DLOG(device->SetMaterial(&lightingMaterial));

	switch (mdfMaterial->blendType) {
	case gfx::MdfBlendType::Alpha:
		renderStates->SetAlphaTestEnable(true);
		renderStates->SetAlphaBlend(true);
		renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
		renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);
		break;
	case gfx::MdfBlendType::Add:
		renderStates->SetAlphaTestEnable(true);
		renderStates->SetAlphaBlend(true);
		renderStates->SetSrcBlend(D3DBLEND_ONE);
		renderStates->SetDestBlend(D3DBLEND_ONE);
		break;
	case gfx::MdfBlendType::AlphaAdd:
		renderStates->SetAlphaTestEnable(true);
		renderStates->SetAlphaBlend(true);
		renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
		renderStates->SetDestBlend(D3DBLEND_ONE);
		break;
	case gfx::MdfBlendType::None:
		renderStates->SetAlphaBlend(false);
		break;
	}

	diffuse = GenerateDiffuse(mdfMaterial->diffuse, vertexCount, diffuse);

	renderStates->SetTextureCoordIndex(0, 0);
	renderStates->SetTextureCoordIndex(0, 1);
	renderStates->SetTextureCoordIndex(0, 2);
	renderStates->SetTextureCoordIndex(0, 3);

	renderStates->SetCullMode(mdfMaterial->faceCulling ? D3DCULL_CCW : D3DCULL_NONE);

	if (mdfMaterial->notLit) {
		normal = nullptr;
	}

	auto lightingWasEnabled = renderStates->IsLighting();
	if (!normal) {
		renderStates->SetLighting(false);
	}

	mRenderer.DrawTris(vertexCount, pos, normal, diffuse, uvs[0], uvs[1], uvs[2], uvs[3], primCount, indices);

	renderStates->SetLighting(lightingWasEnabled);

	for (auto sampler = 0; sampler < 4; ++sampler) {
		if (mdfMaterial->linearFiltering) {
			renderStates->SetTextureMinFilter(sampler, D3DTEXF_POINT);
			renderStates->SetTextureMagFilter(sampler, D3DTEXF_POINT);
			renderStates->SetTextureMipFilter(sampler, D3DTEXF_POINT);
		}

		renderStates->SetTexture(sampler, nullptr);
		renderStates->SetTextureAddressU(sampler, D3DTADDRESS_CLAMP);
		renderStates->SetTextureAddressV(sampler, D3DTADDRESS_CLAMP);

		if (sampler == 0) {
			renderStates->SetTextureColorOp(0, D3DTOP_MODULATE);
			renderStates->SetTextureColorArg1(0, D3DTA_TEXTURE);
			renderStates->SetTextureColorArg2(0, D3DTA_CURRENT);
			renderStates->SetTextureAlphaOp(0, D3DTOP_SELECTARG1);
			renderStates->SetTextureAlphaArg1(0, D3DTA_TEXTURE);
			renderStates->SetTextureAlphaArg2(0, D3DTA_CURRENT);
		} else {
			renderStates->SetTextureColorOp(sampler, D3DTOP_DISABLE);
			renderStates->SetTextureAlphaOp(sampler, D3DTOP_DISABLE);
		}
		renderStates->SetTextureTransformFlags(sampler, D3DTTFF_DISABLE);
	}

	renderStates->SetAlphaTestEnable(false);
	renderStates->SetCullMode(D3DCULL_NONE);
	if (mdfMaterial->colorFillOnly) {
		renderStates->SetZWriteEnable(true);
	}
	if (mdfMaterial->disableZ) {
		renderStates->SetZEnable(true);
	}
	if (mdfMaterial->zFillOnly) {
		renderStates->SetColorWriteEnable(true, true, true, true);
	}

}

void MdfRenderer::SetTextureTransform(int sampler, const gfx::Texture* texture) {

	static std::array<D3DTRANSFORMSTATETYPE, 8> sTransformTypes{
		D3DTS_TEXTURE0,
		D3DTS_TEXTURE1,
		D3DTS_TEXTURE2,
		D3DTS_TEXTURE3,
		D3DTS_TEXTURE4,
		D3DTS_TEXTURE5,
		D3DTS_TEXTURE6,
		D3DTS_TEXTURE7
	};

	auto device = mGraphics.device();

	auto& contentRect = texture->GetContentRect();
	auto& size = texture->GetSize();
	auto hasTranslation = contentRect.x || contentRect.y;
	auto hasScaling = contentRect.width != size.width || contentRect.height != size.height;
	if (hasTranslation || hasScaling) {
		D3DXMATRIX translation;
		D3DXMATRIX scaling;
		D3DXMATRIX combined;
		D3DXMATRIX* transform = nullptr;
		if (hasTranslation) {
			auto dx = contentRect.x / (float)size.width;
			auto dy = contentRect.y / (float)size.height;
			D3DXMatrixTranslation(&translation, dx, dy, 1.0);
			transform = &translation;
		}
		if (hasScaling) {
			auto sw = contentRect.width / (float)size.width;
			auto sh = contentRect.height / (float)size.height;
			D3DXMatrixScaling(&scaling, sw, sh, 0.0);
			transform = &scaling;
		}
		if (hasTranslation && hasScaling) {
			D3DXMatrixMultiply(&combined, &scaling, &translation);
			transform = &combined;
		}

		device->SetTransform(sTransformTypes[sampler], transform);
		renderStates->SetTextureTransformFlags(sampler, D3DTTFF_COUNT2);
	} else {
		renderStates->SetTextureTransformFlags(sampler, D3DTTFF_DISABLE);
	}

}

void MdfRenderer::SetTextureBlending(int sampler, gfx::MdfTextureBlendType blendType) {

	renderStates->SetTextureColorArg1(sampler, D3DTA_TEXTURE);
	renderStates->SetTextureColorArg2(sampler, D3DTA_CURRENT);
	renderStates->SetTextureAlphaArg1(sampler, D3DTA_TEXTURE);
	renderStates->SetTextureAlphaArg2(sampler, D3DTA_CURRENT);

	switch (blendType) {
	default:
	case gfx::MdfTextureBlendType::Modulate:
		renderStates->SetTextureColorOp(sampler, D3DTOP_MODULATE);
		renderStates->SetTextureAlphaOp(sampler, D3DTOP_MODULATE);
		break;
	case gfx::MdfTextureBlendType::Add:
		renderStates->SetTextureColorOp(sampler, D3DTOP_ADD);
		renderStates->SetTextureAlphaOp(sampler, D3DTOP_SELECTARG2);
		break;
	case gfx::MdfTextureBlendType::TextureAlpha:
		renderStates->SetTextureColorOp(sampler, D3DTOP_BLENDTEXTUREALPHA);
		renderStates->SetTextureAlphaOp(sampler, D3DTOP_SELECTARG2);
		break;
	case gfx::MdfTextureBlendType::CurrentAlpha:
		renderStates->SetTextureColorOp(sampler, D3DTOP_BLENDCURRENTALPHA);
		renderStates->SetTextureAlphaOp(sampler, D3DTOP_SELECTARG2);
		renderStates->SetTextureAlphaArg2(sampler, D3DTA_DIFFUSE);
		break;
	case gfx::MdfTextureBlendType::CurrentAlphaAdd:
		renderStates->SetTextureColorOp(sampler, D3DTOP_MODULATEALPHA_ADDCOLOR);
		renderStates->SetTextureAlphaOp(sampler, D3DTOP_SELECTARG2);
		renderStates->SetTextureColorArg1(sampler, D3DTA_CURRENT);
		renderStates->SetTextureColorArg2(sampler, D3DTA_TEXTURE);

		renderStates->SetTextureAlphaArg1(sampler, D3DTA_CURRENT);
		renderStates->SetTextureAlphaArg2(sampler, D3DTA_DIFFUSE);
		break;
	}

}

D3DXVECTOR4* MdfRenderer::RecalcNormals(int vertexCount, D3DXVECTOR4* pos, D3DXVECTOR4* normal, int primCount, uint16_t* indices) {
	static D3DXVECTOR4 recalcNormalsBuffer[0x8000];
	memset(recalcNormalsBuffer, 0, vertexCount * sizeof(D3DXVECTOR4));

	// Process every TRI we have
	auto curIdx = indices;
	for (auto tri = 0; tri < primCount; ++tri) {
		// Indices of the three vertices making up this triangle
		auto idx1 = *curIdx++;
		auto idx2 = *curIdx++;
		auto idx3 = *curIdx++;

		auto pos1 = reinterpret_cast<D3DXVECTOR3*>(&pos[idx1]);
		auto pos2 = reinterpret_cast<D3DXVECTOR3*>(&pos[idx2]);
		auto pos3 = reinterpret_cast<D3DXVECTOR3*>(&pos[idx3]);

		D3DXVECTOR3 v1to2;
		D3DXVec3Subtract(&v1to2, pos2, pos1);

		D3DXVECTOR3 v1to3;
		D3DXVec3Subtract(&v1to3, pos3, pos1);

		// Calculate the surface normal of the surface defined 
		// by the two directional vectors
		D3DXVECTOR3 surfNormal;
		D3DXVec3Cross(&surfNormal, &v1to2, &v1to3);
		D3DXVec3Scale(&surfNormal, &surfNormal, -1);

		// The surface normal contributes to all three vertex normals
		recalcNormalsBuffer[idx1].x += surfNormal.x;
		recalcNormalsBuffer[idx1].y += surfNormal.y;
		recalcNormalsBuffer[idx1].z += surfNormal.z;

		recalcNormalsBuffer[idx2].x += surfNormal.x;
		recalcNormalsBuffer[idx2].y += surfNormal.y;
		recalcNormalsBuffer[idx2].z += surfNormal.z;

		recalcNormalsBuffer[idx3].x += surfNormal.x;
		recalcNormalsBuffer[idx3].y += surfNormal.y;
		recalcNormalsBuffer[idx3].z += surfNormal.z;
	}

	// Re-Normalize the normals we calculated
	for (auto i = 0; i < vertexCount; ++i) {
		D3DXVec3Normalize(
			(D3DXVECTOR3*)&recalcNormalsBuffer[i],
			(D3DXVECTOR3*)&recalcNormalsBuffer[i]
		);
	}

	return recalcNormalsBuffer;
}

void MdfRenderer::Render(MdfRenderMaterial* material,
                         int vertexCount,
                         D3DXVECTOR4* pos,
                         D3DXVECTOR4* normal,
                         D3DCOLOR* diffuse,
                         D3DXVECTOR2* uv,
                         int primCount,
                         uint16_t* indices) {

	auto mdfMaterial = material->GetSpec();

	switch (mdfMaterial->GetType()) {
	case gfx::MdfType::Textured:
		RenderTextured(material, vertexCount, pos, normal, diffuse, uv, primCount, indices);
		return;
	case gfx::MdfType::General:
		RenderGeneral(material, vertexCount, pos, normal, diffuse, uv, primCount, indices);
		return;
	case gfx::MdfType::Clipper:
		return;
	default:
		break;
	}
}


void MdfRenderer::RenderTextured(MdfRenderMaterial* material,
                                 int vertexCount,
                                 D3DXVECTOR4* pos,
                                 D3DXVECTOR4* normal,
                                 D3DCOLOR* diffuse,
                                 D3DXVECTOR2* uv,
                                 int primCount,
                                 uint16_t* indices) {

	auto mdfMaterial = static_cast<const gfx::MdfTexturedMaterial*>(material->GetSpec());

	static D3DCOLOR mixedColors[0x7FFF];

	if (mdfMaterial->GetDiffuse() != 0xFFFFFFFF) {
		if (diffuse) {
			// Fills vertex colors with blended colors from diffuse + shaderColor
			for (auto i = 0; i < vertexCount; ++i) {
				mixedColors[i] = MultiplyColors(
					mdfMaterial->GetDiffuse(),
					diffuse[i]);
			}
		} else {
			for (auto i = 0; i < vertexCount; ++i) {
				mixedColors[i] = mdfMaterial->GetDiffuse();
			}
		}
		diffuse = mixedColors;
	}

	// notlit
	if (mdfMaterial->GetNotLit())
		normal = nullptr;

	// disable Z
	if (mdfMaterial->GetDisableZ()) {
		renderStates->SetZEnable(false);
	}

	// z write only
	if (mdfMaterial->GetColorFillOnly()) {
		renderStates->SetZWriteEnable(false);
	}

	auto texture = material->GetTextures()[0];
	if (texture && uv) {
		if (!texture->IsValid()) {
			return;
		}

		renderStates->SetTexture(0, texture->GetDeviceTexture());

		if (!mdfMaterial->GetClamp()) {
			renderStates->SetTextureAddressU(0, D3DTADDRESS_WRAP);
			renderStates->SetTextureAddressV(0, D3DTADDRESS_WRAP);
		}

		SetTextureTransform(0, texture.get());
	} else {
		renderStates->SetTexture(0, nullptr);
		uv = nullptr;
	}

	renderStates->SetTextureColorOp(0, D3DTOP_MODULATE);
	renderStates->SetTextureColorArg1(0, D3DTA_TEXTURE);
	renderStates->SetTextureColorArg2(0, D3DTA_DIFFUSE);

	renderStates->SetTextureAlphaOp(0, D3DTOP_MODULATE);
	renderStates->SetTextureAlphaArg1(0, D3DTA_TEXTURE);
	renderStates->SetTextureAlphaArg2(0, D3DTA_DIFFUSE);

	renderStates->SetAlphaBlend(true);
	renderStates->SetTextureCoordIndex(0, 0);
	renderStates->SetColorVertex(diffuse != nullptr);
	renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
	renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);

	auto lightingWasEnabled = renderStates->IsLighting();
	if (!normal) {
		renderStates->SetLighting(false);
	}

	renderStates->SetCullMode(mdfMaterial->GetDouble() ? D3DCULL_NONE : D3DCULL_CCW);

	mRenderer.DrawTris(vertexCount, pos, normal, diffuse, uv, nullptr, nullptr, nullptr, primCount, indices);

	renderStates->SetCullMode(D3DCULL_NONE);
	renderStates->SetLighting(lightingWasEnabled);

	if (texture) {
		renderStates->SetTextureTransformFlags(0, D3DTTFF_DISABLE);
		if (!mdfMaterial->GetClamp()) {
			renderStates->SetTextureAddressU(0, D3DTADDRESS_CLAMP);
			renderStates->SetTextureAddressV(0, D3DTADDRESS_CLAMP);
		}
	}

	if (mdfMaterial->GetDisableZ()) {
		renderStates->SetZEnable(true);
	}
	if (mdfMaterial->GetColorFillOnly()) {
		renderStates->SetZWriteEnable(true);
	}
}

D3DXVECTOR2* MdfRenderer::GenerateUVs(int sampler, const gfx::MdfGeneralMaterialSampler& mdfSampler, int vertexCount, D3DXVECTOR4* normals, D3DXVECTOR2* uv) {

	// This is static scratch space for generating up to 4 full sets of generated UV coordinates,
	// one per sampler
	static std::array<std::array<D3DXVECTOR2, 0x8000>, 4> generatedUvs;

	auto uvType = mdfSampler.uvType;

	// this is where stuff gets really annoying: arbitrary UV transforms

	if (uvType == gfx::MdfUvType::Mesh) {
		return uv;
	}

	auto& generatedUv = generatedUvs[sampler];

	constexpr auto Sin45Deg = 0.70710599f;
	auto factorU = mdfSampler.speedU / 60.0f * mTextureAnimTime / 1000.0f;
	auto factorV = mdfSampler.speedV / 60.0f * mTextureAnimTime / 1000.0f;

	switch (uvType) {
	case gfx::MdfUvType::Environment:
		// Calculate the UV for each vertex based on it's normal
		for (auto i = 0; i < vertexCount; ++i) {
			auto& curNormal = normals[i];
			generatedUv[i].x = 0.5f + (curNormal.x - curNormal.z) * Sin45Deg * 0.5f;
			generatedUv[i].y = 0.5f - curNormal.y * 0.5f;
		}
		break;
	case gfx::MdfUvType::Drift:
		for (auto i = 0; i < vertexCount; i++) {
			generatedUv[i].x = uv[i].x + factorU;
			generatedUv[i].y = uv[i].y + factorV;
		}
		break;
	case gfx::MdfUvType::Swirl: {
		// This means speedU is in "full rotations every 60 seconds" -> RPM
		auto swirlU = cosf(factorU * (float)M_PI_2) * 0.1f;
		auto swirlV = sinf(factorV * (float)M_PI_2) * 0.1f;

		for (auto i = 0; i < vertexCount; i++) {
			generatedUv[i].x = uv[i].x + swirlU;
			generatedUv[i].y = uv[i].y + swirlV;
		}
		break;
	}
	case gfx::MdfUvType::Wavey:
		for (auto i = 0; i < vertexCount; ++i) {
			generatedUv[i].x = uv[i].x + cosf((factorU + uv[i].x) * (float)M_PI_2) * 0.1f;
			generatedUv[i].y = uv[i].y + sinf((factorV + uv[i].y) * (float)M_PI_2) * 0.1f;
		}
		break;
	default:
		throw TempleException("Unexpected UV type");
	}

	return &generatedUv[0];

}

D3DCOLOR* MdfRenderer::GenerateDiffuse(D3DCOLOR materialDiffuse, int vertexCount, D3DCOLOR* diffuse) {
	static std::array<D3DCOLOR, 0x8000> generatedDiffuse;

	if (!diffuse) {
		// If no diffuse vertex colors are given, generate them
		// from the MDF color
		for (auto i = 0; i < vertexCount; ++i) {
			generatedDiffuse[i] = materialDiffuse;
		}
		return &generatedDiffuse[0];
	} else if (materialDiffuse != 0xFFFFFFFF) {
		// If the MDF diffuse is not white, but vertex colors were given
		// blend them by multiplying
		for (auto i = 0; i < vertexCount; ++i) {
			generatedDiffuse[i] = MultiplyColors(
				materialDiffuse,
				diffuse[i]
			);
		}
		return &generatedDiffuse[0];
	} else {
		return diffuse;
	}

}

D3DCOLOR MdfRenderer::MultiplyColors(D3DCOLOR a, D3DCOLOR b) {
	uint8_t r1 = a & 0xFF;
	uint8_t r2 = b & 0xFF;
	uint8_t g1 = (a >> 8) & 0xFF;
	uint8_t g2 = (b >> 8) & 0xFF;
	uint8_t b1 = (a >> 16) & 0xFF;
	uint8_t b2 = (b >> 16) & 0xFF;
	uint8_t a1 = (a >> 24) & 0xFF;
	int8_t a2 = (b >> 24) & 0xFF;

	uint32_t result = (uint8_t)(r1 * r2 / 255);
	result |= (uint8_t)(g1 * g2 / 255) << 8;
	result |= (uint8_t)(b1 * b2 / 255) << 16;
	result |= (int8_t)(a1 * a2 / 255) << 24;

	return result;
}
