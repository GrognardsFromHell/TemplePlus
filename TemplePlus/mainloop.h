#pragma once

#include "gamesystems/gamerenderer.h"

class Updater;
class TigInitializer;
class GameSystems;

namespace gfx {
	using RenderTargetTexturePtr = std::shared_ptr<class RenderTargetTexture>;
	using RenderTargetDepthStencilPtr = std::shared_ptr<class RenderTargetDepthStencil>;
}

class GameLoop {
friend class MainLoopHooks;
public:

	GameLoop(TigInitializer& tig,
	         GameSystems& gameSystems,
			 Updater &updater);
	~GameLoop();

	void Run();

private:

	void DoMouseScrolling();
	void SetScrollDirection(int direction);
	void RenderVersion();
	void RenderFrame();
	
	TigInitializer& mTig;
	GameSystems& mGameSystems;
	Updater& mUpdater;
	gfx::RenderTargetTexturePtr mSceneColor;
	gfx::RenderTargetDepthStencilPtr mSceneDepth;
	GameRenderer mGameRenderer;
	std::unique_ptr<class DiagScreen> mDiagScreen;
};
