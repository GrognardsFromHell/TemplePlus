#pragma once

class MainWindow;
class Graphics;

class GameLoop {
public:

	GameLoop(MainWindow& mainWindow) : mMainWindow(mainWindow) {
	}

	void Run();

private:
	MainWindow& mMainWindow;
};
