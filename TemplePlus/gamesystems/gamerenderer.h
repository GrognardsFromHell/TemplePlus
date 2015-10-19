
#pragma once

class Graphics;

struct RenderWorldInfo;

class GameRenderer {
public:
	GameRenderer(Graphics &graphics);
	~GameRenderer();

	void Render();

private:

	void RenderWorld(RenderWorldInfo *info);

	Graphics &mGraphics;
};

