
#include "graphics/device.h"
#include "graphics/buffers.h"
#include "graphics/bufferbinding.h"
#include "graphics/materials.h"
#include "graphics/mdfmaterials.h"
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

	Material outlineMaterial;
	Material lineMaterial;
	Material untexturedMaterial;
	Material texturedMaterial;
	Material texturedWithMaskMaterial;
	SamplerState samplerWrapState;
	SamplerState samplerClampState;
	
	static Material CreateMaterial(RenderingDevice &device,
		const char *pixelShaderName,
		bool forLines = false);
	static Material CreateOutlineMaterial(RenderingDevice &device);
};

ShapeRenderer2d::Impl::Impl(RenderingDevice &device)
	: device(device),
	untexturedMaterial(CreateMaterial(device, "diffuse_only_ps")),
	texturedMaterial(CreateMaterial(device, "textured_simple_ps")),
	texturedWithMaskMaterial(CreateMaterial(device, "textured_two_ps")),
	lineMaterial(CreateMaterial(device, "diffuse_only_ps", true)),
	outlineMaterial(CreateOutlineMaterial(device)) {

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
	{}, vertexShader, pixelShader);

}

Material ShapeRenderer2d::Impl::CreateOutlineMaterial(RenderingDevice& device) {
	BlendState blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = D3DBLEND_SRCALPHA;
	blendState.destBlend = D3DBLEND_INVSRCALPHA;
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;
	auto vertexShader(device.GetShaders().LoadVertexShader("line_vs"));
	auto pixelShader(device.GetShaders().LoadPixelShader("diffuse_only_ps"));

	return Material(blendState, depthStencilState, {},
	{}, vertexShader, pixelShader);
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

void ShapeRenderer2d::DrawRectangle(gsl::array_view<Vertex2d, 4> corners,
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

	void ShapeRenderer2d::DrawRectangle(gsl::array_view<Vertex2d, 4> corners, 
		const gfx::MdfRenderMaterialPtr& material) {

		MdfRenderOverrides overrides;
		overrides.ignoreLighting = true;
		material->Bind(mImpl->device, {}, &overrides);

		DepthStencilState depthState;
		depthState.depthEnable = false;
		mImpl->device.SetDepthStencilState(depthState);

		DrawRectangle(corners);
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

	void ShapeRenderer2d::DrawFullScreenQuad() {

		std::array<Vertex2d, 4> fullScreenCorners;
		fullScreenCorners[0].pos = { -1, -1, 0 };
		fullScreenCorners[0].uv = { 0, 0 };
		fullScreenCorners[1].pos = { 1, -1, 0 };
		fullScreenCorners[1].uv = { 1, 0 };
		fullScreenCorners[2].pos = { 1, 1, 0 };
		fullScreenCorners[2].uv = { 1, 1 };
		fullScreenCorners[3].pos = { -1, 1, 0 };
		fullScreenCorners[3].uv = { 0, 1 };
		DrawRectangle(fullScreenCorners);

	}

	void ShapeRenderer2d::DrawPieSegment(int segments, 
		int x, int y, 
		float angleCenter, float angleWidth, 
		int innerRadius, int innerOffset, 
		int outerRadius, int outerOffset, 
		XMCOLOR color1, 
		XMCOLOR color2) {
		
		Expects(segments <= 99);
		std::array<XMFLOAT3, 100> positions;

		auto angleStep = angleWidth / segments;
		auto angleStart = angleCenter - angleWidth / 2;

		for (auto i = 0; i < segments - 1; ++i) {
			auto angle = angleStart + i * angleStep;
			auto& pos = positions[i];

			// The generated positions alternate between the outside
			// and inner circle
			float radius, offset;
			if (i % 2 == 0) {
				radius = (float)innerRadius;
				offset = (float)innerOffset;
			} else {
				radius = (float)outerRadius;
				offset = (float)outerOffset;
			}

			pos.x = x + cosf(angle) * radius - sinf(angle) * offset;
			pos.y = y + sinf(angle) * radius + cosf(angle) * offset;
			pos.z = 0;
		}

		auto& device = mImpl->device;

		device.SetMaterial(mImpl->outlineMaterial);

		gfx::BufferBinding bufferBinding;
		bufferBinding.AddBuffer(device.CreateVertexBuffer<XMFLOAT3>(positions), 0, sizeof(XMFLOAT3))
			.AddElement(gfx::VertexElementType::Float3, gfx::VertexElementSemantic::Position);
		bufferBinding.Bind();

		device.GetDevice()->SetVertexShaderConstantF(0, &device.GetCamera().GetUiProjection()._11, 4);
		XMFLOAT4 colors;
		XMStoreFloat4(&colors, PackedVector::XMLoadColor(&color1));
		device.GetDevice()->SetVertexShaderConstantF(4, &colors.x, 1);

		device.GetDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, segments - 2);

		// Change to the outline color
		XMStoreFloat4(&colors, PackedVector::XMLoadColor(&color2));
		device.GetDevice()->SetVertexShaderConstantF(4, &colors.x, 1);

		/*
			Build an index buffer that draws an outline around the pie
			segment using the previously generated positions.
		*/
		std::array<uint16_t, 99> outlineIndices;
		auto i = 0;
		auto j = 0;
		// The first run of indices is along the inner radius
		while (i < segments / 2) {
			outlineIndices[i++] = j;
			j += 2;
		}
		// Then backwards along the outer radius
		j = segments - 2;
		while (i < segments - 1) {
			outlineIndices[i++] = j;
			j -= 2;
		}
		// And finally it goes back to the starting point
		outlineIndices[segments - 1] = 0;
		auto ib(device.CreateIndexBuffer(outlineIndices));
		device.GetDevice()->SetIndices(ib->GetBuffer());

		device.GetDevice()->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, segments, 0, segments - 1);
		
	}

}
