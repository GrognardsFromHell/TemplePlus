
#pragma once

namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
	class AnimatedModelFactory;
	class AnimatedModelRenderer;
}
namespace particles {
	class ParticleRendererManager;
	class PartSys;
}

class ParticleSysSystem;

class ParticleSystemsRenderer {
public:
	ParticleSystemsRenderer(
		gfx::RenderingDevice &mRenderingDevice,
		gfx::ShapeRenderer2d &mShapeRenderer2d,
		gfx::AnimatedModelFactory &modelFactory,
		gfx::AnimatedModelRenderer &modelRenderer,
		ParticleSysSystem &mParticleSysSystem
	);

	void Render();

private:
	gfx::RenderingDevice &mRenderingDevice;
	gfx::ShapeRenderer2d &mShapeRenderer2d;
	ParticleSysSystem &mParticleSysSystem;

	void RenderDebugInfo(const particles::PartSys&);

	std::unique_ptr<particles::ParticleRendererManager> mRendererManager;
};
