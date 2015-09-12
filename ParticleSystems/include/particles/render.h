
#pragma once

#include <memory>

class PartSysEmitter;
enum class PartSysParticleType;
class PartSys;
struct IDirect3DDevice9;
typedef struct _D3DMATRIX D3DMATRIX;

namespace particles {

	bool GetEmitterWorldMatrix(const PartSysEmitter *emitter, D3DMATRIX &matrix);

	class ParticleRenderer {
	public:
		virtual void Render(const PartSysEmitter *emitter) = 0;
	protected:
		virtual ~ParticleRenderer() = 0;		
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
