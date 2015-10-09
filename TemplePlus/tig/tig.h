
#pragma once

struct ColorRect {
	D3DCOLOR topLeft;
	D3DCOLOR topRight;
	D3DCOLOR bottomLeft;
	D3DCOLOR bottomRight;

	ColorRect() {}

	explicit ColorRect(D3DCOLOR fill) : topLeft(fill), topRight(fill), bottomLeft(fill), bottomRight(fill) {}
};

struct TigRect {
	int x;
	int y;
	int width;
	int height;

	TigRect() : x(0), y(0), width(0), height(0) {}
	TigRect(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->width = w;
		this->height = h;
	}

	RECT ToRect();
	void FitInto(const TigRect &boundingRect);
};
