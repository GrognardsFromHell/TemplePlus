#pragma once

#include "gamesystems/gamerenderer.h"

class TigInitializer;
class GameSystems;

class GameLoop {
public:

	GameLoop(TigInitializer& mainWindow,
	         GameSystems& gameSystems);
	~GameLoop();

	void Run();

private:

	void DoMouseScrolling();
	void RenderVersion();
	void RenderFrame();

	TigInitializer& mTig;
	GameSystems& mGameSystems;
	GameRenderer mGameRenderer;
	std::unique_ptr<class DiagScreen> mDiagScreen;
};
