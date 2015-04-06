
#include "stdafx.h"
#include "ui_render.h"
#include "addresses.h"

#pragma pack(push, 1)
struct DrawTexturedQuadArgs {
	int flags = 0;
	int textureId;
	int field8;
	void *texBuffer;
	int shaderId;
	const TigRect *srcRect;
	const TigRect *destRect;	
	ColorRect *vertexColors;
	float vertexZ;
	int field24; // May be padding from here on out
	int field28;
	int field2c;
};
#pragma pack(pop)

static struct UiRenderFuncs : AddressTable {
	
	/*
		This is one of the primary functions used to draw textured quads for UI purposes.
		It's a pretty flexible function. Not all arguments have been discovered yet.
	*/
	int (__cdecl *DrawTexturedQuad)(const DrawTexturedQuadArgs &);

	UiRenderFuncs() {
		rebase(DrawTexturedQuad, 0x101D9300);
	}
} uiRenderFuncs;

void UiRenderer::DrawTexture(int texId, const TigRect &destRect) {

	DrawTexturedQuadArgs args;
	args.destRect = &destRect;

	// This function assumes dest rect encompasses the entire src rect at 0,0
	TigRect srcRect(0, 0, destRect.width, destRect.height);
	args.srcRect = &srcRect;

	args.textureId = texId;

	if (uiRenderFuncs.DrawTexturedQuad(args)) {
		logger->warn("DrawTexturedQuad failed!");
	}

}
