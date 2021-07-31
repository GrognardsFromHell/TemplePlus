
#include <platform/d3d.h>
#include <infrastructure/logging.h>

#include <graphics/bufferbinding.h>

#include "render_general.h"
#include "particles/render_point.h"
#include "particles/instances.h"

using namespace DirectX;

namespace particles {

	using namespace gfx;

	struct SpriteVertex {
		XMFLOAT4 pos;
		XMCOLOR diffuse;
		float u;
		float v;
	};

	/**
	* The rendering state associated with a point emitter.
	*/
	struct QuadEmitterRenderState : GeneralEmitterRenderState {
		QuadEmitterRenderState(RenderingDevice &device, PartSysEmitter &emitter);

		VertexBufferPtr vertexBuffer;
		BufferBinding bufferBinding;
	};

	QuadEmitterRenderState::QuadEmitterRenderState(RenderingDevice &device,
		PartSysEmitter &emitter)
		: GeneralEmitterRenderState(device, emitter, false),
		  bufferBinding(material.GetVertexShader()) {

		auto maxCount = emitter.GetSpec()->GetMaxParticles();

		vertexBuffer =
			device.CreateEmptyVertexBuffer(sizeof(SpriteVertex) * 4 * maxCount);

		bufferBinding.AddBuffer(vertexBuffer, 0, sizeof(SpriteVertex))
			.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
			.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
			.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
	}

	QuadParticleRenderer::QuadParticleRenderer(RenderingDevice &device)
		: ParticleRenderer(device), mDevice(device) {

		// Set up an index bufer for the max. quad drawing
		std::vector<uint16_t> indices;
		indices.reserve(0x8000 / 4 * 6);
		for (uint16_t i = 0; i < 0x8000; i += 4) {
			indices.push_back(i + 0);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
			indices.push_back(i + 2);
			indices.push_back(i + 3);
			indices.push_back(i + 0);
		}
		mIndexBuffer = device.CreateIndexBuffer(indices);
	}

	void QuadParticleRenderer::Render(PartSysEmitter &emitter) {

		XMFLOAT4X4 worldMatrix;
		if (!GetEmitterWorldMatrix(emitter, worldMatrix)) {
			return;
		}
		
		mDevice.SetVertexShaderConstants(0, worldMatrix);
		mDevice.SetIndexBuffer(*mIndexBuffer);

		RenderParticles(emitter);
	}

	void QuadParticleRenderer::RenderParticles(PartSysEmitter &emitter) {
		auto it = emitter.NewIterator();
		auto totalCount = emitter.GetActiveCount();

		if (totalCount == 0) {
			return;
		}

		// Lazily initialize render state
		if (!emitter.HasRenderState()) {
			emitter.SetRenderState(
				std::make_unique<QuadEmitterRenderState>(mDevice, emitter));
		}

		auto renderState =
			static_cast<QuadEmitterRenderState &>(emitter.GetRenderState());

		static eastl::vector<SpriteVertex> sUpdateBuffer;
		sUpdateBuffer.resize(4 * totalCount);

		int i = 0;
		while (it.HasNext()) {
			auto particleIdx = it.Next();
			std::span<SpriteVertex, 4> vertexSegment(&sUpdateBuffer[i], 4);
			FillVertex(emitter, particleIdx, vertexSegment);
			i += 4;
		}

		renderState.vertexBuffer->Update<SpriteVertex>({ sUpdateBuffer.data(), 4u * totalCount });

		mDevice.SetMaterial(renderState.material);
		renderState.bufferBinding.Bind();

		// Draw the batch
		mDevice.DrawIndexed(gfx::PrimitiveType::TriangleList, 4 * totalCount, 6 * totalCount);
	}

	void SpriteParticleRenderer::FillVertex(const PartSysEmitter &emitter,
		int particleIdx,
		std::span<SpriteVertex, 4> vertices) {

		// Calculate the particle scale (default is 1)
		auto scale = 1.0f;
		auto scaleParam = emitter.GetParamState(part_scale_X);
		if (scaleParam) {
			scale = scaleParam->GetValue(&emitter, particleIdx,
				emitter.GetParticleAge(particleIdx));
		}

		XMVECTOR halfPartHeightX;
		XMVECTOR halfPartHeightY;
		auto rotationParam = emitter.GetParamState(part_yaw);
		if (rotationParam) {
			auto rotation = rotationParam->GetValue(
				&emitter, particleIdx, emitter.GetParticleAge(particleIdx));
			rotation += emitter.GetParticleState().GetState(PSF_ROTATION, particleIdx);
			rotation = XMConvertToRadians(rotation);

			auto cosRot = cosf(rotation) * scale;
			auto sinRot = sinf(rotation) * scale;
			halfPartHeightX = XMLoadFloat4(&screenSpaceUnitX) * cosRot - XMLoadFloat4(&screenSpaceUnitY) * sinRot;
			halfPartHeightY = XMLoadFloat4(&screenSpaceUnitY) * cosRot + XMLoadFloat4(&screenSpaceUnitX) * sinRot;
		}
		else {
			halfPartHeightX = XMLoadFloat4(&screenSpaceUnitX) * scale;
			halfPartHeightY = XMLoadFloat4(&screenSpaceUnitY) * scale;
		}

		auto partPos(XMVectorSet(
			emitter.GetParticleState().GetState(PSF_POS_VAR_X, particleIdx),
			emitter.GetParticleState().GetState(PSF_POS_VAR_Y, particleIdx),
			emitter.GetParticleState().GetState(PSF_POS_VAR_Z, particleIdx),
			1
			));

		// Upper left corner
		XMStoreFloat4(&vertices[0].pos, partPos - halfPartHeightX + halfPartHeightY);
		// Upper right corner
		XMStoreFloat4(&vertices[1].pos, partPos + halfPartHeightX + halfPartHeightY);
		// Lower right corner
		XMStoreFloat4(&vertices[2].pos, partPos + halfPartHeightX - halfPartHeightY);
		// Lower left corner
		XMStoreFloat4(&vertices[3].pos, partPos - halfPartHeightX - halfPartHeightY);

		// Set the diffuse color for all corners
		auto diffuse = GetParticleColor(emitter, particleIdx);
		vertices[0].diffuse = diffuse;
		vertices[1].diffuse = diffuse;
		vertices[2].diffuse = diffuse;
		vertices[3].diffuse = diffuse;

		// Set UV coordinates for the sprite. We need to do this every frame
		// because we're using DISCARD for locking
		vertices[0].u = 0;
		vertices[0].v = 1;
		vertices[1].u = 1;
		vertices[1].v = 1;
		vertices[2].u = 1;
		vertices[2].v = 0;
		vertices[3].u = 0;
		vertices[3].v = 0;
	}

	void DiscParticleRenderer::FillVertex(const PartSysEmitter &emitter,
		int particleIdx,
		std::span<SpriteVertex, 4> vertex) {

		// Calculate the particle scale (default is 1)
		auto scale = 1.0f;
		auto scaleParam = emitter.GetParamState(part_scale_X);
		if (scaleParam) {
			scale = scaleParam->GetValue(&emitter, particleIdx,
				emitter.GetParticleAge(particleIdx));
		}

		XMVECTOR halfPartHeightX;
		XMVECTOR halfPartHeightY;
		auto rotationParam = emitter.GetParamState(part_yaw);
		if (rotationParam) {
			auto rotation = rotationParam->GetValue(
				&emitter, particleIdx, emitter.GetParticleAge(particleIdx));
			rotation += emitter.GetParticleState().GetState(PSF_ROTATION, particleIdx);
			rotation = XMConvertToRadians(rotation);

			auto cosRot = cosf(rotation) * scale;
			auto sinRot = sinf(rotation) * scale;
			halfPartHeightX = { -cosRot, 0, sinRot };
			halfPartHeightY = { sinRot, 0, cosRot };
		}
		else {
			halfPartHeightX = { scale, 0, 0 };
			halfPartHeightY = { 0, 0, scale };
		}

		auto partPos(XMVectorSet(
			emitter.GetParticleState().GetState(PSF_POS_VAR_X, particleIdx),
			emitter.GetParticleState().GetState(PSF_POS_VAR_Y, particleIdx),
			emitter.GetParticleState().GetState(PSF_POS_VAR_Z, particleIdx), 1));

		// Upper left corner
		XMStoreFloat4(&vertex[0].pos, partPos - halfPartHeightX - halfPartHeightY);
		// Upper right corner													   
		XMStoreFloat4(&vertex[1].pos, partPos + halfPartHeightX - halfPartHeightY);
		// Lower right corner													   
		XMStoreFloat4(&vertex[2].pos, partPos + halfPartHeightX + halfPartHeightY);
		// Lower left corner													   
		XMStoreFloat4(&vertex[3].pos, partPos - halfPartHeightX + halfPartHeightY);

		// Set the diffuse color for all corners
		auto diffuse = GetParticleColor(emitter, particleIdx);
		vertex[0].diffuse = diffuse;
		vertex[1].diffuse = diffuse;
		vertex[2].diffuse = diffuse;
		vertex[3].diffuse = diffuse;

		// Set UV coordinates for the sprite. We need to do this every frame
		// because we're using DISCARD for locking
		vertex[0].u = 0;
		vertex[0].v = 0;
		vertex[1].u = 1;
		vertex[1].v = 0;
		vertex[2].u = 1;
		vertex[2].v = 1;
		vertex[3].u = 0;
		vertex[3].v = 1;
	}
}
