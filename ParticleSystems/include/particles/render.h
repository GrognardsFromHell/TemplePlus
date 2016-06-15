
#pragma once

#include <memory>
#include "graphics/math.h"
#include "infrastructure/macros.h"

namespace gfx {
	class RenderingDevice;
	class AnimatedModelFactory;
	class AnimatedModelRenderer;
}

namespace particles {

	class PartSysEmitter;
	enum class PartSysParticleType;
	class PartSys;

	class ParticleRenderer {
	public:
		virtual void Render(PartSysEmitter &emitter) = 0;
	protected:
		ParticleRenderer(gfx::RenderingDevice &device) : mDevice(device) {
		}
		virtual ~ParticleRenderer() = 0;

		bool GetEmitterWorldMatrix(const PartSysEmitter &emitter, DirectX::XMFLOAT4X4 &matrix);
		
		/*
		These vectors can be multiplied with screen space
		coordinates to get world coordinates.
		*/
		XMFLOAT4 screenSpaceUnitX;
		XMFLOAT4 screenSpaceUnitY;
		XMFLOAT4 screenSpaceUnitZ;

		NO_COPY_OR_MOVE(ParticleRenderer);
	private:
		void ExtractScreenSpaceUnitVectors(const DirectX::XMFLOAT4X4& projWorldMatrix);
		void ExtractScreenSpaceUnitVectors2(const DirectX::XMFLOAT4X4& projWorldMatrix);
		gfx::RenderingDevice &mDevice;
	};

	inline ParticleRenderer::~ParticleRenderer() = default;

	class ParticleRendererManager {
	public:
		explicit ParticleRendererManager(gfx::RenderingDevice &device,
			gfx::AnimatedModelFactory &modelFactory,
			gfx::AnimatedModelRenderer &modelRenderer);
		~ParticleRendererManager();

		ParticleRenderer &GetRenderer(PartSysParticleType type);

	private:
		class Impl;
		std::unique_ptr<Impl> mImpl;
	};
	
}
