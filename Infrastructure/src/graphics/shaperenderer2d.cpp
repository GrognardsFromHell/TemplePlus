
#include "graphics/device.h"
#include "graphics/buffers.h"
#include "graphics/bufferbinding.h"
#include "graphics/materials.h"
#include "graphics/mdfmaterials.h"
#include "graphics/shaders.h"
#include "graphics/textures.h"
#include "graphics/shaperenderer2d.h"

#include <gsl/gsl>

using namespace DirectX;

namespace gfx {
struct ShapeRenderer2d::Impl {
	Impl(RenderingDevice &device);

	RenderingDevice& device;

	VertexBufferPtr vertexBuffer;
	IndexBufferPtr indexBuffer;

	VertexBufferPtr lineVertexBuffer;

	Material outlineMaterial;
	Material pieFillMaterial;
	Material lineMaterial;
	Material untexturedMaterial;
	Material texturedMaterial;
	Material texturedWithoutBlendingMaterial;
	Material texturedWithMaskMaterial;
	SamplerStatePtr samplerWrapState;
	SamplerStatePtr samplerClampPointState;
	SamplerStatePtr samplerClampState;
	DepthStencilStatePtr noDepthState;

	BufferBinding bufferBinding;
	BufferBinding mdfBufferBinding;
	BufferBinding lineBufferBinding;

	static Material CreateMaterial(RenderingDevice &device,
		const char *pixelShaderName,
		bool forLines = false,
		bool blending = true);
	static Material CreateOutlineMaterial(RenderingDevice &device);
	static Material CreatePieFillMaterial(RenderingDevice &device);
};

ShapeRenderer2d::Impl::Impl(RenderingDevice &device)
	: device(device),
	untexturedMaterial(CreateMaterial(device, "diffuse_only_ps")),
	texturedMaterial(CreateMaterial(device, "textured_simple_ps")),
	texturedWithoutBlendingMaterial(CreateMaterial(device, "textured_simple_ps", false, false)),
	texturedWithMaskMaterial(CreateMaterial(device, "textured_two_ps")),
	lineMaterial(CreateMaterial(device, "diffuse_only_ps", true)),
	outlineMaterial(CreateOutlineMaterial(device)),
	pieFillMaterial(CreatePieFillMaterial(device)),
	bufferBinding(texturedMaterial.GetVertexShader()),
	mdfBufferBinding(device.CreateMdfBufferBinding()),
	lineBufferBinding(lineMaterial.GetVertexShader()) {

	SamplerSpec samplerWrapSpec;
	samplerWrapSpec.minFilter = TextureFilterType::Linear;
	samplerWrapSpec.magFilter = TextureFilterType::Linear;
	samplerWrapSpec.mipFilter = TextureFilterType::Linear;
	samplerWrapState = device.CreateSamplerState(samplerWrapSpec);

	SamplerSpec samplerClampPointSpec;
	samplerClampPointSpec.addressU = TextureAddress::Clamp;
	samplerClampPointSpec.addressV = TextureAddress::Clamp;
	samplerClampPointSpec.minFilter = TextureFilterType::NearestNeighbor;
	samplerClampPointSpec.magFilter = TextureFilterType::NearestNeighbor;
	samplerClampPointSpec.mipFilter = TextureFilterType::NearestNeighbor;
	samplerClampPointState = device.CreateSamplerState(samplerClampPointSpec);

	SamplerSpec samplerClampSpec;
	samplerClampSpec.addressU = TextureAddress::Clamp;
	samplerClampSpec.addressV = TextureAddress::Clamp;
	samplerClampSpec.minFilter = TextureFilterType::Linear;
	samplerClampSpec.magFilter = TextureFilterType::Linear;
	samplerClampSpec.mipFilter = TextureFilterType::Linear;
	samplerClampState = device.CreateSamplerState(samplerClampSpec);

	DepthStencilSpec noDepthSpec;
	noDepthSpec.depthEnable = false;
	noDepthState = device.CreateDepthStencilState(noDepthSpec);

	vertexBuffer = device.CreateEmptyVertexBuffer(sizeof(Vertex2d) * 8);

	std::array<uint16_t, 6> indexData{
		0, 1, 2,
		2, 3, 0
	};
	indexBuffer = device.CreateIndexBuffer(indexData);

	bufferBinding.AddBuffer(vertexBuffer, 0, sizeof(Vertex2d))
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
	
	mdfBufferBinding.AddBuffer(vertexBuffer, 0, sizeof(Vertex2d))
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
}

Material ShapeRenderer2d::Impl::CreateMaterial(RenderingDevice &device,
	const char *pixelShaderName,
	bool forLine,
	bool blending) {

	BlendSpec blendSpec;
	blendSpec.blendEnable = blending;
	blendSpec.srcBlend = BlendOperand::SrcAlpha;
	blendSpec.destBlend = BlendOperand::InvSrcAlpha;
	DepthStencilSpec depthStencilSpec;
	depthStencilSpec.depthEnable = false;
	Shaders::ShaderDefines vsDefines;
	if (forLine) {
		vsDefines["DRAW_LINES"] = "1";
	}
	auto vertexShader(device.GetShaders().LoadVertexShader("gui_vs", vsDefines));
	auto pixelShader(device.GetShaders().LoadPixelShader(pixelShaderName));

	return device.CreateMaterial(blendSpec, depthStencilSpec, {},
		{}, vertexShader, pixelShader);

}

Material ShapeRenderer2d::Impl::CreateOutlineMaterial(RenderingDevice& device) {
	BlendSpec blendSpec;
	blendSpec.blendEnable = true;
	blendSpec.srcBlend = BlendOperand::SrcAlpha;
	blendSpec.destBlend = BlendOperand::InvSrcAlpha;
	DepthStencilSpec depthStencilSpec;
	depthStencilSpec.depthEnable = false;
	auto vertexShader(device.GetShaders().LoadVertexShader("line_vs"));
	auto pixelShader(device.GetShaders().LoadPixelShader("diffuse_only_ps"));

	return device.CreateMaterial(blendSpec, depthStencilSpec, {}, {}, vertexShader, pixelShader);
}

Material ShapeRenderer2d::Impl::CreatePieFillMaterial(RenderingDevice& device) {
	BlendSpec blendSpec;
	blendSpec.blendEnable = true;
	blendSpec.srcBlend = BlendOperand::SrcAlpha;
	blendSpec.destBlend = BlendOperand::InvSrcAlpha;
	DepthStencilSpec depthStencilSpec;
	depthStencilSpec.depthEnable = false;
	RasterizerSpec rasterizerSpec;
	rasterizerSpec.cullMode = CullMode::None;

	auto vertexShader(device.GetShaders().LoadVertexShader("gui_vs"));
	auto pixelShader(device.GetShaders().LoadPixelShader("diffuse_only_ps"));

	return device.CreateMaterial(blendSpec, depthStencilSpec, rasterizerSpec,
		{}, vertexShader, pixelShader);
}

ShapeRenderer2d::ShapeRenderer2d(RenderingDevice& device) : mImpl(std::make_unique<Impl>(device)) {
}

ShapeRenderer2d::~ShapeRenderer2d() {
}

void ShapeRenderer2d::DrawRectangle(float x, float y, float width, float height, gfx::Texture* texture, uint32_t color, SamplerType2d samplerType) {

	if (texture) {
		mImpl->device.SetMaterial(mImpl->texturedMaterial);
	} else {
		mImpl->device.SetMaterial(mImpl->untexturedMaterial);
	}

	// Generate the vertex data
	std::array<Vertex2d, 4> vertices;

	// Upper Left
	vertices[0].pos = XMFLOAT4(x, y, 0.5f, 1);
	vertices[0].uv = XMFLOAT2(0, 0);
	vertices[0].diffuse = color;

	// Upper Right
	vertices[1].pos = XMFLOAT4(x + width, y, 0.5f, 1);
	vertices[1].uv = XMFLOAT2(1, 0);
	vertices[1].diffuse = color;

	// Lower Right
	vertices[2].pos = XMFLOAT4(x + width, y + height, 0.5f, 1);
	vertices[2].uv = XMFLOAT2(1, 1);
	vertices[2].diffuse = color;

	// Lower Left
	vertices[3].pos = XMFLOAT4(x, y + height, 0.5f, 1);
	vertices[3].uv = XMFLOAT2(0, 1);
	vertices[3].diffuse = color;

	DrawRectangle(vertices, texture, nullptr, samplerType);

}

void ShapeRenderer2d::DrawRectangle(gsl::span<Vertex2d, 4> corners,
	gfx::Texture* texture,
	gfx::Texture* mask,
	SamplerType2d samplerType,
	bool blending) {

	auto& samplerState = getSamplerState(samplerType);

	if (texture && mask) {
		Expects(blending);
		mImpl->device.SetMaterial(mImpl->texturedWithMaskMaterial);
		mImpl->device.SetSamplerState(0, samplerState);
		mImpl->device.SetSamplerState(1, samplerState);
		mImpl->device.SetTexture(0, *texture);
		mImpl->device.SetTexture(1, *mask);
	} else if (texture) {
		if (blending) {
			mImpl->device.SetMaterial(mImpl->texturedMaterial);
		} else {
			mImpl->device.SetMaterial(mImpl->texturedWithoutBlendingMaterial);
		}
		mImpl->device.SetSamplerState(0, samplerState);
		mImpl->device.SetTexture(0, *texture);
	} else {
		mImpl->device.SetMaterial(mImpl->untexturedMaterial);
	}

	DrawRectangle(corners);

	if (texture) {
		mImpl->device.SetTexture(0, *Texture::GetInvalidTexture());
	}
	if (mask) {
		mImpl->device.SetTexture(1, *Texture::GetInvalidTexture());
	}

}

	SamplerState &ShapeRenderer2d::getSamplerState(SamplerType2d type) const {
		switch (type) {
		default:
		case SamplerType2d::CLAMP:
			return *mImpl->samplerClampState;
		case SamplerType2d::POINT:
			return *mImpl->samplerClampPointState;
		case SamplerType2d::WRAP:
			return *mImpl->samplerWrapState;
		}
	}

	void ShapeRenderer2d::DrawRectangle(gsl::span<Vertex2d, 4> corners,
		const gfx::MdfRenderMaterialPtr& material) {

		MdfRenderOverrides overrides;
		overrides.ignoreLighting = true;
		overrides.uiProjection = true;
		material->Bind(mImpl->device, {}, &overrides);

		mImpl->device.SetDepthStencilState(*mImpl->noDepthState);

		for (auto &vertex : corners) {
			vertex.normal = XMFLOAT4(0, 0, -1, 0);
		}

		// Copy the vertices
		mImpl->device.UpdateBuffer<Vertex2d>(*mImpl->vertexBuffer, corners);

		mImpl->mdfBufferBinding.Bind();

		mImpl->device.SetIndexBuffer(*mImpl->indexBuffer);

		mImpl->device.DrawIndexed(PrimitiveType::TriangleList, 4, 6);
	}

	void ShapeRenderer2d::DrawRectangle(gsl::span<Vertex2d, 4> corners) {

		for (auto &vertex : corners) {
			vertex.normal = XMFLOAT4(0, 0, -1, 0);
		}

	// Copy the vertices
	mImpl->device.UpdateBuffer<Vertex2d>(*mImpl->vertexBuffer, corners);
	
	mImpl->device.SetVertexShaderConstant(0, gfx::StandardSlotSemantic::UiProjMatrix);

	mImpl->bufferBinding.Bind();

	mImpl->device.SetIndexBuffer(*mImpl->indexBuffer);

	mImpl->device.DrawIndexed(PrimitiveType::TriangleList, 4, 6);
}

void ShapeRenderer2d::DrawLines(gsl::span<Line2d> lines) {

	mImpl->device.SetMaterial(mImpl->lineMaterial);

	mImpl->device.SetVertexShaderConstant(0, gfx::StandardSlotSemantic::UiProjMatrix);
	
	mImpl->bufferBinding.Bind();

	// Render in batches of 4
	for (auto &line : lines) {

		// Generate the vertex data
		auto locked(mImpl->device.Map<Vertex2d>(*mImpl->vertexBuffer));

		locked[0].pos.x = line.from.x;
		locked[0].pos.y = line.from.y;
		locked[0].pos.z = 0.5f;
		locked[0].pos.w = 1;
		locked[0].diffuse = line.diffuse;

		locked[1].pos.x = line.to.x;
		locked[1].pos.y = line.to.y;
		locked[1].pos.z = 0.5f;
		locked[1].pos.w = 1;
		locked[1].diffuse = line.diffuse;

		locked.Unmap();

		mImpl->device.Draw(gfx::PrimitiveType::LineList, 2);
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
		fullScreenCorners[0].pos = { -1, -1, 0, 1 };
		fullScreenCorners[0].uv = { 0, 0 };
		fullScreenCorners[1].pos = { 1, -1, 0, 1 };
		fullScreenCorners[1].uv = { 1, 0 };
		fullScreenCorners[2].pos = { 1, 1, 0, 1 };
		fullScreenCorners[2].uv = { 1, 1 };
		fullScreenCorners[3].pos = { -1, 1, 0, 1 };
		fullScreenCorners[3].uv = { 0, 1 };
		DrawRectangle(fullScreenCorners);

	}

	static constexpr size_t MaxSegments = 50;

	struct PieSegmentGlobals {
		XMFLOAT4X4 projMat;
		XMFLOAT4 colors;
	};

	void ShapeRenderer2d::DrawPieSegment(int segments, 
		int x, int y, 
		float angleCenter, float angleWidth, 
		int innerRadius, int innerOffset, 
		int outerRadius, int outerOffset, 
		XMCOLOR color1, 
		XMCOLOR color2) {

		Expects(segments <= MaxSegments);

		auto posCount = segments * 2 + 2;
		
		// There are two positions for the start and 2 more for each segment thereafter
		static constexpr size_t MaxPositions = MaxSegments * 2 + 2;
		std::array<Vertex2d, MaxPositions> vertices;

		auto angleStep = angleWidth / (posCount);
		auto angleStart = angleCenter - angleWidth * 0.5f;

		// We generate one more position because of the starting points
		for (auto i = 0; i < posCount; ++i) {
			auto angle = angleStart + i * angleStep;
			auto& pos = vertices[i].pos;
			vertices[i].diffuse = color1;

			// The generated positions alternate between the outside
			// and inner circle
			if (i % 2 == 0) {
				pos.x = x + cosf(angle) * innerRadius - sinf(angle) * innerOffset;
				pos.y = y + sinf(angle) * innerRadius + cosf(angle) * innerOffset;
			} else {
				pos.x = x + cosf(angle) * outerRadius - sinf(angle) * outerOffset;
				pos.y = y + sinf(angle) * outerRadius + cosf(angle) * outerOffset;
			}
			pos.z = 0;
			pos.w = 1;
		}

		auto& device = mImpl->device;

		device.SetMaterial(mImpl->pieFillMaterial);

		gfx::BufferBinding bufferBinding(mImpl->pieFillMaterial.GetVertexShader());
		auto buffer(device.CreateVertexBuffer<Vertex2d>(vertices, false));
		bufferBinding.AddBuffer(buffer, 0, sizeof(Vertex2d))
			.AddElement(gfx::VertexElementType::Float4, gfx::VertexElementSemantic::Position)
			.AddElement(gfx::VertexElementType::Float4, gfx::VertexElementSemantic::Normal)
			.AddElement(gfx::VertexElementType::Color, gfx::VertexElementSemantic::Color)
			.AddElement(gfx::VertexElementType::Float2, gfx::VertexElementSemantic::TexCoord);
		bufferBinding.Bind();

		PieSegmentGlobals globals;
		globals.projMat = device.GetCamera().GetUiProjection();
		XMStoreFloat4(&globals.colors, PackedVector::XMLoadColor(&color1));
		device.SetVertexShaderConstants(0, globals);

		device.Draw(gfx::PrimitiveType::TriangleStrip, posCount);

		device.SetMaterial(mImpl->pieFillMaterial);

		// Change to the outline color
		XMStoreFloat4(&globals.colors, PackedVector::XMLoadColor(&color2));
		device.SetVertexShaderConstants(0, globals);

		// We generate one more position because of the starting points
		for (auto i = 0; i < segments + 1; ++i) {
			vertices[i * 2].diffuse = color2;
			vertices[i * 2 + 1].diffuse = color2;
		}
		buffer->Update<Vertex2d>(vertices);

		/*
			Build an index buffer that draws an outline around the pie
			segment using the previously generated positions.
		*/
		std::array<uint16_t, MaxPositions + 1> outlineIndices;
		auto i = 0;
		auto j = 0;
		// The first run of indices is along the inner radius
		while (i < posCount / 2) {
			outlineIndices[i++] = j;
			j += 2;
		}
		// Then backwards along the outer radius
		j = posCount - 1;
		while (i < posCount) {
			outlineIndices[i++] = j;
			j -= 2;
		}
		// And finally it goes back to the starting point
		outlineIndices[posCount] = 0;
		auto ib(device.CreateIndexBuffer(outlineIndices));
		device.SetIndexBuffer(*ib);

		device.DrawIndexed(gfx::PrimitiveType::LineStrip, posCount, posCount);
		
	}

}
