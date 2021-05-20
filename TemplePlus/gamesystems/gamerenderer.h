
#pragma once

class TigInitializer;
namespace gfx {
	class RenderingDevice;
	class WorldCamera;
}
namespace aas {
	class Renderer;
}
namespace particles {
	class ParticleRendererManager;
}

struct RenderWorldInfo;

class MapClipping;
class GameSystems;
class ParticleSystemsRenderer;
class MapObjectRenderer;
class GMeshRenderer;
class LightningRenderer;
class FogOfWarRenderer;
class IntgameRenderer;
class TileRenderer;

class GameRenderer {
public:
	GameRenderer(TigInitializer &tig, gfx::WorldCamera &camera, GameSystems &gameSystems);
	~GameRenderer();

	void Render();

	ParticleSystemsRenderer& GetParticleSysRenderer() const {
		return *mParticleSysRenderer;
	}

	MapObjectRenderer& GetMapObjectRenderer() const {
		return *mMapObjectRenderer;
	}

private:

	void RenderWorld(RenderWorldInfo *info);
	
	gfx::RenderingDevice& mRenderingDevice;
	gfx::WorldCamera& mCamera;
	GameSystems &mGameSystems;

	std::unique_ptr<aas::Renderer> mAasRenderer;
	std::unique_ptr<MapObjectRenderer> mMapObjectRenderer;
	std::unique_ptr<ParticleSystemsRenderer> mParticleSysRenderer;
	std::unique_ptr<GMeshRenderer> mGmeshRenderer;
	std::unique_ptr<LightningRenderer> mLightningRenderer;
	std::unique_ptr<FogOfWarRenderer> mFogOfWarRenderer;
	std::unique_ptr<IntgameRenderer> mIntgameRenderer;
	std::unique_ptr <TileRenderer> mTileRenderer;
};

extern GameRenderer *gameRenderer;
