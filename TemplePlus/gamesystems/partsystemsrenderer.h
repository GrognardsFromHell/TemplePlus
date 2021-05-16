
#pragma once

namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
	class AnimatedModelFactory;
	class AnimatedModelRenderer;
	class WorldCamera;
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
		gfx::WorldCamera &mCamera,
		gfx::ShapeRenderer2d &mShapeRenderer2d,
		gfx::AnimatedModelFactory &modelFactory,
		gfx::AnimatedModelRenderer &modelRenderer,
		ParticleSysSystem &mParticleSysSystem
	);

	void Render();

	size_t GetRenderedLastFrame() const {
		return mRenderedLastFrame;
	}
	size_t GetTotalLastFrame() const {
		return mTotalLastFrame;
	}
	size_t GetRenderTimeAvg() const {
		size_t sum = 0;
		for (auto renderTime : mRenderTimes) {
			sum += renderTime;
		}
		return sum / mRenderTimes.size();
	}

private:
	gfx::RenderingDevice &mRenderingDevice;
	gfx::WorldCamera &mCamera;
	gfx::ShapeRenderer2d &mShapeRenderer2d;
	ParticleSysSystem &mParticleSysSystem;

	void RenderDebugInfo(const particles::PartSys&);

	size_t mRenderedLastFrame = 0;
	size_t mTotalLastFrame = 0;
	std::array<size_t, 100> mRenderTimes;
	size_t mRenderTimesPos = 0;

	std::unique_ptr<particles::ParticleRendererManager> mRendererManager;
};
