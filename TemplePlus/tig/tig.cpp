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

RECT TigRect::ToRect() {
	return{x, y, x + width, y + height};
}
