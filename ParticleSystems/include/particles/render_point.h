#pragma once

#include "render.h"

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

	class PointParticleRenderer : public ParticleRenderer {
	public:
		explicit PointParticleRenderer(RenderingDevice& device);

		void Render(PartSysEmitter& emitter) override;

	private:
		void FillPointVertex(const PartSysEmitter& emitter, int particleIdx, PointVertex& vertex);
		void RenderParticles(PartSysEmitter& emitter);

		void EnablePointStates();
		void DisablePointStates();

		RenderingDevice& mDevice;
	};

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
			gsl::span<SpriteVertex, 4> vertices) = 0;

	};

	class SpriteParticleRenderer : public QuadParticleRenderer {
	public:
		explicit SpriteParticleRenderer(RenderingDevice& device)
			: QuadParticleRenderer(device) {
		}

	protected:

		void FillVertex(const PartSysEmitter& emitter,
			int particleIdx,
			gsl::span<SpriteVertex, 4> vertices) override;
	};

	class DiscParticleRenderer : public QuadParticleRenderer {
	public:
		explicit DiscParticleRenderer(RenderingDevice& device)
			: QuadParticleRenderer(device) {
		}

	protected:

		void FillVertex(const PartSysEmitter& emitter,
			int particleIdx,
			gsl::span<SpriteVertex, 4> vertices) override;

	};

}
