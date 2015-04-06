
#pragma once

struct TigRect {
	int x;
	int y;
	int width;
	int height;

	TigRect() {}
	TigRect(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->width = w;
		this->height = h;
	}
};
