#pragma once

#include "render.h"

#include <span>

#include <platform/d3d.h>
#include <atlcomcli.h>

#include <graphics/device.h>

namespace gfx {
	class AnimatedModelFactory;
	class AnimatedModelRenderer;
}
namespace temple {
	class AasRenderer;
}

namespace particles {
	struct PointVertex;
	struct SpriteVertex;

	using namespace gfx;

	class ModelParticleRenderer : public ParticleRenderer {
	public:
		explicit ModelParticleRenderer(RenderingDevice& device,
			AnimatedModelFactory &aasFactory,
			AnimatedModelRenderer &aasRenderer);

		void Render(PartSysEmitter& emitter) override;

	private:
		RenderingDevice& mDevice;
		AnimatedModelFactory& mModelFactory;
		AnimatedModelRenderer& mModelRenderer;

		std::string ResolveBasename(const std::string &modelName);
	};

	class QuadParticleRenderer : public ParticleRenderer {
	public:
		explicit QuadParticleRenderer(RenderingDevice& device);

		void Render(PartSysEmitter& emitter) override;

	protected:
		IndexBufferPtr mIndexBuffer;
		RenderingDevice &mDevice;

	private:
		void RenderParticles(PartSysEmitter& emitter);

		virtual void FillVertex(const PartSysEmitter& emitter, 
			int particleIdx, 
			std::span<SpriteVertex, 4> vertices) = 0;

	};

	class SpriteParticleRenderer : public QuadParticleRenderer {
	public:
		explicit SpriteParticleRenderer(RenderingDevice& device)
			: QuadParticleRenderer(device) {
		}

	protected:

		void FillVertex(const PartSysEmitter& emitter,
			int particleIdx,
			std::span<SpriteVertex, 4> vertices) override;
	};

	class DiscParticleRenderer : public QuadParticleRenderer {
	public:
		explicit DiscParticleRenderer(RenderingDevice& device)
			: QuadParticleRenderer(device) {
		}

	protected:

		void FillVertex(const PartSysEmitter& emitter,
			int particleIdx,
			std::span<SpriteVertex, 4> vertices) override;

	};

}
