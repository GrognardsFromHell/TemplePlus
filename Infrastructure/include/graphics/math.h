
#pragma once

#include <DirectXMath.h>
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using DirectX::XMFLOAT4X4;

#include <DirectXPackedVector.h>
using DirectX::PackedVector::XMCOLOR;

struct ColorRect {
	XMCOLOR topLeft;
	XMCOLOR topRight;
	XMCOLOR bottomLeft;
	XMCOLOR bottomRight;

	ColorRect() {}

	explicit ColorRect(XMCOLOR fill) : topLeft(fill), topRight(fill), bottomLeft(fill), bottomRight(fill) {}
};

struct tagRECT;
typedef struct tagRECT RECT;

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
	bool Intersects(const TigRect &other);
	bool Intersects(const TigRect &other, TigRect &intersection);
};
