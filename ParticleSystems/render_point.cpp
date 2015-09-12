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

	static void FillPointVertex(const PartSysEmitter* emitter, int particleIdx, PointVertex* vertex) {

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

	static bool wasLightingEnabled;

	static void EnableParticleRenderStates() {
		renderStates->SetZEnable(false); // FIXME: set back to true
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

	static void SetupTextureState(IDirect3DDevice9* device, const PartSysEmitter* emitter) {

		auto material = emitter->GetSpec()->GetMaterial();

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
		auto sx = contentRect.width / (float) texSize.width;
		auto sy = contentRect.height / (float) texSize.height;
		D3DXMatrixScaling(&transform, sx, sy, 1.0);
		
		// Translation is only needed in very rare cases (texture atlases)
		if (contentRect.x || contentRect.y) {
			auto tx = contentRect.x / (float) texSize.width;
			auto ty = contentRect.y / (float) texSize.height;			
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

}
