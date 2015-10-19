

#include "stdafx.h"
#include <temple/dll.h>
#include "tig/tig.h"
#include "graphics/graphics.h"
#include "temple_functions.h"
#include "gamerenderer.h"

#pragma pack(push, 1)

struct TileRect {
	int64_t x1;
	int64_t y1;
	int64_t x2;
	int64_t y2;
};

struct SectorList {
	locationSec sector;
	locXY someTile; // tile coords
	locXY someTileInSec; // coords in sector
	SectorList *next;
	// There is 4 bytes padding here but we dont rely on the size here
};

struct RenderUnknown {
	int v[114];
};

struct TigRectList {
	TigRect rect;
	TigRectList *next;
};

struct RenderWorldInfo {
	const TigRect *viewportSize;
	const TileRect *tiles;
	RenderUnknown *unknown;
	const SectorList *sectors;
	TigRectList **rectList;
};
#pragma pack(pop)

static struct GameRenderFuncs : temple::AddressTable {

	/*
	Given a rectangle in screen coordinates, calculates the rectangle in tile-space that
	is visible.
	*/
	bool(__cdecl *GetVisibleTileRect)(const TigRect &screenRect, TileRect &tiles);

	/*
	These two functions build a linked list of all the sectors encompassed by the given tile
	rectangle. The list is using shared global memory from a pool and it should be returned
	to the pool using the second function.
	*/
	SectorList* (__cdecl *SectorListBuild)(const TileRect &tiles);
	void(__cdecl *SectorListFree)(SectorList *sectorList);

	/*
	Builds an unknown info blob in the second argument.
	*/
	bool(__cdecl *BuildUnk)(const TileRect &tiles, RenderUnknown &unk);

	/*
	Seems to be used to propagate the screen rectangle to the scratchbuffer.
	*/
	void(__cdecl *ScratchbufferRelated)(const TigRect &rect);

	int *gameDrawEnableCount;
	int *unkFlag2;

	// Seems to be manipulated by ScratchbufferRelated
	TigRectList **globalRectList;

	void (*RenderGround)(RenderWorldInfo *info);
	void (*RenderMapObj)(RenderWorldInfo *info);
	void (*PerformFogChecks)();
	void (*RenderClipping)();
	void (*RenderGMesh)();
	void (*RenderPfxLighting)();
	void (*RenderPartSys)();
	void (*RenderFogOfWar)();
	void (*RenderOcclusion)(RenderWorldInfo *info);
	void(*RenderUiRelated)(RenderWorldInfo *info);
	void(*RenderTextBubbles)(RenderWorldInfo *info);
	void(*RenderTextFloaters)(RenderWorldInfo *info);

	GameRenderFuncs() {
		rebase(GetVisibleTileRect, 0x1002A6B0);
		rebase(SectorListBuild, 0x10084650);
		rebase(SectorListFree, 0x10081A30);
		rebase(BuildUnk, 0x100824D0);

		rebase(ScratchbufferRelated, 0x10002530);

		rebase(gameDrawEnableCount, 0x102ABED8);
		rebase(unkFlag2, 0x1030728C);
		rebase(globalRectList, 0x10306C0C);

		rebase(RenderGround, 0x1002DC70);
		rebase(PerformFogChecks, 0x100336B0);
		rebase(RenderClipping, 0x100A4FB0);
		rebase(RenderMapObj, 0x10028AC0);
		rebase(RenderGMesh, 0x100A3AE0);
		rebase(RenderPfxLighting, 0x10087E60);
		rebase(RenderPartSys, 0x101E7B80);
		rebase(RenderFogOfWar, 0x10030830);
		rebase(RenderOcclusion, 0x10024750);

		rebase(RenderUiRelated, 0x1014E190);
		rebase(RenderTextBubbles, 0x100A3210);
		rebase(RenderTextFloaters, 0x100A2600);
	}

} renderFuncs;

GameRenderer::GameRenderer(Graphics &graphics) : mGraphics(graphics) {
}

GameRenderer::~GameRenderer() {
}

void GameRenderer::Render() {

	/*
	Without this call, the ground JPGs will not be rendered.
	*/
	// TigRect rect(0, 0, graphics->GetSceneWidth(), graphics->GetSceneHeight());
	// renderFuncs.ScratchbufferRelated(rect);

	if (renderFuncs.gameDrawEnableCount <= 0) {
		return;
	}

	TigRect viewportSize;
	viewportSize.y = -256;
	viewportSize.width = mGraphics.GetSceneWidth() + 512;
	viewportSize.x = -256;
	viewportSize.height = mGraphics.GetSceneHeight() + 512;

	TileRect tiles;

	if (renderFuncs.GetVisibleTileRect(viewportSize, tiles))
	{
		RenderUnknown unk;
		renderFuncs.BuildUnk(tiles, unk);

		auto sectorList = renderFuncs.SectorListBuild(tiles);

		RenderWorldInfo renderInfo;
		renderInfo.sectors = sectorList;
		renderInfo.unknown = &unk;
		renderInfo.tiles = &tiles;
		renderInfo.viewportSize = &viewportSize;

		// I think this maybe a 2D arcanum leftover when the map could be incrementally drawn based on "dirty rects"
		TigRectList dirtyList;
		dirtyList.rect = TigRect(0, 0, mGraphics.GetSceneWidth(), mGraphics.GetSceneHeight());
		dirtyList.next = nullptr;
		TigRectList *dirtyRectPtr = &dirtyList;
		renderInfo.rectList = &dirtyRectPtr;

		RenderWorld(&renderInfo);

		renderFuncs.SectorListFree(sectorList);
	}

}

void GameRenderer::RenderWorld(RenderWorldInfo* info) {

	if (mGraphics.BeginFrame()) {
		renderFuncs.RenderGround(info);
		renderFuncs.PerformFogChecks();
		renderFuncs.RenderClipping();

		graphics->EnableLighting();
		renderFuncs.RenderMapObj(info);
		renderFuncs.RenderGMesh();
		renderFuncs.RenderPfxLighting();
		graphics->DisableLighting();
		
		renderFuncs.RenderPartSys();
		renderFuncs.RenderFogOfWar();

		graphics->EnableLighting();
		renderFuncs.RenderOcclusion(info);
		graphics->DisableLighting();

		renderFuncs.RenderUiRelated(info);
		renderFuncs.RenderTextBubbles(info);
		renderFuncs.RenderTextFloaters(info);

		graphics->Present();
	}

}
