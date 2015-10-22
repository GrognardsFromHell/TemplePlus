
#pragma once

class Graphics;

struct RenderWorldInfo;

class MapClipping;
class GameSystems;

class GameRenderer {
public:
	GameRenderer(Graphics &graphics, GameSystems &gameSystems);

	void Render();

private:

	void RenderWorld(RenderWorldInfo *info);
	
	Graphics &mGraphics;
	GameSystems &mGameSystems;
};
