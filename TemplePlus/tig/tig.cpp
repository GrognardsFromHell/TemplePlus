#include "stdafx.h"
#include "tig.h"
#include "tig_tabparser.h"

TigTabParserFuncs tigTabParserFuncs;

TigTabParserFuncs::TigTabParserFuncs() {
	rebase(Init, 0x101F2C10);
	rebase(Open, 0x101F2E40);
	rebase(GetLineCount, 0x101F2D40);
	rebase(Process, 0x101F2C70);
	rebase(Close, 0x101F2C30);
}

void TigRect::FitInto(const TigRect& boundingRect) {

	/*
	Calculates the rectangle within the back buffer that the scene
	will be drawn in. This accounts for "fit to width/height" scenarios
	where the back buffer has a different aspect ratio.
	*/
	float w = static_cast<float>(boundingRect.width);
	float h = static_cast<float>(boundingRect.height);
	float wFactor = (float)w / width;
	float hFactor = (float)h / height;
	float scale = min(wFactor, hFactor);
	width = (int)round(scale * width);
	height = (int)round(scale * height);

	// Center in bounding Rect
	x = boundingRect.x + (boundingRect.width - width) / 2;
	y = boundingRect.y + (boundingRect.height - height) / 2;
}

bool TigRect::Intersects(const TigRect& other) {

	// Basically we select any component that leads to a 
	// smaller surface area of the intersection and then
	// discard if the surface area would become negative or zero
	auto intersectionX = std::max(x, other.x);
	auto intersectionY = std::max(y, other.y);

	auto right = x + width;
	auto otherRight = other.x + other.width;
	auto intersectionWidth = std::min(right, otherRight) - intersectionX;
	if (intersectionWidth <= 0) {
		return false;
	}

	auto bottom = y + height;
	auto otherBottom = other.y + other.height;
	auto intersectionHeight = std::min(bottom, otherBottom) - intersectionY;

	return intersectionHeight > 0;

}

bool TigRect::Intersects(const TigRect& other, TigRect& intersection) {
	// Basically we select any component that leads to a 
	// smaller surface area of the intersection and then
	// discard if the surface area would become negative or zero
	auto intersectionX = std::max(x, other.x);
	auto intersectionY = std::max(y, other.y);

	auto right = x + width;
	auto otherRight = other.x + other.width;
	auto intersectionWidth = std::min(right, otherRight) - intersectionX;
	if (intersectionWidth <= 0) {
		return false;
	}

	auto bottom = y + height;
	auto otherBottom = other.y + other.height;
	auto intersectionHeight = std::min(bottom, otherBottom) - intersectionY;
	if (intersectionHeight <= 0) {
		return false;
	}

	intersection.x = intersectionX;
	intersection.y = intersectionY;
	intersection.width = intersectionWidth;
	intersection.height = intersectionHeight;
	return true;	
}

RECT TigRect::ToRect() {
	return{x, y, x + width, y + height};
}
