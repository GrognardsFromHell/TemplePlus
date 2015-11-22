
#pragma once

class TigInitializer;
namespace gfx {
	class RenderingDevice;
}
namespace temple {
	struct AasConfig;
	class AasAnimatedModelFactory;
	class AasRenderer;
}
namespace particles {
	class ParticleRendererManager;
}

struct RenderWorldInfo;

class MapClipping;
class GameSystems;

class GameRenderer {
public:
	GameRenderer(TigInitializer &tig, GameSystems &gameSystems);
	~GameRenderer();

	void Render();

private:

	void RenderWorld(RenderWorldInfo *info);
	
	gfx::RenderingDevice& mRenderingDevice;
	GameSystems &mGameSystems;

	std::unique_ptr<class temple::AasRenderer> mAasRenderer;
	std::unique_ptr<class MapObjectRenderer> mMapObjectRenderer;
	std::unique_ptr<class ParticleSystemsRenderer> mParticleSysRenderer;
};
