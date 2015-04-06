
#pragma once

struct ColorRect {
	D3DCOLOR topLeft;
	D3DCOLOR topRight;
	D3DCOLOR bottomLeft;
	D3DCOLOR bottomRight;
};

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
