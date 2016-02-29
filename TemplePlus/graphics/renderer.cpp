#include "stdafx.h"

#include <infrastructure/renderstates.h>

/*
#include "renderer.h"
#include "graphics.h"
#include "shaders.h"
#include <config/config.h>

constexpr auto CalcVertexSize(bool hasNormals, bool hasDiffuse, int texCoords) {
	return sizeof(D3DXVECTOR3)
		+ (hasNormals ? sizeof(D3DXVECTOR3) : 0)
		+ (hasDiffuse ? sizeof(D3DCOLOR) : 0)
		+ (texCoords * sizeof(D3DXVECTOR2));
}

constexpr auto GetUvOffset(bool hasNormals, bool hasDiffuse) {
	return sizeof(D3DXVECTOR3)
		+ (hasNormals ? sizeof(D3DXVECTOR3) : 0)
		+ (hasDiffuse ? sizeof(D3DCOLOR) : 0);
}

constexpr auto GetDiffuseOffset(bool hasNormals) {
	return sizeof(D3DXVECTOR3)
		+ (hasNormals ? sizeof(D3DXVECTOR3) : 0);
}

template <bool hasNormals, bool hasDiffuse, int texCoords>
void FillBufferUv(void* dataOut, int vertexCount, D3DXVECTOR4* pos, D3DXVECTOR4* normal, D3DCOLOR* diffuse, D3DXVECTOR2* uv) {

	constexpr auto vertexSize = CalcVertexSize(hasNormals, hasDiffuse, texCoords);

	auto vertexData = reinterpret_cast<uint8_t*>(dataOut);

	for (int i = 0; i < vertexCount; ++i) {

		auto posOut = reinterpret_cast<float*>(vertexData);
		*posOut++ = pos->x;
		*posOut++ = pos->y;
		*posOut++ = pos->z;
		pos++;

		if (hasNormals) {
			*posOut++ = normal->x;
			*posOut++ = normal->y;
			*posOut++ = normal->z;
			normal++;
		}

		if (hasDiffuse) {
			auto diffuseOut = reinterpret_cast<D3DCOLOR*>(vertexData + GetDiffuseOffset(hasNormals));
			*diffuseOut = *diffuse++;
		}

		if (texCoords > 0) {
			auto uvOut = reinterpret_cast<float*>(vertexData + GetUvOffset(hasNormals, hasDiffuse));
			for (auto sampler = 0; sampler < texCoords; ++sampler) {
				*uvOut++ = uv->x;
				*uvOut++ = uv->y;
			}
			uv++;
		}

		vertexData += vertexSize;
	}

}

template <bool hasNormals, bool hasDiffuse>
void FillBuffer(void* dataOut, int vertexCount, D3DXVECTOR4* pos, D3DXVECTOR4* normal, D3DCOLOR* diffuse, D3DXVECTOR2* uv, int texCoordCount) {
	switch (texCoordCount) {
	case 0:
		FillBufferUv<hasNormals, hasDiffuse, 0>(dataOut, vertexCount, pos, normal, diffuse, uv);
		break;
	case 1:
		FillBufferUv<hasNormals, hasDiffuse, 1>(dataOut, vertexCount, pos, normal, diffuse, uv);
		break;
	case 2:
		FillBufferUv<hasNormals, hasDiffuse, 2>(dataOut, vertexCount, pos, normal, diffuse, uv);
		break;
	case 3:
		FillBufferUv<hasNormals, hasDiffuse, 3>(dataOut, vertexCount, pos, normal, diffuse, uv);
		break;
	default:
	case 4:
		FillBufferUv<hasNormals, hasDiffuse, 4>(dataOut, vertexCount, pos, normal, diffuse, uv);
		break;
	}
}


Renderer::Renderer(Graphics& graphics) : mGraphics(graphics) {
}

void Renderer::DrawTris(int vertexCount,
                        D3DXVECTOR4* pos,
                        D3DXVECTOR4* normal,
                        D3DCOLOR* diffuse,
                        D3DXVECTOR2* uv1,
                        D3DXVECTOR2* uv2,
                        D3DXVECTOR2* uv3,
                        D3DXVECTOR2* uv4,
                        int primCount,
                        uint16_t* indices) {

	auto device = mGraphics.device();

	auto hasDiffuse = !!diffuse;
	auto hasNormals = !!normal;
	int texCoords = 0;
	if (uv1)
		texCoords++;
	if (uv2)
		texCoords++;
	if (uv3)
		texCoords++;
	if (uv4)
		texCoords++;
	auto vertexSize = CalcVertexSize(hasNormals, hasDiffuse, texCoords);
	auto fvf = D3DFVF_XYZ;
	if (normal) {
		fvf |= D3DFVF_NORMAL;
	}
	if (diffuse) {
		fvf |= D3DFVF_DIFFUSE;
	}
	switch (texCoords) {
	case 0:
		fvf |= D3DFVF_TEX0;
		break;
	case 1:
		fvf |= D3DFVF_TEX1;
		break;
	case 2:
		fvf |= D3DFVF_TEX2;
		break;
	case 3:
		fvf |= D3DFVF_TEX3;
		break;
	default:
	case 4:
		fvf |= D3DFVF_TEX4;
		break;
	}

	auto bufferSize = vertexSize * vertexCount;

	CComPtr<IDirect3DVertexBuffer9> buffer;
	if (D3DLOG(device->CreateVertexBuffer(bufferSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, fvf, D3DPOOL_DEFAULT, &buffer, nullptr)) != D3D_OK) {
		return;
	}

	void* vertexData;
	if (D3DLOG(buffer->Lock(0, bufferSize, (void**)&vertexData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}

	if (hasNormals) {
		if (hasDiffuse) {
			FillBuffer<true, true>(vertexData, vertexCount, pos, normal, diffuse, uv1, texCoords);
		} else {
			FillBuffer<true, false>(vertexData, vertexCount, pos, normal, diffuse, uv1, texCoords);
		}
	} else {
		if (hasDiffuse) {
			FillBuffer<false, true>(vertexData, vertexCount, pos, normal, diffuse, uv1, texCoords);
		} else {
			FillBuffer<false, false>(vertexData, vertexCount, pos, normal, diffuse, uv1, texCoords);
		}
	}

	if (D3DLOG(buffer->Unlock()) != D3D_OK) {
		return;
	}

	CComPtr<IDirect3DIndexBuffer9> indexBuffer;
	auto indexBufferSize = sizeof(uint16_t) * 3 * primCount;
	if (D3DLOG(device->CreateIndexBuffer(indexBufferSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &indexBuffer, nullptr)) != D3D_OK) {
		return;
	}

	void* indexBufferData;
	if (D3DLOG(indexBuffer->Lock(0, indexBufferSize, &indexBufferData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}
	memcpy(indexBufferData, indices, indexBufferSize);
	if (D3DLOG(indexBuffer->Unlock()) != D3D_OK) {
		return;
	}
	renderStates->SetFVF(fvf);

	renderStates->SetStreamSource(0, buffer, vertexSize);
	renderStates->SetIndexBuffer(indexBuffer, 0);

	renderStates->Commit();

	D3DLOG(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, primCount));

}

#pragma pack(push, 1)
struct ScreenSpaceVertex {
	D3DXVECTOR4 pos;
	D3DCOLOR diffuse;
	D3DXVECTOR2 uv;
};
#pragma pack(pop)

void Renderer::DrawTrisScreenSpace(int vertexCount,
                                   D3DXVECTOR4* pos,
                                   D3DCOLOR* diffuse,
                                   D3DXVECTOR2* uv,
                                   int primCount,
                                   uint16_t* indices) {

	auto device = mGraphics.device();
	
	auto bufferSize = vertexCount * sizeof(ScreenSpaceVertex);

	CComPtr<IDirect3DVertexBuffer9> buffer;
	if (D3DLOG(device->CreateVertexBuffer(bufferSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &buffer, nullptr)) != D3D_OK) {
		return;
	}

	ScreenSpaceVertex* vertexData;
	if (D3DLOG(buffer->Lock(0, bufferSize, (void**)&vertexData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}

	for (auto i = 0; i < vertexCount; ++i) {
		vertexData->pos = pos[i];
		vertexData->diffuse = diffuse[i];
		vertexData->uv = uv[i];
		vertexData++;
	}

	if (D3DLOG(buffer->Unlock()) != D3D_OK) {
		return;
	}

	CComPtr<IDirect3DIndexBuffer9> indexBuffer;
	auto indexBufferSize = sizeof(uint16_t) * 3 * primCount;
	if (D3DLOG(device->CreateIndexBuffer(indexBufferSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &indexBuffer, nullptr)) != D3D_OK) {
		return;
	}

	void* indexBufferData;
	if (D3DLOG(indexBuffer->Lock(0, indexBufferSize, &indexBufferData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}
	memcpy(indexBufferData, indices, indexBufferSize);
	if (D3DLOG(indexBuffer->Unlock()) != D3D_OK) {
		return;
	}
	
	CComPtr<IDirect3DVertexDeclaration9> mVertexDecl;
	
	D3DVERTEXELEMENT9 elements[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	D3DLOG(device->CreateVertexDeclaration(&elements[0], &mVertexDecl));

	renderStates->SetStreamSource(0, buffer, sizeof(ScreenSpaceVertex));
	renderStates->SetIndexBuffer(indexBuffer, 0);
	renderStates->Commit();
	
	device->SetVertexDeclaration(mVertexDecl);

	auto vs = mGraphics.GetShaders().LoadVertexShader("gui_vs");
	vs->Bind();

	D3DXVECTOR4 dim{ (float) config.renderWidth, (float) config.renderHeight, 0, 0 };
	device->SetVertexShaderConstantF(0, &dim[0], 1);

	D3DLOG(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, primCount));
	
}

#pragma pack(push, 1)
struct LineVertex {
	XMFLOAT3 pos;
	D3DCOLOR diffuse;
};
#pragma pack(pop)

void Renderer::DrawLines(const std::vector<Line>& lines, bool screenSpace) {

	auto device = mGraphics.device();

	auto fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	auto bufferSize = lines.size() * 2 * sizeof(LineVertex);

	CComPtr<IDirect3DVertexBuffer9> buffer;
	if (D3DLOG(device->CreateVertexBuffer(bufferSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, fvf, D3DPOOL_DEFAULT, &buffer, nullptr)) != D3D_OK) {
		return;
	}

	LineVertex* vertexData;
	if (D3DLOG(buffer->Lock(0, bufferSize, (void**)&vertexData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}

	for (const auto &line : lines) {
		vertexData->pos = line.from;
		vertexData->diffuse = line.diffuse;
		vertexData++;
		vertexData->pos = line.to;
		vertexData->diffuse = line.diffuse;
		vertexData++;
	}

	if (D3DLOG(buffer->Unlock()) != D3D_OK) {
		return;
	}

	PixelShaderPtr ps;
	VertexShaderPtr vs; 
	CComPtr<IDirect3DVertexDeclaration9> vertexDecl; // Should be cached

	renderStates->SetAlphaBlend(true);
	renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
	renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);
	if (screenSpace) {
		renderStates->SetProjectionMatrix(renderStates->Get3dProjectionMatrix());
		ps = mGraphics.GetShaders().LoadPixelShader("diffuse_only_ps");
		ps->Bind();
		vs = mGraphics.GetShaders().LoadVertexShader("gui_vs");
		vs->Bind();

		D3DVERTEXELEMENT9 elements[] = {
			{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			D3DDECL_END()
		};
		D3DLOG(device->CreateVertexDeclaration(&elements[0], &vertexDecl));
		device->SetVertexDeclaration(vertexDecl);

		renderStates->SetZEnable(false);
		renderStates->SetZWriteEnable(false);

	} else {
		renderStates->SetZEnable(true);
		renderStates->SetZWriteEnable(false);
		renderStates->SetTexture(0, nullptr);
		renderStates->SetColorVertex(true);
		renderStates->SetTextureColorOp(0, D3DTOP_SELECTARG1);
		renderStates->SetTextureColorArg1(0, D3DTA_DIFFUSE);
		renderStates->SetTextureAlphaOp(0, D3DTOP_SELECTARG1);
		renderStates->SetTextureAlphaArg1(0, D3DTA_DIFFUSE);
		renderStates->SetProjectionMatrix(renderStates->Get3dProjectionMatrix());
		renderStates->SetFVF(fvf);
		renderStates->SetLighting(false);
	}	

	renderStates->SetStreamSource(0, buffer, sizeof(LineVertex));
	renderStates->SetIndexBuffer(nullptr, 0);
	renderStates->Commit();

	D3DLOG(device->DrawPrimitive(D3DPT_LINELIST, 0, lines.size()));

	if (ps) {
		ps->Unbind();
	}
	if (vs) {
		vs->Unbind();
	}
	if (vertexDecl) {
		device->SetVertexDeclaration(nullptr);
	}

}

*/