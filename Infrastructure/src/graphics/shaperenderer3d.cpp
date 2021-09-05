
#include "graphics/bufferbinding.h"
#include "graphics/shaperenderer3d.h"

// When drawing circles, how many segments do we use?
constexpr static int sCircleSegments = 74;

using namespace DirectX;

namespace gfx {	
	
	struct ShapeRenderer3d::Impl : ResourceListener {
		Impl(RenderingDevice& device);
		
		std::array<ShapeVertex3d, 16> discVerticesTpl;
		VertexBufferPtr discVertexBuffer;
		IndexBufferPtr discIndexBuffer;
		BufferBinding discBufferBinding;
		Material quadMaterial;
		Material lineMaterial;
		Material lineOccludedMaterial;
		VertexBufferPtr lineVertexBuffer;
		BufferBinding lineBinding;

		VertexBufferPtr circleVertexBuffer;
		IndexBufferPtr circleIndexBuffer;
		BufferBinding circleBinding;

		VertexBufferPtr donutVertexBuffer;
		IndexBufferPtr donutIndexBuffer;
		BufferBinding donutBinding;

		RenderingDevice &device;
		
		ResourceListenerRegistration mRegistration;

		void CreateResources(RenderingDevice&) override;
		void FreeResources(RenderingDevice&) override;

		static Material CreateLineMaterial(RenderingDevice &device, bool occludedOnly);
		static Material CreateQuadMaterial(RenderingDevice &device);

		void BindLineMaterial(XMCOLOR color, bool occludedOnly = false);
		void BindQuadMaterial(XMCOLOR color, const gfx::TextureRef &texture);
	};

	void ShapeRenderer3d::Impl::CreateResources(RenderingDevice&) {

		discVerticesTpl[0].pos = { -1.0f, 0.0f, -1.0f, 1 };
		discVerticesTpl[1].pos = { 0.0f, 0.0f, -1.0f, 1 };
		discVerticesTpl[2].pos = { 0.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[3].pos = { -1.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[4].pos = { 0.0f, 0.0f, -1.0f, 1 };
		discVerticesTpl[5].pos = { 1.0f, 0.0f, -1.0f, 1 };
		discVerticesTpl[6].pos = { 1.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[7].pos = { 0.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[8].pos = { 0.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[9].pos = { 1.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[10].pos = { 1.0f, 0.0f, 1.0f, 1 };
		discVerticesTpl[11].pos = { 0.0f, 0.0f, 1.0f, 1 };
		discVerticesTpl[12].pos = { -1.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[13].pos = { 0.0f, 0.0f, 0.0f, 1 };
		discVerticesTpl[14].pos = { 0.0f, 0.0f, 1.0f, 1 };
		discVerticesTpl[15].pos = { -1.0f, 0.0f, 1.0f, 1 };

		for (auto &discVertex : discVerticesTpl) {
			discVertex.normal = { 0, 1, 0, 0 };
		}

		discVerticesTpl[0].uv = { 0, 0 };
		discVerticesTpl[1].uv = { 1, 0 };
		discVerticesTpl[2].uv = { 1, 1 };
		discVerticesTpl[3].uv = { 0, 1 };
		discVerticesTpl[4].uv = { 0, 1 };
		discVerticesTpl[5].uv = { 0, 0 };
		discVerticesTpl[6].uv = { 1, 0 };
		discVerticesTpl[7].uv = { 1, 1 };
		discVerticesTpl[8].uv = { 1, 1 };
		discVerticesTpl[9].uv = { 0, 1 };
		discVerticesTpl[10].uv = { 0, 0 };
		discVerticesTpl[11].uv = { 1, 0 };
		discVerticesTpl[12].uv = { 1, 0 };
		discVerticesTpl[13].uv = { 1, 1 };
		discVerticesTpl[14].uv = { 0, 1 };
		discVerticesTpl[15].uv = { 0, 0 };

		std::array<uint16_t, 24> indices{
			0, 2, 1,
			0, 3, 2,
			4, 6, 5,
			4, 7, 6, 
			8, 10, 9,
			8, 11, 10,
			12, 14, 13,
			12, 15, 14
		};
		discIndexBuffer = device.CreateIndexBuffer(indices);

		discVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(ShapeVertex3d) * 16);

		discBufferBinding.AddBuffer(discVertexBuffer, 0, sizeof(ShapeVertex3d))
			.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
			.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal)
			.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

		{
			// +3 because for n lines, you need n+1 points and in this case, we have to repeat
			// the first point again to close the loop (so +2) and also include the center point
			// to draw a circle at the end (+3)
			circleVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(XMFLOAT3) * (sCircleSegments + 3));

			// Pre-generate the circle indexbuffer. 
			// One triangle per circle segment
			eastl::fixed_vector<uint16_t, sCircleSegments * 3> circleIndices;
			for (uint16_t i = 0; i < sCircleSegments; i++) {
				circleIndices.push_back(i + 2);
				circleIndices.push_back(0); // The center point has to be in the triangle
				circleIndices.push_back(i + 1);
			}
			circleIndexBuffer = device.CreateIndexBuffer(circleIndices, true);

			circleBinding.AddBuffer(circleVertexBuffer, 0, sizeof(XMFLOAT3))
				.AddElement(VertexElementType::Float3, VertexElementSemantic::Position);
		}
		

		{
			donutVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(XMFLOAT3) * (sCircleSegments));
			// Pre-generate the donut indexbuffer. 
			// Two triangles per circle segment
			eastl::fixed_vector<uint16_t, sCircleSegments * 6> donutIndices;
			for (uint16_t i = 0; i < 2*sCircleSegments-2; i++) {
				if (i % 2 == 0) {
					donutIndices.push_back(i + 1);
					donutIndices.push_back(i + 0);
					donutIndices.push_back(i + 2);
				}
				else {
					donutIndices.push_back(i);
					donutIndices.push_back(i + 1);
					donutIndices.push_back(i + 2);
				}
			}
			donutIndices.push_back(sCircleSegments-1);
			donutIndices.push_back(0);
			donutIndices.push_back(1);

			donutIndices.push_back(sCircleSegments - 1);
			donutIndices.push_back(sCircleSegments - 2);
			donutIndices.push_back(0);

			donutIndexBuffer = device.CreateIndexBuffer(donutIndices, true);

			donutBinding.AddBuffer(donutVertexBuffer, 0, sizeof(XMFLOAT3))
				.AddElement(VertexElementType::Float3, VertexElementSemantic::Position);
		}
		

		// Just the two end points of a line
		lineVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(XMFLOAT3) * 2);
		lineBinding.AddBuffer(lineVertexBuffer, 0, sizeof(XMFLOAT3))
			.AddElement(VertexElementType::Float3, VertexElementSemantic::Position);

	}

	void ShapeRenderer3d::Impl::FreeResources(RenderingDevice&) {
		discVertexBuffer.reset();
		discIndexBuffer.reset();
	}

	Material ShapeRenderer3d::Impl::CreateLineMaterial(RenderingDevice& device, bool occludedOnly) {
		BlendSpec blendState;
		blendState.blendEnable = true;
		blendState.srcBlend = BlendOperand::SrcAlpha;
		blendState.destBlend = BlendOperand::InvSrcAlpha;
		DepthStencilSpec depthState;
		depthState.depthEnable = true;
		depthState.depthWrite = false;
		if (occludedOnly) {
			depthState.depthFunc = ComparisonFunc::GreaterEqual;
		}

		auto vs = device.GetShaders().LoadVertexShader("line_vs");
		auto ps = device.GetShaders().LoadPixelShader("diffuse_only_ps");

		return device.CreateMaterial(
			blendState,
			depthState,
			{},
			{},
			vs,
			ps
		);
	}

	Material ShapeRenderer3d::Impl::CreateQuadMaterial(RenderingDevice & device)
	{
		BlendSpec blendState;
		blendState.blendEnable = true;
		blendState.srcBlend = BlendOperand::SrcAlpha;
		blendState.destBlend = BlendOperand::InvSrcAlpha;
		DepthStencilSpec depthState;
		depthState.depthEnable = true;
		depthState.depthWrite = false;
		RasterizerSpec rasterizerState;
		rasterizerState.cullMode = CullMode::None;
		Shaders::ShaderDefines vsDefines;
		vsDefines["TEXTURE_STAGES"] = "1";
		auto vs = device.GetShaders().LoadVertexShader("mdf_vs", vsDefines);
		auto ps = device.GetShaders().LoadPixelShader("textured_simple_ps");
		SamplerSpec samplerState;
		samplerState.addressU = TextureAddress::Clamp;
		samplerState.addressV = TextureAddress::Clamp;
		samplerState.minFilter = TextureFilterType::Linear;
		samplerState.magFilter = TextureFilterType::Linear;
		samplerState.mipFilter = TextureFilterType::Linear;
		std::vector<MaterialSamplerSpec> samplers{ { nullptr, samplerState } };
		
		return device.CreateMaterial(blendState, depthState, rasterizerState, samplers, vs, ps);
	}

	struct Shape3dGlobals {
		XMFLOAT4X4 viewProj;
		XMFLOAT4 colors;
	};

	void ShapeRenderer3d::Impl::BindLineMaterial(XMCOLOR color, bool occludedOnly) {
		if (occludedOnly) {
			device.SetMaterial(lineOccludedMaterial);
		} else {
			device.SetMaterial(lineMaterial);
		}

		Shape3dGlobals globals;
		globals.viewProj = device.GetCamera().GetViewProj();
		XMStoreFloat4(&globals.colors, PackedVector::XMLoadColor(&color));
		
		device.SetVertexShaderConstants(0, globals);
	}

	void ShapeRenderer3d::Impl::BindQuadMaterial(XMCOLOR color, const gfx::TextureRef & texture)
	{
		device.SetMaterial(quadMaterial);

		Shape3dGlobals globals;
		globals.viewProj = device.GetCamera().GetViewProj();
		XMStoreFloat4(&globals.colors, PackedVector::XMLoadColor(&color));

		device.SetVertexShaderConstants(0, globals);

		device.SetTexture(0, *texture);
	}

	ShapeRenderer3d::Impl::Impl(RenderingDevice& device)
		: device(device), 
		  mRegistration(device, this), 
		  lineMaterial(CreateLineMaterial(device, false)),
		  quadMaterial(CreateQuadMaterial(device)),
		  lineOccludedMaterial(CreateLineMaterial(device, true)),
		  discBufferBinding(device.CreateMdfBufferBinding()),
		  lineBinding(lineMaterial.GetVertexShader()),
		  circleBinding(lineMaterial.GetVertexShader()),
		  donutBinding(lineMaterial.GetVertexShader()){
	}

	ShapeRenderer3d::ShapeRenderer3d(RenderingDevice& device)
	 : mImpl(std::make_unique<Impl>(device)) {
	}

	ShapeRenderer3d::~ShapeRenderer3d() {
	}

	void ShapeRenderer3d::DrawDisc(
		const XMFLOAT3& center, 
		float rotation, 
		float radius,
		gfx::MdfRenderMaterialPtr& material) {
		
		std::array<ShapeVertex3d, 16> vertices = mImpl->discVerticesTpl;

		// There is some sort of rotation going on here that is related to 
		// the view transformation
		auto v8 = cosf(-0.77539754f) * -1.5f;
		auto v9 = sinf(-0.77539754f) * -1.5f;

		for (size_t i = 0; i < 16; ++i) {

			auto orgx = mImpl->discVerticesTpl[i].pos.x;
			auto orgz = mImpl->discVerticesTpl[i].pos.z;

			// The cos/sin magic rotates around the Y axis
			vertices[i].pos.x = sinf(rotation) * orgz + cosf(rotation) * orgx;
			vertices[i].pos.y = v9;
			vertices[i].pos.z = cosf(rotation) * orgz - sinf(rotation) * orgx;
			
			// Scale is being applied here
			vertices[i].pos.x = radius * vertices[i].pos.x;
			vertices[i].pos.z = radius * vertices[i].pos.z;

			vertices[i].pos.x = center.x + cosf(2.3561945f) * v8 + vertices[i].pos.x;
			vertices[i].pos.z = center.z + cosf(2.3561945f) * v8 + vertices[i].pos.z;
		}

		mImpl->discVertexBuffer->Update<ShapeVertex3d>(vertices);
		mImpl->discBufferBinding.Bind();
		material->Bind(mImpl->device, {});

		mImpl->device.SetIndexBuffer(*mImpl->discIndexBuffer);
		mImpl->device.DrawIndexed(gfx::PrimitiveType::TriangleList, 16, 8 * 3);

	}

	void ShapeRenderer3d::DrawQuad(const std::array<ShapeVertex3d, 4> &corners,
		XMCOLOR color,
		const gfx::TextureRef &texture) {

		mImpl->discVertexBuffer->Update<const ShapeVertex3d>(corners);
		mImpl->discBufferBinding.Bind();

		mImpl->BindQuadMaterial(color, texture);

		mImpl->device.SetIndexBuffer(*mImpl->discIndexBuffer);
		mImpl->device.DrawIndexed(gfx::PrimitiveType::TriangleList, 4, 2 * 3);

	}

	void ShapeRenderer3d::DrawQuad(const std::array<ShapeVertex3d, 4> &corners,
		gfx::MdfRenderMaterial &material,
		XMCOLOR color) {

		mImpl->discVertexBuffer->Update<const ShapeVertex3d>(corners);
		mImpl->discBufferBinding.Bind();

		MdfRenderOverrides overrides;
		overrides.overrideDiffuse = true;
		overrides.overrideColor = color;
		material.Bind(mImpl->device, {}, &overrides);
				
		mImpl->device.SetIndexBuffer(*mImpl->discIndexBuffer);
		mImpl->device.DrawIndexed(gfx::PrimitiveType::TriangleList, 4, 2 * 3);
	}

	void ShapeRenderer3d::DrawLine(const XMFLOAT3& from, const XMFLOAT3& to, XMCOLOR color) {

		std::array<XMFLOAT3, 2> positions{
			from,
			to
		};

		mImpl->lineVertexBuffer->Update<XMFLOAT3>(positions);
		mImpl->lineBinding.Bind();
		mImpl->BindLineMaterial(color);
		
		mImpl->device.Draw(gfx::PrimitiveType::LineList, 2);

	}

	void ShapeRenderer3d::DrawLineWithoutDepth(const XMFLOAT3& from, const XMFLOAT3& to, XMCOLOR color) {

		std::array<XMFLOAT3, 2> positions{
			from,
			to
		};

		mImpl->lineVertexBuffer->Update<XMFLOAT3>(positions);
		mImpl->lineBinding.Bind();
		mImpl->BindLineMaterial(color);
		mImpl->device.Draw(gfx::PrimitiveType::LineList, 2);

		mImpl->BindLineMaterial(color, true);
		mImpl->device.Draw(gfx::PrimitiveType::LineList, 2);

	}

	void ShapeRenderer3d::DrawFilledCircle(const XMFLOAT3& center, float radius, XMCOLOR borderColor, XMCOLOR fillColor, bool occludedOnly) {

		// The positions array contains the following:
		// 0 -> The center of the circle
		// 1 - (sCircleSegments + 1) -> Positions on the diameter of the circle
		// sCircleSegments + 2 -> The first position again to close the circle
		std::array<XMFLOAT3, sCircleSegments + 3> positions;

		// A full rotation divided by the number of segments
		auto rotPerSegment = XM_2PI / sCircleSegments;

		positions[0] = center;
		for (auto i = 1; i < sCircleSegments + 3; ++i) {
			auto rot = (sCircleSegments - i) * rotPerSegment;
			positions[i].x = center.x + cosf(rot) * radius - sinf(rot) * 0.0f;
			positions[i].y = center.y;
			positions[i].z = center.z + cosf(rot) * 0.0f + sinf(rot) * radius;
		}
		positions.back() = positions[1];
		
		mImpl->circleVertexBuffer->Update<XMFLOAT3>(positions);
		mImpl->circleBinding.Bind();
		
		mImpl->BindLineMaterial(borderColor, occludedOnly);

		// The first vertex is the center vertex, so we skip it for line drawing
		mImpl->device.Draw(gfx::PrimitiveType::LineStrip, sCircleSegments + 1, 1);

		mImpl->BindLineMaterial(fillColor, occludedOnly);

		mImpl->device.SetIndexBuffer(*mImpl->circleIndexBuffer);

		mImpl->device.DrawIndexed(gfx::PrimitiveType::TriangleList, 0, sCircleSegments * 3);
	}

	void ShapeRenderer3d::DrawDonut(const XMFLOAT3& center, float radiusInner, float radiusOuter, XMCOLOR borderColor, XMCOLOR fillColor, bool occludedOnly)
	{
		// The positions array contains the following:
		// 0 to sCircleSegments-1 -> Positions on the diameter of the circle
		// sCircleSegments  -> The first position again to close the circle
		std::array<XMFLOAT3, sCircleSegments > positions;

		// A full rotation divided by the number of segments
		auto rotPerSegment = XM_2PI / sCircleSegments;

		for (auto i = 0; i < sCircleSegments ; ++i) {
			auto rot = i * rotPerSegment;
			auto radius = (i % 2) == 0 ? radiusInner : radiusOuter;
			positions[i].x = center.x + cosf(rot) * radius - sinf(rot) * 0.0f;
			positions[i].y = center.y;
			positions[i].z = center.z + cosf(rot) * 0.0f + sinf(rot) * radius;
		}
		
		mImpl->donutVertexBuffer->Update<XMFLOAT3>(positions);
		mImpl->donutBinding.Bind();

		mImpl->BindLineMaterial(fillColor, occludedOnly);

		mImpl->device.SetIndexBuffer(*mImpl->donutIndexBuffer);

		mImpl->device.DrawIndexed(gfx::PrimitiveType::TriangleList, 0, sCircleSegments * 6 );
		
		// Outline
		mImpl->BindLineMaterial(borderColor, occludedOnly);

		/*
			Build an index buffer that draws an outline around the pie
			segment using the previously generated positions.
		*/
		static constexpr size_t MaxPositions = sCircleSegments*2 + 2;
		std::array<uint16_t, MaxPositions > outlineIndices;
		auto i = 0;
		auto j = 0; // The first vertex is the center vertex, so we skip it for line drawing
		// The first run of indices is along the inner radius
		while (i < sCircleSegments/2 ) {
			outlineIndices[i++] = j;
			j += 2;
		}
		
		// And finally it goes back to the starting point
		outlineIndices[sCircleSegments / 2] = 0;
		auto ib(mImpl->device.CreateIndexBuffer(outlineIndices, false));
		mImpl->device.SetIndexBuffer(*ib);

		mImpl->device.DrawIndexed(gfx::PrimitiveType::LineStrip, sCircleSegments/2 + 1 , sCircleSegments/2 + 1 );
		
		i = 0;
		j = 1;
		while (i < sCircleSegments / 2) {
			outlineIndices[i++] = j;
			j += 2;
		}
		outlineIndices[sCircleSegments / 2] = 1;
		ib->Update(outlineIndices);
		mImpl->device.DrawIndexed(gfx::PrimitiveType::LineStrip, sCircleSegments / 2 + 1, sCircleSegments / 2 + 1 );

	}

	static constexpr float cos45 = 0.70709997f;

	void ShapeRenderer3d::DrawCylinder(const XMFLOAT3 &pos, float radius, float height) {
		// Draw the 3d bounding cylinder of the object
		static XMCOLOR diffuse(0, 1.0f, 0, 0.5f);

		float x = pos.x;
		float y = pos.y;
		float z = pos.z;

		auto scaledRadius = radius * cos45;
		XMFLOAT3 from, to;

		from.x = x + scaledRadius;
		from.y = y;
		from.z = z - scaledRadius;
		to.x = from.x;
		to.y = y + height;
		to.z = from.z;

		DrawLine(from, to, diffuse);

		from.x = x - scaledRadius;
		from.z = z + scaledRadius;
		to.x = from.x;
		to.z = from.z;
		DrawLine(from, to, diffuse);

		/*
		Draw the circle on top and on the bottom
		of the cylinder.
		*/
		for (auto i = 0; i < 24; ++i) {
			// We rotate 360° in 24 steps of 15° each
			auto rot = i * XMConvertToRadians(15);
			auto nextRot = rot + XMConvertToRadians(15);

			// This is the bottom cap
			from.x = x + cosf(rot) * radius;
			from.y = y;
			from.z = z - sinf(rot) * radius;
			to.x = x + cosf(nextRot) * radius;
			to.y = y;
			to.z = z - sinf(nextRot) * radius;
			DrawLine(from, to, diffuse);

			// This is the top cap
			from.x = x + cosf(rot) * radius;
			from.y = y + height;
			from.z = z - sinf(rot) * radius;
			to.x = x + cosf(nextRot) * radius;
			to.y = y + height;
			to.z = z - sinf(nextRot) * radius;
			DrawLine(from, to, diffuse);
		}
	}

}
