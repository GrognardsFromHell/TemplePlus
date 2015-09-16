
#pragma once

#include <memory>
#include <d3dx9math.h>

struct IDirect3DDevice9;
typedef struct _D3DMATRIX D3DMATRIX;

namespace particles {

	class PartSysEmitter;
	enum class PartSysParticleType;
	class PartSys;

	class ParticleRenderer {
	public:
		virtual void Render(const PartSysEmitter *emitter) = 0;
	protected:
		virtual ~ParticleRenderer() = 0;

		bool GetEmitterWorldMatrix(const PartSysEmitter *emitter, D3DMATRIX &matrix);
		
		/*
		These vectors can be multiplied with screen space
		coordinates to get world coordinates.
		*/
		D3DXVECTOR3 screenSpaceUnitX;
		D3DXVECTOR3 screenSpaceUnitY;
		D3DXVECTOR3 screenSpaceUnitZ;
	private:
		void ExtractScreenSpaceUnitVectors(const D3DXMATRIX& projWorldMatrix);
		void ExtractScreenSpaceUnitVectors2(const D3DXMATRIX& projWorldMatrix);
	};

	inline ParticleRenderer::~ParticleRenderer() = default;

	class ParticleRendererManager {
	public:
		explicit ParticleRendererManager(IDirect3DDevice9 *device);
		~ParticleRendererManager();

		ParticleRenderer *GetRenderer(PartSysParticleType type);

	private:
		class Impl;
		std::unique_ptr<Impl> mImpl;
	};
	
}
