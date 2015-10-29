
#pragma once
#include <tig/tig.h>

class Graphics;

/*
	Draws a rectangle on screen.
*/
class UiRectangle {
public:
	UiRectangle();
	~UiRectangle();

	void SetX(int x);
	void SetY(int y);
	int GetX() const {
		return mRect.x;
	}
	int GetY() const {
		return mRect.y;
	}
	void SetWidth(int width);
	void SetHeight(int height);
	void SetColor(uint32_t color);

	void Render();

private:
	std::unique_ptr<struct Render2dArgs> mArgs;
	std::array<uint32_t, 4> mColors;
	TigRect mRect;
};
