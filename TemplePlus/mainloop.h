#pragma once

#include "gamesystems/gamerenderer.h"

class MainWindow;
class Graphics;
class GameSystems;

class GameLoop {
public:

	GameLoop(MainWindow& mainWindow,
	         GameSystems& gameSystems,
	         Graphics& graphics);
	~GameLoop();

	void Run();

private:

	void DoMouseScrolling();
	void SetScrollDirection(int direction);
	void RenderVersion();
	void RenderFrame();

	MainWindow& mMainWindow;
	GameSystems& mGameSystems;
	GameRenderer mGameRenderer;
	std::unique_ptr<class DiagScreen> mDiagScreen;
};
