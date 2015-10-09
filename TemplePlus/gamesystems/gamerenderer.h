
#pragma once

class Graphics;

class GameRenderer {
public:
	GameRenderer(Graphics &graphics);
	~GameRenderer();

	void Render();

private:
	Graphics &mGraphics;
};

