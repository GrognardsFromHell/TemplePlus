#include <logging.h>
#include <renderstates.h>
#include <d3dx9math.h>

#include "particles/render_point.h"
#include "particles/instances.h"

namespace particles {

	static const int MaxBatchSize = 1000;

	// Structure used for vertices
	struct PointVertex {
		float x;
		float y;
		float z;
		float size;
		D3DCOLOR diffuse;
	};

	static uint8_t GetParticleColorComponent(ParticleStateField stateField,
	                                         PartSysParamId paramId,
	                                         const PartSysEmitter* emitter,
	                                         int particleIdx) {
		auto colorParam = emitter->GetParamState(paramId);
		uint8_t value;
		if (colorParam) {
			auto partAge = emitter->GetParticleAge(particleIdx);
			auto partColor = colorParam->GetValue(emitter, particleIdx, partAge);
			partColor += emitter->GetParticleState().GetState(stateField, particleIdx);
			if (partColor >= 255) {
				value = 255;
			} else if (partColor < 0) {
				value = 0;
			} else {
				value = (uint8_t)partColor;
			}
		} else {
			value = (uint8_t)emitter->GetParticleState().GetState(stateField, particleIdx);
		}
		return value;
	}

	static D3DCOLOR GetParticleColor(const PartSysEmitter* emitter, int particleIdx) {
		auto red = GetParticleColorComponent(PSF_RED, part_red, emitter, particleIdx);
		auto green = GetParticleColorComponent(PSF_GREEN, part_green, emitter, particleIdx);
		auto blue = GetParticleColorComponent(PSF_BLUE, part_blue, emitter, particleIdx);
		auto alpha = GetParticleColorComponent(PSF_ALPHA, part_alpha, emitter, particleIdx);

		return D3DCOLOR_ARGB(alpha, red, green, blue);
	}

	static bool wasLightingEnabled;

	static void EnableParticleRenderStates() {
		renderStates->SetZEnable(true);
		renderStates->SetColorVertex(true);
		renderStates->SetZWriteEnable(false);
		
		renderStates->SetTextureAlphaOp(0, D3DTOP_MODULATE);
		renderStates->SetTextureAlphaArg1(0, D3DTA_TEXTURE);
		renderStates->SetTextureAlphaArg2(0, D3DTA_DIFFUSE);

		renderStates->SetTextureColorOp(0, D3DTOP_MODULATE);
		renderStates->SetTextureColorArg1(0, D3DTA_TEXTURE);
		renderStates->SetTextureColorArg2(0, D3DTA_DIFFUSE);

		wasLightingEnabled = renderStates->IsLighting();
		renderStates->SetLighting(false);
	}

	static void DisableParticleRenderStates() {
		renderStates->SetZEnable(false);
		renderStates->SetAlphaBlend(false);
		renderStates->SetTextureTransformFlags(0, D3DTTFF_DISABLE);
		renderStates->SetZWriteEnable(true);
		renderStates->SetLighting(wasLightingEnabled);
	}

	static DWORD CoerceToInteger(float value) {
		return *(DWORD*)&value;
	}

	static void EnablePointRenderStates(IDirect3DDevice9* device) {
		device->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
		device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
		device->SetRenderState(D3DRS_POINTSIZE, CoerceToInteger(30.08f));
		device->SetRenderState(D3DRS_POINTSIZE_MIN, CoerceToInteger(10.0f));
		device->SetRenderState(D3DRS_POINTSIZE_MAX, CoerceToInteger(64.0f));
		device->SetRenderState(D3DRS_POINTSCALE_A, CoerceToInteger(10.0f));
		device->SetRenderState(D3DRS_POINTSCALE_B, CoerceToInteger(10.0f));
		device->SetRenderState(D3DRS_POINTSCALE_C, CoerceToInteger(10.0f));
		renderStates->SetFVF(D3DFVF_PSIZE | D3DFVF_DIFFUSE | D3DFVF_XYZ);
	}

	static void DisablePointRenderStates(IDirect3DDevice9* device) {
		device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	}

	static void SetupTextureState(IDirect3DDevice9* device, const PartSysEmitter* emitter) {

		auto material = emitter->GetSpec()->GetMaterial();

		if (!material) {
			return;
		}

		auto texture = material->GetPrimaryTexture();

		if (!texture) {
			return;
		}

		auto deviceTexture = texture->GetDeviceTexture();
		renderStates->SetTexture(0, deviceTexture);

		// Set up a texture transform if the actual content rect does not 
		// span the entire GPU texture. This is required for point sprites
		// since the UV coordinates are "non negotiable" in a sense.
		// D3D9 will always generate 0 and 1 at the corners. Not what we'd
		// actually need here.
		const auto& contentRect = texture->GetContentRect();
		const auto& texSize = texture->GetSize();

		auto needsTransform = (contentRect.x
			|| contentRect.y
			|| contentRect.width != texSize.width
			|| contentRect.height != texSize.height);

		if (!needsTransform) {
			renderStates->SetTextureTransformFlags(0, D3DTTFF_DISABLE);
			return; // We're good, no transform needed
		}

		D3DXMATRIX transform;
		auto sx = contentRect.width / (float)texSize.width;
		auto sy = contentRect.height / (float)texSize.height;
		D3DXMatrixScaling(&transform, sx, sy, 1.0);

		// Translation is only needed in very rare cases (texture atlases)
		if (contentRect.x || contentRect.y) {
			auto tx = contentRect.x / (float)texSize.width;
			auto ty = contentRect.y / (float)texSize.height;
			D3DXMATRIX translation;
			D3DXMatrixTranslation(&translation, tx, ty, 0.0);
			D3DXMatrixMultiply(&transform, &translation, &transform);
		}

		renderStates->SetTextureTransformFlags(0, D3DTTFF_COUNT2);
		auto result = device->SetTransform(D3DTS_TEXTURE0, &transform);
		if (!SUCCEEDED(result)) {
			logger->warn("Unable to set texture transform for rendering point sprites: {}", result);
		}

	}

	static void SetupBlending(IDirect3DDevice9* device, const PartSysEmitter* emitter) {
		renderStates->SetAlphaBlend(true);

		switch (emitter->GetSpec()->GetBlendMode()) {
		case PartSysBlendMode::Add:
			renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
			renderStates->SetDestBlend(D3DBLEND_ONE);
			break;
		case PartSysBlendMode::Subtract:
			renderStates->SetSrcBlend(D3DBLEND_ZERO);
			renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);
			break;
		case PartSysBlendMode::Blend:
			renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
			renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);
			break;
		case PartSysBlendMode::Multiply:
			renderStates->SetSrcBlend(D3DBLEND_ZERO);
			renderStates->SetDestBlend(D3DBLEND_SRCCOLOR);
			break;
		default:
			break;
		}
	}

	PointParticleRenderer::PointParticleRenderer(IDirect3DDevice9* device) : mDevice(device) {

		auto result = device->CreateVertexBuffer(sizeof(PointVertex) * MaxBatchSize,
		                                         D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
		                                         D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_PSIZE,
		                                         D3DPOOL_DEFAULT,
		                                         &mBuffer,
		                                         nullptr);

		if (!SUCCEEDED(result)) {
			logger->error("Unable to create particle system point sprite buffer: {}", result);
		}

	}

	void PointParticleRenderer::RenderParticles(const PartSysEmitter* emitter) {
		PointVertex* vertices;
		auto it = emitter->NewIterator();
		auto totalCount = emitter->GetActiveCount();

		for (auto offset = 0; offset < totalCount; offset += MaxBatchSize) {

			auto sizeOfBatch = std::min<int>(MaxBatchSize, totalCount - offset);

			// Lock the vertex buffer for exactly the number of particles we're gonna draw
			auto result = mBuffer->Lock(0, sizeOfBatch * sizeof(PointVertex), (void**) &vertices, D3DLOCK_DISCARD);
			if (!SUCCEEDED(result)) {
				logger->error("Unable to lock particle vertex buffer.");
				return;
			}

			while (it.HasNext()) {
				auto particleIdx = it.Next();

				FillPointVertex(emitter, particleIdx, vertices++);
			}

			result = mBuffer->Unlock();
			if (!SUCCEEDED(result)) {
				logger->error("Unable to unlock particle vertex buffer.");
				return;
			}

			// Draw the batch
			result = mDevice->DrawPrimitive(D3DPT_POINTLIST, 0, sizeOfBatch);
			if (!SUCCEEDED(result)) {
				logger->error("Unable to draw the point particles.");
				return;
			}
		}
	}

	void PointParticleRenderer::Render(const PartSysEmitter* emitter) {

		D3DMATRIX worldMatrix;
		if (!GetEmitterWorldMatrix(emitter, worldMatrix)) {
			return;
		}

		renderStates->SetProjectionMatrix(worldMatrix);
		

		// Apply particle drawing render states
		SetupTextureState(mDevice, emitter);
		SetupBlending(mDevice, emitter);
		EnableParticleRenderStates();
		EnablePointRenderStates(mDevice);
		renderStates->SetStreamSource(0, mBuffer, sizeof(PointVertex));
		renderStates->Commit();

		RenderParticles(emitter);

		DisableParticleRenderStates();
		DisablePointRenderStates(mDevice);

	}

	void PointParticleRenderer::FillPointVertex(const PartSysEmitter* emitter, int particleIdx, PointVertex* vertex) {

		vertex->diffuse = GetParticleColor(emitter, particleIdx);
		vertex->x = emitter->GetParticleState().GetState(PSF_POS_VAR_X, particleIdx);
		vertex->y = emitter->GetParticleState().GetState(PSF_POS_VAR_Y, particleIdx);
		vertex->z = emitter->GetParticleState().GetState(PSF_POS_VAR_Z, particleIdx);

		auto scaleParam = emitter->GetParamState(part_scale_X);
		if (scaleParam) {
			vertex->size = scaleParam->GetValue(emitter, particleIdx, emitter->GetParticleAge(particleIdx));
		} else {
			vertex->size = 1.0f;
		}

	}

	struct SpriteVertex {
		D3DXVECTOR3 pos;
		D3DCOLOR diffuse;
		float u;
		float v;

		static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	};

	SpriteParticleRenderer::SpriteParticleRenderer(IDirect3DDevice9* device) : mDevice(device) {

		auto result = device->CreateVertexBuffer(sizeof(SpriteVertex) * 4 * MaxBatchSize,
		                                         D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		                                         SpriteVertex::FVF,
		                                         D3DPOOL_DEFAULT,
		                                         &mBuffer,
		                                         nullptr);

		if (!SUCCEEDED(result)) {
			logger->error("Unable to create particle system sprite buffer: {}", result);
		}

		auto indicesSize = sizeof(uint16_t) * 6 * MaxBatchSize;
		result = device->CreateIndexBuffer(indicesSize,
		                                   D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		                                   D3DFMT_INDEX16,
		                                   D3DPOOL_DEFAULT,
		                                   &mIndices,
		                                   nullptr);

		if (!SUCCEEDED(result)) {
			logger->error("Unable to create particle system sprite index buffer: {}", result);
		}

		if (!mIndices) {
			return;
		}

		// Fill the index buffer since it'll be constant throughout it's lifetime
		uint16_t* indicesData;
		result = mIndices->Lock(0, indicesSize, (void**) &indicesData, D3DLOCK_DISCARD);
		if (!SUCCEEDED(result)) {
			logger->error("Unable to lock index buffer for filling in initial indices: {}", result);
		}

		/*
			We set up the indices to effectively draw a quad
		*/
		auto baseVertex = 0;
		for (auto i = 0; i < MaxBatchSize; ++i) {
			// NOTE: in Toee this actually seems the wrong way around since culling there
			// seems to be disabled (?)

			// Top left triangle (top left, top right, bottom left)
			indicesData[2] = baseVertex;
			indicesData[1] = baseVertex + 1;
			indicesData[0] = baseVertex + 3;
			// Lower right triangle (top right, bottom right, bottom left)
			indicesData[5] = baseVertex + 1;
			indicesData[4] = baseVertex + 2;
			indicesData[3] = baseVertex + 3;
			indicesData += 6;
			baseVertex += 4;
		}

		result = mIndices->Unlock();
		if (!SUCCEEDED(result)) {
			logger->error("Unable to unlock index buffer: {}", result);
		}

	}

	void SpriteParticleRenderer::Render(const PartSysEmitter* emitter) {

		D3DMATRIX worldMatrix;
		if (!GetEmitterWorldMatrix(emitter, worldMatrix)) {
			return;
		}

		renderStates->SetProjectionMatrix(worldMatrix);

		// Apply particle drawing render states
		SetupTextureState(mDevice, emitter);
		SetupBlending(mDevice, emitter);
		EnableParticleRenderStates();

		renderStates->SetFVF(SpriteVertex::FVF);
		renderStates->SetStreamSource(0, mBuffer, sizeof(SpriteVertex));
		renderStates->SetIndexBuffer(mIndices, 0);
		renderStates->Commit();

		RenderParticles(emitter);

		DisableParticleRenderStates();

	}

	void SpriteParticleRenderer::FillSpriteVertex(const PartSysEmitter* emitter, int particleIdx, SpriteVertex* vertex) {

		// Calculate the particle scale (default is 1)
		auto scale = 1.0f;
		auto scaleParam = emitter->GetParamState(part_scale_X);
		if (scaleParam) {
			scale = scaleParam->GetValue(emitter, particleIdx, emitter->GetParticleAge(particleIdx));
		}

		D3DXVECTOR3 halfPartHeightX;
		D3DXVECTOR3 halfPartHeightY;
		auto rotationParam = emitter->GetParamState(part_yaw);
		if (rotationParam) {
			auto rotation = rotationParam->GetValue(emitter, particleIdx, emitter->GetParticleAge(particleIdx));
			rotation += emitter->GetParticleState().GetState(PSF_ROTATION, particleIdx);
			rotation = D3DXToRadian(rotation);

			auto cosRot = cosf(rotation) * scale;
			auto sinRot = sinf(rotation) * scale;
			halfPartHeightX = screenSpaceUnitX * cosRot - screenSpaceUnitY * sinRot;
			halfPartHeightY = screenSpaceUnitY * cosRot + screenSpaceUnitX * sinRot;
		} else {
			halfPartHeightX = screenSpaceUnitX * scale;
			halfPartHeightY = screenSpaceUnitY * scale;
		}

		D3DXVECTOR3 partPos;
		partPos.x = emitter->GetParticleState().GetState(PSF_POS_VAR_X, particleIdx);
		partPos.y = emitter->GetParticleState().GetState(PSF_POS_VAR_Y, particleIdx);
		partPos.z = emitter->GetParticleState().GetState(PSF_POS_VAR_Z, particleIdx);

		// Upper left corner
		vertex[0].pos = partPos - halfPartHeightX - halfPartHeightY;
		// Upper right corner
		vertex[1].pos = partPos + halfPartHeightX - halfPartHeightY;
		// Lower right corner
		vertex[2].pos = partPos + halfPartHeightX + halfPartHeightY;
		// Lower left corner
		vertex[3].pos = partPos - halfPartHeightX + halfPartHeightY;

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

	void SpriteParticleRenderer::RenderParticles(const PartSysEmitter* emitter) {
		SpriteVertex* vertices;
		auto it = emitter->NewIterator();
		auto totalCount = emitter->GetActiveCount();

		for (auto offset = 0; offset < totalCount; offset += MaxBatchSize) {

			auto sizeOfBatch = std::min<int>(MaxBatchSize, totalCount - offset);

			// Lock the vertex buffer for exactly the number of particles we're gonna draw
			// We're using 4 vertices per particle (quads)
			auto result = mBuffer->Lock(0, sizeOfBatch * 4 * sizeof(SpriteVertex), (void**)&vertices, D3DLOCK_DISCARD);
			if (!SUCCEEDED(result)) {
				logger->error("Unable to lock particle vertex buffer: {}", result);
				return;
			}

			while (it.HasNext()) {
				auto particleIdx = it.Next();

				FillSpriteVertex(emitter, particleIdx, vertices);
				vertices += 4;
			}

			result = mBuffer->Unlock();
			if (!SUCCEEDED(result)) {
				logger->error("Unable to unlock particle vertex buffer: {}", result);
				return;
			}

			// Draw the batch
			result = mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4 * sizeOfBatch, 0, 2 * sizeOfBatch);
			if (!SUCCEEDED(result)) {
				logger->error("Unable to draw particles: {}", result);
				return;
			}
		}
	}

}
