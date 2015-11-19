
#include "graphics/device.h"
#include "graphics/buffers.h"
#include "graphics/bufferbinding.h"
#include "graphics/materials.h"
#include "graphics/shaders.h"
#include "graphics/textures.h"
#include "graphics/shaperenderer2d.h"

#include "gsl/gsl.h"

namespace gfx {

struct ShapeRenderer2d::Impl {
	Impl(RenderingDevice &device);

	RenderingDevice& device;

	VertexBufferPtr vertexBuffer;
	IndexBufferPtr indexBuffer;
	BufferBinding bufferBinding;

	VertexBufferPtr lineVertexBuffer;
	BufferBinding lineBufferBinding;

	Material lineMaterial;
	Material untexturedMaterial;
	Material texturedMaterial;
	Material texturedWithMaskMaterial;
	SamplerState samplerWrapState;
	SamplerState samplerClampState;

	static Material CreateMaterial(RenderingDevice &device,
		const char *pixelShaderName,
		bool forLines = false);
};

ShapeRenderer2d::Impl::Impl(RenderingDevice &device)
	: device(device),
	untexturedMaterial(CreateMaterial(device, "diffuse_only_ps")),
	texturedMaterial(CreateMaterial(device, "textured_simple_ps")),
	texturedWithMaskMaterial(CreateMaterial(device, "textured_two_ps")),
	lineMaterial(CreateMaterial(device, "diffuse_only_ps", true)) {

	samplerWrapState.minFilter = D3DTEXF_LINEAR;
	samplerWrapState.magFilter = D3DTEXF_LINEAR;
	samplerWrapState.mipFilter = D3DTEXF_LINEAR;

	samplerClampState.addressU = D3DTADDRESS_CLAMP;
	samplerClampState.addressV = D3DTADDRESS_CLAMP;
	samplerClampState.minFilter = D3DTEXF_LINEAR;
	samplerClampState.magFilter = D3DTEXF_LINEAR;
	samplerClampState.mipFilter = D3DTEXF_LINEAR;

	vertexBuffer = device.CreateEmptyVertexBuffer(sizeof(Vertex2d) * 8);

	std::array<uint16_t, 6> indexData{
		0, 1, 2,
		2, 3, 0
	};
	indexBuffer = device.CreateIndexBuffer(indexData);

	bufferBinding.AddBuffer(vertexBuffer, 0, sizeof(Vertex2d))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

}

Material ShapeRenderer2d::Impl::CreateMaterial(RenderingDevice &device,
	const char *pixelShaderName,
	bool forLine) {

	BlendState blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = D3DBLEND_SRCALPHA;
	blendState.destBlend = D3DBLEND_INVSRCALPHA;
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;
	RasterizerState rasterizerState;
	Shaders::ShaderDefines vsDefines;
	if (forLine) {
		vsDefines["DRAW_LINES"] = "1";
	}
	auto vertexShader(device.GetShaders().LoadVertexShader("gui_vs", vsDefines));
	auto pixelShader(device.GetShaders().LoadPixelShader(pixelShaderName));

	return Material(blendState, depthStencilState, rasterizerState,
	{},
		vertexShader, pixelShader);

}

ShapeRenderer2d::ShapeRenderer2d(RenderingDevice& device) : mImpl(std::make_unique<Impl>(device)) {
}

ShapeRenderer2d::~ShapeRenderer2d() {
}

void ShapeRenderer2d::DrawRectangle(float x, float y, float width, float height, const gfx::TextureRef& texture, uint32_t color) {

	if (texture) {
		mImpl->device.SetMaterial(mImpl->texturedMaterial);
	} else {
		mImpl->device.SetMaterial(mImpl->untexturedMaterial);
	}

	// Generate the vertex data
	std::array<Vertex2d, 4> vertices;

	// Upper Left
	vertices[0].pos = XMFLOAT3(x, y, 0.5f);
	vertices[0].uv = XMFLOAT2(0, 0);
	vertices[0].diffuse = color;

	// Upper Right
	vertices[1].pos = XMFLOAT3(x + width, y, 0.5f);
	vertices[1].uv = XMFLOAT2(1, 0);
	vertices[1].diffuse = color;

	// Lower Right
	vertices[2].pos = XMFLOAT3(x + width, y + height, 0.5f);
	vertices[2].uv = XMFLOAT2(1, 1);
	vertices[2].diffuse = color;

	// Lower Left
	vertices[3].pos = XMFLOAT3(x, y + height, 0.5f);
	vertices[3].uv = XMFLOAT2(0, 1);
	vertices[3].diffuse = color;

	if (texture) {
		DrawRectangle(vertices, texture->GetDeviceTexture());
	} else {
		DrawRectangle(vertices, nullptr);
	}

}

void ShapeRenderer2d::DrawRectangle(float x, float y, float width, float height, uint32_t color) {
	DrawRectangle(x, y, width, height, nullptr, color);
}

void ShapeRenderer2d::DrawRectangle(gsl::array_view<Vertex2d> corners,
	IDirect3DTexture9* texture,
	IDirect3DTexture9* mask,
	bool wrap) {

	auto device = mImpl->device.GetDevice();

	auto& samplerState = (wrap ? mImpl->samplerWrapState : mImpl->samplerClampState);

	if (texture && mask) {
		mImpl->device.SetMaterial(mImpl->texturedWithMaskMaterial);
		mImpl->device.SetSamplerState(0, samplerState);
		mImpl->device.SetSamplerState(1, samplerState);
		device->SetTexture(0, texture);
		device->SetTexture(1, mask);
	} else if (texture) {
		mImpl->device.SetMaterial(mImpl->texturedMaterial);
		mImpl->device.SetSamplerState(0, samplerState);
		device->SetTexture(0, texture);
	} else {
		mImpl->device.SetMaterial(mImpl->untexturedMaterial);
	}

	DrawRectangle(corners);

	if (texture) {
		device->SetTexture(0, nullptr);
	}
	if (mask) {
		device->SetTexture(1, nullptr);
	}

}

void ShapeRenderer2d::DrawRectangle(gsl::array_view<Vertex2d, 4> corners) {
	// Copy the vertices
	auto locked(mImpl->vertexBuffer->Lock<Vertex2d>());
	locked[0] = corners[0];
	locked[1] = corners[1];
	locked[2] = corners[2];
	locked[3] = corners[3];
	locked.Unlock();

	auto device = mImpl->device.GetDevice();

	// Set vertex shader properties (GUI specific)
	device->SetVertexShaderConstantF(0, &mImpl->device.GetCamera().GetUiProjection()._11, 4);

	mImpl->bufferBinding.Bind();

	device->SetIndices(mImpl->indexBuffer->GetBuffer());

	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

}

void ShapeRenderer2d::DrawLines(gsl::array_view<Line2d> lines) {

	mImpl->device.SetMaterial(mImpl->lineMaterial);

	auto device = mImpl->device.GetDevice();

	// Set vertex shader properties (GUI specific)
	device->SetVertexShaderConstantF(0, &mImpl->device.GetCamera().GetUiProjection()._11, 4);

	mImpl->bufferBinding.Bind();

	// Render in batches of 4
	for (auto &line : lines) {

		// Generate the vertex data
		auto locked(mImpl->vertexBuffer->Lock<Vertex2d>());

		locked[0].pos.x = line.from.x;
		locked[0].pos.y = line.from.y;
		locked[0].pos.z = 0.5f;
		locked[0].diffuse = line.diffuse;

		locked[1].pos.x = line.to.x;
		locked[1].pos.y = line.to.y;
		locked[1].pos.z = 0.5f;
		locked[1].diffuse = line.diffuse;

		locked.Unlock();

		device->DrawPrimitive(D3DPT_LINELIST, 0, 1);
	}

}

	void ShapeRenderer2d::DrawRectangleOutline(XMFLOAT2 topLeft, XMFLOAT2 bottomRight, XMCOLOR color) {
		
		XMFLOAT2 topRight(bottomRight.x, topLeft.y);
		XMFLOAT2 bottomLeft(topLeft.x, bottomRight.y);
		
		XMFLOAT2 lastBottomRight(bottomRight.x, bottomRight.y + 1);

		std::array<Line2d, 4> lines{
			Line2d(topLeft, topRight, color),
			Line2d(bottomLeft, bottomRight, color),
			Line2d(topLeft, bottomLeft, color),			
			Line2d(topRight, lastBottomRight, color)
		};

		DrawLines(lines);

	}

	/*
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
	}
	else {
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
	}
	else {
	diffuse[3] = 0xFFFFFFFF;
	diffuse[2] = 0xFFFFFFFF;
	diffuse[1] = 0xFFFFFFFF;
	diffuse[0] = 0xFFFFFFFF;
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
	texwidth = (float)size.width;
	texheight = (float)size.height;
	}
	else {
	texwidth = (float)args->destRect->width;
	texheight = (float)args->destRect->height;
	}
	}
	else {
	if (!args->texBuffer) {
	return 17;
	}
	deviceTexture = GetTextureDelegate(args->texBuffer->d3dtexture);
	if (!deviceTexture) {
	return 17;
	}
	texwidth = (float)args->texBuffer->texturewidth;
	texheight = (float)args->texBuffer->textureheight;
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
	}
	else {
	auto destRect = args->destRect;
	vertices[0].x = (float)destRect->x;
	vertices[0].y = (float)destRect->y;
	vertices[0].z = vertexZ;
	vertices[1].x = (float)destRect->x + destRect->width;
	vertices[1].y = (float)destRect->y;
	vertices[1].z = vertexZ;
	vertices[2].x = (float)destRect->x + destRect->width;
	vertices[2].y = (float)destRect->y + destRect->height;
	vertices[2].z = vertexZ;
	vertices[3].x = (float)destRect->x;
	vertices[3].y = (float)destRect->y + destRect->height;
	vertices[3].z = vertexZ;
	}
	
	renderStates->SetAlphaTestEnable(true);
	renderStates->SetAlphaBlend(true);
	renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
	renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);
	
	const char* shaderName;
	
	if (deviceTexture) {
	renderStates->SetTexture(0, deviceTexture);
	shaderName = "textured_simple_ps";
	
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
	
	shaderName = "textured_two_ps";
	}
	}
	else {
	// Disable texturing and instead grab the color/alpha
	// from the vertex colors
	shaderName = "diffuse_only_ps";
	}
	
	if (args->flags & Render2dArgs::FLAG_WRAP) {
	renderStates->SetTextureAddressU(0, D3DTADDRESS_WRAP);
	renderStates->SetTextureAddressV(0, D3DTADDRESS_WRAP);
	}
	
	auto ps = graphics->GetShaders().LoadPixelShader(shaderName);
	ps->Bind();
	
	static std::array<uint16_t, 6> sIndices{ 0, 1, 2, 0, 2, 3 };
	
	Renderer renderer(*graphics);
	renderer.DrawTrisScreenSpace(4, &vertices[0], &diffuse[0], &uv[0], 2, &sIndices[0]);
	
	ps->Unbind();
	
	if (args->flags & Render2dArgs::FLAG_WRAP) {
	renderStates->SetTextureAddressU(0, D3DTADDRESS_CLAMP);
	renderStates->SetTextureAddressV(0, D3DTADDRESS_CLAMP);
	}
	if (args->flags & 0x400) {
	renderStates->SetTexture(1, nullptr);
	}
	renderStates->SetAlphaBlend(false);
	return 0;
	*/

}
