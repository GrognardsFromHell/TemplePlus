#pragma once

#include "gamesystems/gamerenderer.h"

class Updater;
class TigInitializer;
class GameSystems;

class GameLoop {
friend class MainLoopHooks;
public:

	GameLoop(TigInitializer& mainWindow,
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
	GameRenderer mGameRenderer;
	std::unique_ptr<class DiagScreen> mDiagScreen;
};
