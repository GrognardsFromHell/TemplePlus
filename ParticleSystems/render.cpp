
#include <platform/d3d.h>
#include <infrastructure/renderstates.h>

#include "particles/render.h"
#include "particles/render_point.h"
#include "particles/instances.h"

namespace particles {

	void ParticleRenderer::ExtractScreenSpaceUnitVectors(const D3DXMATRIX& projWorldMatrix) {

		// inverse, so screen->world?
		D3DXMATRIX invProjWorld;
		D3DXMatrixTranspose(&invProjWorld, &projWorldMatrix);
		screenSpaceUnitX = *(D3DVECTOR*)&invProjWorld._11;
		screenSpaceUnitY = *(D3DVECTOR *)&invProjWorld._21;
		screenSpaceUnitZ = *(D3DVECTOR *)&invProjWorld._31;
		D3DXVec3Normalize(&screenSpaceUnitX, &screenSpaceUnitX);
		D3DXVec3Normalize(&screenSpaceUnitY, &screenSpaceUnitY);
		D3DXVec3Normalize(&screenSpaceUnitZ, &screenSpaceUnitZ);
	}

	// No idea why this is a separate function
	void ParticleRenderer::ExtractScreenSpaceUnitVectors2(const D3DXMATRIX& projWorldMatrix) {
		screenSpaceUnitX.x = projWorldMatrix._11;
		screenSpaceUnitX.y = projWorldMatrix._12;
		screenSpaceUnitX.z = projWorldMatrix._13;
		screenSpaceUnitY.x = projWorldMatrix._31;
		screenSpaceUnitY.y = projWorldMatrix._32;
		screenSpaceUnitY.z = projWorldMatrix._33;
		screenSpaceUnitZ.x = projWorldMatrix._21;
		screenSpaceUnitZ.y = projWorldMatrix._22;
		screenSpaceUnitZ.z = projWorldMatrix._23;
		D3DXVec3Normalize(&screenSpaceUnitX, &screenSpaceUnitX);
		D3DXVec3Normalize(&screenSpaceUnitY, &screenSpaceUnitY);
		D3DXVec3Normalize(&screenSpaceUnitZ, &screenSpaceUnitZ);
	}

	class ParticleRendererManager::Impl {
	public:
		explicit Impl(IDirect3DDevice9* device) : mPointRenderer(device), mSpriteRenderer(device), mDiscRenderer(device) {
		}

		PointParticleRenderer mPointRenderer;
		SpriteParticleRenderer mSpriteRenderer;
		DiscParticleRenderer mDiscRenderer;
	};

	bool ParticleRenderer::GetEmitterWorldMatrix(const PartSysEmitter* emitter, D3DMATRIX& worldMatrix) {
		D3DXMATRIX pM1;

		auto spec = emitter->GetSpec();
		auto particleSpace = spec->GetParticleSpace();
		auto emitterSpace = spec->GetSpace();
		if (particleSpace == PartSysParticleSpace::SameAsEmitter) {

			if (emitterSpace == PartSysEmitterSpace::ObjectPos || emitterSpace == PartSysEmitterSpace::ObjectYpr) {
				if (emitterSpace == PartSysEmitterSpace::ObjectYpr) {
					auto v6 = emitter->GetObjRotation() + 3.1415927f;
					D3DXMatrixRotationY(&pM1, v6);
				} else {
					D3DXMatrixIdentity(&pM1);
				}

				// SEt the translation component of the transformation matrix
				pM1._41 = emitter->GetObjPos().x;
				pM1._42 = emitter->GetObjPos().y;
				pM1._43 = emitter->GetObjPos().z;

				D3DXMatrixMultiply((D3DXMATRIX*)&worldMatrix, &pM1, (const D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());
				ExtractScreenSpaceUnitVectors(worldMatrix);
				return true;
			}
			if (emitterSpace == PartSysEmitterSpace::NodePos || emitterSpace == PartSysEmitterSpace::NodeYpr) {
				auto external = IPartSysExternal::GetCurrent();

				if (emitterSpace == PartSysEmitterSpace::NodeYpr) {
					D3DXMATRIX boneMatrix;
					if (!external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&)boneMatrix)) {
						// This effectively acts as a fallback if the bone doesn't exist
						auto x = emitter->GetObjPos().x;
						auto y = emitter->GetObjPos().y;
						auto z = emitter->GetObjPos().z;
						D3DXMatrixTranslation(&boneMatrix, x, y, z);
					}

					D3DXMatrixMultiply((D3DXMATRIX*) &worldMatrix, &pM1, (const D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());
					ExtractScreenSpaceUnitVectors(worldMatrix);
					return true;
				}

				D3DXMATRIX boneMatrix;
				if (external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&)boneMatrix)) {
					auto x = boneMatrix._41;
					auto y = boneMatrix._42;
					auto z = boneMatrix._43;
					D3DXMatrixTranslation(&pM1, x, y, z); // TODO: This might not be needed...

					D3DXMatrixMultiply((D3DXMATRIX*)&worldMatrix,
					                   &pM1,
					                   (const D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());

					ExtractScreenSpaceUnitVectors(worldMatrix);
					return true;
				}

				return false;
			}

			worldMatrix = renderStates->Get3dProjectionMatrix();
			ExtractScreenSpaceUnitVectors(worldMatrix);
			return true;
		}

		if (particleSpace == PartSysParticleSpace::World) {
			worldMatrix = renderStates->Get3dProjectionMatrix();
			ExtractScreenSpaceUnitVectors(worldMatrix);
			return true;
		}

		if (emitterSpace != PartSysEmitterSpace::ObjectPos && emitterSpace != PartSysEmitterSpace::ObjectYpr) {
			if (emitterSpace != PartSysEmitterSpace::NodePos && emitterSpace != PartSysEmitterSpace::NodeYpr)
				return true;

			auto external = IPartSysExternal::GetCurrent();
			D3DXMatrixIdentity(&pM1);

			if (emitterSpace == PartSysEmitterSpace::NodeYpr) {
				// Use the entire bone matrix if possible
				external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&)pM1);
			} else {
				// Only use the bone translation part
				D3DXMATRIX boneMatrix;
				if (!external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&) boneMatrix))
					return false;
				pM1._41 = boneMatrix._41;
				pM1._42 = boneMatrix._42;
				pM1._43 = boneMatrix._43;
			}
			worldMatrix = renderStates->Get3dProjectionMatrix();
			ExtractScreenSpaceUnitVectors2(pM1);
			return 1;
		}
		if (emitterSpace == PartSysEmitterSpace::ObjectYpr) {
			auto v6 = emitter->GetObjRotation() + 3.1415927f;
			D3DXMatrixRotationY(&pM1, v6);
		} else {
			D3DXMatrixIdentity(&pM1);
		}
		worldMatrix = renderStates->Get3dProjectionMatrix();
		ExtractScreenSpaceUnitVectors2(pM1);
		return true;
	}

	ParticleRendererManager::ParticleRendererManager(IDirect3DDevice9* device) : mImpl(new Impl(device)) {
	}

	ParticleRendererManager::~ParticleRendererManager() = default;

	ParticleRenderer* ParticleRendererManager::GetRenderer(PartSysParticleType type) {
		switch (type) {
		case PartSysParticleType::Point:
			return &mImpl->mPointRenderer;
		case PartSysParticleType::Sprite:
			return &mImpl->mSpriteRenderer;
		case PartSysParticleType::Disc:
			return &mImpl->mDiscRenderer;
		default:
			throw TempleException("Cannot render particle type");
		}
	}
}
