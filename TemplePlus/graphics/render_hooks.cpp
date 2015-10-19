#include "stdafx.h"

#include <infrastructure/materials.h>
#include <infrastructure/renderstates.h>

#include <util/fixes.h>

#include "mdfrenderer.h"
#include "graphics.h"
#include "materials.h"
#include "renderer.h"
#include "renderstates_hooks.h"

#include <tig/tig.h>
#include <tig/tig_texture.h>
#include <ui/ui.h>

#include "render_hooks.h"

static RenderHooks fix;

const char* RenderHooks::name() {
	return "Rendering Replacements";
}

void RenderHooks::apply() {
	replaceFunction(0x101DA2D0, ShaderRender3d);
	replaceFunction(0x101D8CE0, ShaderRender2d);
	replaceFunction(0x101D9300, TextureRender2d);
	replaceFunction(0x101E8460, RenderImgFile);
}

int RenderHooks::ShaderRender3d(int vertexCount, D3DXVECTOR4* pos, D3DXVECTOR4* normals, D3DCOLOR* diffuse, D3DXVECTOR2* uv, int primCount, uint16_t* indices, int shaderId) {

	// Shader has to have been registered earlier
	auto mdfFactory = static_cast<MdfMaterialFactory*>(gfx::gMdfMaterialFactory);
	auto material = mdfFactory->GetById(shaderId);

	if (!material->IsValid()) {
		logger->error("Legacy shader with id {} wasn't found.", shaderId);
		return 17;
	}

	auto mdfMaterial = static_cast<MdfRenderMaterial*>(material.get());

	// Copy over lighting state from ToEE
	CopyLightingState();

	renderStates->SetProjectionMatrix(renderStates->Get3dProjectionMatrix());
	renderStates->SetZEnable(true);
	renderStates->SetZWriteEnable(true);

	MdfRenderer renderer(*graphics);
	renderer.Render(mdfMaterial,
	                vertexCount,
	                pos,
	                normals,
	                diffuse,
	                uv,
	                primCount,
	                indices);

	renderStates->SetZEnable(false);
	renderStates->SetZWriteEnable(false);

	return 0;
}

int RenderHooks::ShaderRender2d(const Render2dArgs* args) {

	if (!args->shaderId)
		return 17;

	// Shader has to have been registered earlier
	auto mdfFactory = static_cast<MdfMaterialFactory*>(gfx::gMdfMaterialFactory);
	auto material = mdfFactory->GetById(args->shaderId);

	if (!material || !material->IsValid()) {
		logger->error("Legacy shader with id {} wasn't found.", args->shaderId);
		return 17;
	}

	auto texture = material->GetPrimaryTexture();
	if (!texture->IsValid()) {
		logger->error("Legacy shader with id {} doesn't have a primary texture.");
		return 17;
	}

	auto textureSize = texture->GetSize();
	auto contentRect = texture->GetContentRect();

	static std::array<uint16_t, 6> indices{
		0, 1, 2, 2, 3, 0
	};

	std::array<D3DXVECTOR2, 4> uv;
	std::array<D3DXVECTOR4, 4> vertices;
	std::array<D3DXVECTOR4, 4> normals;

	auto vertexZ = 0.5f;
	if (args->flags & Render2dArgs::FLAG_VERTEXZ) {
		vertexZ = args->vertexZ;
	}

	D3DCOLOR* diffuse = nullptr;
	if (args->flags & Render2dArgs::FLAG_VERTEXCOLORS) {
		diffuse = args->vertexColors;
	}

	auto left = args->srcRect->x;
	auto right = args->srcRect->x + args->srcRect->width;
	auto top = args->srcRect->y;
	auto bottom = args->srcRect->y + args->srcRect->height;
	if (args->flags & 0x10) {
		left = textureSize.width - left - 1;
		right = textureSize.width - right - 1;
	}
	if (args->flags & 0x20) {
		top = textureSize.height - top - 1;
		bottom = textureSize.height - bottom - 1;
	}

	for (auto& normal : normals) {
		normal.x = 0;
		normal.y = 1;
		normal.z = 0;
	}

	auto destRect = args->destRect;
	auto widthFactor = 1.0f / (graphics->GetSceneWidth() / 2);
	auto heightFactor = 1.0f / (graphics->GetSceneHeight() / 2);

	vertices[0].x = destRect->x * widthFactor - 1.0f;
	vertices[0].y = 1.0f - destRect->y * heightFactor;
	vertices[0].z = vertexZ;
	uv[0].x = (left + 0.5f) / contentRect.width;
	uv[0].y = (top + 0.5f) / contentRect.height;

	vertices[1].x = (destRect->x + destRect->width) * widthFactor - 1.0f;
	vertices[1].y = 1.0f - destRect->y * heightFactor;
	vertices[1].z = vertexZ;
	uv[1].x = (right + 0.5f) / contentRect.width;
	uv[1].y = uv[0].y;

	vertices[2].x = (destRect->x + destRect->width) * widthFactor - 1.0f;
	vertices[2].y = 1.0f - (destRect->height + destRect->y) * heightFactor;
	vertices[2].z = vertexZ;
	uv[2].x = uv[1].x;
	uv[2].y = (bottom + 0.5f) / contentRect.height;

	vertices[3].x = destRect->x * widthFactor - 1.0f;
	vertices[3].y = 1.0f - (destRect->height + destRect->y) * heightFactor;
	vertices[3].z = vertexZ;
	uv[3].x = uv[0].x;
	uv[3].y = uv[2].y;

	D3DXMATRIX projMatrix;
	D3DXMatrixIdentity(&projMatrix);
	renderStates->SetProjectionMatrix(projMatrix);

	auto mdfMaterial = static_cast<MdfRenderMaterial*>(material.get());
	MdfRenderer renderer(*graphics);
	renderer.Render(mdfMaterial, 4, &vertices[0], &normals[0], &diffuse[0], &uv[0], 2, &indices[0]);
	return 0;

}

int RenderHooks::TextureRender2d(const Render2dArgs* args) {

	CopyLightingState();

	float texwidth;
	float texheight;
	float srcX;
	float srcY;
	float srcWidth;
	float srcHeight;
	std::array<D3DXVECTOR4, 4> vertices;
	std::array<D3DXVECTOR2, 4> uv;

	// The townmap UI uses floating point coordinates for the srcrect
	// for whatever reason. They are passed in place of the integer coordinates
	// And need to be reinterpreted
	if (args->flags & Render2dArgs::FLAG_FLOATSRCRECT) {
		srcX = *(float *)&args->srcRect->x;
		srcY = *(float *)&args->srcRect->y;
		srcWidth = *(float *)&args->srcRect->width;
		srcHeight = *(float *)&args->srcRect->height;
	} else {
		srcX = (float)args->srcRect->x;
		srcY = (float)args->srcRect->y;
		srcWidth = (float)args->srcRect->width;
		srcHeight = (float)args->srcRect->height;
	}

	// Has a special vertex z value been set? Otherwise we render all UI
	// on the same level
	auto vertexZ = 0.5f;
	if (args->flags & Render2dArgs::FLAG_VERTEXZ) {
		vertexZ = args->vertexZ;
	}

	// Inherit vertex colors from the caller
	std::array<D3DCOLOR, 4> diffuse;
	if (args->flags & Render2dArgs::FLAG_VERTEXCOLORS) {
		// Previously, ToEE tried to compute some gradient stuff here
		// which we removed because it was never actually utilized properly
		diffuse[0] = args->vertexColors[0] | 0xFF000000;
		diffuse[1] = args->vertexColors[1] | 0xFF000000;
		diffuse[2] = args->vertexColors[2] | 0xFF000000;
		diffuse[3] = args->vertexColors[3] | 0xFF000000;
		renderStates->SetColorVertex(true);
	} else {
		diffuse[3] = 0xFFFFFFFF;
		diffuse[2] = 0xFFFFFFFF;
		diffuse[1] = 0xFFFFFFFF;
		diffuse[0] = 0xFFFFFFFF;
		renderStates->SetColorVertex(false);
	}

	// Only if this flag is set, is the alpha value of 
	// the vertex colors used
	if (args->flags & Render2dArgs::FLAG_VERTEXALPHA) {
		diffuse[0] &= args->vertexColors[0] | 0xFFFFFF;
		diffuse[1] &= args->vertexColors[1] | 0xFFFFFF;
		diffuse[2] &= args->vertexColors[2] | 0xFFFFFF;
		diffuse[3] &= args->vertexColors[3] | 0xFFFFFF;
	}

	// Load the associated texture
	IDirect3DTexture9* deviceTexture = nullptr;
	if (!(args->flags & Render2dArgs::FLAG_BUFFER)) {
		if (args->textureId) {
			auto texture = gfx::textureManager->GetById(args->textureId);
			if (!texture || !texture->IsValid()) {
				return 17;
			}

			deviceTexture = texture->GetDeviceTexture();
			if (deviceTexture == nullptr) {
				return 17;
			}

			auto size = texture->GetSize();
			texwidth = (float) size.width;
			texheight = (float) size.height;
		} else {
			texwidth = (float) args->destRect->width;
			texheight = (float) args->destRect->height;
		}
	} else {
		if (!args->texBuffer) {
			return 17;
		}
		deviceTexture = GetTextureDelegate(args->texBuffer->d3dtexture);
		if (!deviceTexture) {
			return 17;
		}
		texwidth = (float) args->texBuffer->texturewidth;
		texheight = (float) args->texBuffer->textureheight;
	}

	auto contentRectLeft = srcX;
	auto contentRectTop = srcY;
	auto contentRectRight = srcX + srcWidth;
	auto contentRectBottom = srcY + srcHeight;
	if (args->flags & Render2dArgs::FLAG_FLIPH) {
		contentRectLeft = srcWidth - srcX - 1.0f;
		contentRectRight = srcWidth - contentRectRight - 1.0f;
	}
	if (args->flags & Render2dArgs::FLAG_FLIPV) {
		contentRectTop = srcHeight - srcY - 1.0f;
		contentRectBottom = srcHeight - contentRectBottom - 1.0f;
	}

	// Create the UV coordinates to honor the contentRect based 
	// on the real texture size
	auto uvLeft = (contentRectLeft + 0.5f) / texwidth;
	auto uvRight = (contentRectRight + 0.5f) / texwidth;
	auto uvTop = (contentRectTop + 0.5f) / texheight;
	auto uvBottom = (contentRectBottom + 0.5f) / texheight;
	uv[0].x = uvLeft;
	uv[0].y = uvTop;
	uv[1].x = uvRight;
	uv[1].y = uvTop;
	uv[2].x = uvRight;
	uv[2].y = uvBottom;
	uv[3].x = uvLeft;
	uv[3].y = uvBottom;

	for (auto i = 0; i < 4; ++i) {
		vertices[i].w = 1;
	}

	if (args->flags & Render2dArgs::FLAG_ROTATE) {
		// Rotation?
		auto cosRot = cosf(args->rotation);
		auto sinRot = sinf(args->rotation);
		auto destRect = args->destRect;
		vertices[0].x = args->rotationX
			+ (destRect->x - args->rotationX) * cosRot
			- (destRect->y - args->rotationY) * sinRot;
		vertices[0].y = args->rotationY
			+ (destRect->y - args->rotationY) * cosRot
			+ (destRect->x - args->rotationX) * sinRot;
		vertices[0].z = vertexZ;

		vertices[1].x = args->rotationX
			+ ((destRect->x + destRect->width) - args->rotationX) * cosRot
			- (destRect->y - args->rotationY) * sinRot;
		vertices[1].y = args->rotationY
			+ ((destRect->x + destRect->width) - args->rotationX) * sinRot
			+ (destRect->y - args->rotationY) * cosRot;
		vertices[1].z = vertexZ;

		vertices[2].x = args->rotationX
			+ ((destRect->x + destRect->width) - args->rotationX) * cosRot
			- ((destRect->y + destRect->width) - args->rotationY) * sinRot;
		vertices[2].y = args->rotationY
			+ ((destRect->y + destRect->width) - args->rotationY) * cosRot
			+ (destRect->x + destRect->width - args->rotationX) * sinRot;
		vertices[2].z = vertexZ;

		vertices[3].x = args->rotationX
			+ (destRect->x - args->rotationX) * cosRot
			- ((destRect->y + destRect->height) - args->rotationY) * sinRot;
		vertices[3].y = args->rotationY
			+ ((destRect->y + destRect->height) - args->rotationY) * cosRot
			+ (destRect->x - args->rotationX) * sinRot;
		vertices[3].z = vertexZ;
	} else {
		auto destRect = args->destRect;
		vertices[0].x = (float) destRect->x;
		vertices[0].y = (float) destRect->y;
		vertices[0].z = vertexZ;
		vertices[1].x = (float) destRect->x + destRect->width;
		vertices[1].y = (float) destRect->y;
		vertices[1].z = vertexZ;
		vertices[2].x = (float) destRect->x + destRect->width;
		vertices[2].y = (float) destRect->y + destRect->height;
		vertices[2].z = vertexZ;
		vertices[3].x = (float) destRect->x;
		vertices[3].y = (float) destRect->y + destRect->height;
		vertices[3].z = vertexZ;
	}

	renderStates->SetAlphaTestEnable(true);
	renderStates->SetAlphaBlend(true);
	renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
	renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);

	if (deviceTexture) {
		renderStates->SetTexture(0, deviceTexture);

		renderStates->SetTextureColorOp(0, D3DTOP_MODULATE);
		renderStates->SetTextureColorArg1(0, D3DTA_TEXTURE);
		renderStates->SetTextureColorArg2(0, D3DTA_CURRENT);
		renderStates->SetTextureAlphaOp(0, D3DTOP_MODULATE);
		renderStates->SetTextureAlphaArg1(0, D3DTA_TEXTURE);
		renderStates->SetTextureAlphaArg2(0, D3DTA_CURRENT);

		renderStates->SetTextureMipFilter(0, D3DTEXF_LINEAR);
		renderStates->SetTextureMinFilter(0, D3DTEXF_LINEAR);
		renderStates->SetTextureMagFilter(0, D3DTEXF_LINEAR);


		// Seems to disable the texture alpha stage (-> discards)
		if (args->flags & 0x800) {
			renderStates->SetTextureAlphaOp(0, D3DTOP_DISABLE);
			renderStates->SetTextureAlphaArg1(0, D3DTA_DIFFUSE);
			renderStates->SetTextureAlphaArg2(0, D3DTA_DIFFUSE);
		}

		// We have a secondary texture
		if (args->flags & 0x400) {
			auto texture = gfx::textureManager->GetById(args->textureId2);
			if (!texture || !texture->IsValid() || !texture->GetDeviceTexture()) {
				return 17;
			}
			auto secondDeviceTexture = texture->GetDeviceTexture();
			renderStates->SetTexture(1, secondDeviceTexture);
			renderStates->SetTextureCoordIndex(1, 0);

			renderStates->SetTextureColorOp(1, D3DTOP_MODULATE);
			renderStates->SetTextureColorArg1(1, D3DTA_TEXTURE);
			renderStates->SetTextureColorArg2(1, D3DTA_CURRENT);
			renderStates->SetTextureAlphaOp(1, D3DTOP_MODULATE);
			renderStates->SetTextureAlphaArg1(1, D3DTA_TEXTURE);
			renderStates->SetTextureAlphaArg2(1, D3DTA_CURRENT);

			renderStates->SetTextureMipFilter(1, D3DTEXF_LINEAR);
			renderStates->SetTextureMinFilter(1, D3DTEXF_LINEAR);
			renderStates->SetTextureMagFilter(1, D3DTEXF_LINEAR);
		}
	} else {
		// Disable texturing and instead grab the color/alpha
		// from the vertex colors
		renderStates->SetTexture(0, nullptr);
		renderStates->SetTextureColorOp(0, D3DTOP_SELECTARG1);
		renderStates->SetTextureColorArg1(0, D3DTA_DIFFUSE);
		renderStates->SetTextureAlphaOp(0, D3DTOP_SELECTARG1);
		renderStates->SetTextureAlphaArg1(0, D3DTA_DIFFUSE);
	}

	if (args->flags & 0x1000) {
		renderStates->SetTextureAddressU(0, D3DTADDRESS_WRAP);
		renderStates->SetTextureAddressV(0, D3DTADDRESS_WRAP);
	}

	static std::array<uint16_t, 6> sIndices{0, 1, 2, 0, 2, 3};

	Renderer renderer(*graphics);
	renderer.DrawTrisScreenSpace(4, &vertices[0], &diffuse[0], &uv[0], 2, &sIndices[0]);

	renderStates->SetTextureMipFilter(0, D3DTEXF_POINT);
	renderStates->SetTextureMinFilter(0, D3DTEXF_POINT);
	renderStates->SetTextureMagFilter(0, D3DTEXF_POINT);

	if (args->flags & 0x1000) {
		renderStates->SetTextureAddressU(0, D3DTADDRESS_CLAMP);
		renderStates->SetTextureAddressV(0, D3DTADDRESS_CLAMP);
	}
	if (args->flags & 0x400) {
		renderStates->SetTexture(1, nullptr);
		renderStates->SetTextureAlphaOp(1, D3DTOP_DISABLE);
		renderStates->SetTextureColorOp(1, D3DTOP_DISABLE);
		renderStates->SetTextureMipFilter(1, D3DTEXF_POINT);
		renderStates->SetTextureMinFilter(1, D3DTEXF_POINT);
		renderStates->SetTextureMagFilter(1, D3DTEXF_POINT);
	}
	renderStates->SetAlphaBlend(false);
	return 0;

}

void RenderHooks::RenderImgFile(ImgFile* img, int x, int y) {

	TigRect srcRect;
	srcRect.x = 0;
	srcRect.y = 0;
	TigRect destRect;

	Render2dArgs drawArgs;
	drawArgs.flags = 0;
	drawArgs.srcRect = &srcRect;
	drawArgs.destRect = &destRect;

	auto texId = &img->textureIds[0];
	auto curX = 0;

	for (auto tileX = 0; tileX < img->tilesX; ++tileX) {

		// The last column fills the remaining space which is less than 256 pixels
		int colWidth;
		if (curX + 256 <= img->width) {
			colWidth = 256;
		} else {
			colWidth = img->width - curX;
		}

		auto curY = img->height;

		for (auto tileY = 0; tileY < img->tilesY; ++tileY) {

			auto rowHeight = std::min(256, curY);
			curY -= rowHeight;

			srcRect.width = colWidth;
			srcRect.height = rowHeight;

			destRect.x = x + curX;
			destRect.y = y + curY;
			destRect.width = colWidth;
			destRect.height = rowHeight;

			drawArgs.textureId = *texId++;

			TextureRender2d(&drawArgs);
		}

		curX += colWidth;

	}

}

#pragma pack(push, 1)
struct RenderRectVertex {
	D3DXVECTOR4 pos;
	D3DCOLOR diffuse;
};
#pragma pack(pop)

int RenderHooks::RenderRect(D3DXVECTOR2 topLeft, D3DXVECTOR2 bottomRight, D3DCOLOR color) {
	auto device = graphics->device();

	constexpr auto fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	constexpr auto vertexCount = 5;
	auto bufferSize = sizeof(RenderRectVertex) * vertexCount;

	CComPtr<IDirect3DVertexBuffer9> vertexBuffer;
	if (D3DLOG(device->CreateVertexBuffer(bufferSize,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		fvf,
		D3DPOOL_DEFAULT,
		&vertexBuffer,
		nullptr)) != D3D_OK) {
		return 17;
	}

	RenderRectVertex *vertices;
	if (D3DLOG(vertexBuffer->Lock(0, bufferSize, (void**)&vertices, D3DLOCK_DISCARD)) != D3D_OK) {
		return 17;
	}

	// Set up vertices for a line strip that goes around the 
	// rectangle
	vertices[0].pos.x = topLeft.x;
	vertices[0].pos.y = topLeft.y;
	vertices[0].pos.z = 0.5f;
	vertices[0].pos.z = 1.0f;
	vertices[0].diffuse = color;

	vertices[1].pos.x = bottomRight.x;
	vertices[1].pos.y = topLeft.y;
	vertices[1].pos.z = 0.5f;
	vertices[1].pos.w = 1.0f;
	vertices[1].diffuse = color;

	vertices[2].pos.x = bottomRight.x;
	vertices[2].pos.y = bottomRight.y;
	vertices[2].pos.z = 0.5f;
	vertices[2].pos.w = 1.0f;
	vertices[2].diffuse = color;

	vertices[3].pos.x = topLeft.x;
	vertices[3].pos.y = bottomRight.y;
	vertices[3].pos.z = 0.5f;
	vertices[3].pos.w = 1.0f;
	vertices[3].diffuse = color;

	vertices[4].pos.x = topLeft.x;
	vertices[4].pos.y = topLeft.y;
	vertices[4].pos.z = 0.5f;
	vertices[4].pos.z = 1.0f;
	vertices[4].diffuse = color;
	D3DLOG(vertexBuffer->Unlock());

	renderStates->SetColorVertex(true);
	renderStates->SetAlphaBlend(false);
	renderStates->SetLighting(false);
	
	renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
	renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);

	renderStates->SetTextureAlphaOp(0, D3DTOP_DISABLE);
	renderStates->SetTextureColorOp(0, D3DTOP_DISABLE);

	renderStates->SetStreamSource(0, vertexBuffer, sizeof(RenderRectVertex));
	renderStates->SetFVF(fvf);
	renderStates->Commit();
	
	if (D3DLOG(device->DrawPrimitive(D3DPT_LINESTRIP, 0, 4)) != D3D_OK) {
		return 17;
	}
	return 0;
}
