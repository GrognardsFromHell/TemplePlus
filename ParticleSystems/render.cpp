
#include <d3d9.h>
#include <renderstates.h>

#include "particles/render.h"
#include "particles/render_point.h"
#include "particles/instances.h"

#include <d3dx9math.h>

namespace particles {

	class ParticleRendererManager::Impl {
	public:
		explicit Impl(IDirect3DDevice9* device) : mPointRenderer(device) {
		}

		PointParticleRenderer mPointRenderer;
	};

	bool GetEmitterWorldMatrix(const PartSysEmitter *emitter, D3DMATRIX &worldMatrix) {
		D3DXMATRIX pM1; // [sp+Ch] [bp-C0h]@6

		auto spec = emitter->GetSpec();
		auto particleSpace = spec->GetParticleSpace();
		auto emitterSpace = spec->GetSpace();
		if (particleSpace == PartSysParticleSpace::SameAsEmitter)
		{
			
			if (emitterSpace == PartSysEmitterSpace::ObjectPos || emitterSpace == PartSysEmitterSpace::ObjectYpr)
			{
				if (emitterSpace == PartSysEmitterSpace::ObjectYpr)
				{
					auto v6 = emitter->GetObjRotation() + 3.1415927f;
					D3DXMatrixRotationY(&pM1, v6);
				}
				else
				{
					D3DXMatrixIdentity(&pM1);
				}

				// SEt the translation component of the transformation matrix
				pM1._41 = emitter->GetObjPos().x;
				pM1._42 = emitter->GetObjPos().y;
				pM1._43 = emitter->GetObjPos().z;

				D3DXMatrixMultiply((D3DXMATRIX*)&worldMatrix, &pM1, (const D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());

				// TODO Proj matrix is used to calculate sth for sprites sub_102019D0(angle);
				return true;
			}
			if (emitterSpace == PartSysEmitterSpace::NodePos || emitterSpace == PartSysEmitterSpace::NodeYpr)
			{
				auto external = IPartSysExternal::GetCurrent();
							
				if (emitterSpace == PartSysEmitterSpace::NodeYpr)
				{
					D3DXMATRIX boneMatrix;
					if (!external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&)boneMatrix)) {
						// This effectively acts as a fallback if the bone doesn't exist
						auto x = emitter->GetObjPos().x;
						auto y = emitter->GetObjPos().y;
						auto z = emitter->GetObjPos().z;
						D3DXMatrixTranslation(&boneMatrix, x, y, z);
					}

					D3DXMatrixMultiply((D3DXMATRIX*) &worldMatrix, &pM1, (const D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());
					// TODO: Set stuff for sprites: sub_102019D0(&pOut);
					return true;
				}

				D3DXMATRIX boneMatrix;
				if (external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&)boneMatrix))
				{
					auto x = boneMatrix._41;
					auto y = boneMatrix._42;
					auto z = boneMatrix._43;
					D3DXMatrixTranslation(&pM1, x, y, z); // TODO: This might not be needed...

					D3DXMatrixMultiply((D3DXMATRIX*)&worldMatrix, 
						&pM1, 
						(const D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());
					
					// TODO: Set stuff for sprites: sub_102019D0(&pOut);
					return true;
				}

				return false;
			}

			worldMatrix = renderStates->Get3dProjectionMatrix();
			// TODO: Set stuff for sprites sub_102019D0(angle);
			return true;
		}

		if (particleSpace == PartSysParticleSpace::World) {
			worldMatrix = renderStates->Get3dProjectionMatrix();
			// TODO: Set stuff for sprites sub_102019D0(angle);
			return true;
		}

		if (emitterSpace != PartSysEmitterSpace::ObjectPos && emitterSpace != PartSysEmitterSpace::ObjectYpr)
		{
			if (emitterSpace != PartSysEmitterSpace::NodePos && emitterSpace != PartSysEmitterSpace::NodeYpr)
				return 1;
						
			auto external = IPartSysExternal::GetCurrent();
			D3DXMatrixIdentity(&pM1);

			if (emitterSpace == PartSysEmitterSpace::NodeYpr)
			{
				// Use the entire bone matrix if possible
				external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&)pM1);
			}
			else
			{
				// Only use the bone translation part
				D3DXMATRIX boneMatrix;
				if (!external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), (Matrix4x4&) boneMatrix))
					return false;
				pM1._41 = boneMatrix._41;
				pM1._42 = boneMatrix._42;
				pM1._43 = boneMatrix._43;
			}
			worldMatrix = pM1;
			// TODO set stuff for sprites sub_10201A70(&pM1);
			return 1;
		}
		if (emitterSpace == PartSysEmitterSpace::ObjectYpr)
		{
			auto v6 = emitter->GetObjRotation() + 3.1415927f;
			D3DXMatrixRotationY(&pM1, v6);
		}
		else
		{
			D3DXMatrixIdentity(&pM1);
		}
		worldMatrix = pM1;
		// TODO: Set Stuff for sprites	sub_10201A70(&pM1);
		return true;
	}

	ParticleRendererManager::ParticleRendererManager(IDirect3DDevice9* device) : mImpl(new Impl(device)) {
	}

	ParticleRendererManager::~ParticleRendererManager() = default;

	ParticleRenderer* ParticleRendererManager::GetRenderer(PartSysParticleType type) {
		switch (type) {
		case PartSysParticleType::Point: 
			return &mImpl->mPointRenderer;
		default: 
			throw new TempleException("Cannot render particle type");
		}
	}
}
