
#include "stdafx.h"
#include "util/addresses.h"
#include "tig/tig.h"
#include "graphics.h"

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

static struct GameRenderFuncs : AddressTable {

	/*
		Given a rectangle in screen coordinates, calculates the rectangle in tile-space that
		is visible.
	*/
	bool (__cdecl *GetVisibleTileRect)(const TigRect &screenRect, TileRect &tiles);
	
	/*
		These two functions build a linked list of all the sectors encompassed by the given tile
		rectangle. The list is using shared global memory from a pool and it should be returned
		to the pool using the second function.
	*/
	SectorList* (__cdecl *SectorListBuild)(const TileRect &tiles);
	void (__cdecl *SectorListFree)(SectorList *sectorList);

	/*
		Builds an unknown info blob in the second argument.
	*/
	bool (__cdecl *BuildUnk)(const TileRect &tiles, RenderUnknown &unk);

	/*
		Seems to be used to propagate the screen rectangle to the scratchbuffer.
	*/
	void (__cdecl *ScratchbufferRelated)(const TigRect &rect);

	int *unkCounter1;
	int *unkFlag2;

	// Seems to be manipulated by ScratchbufferRelated
	TigRectList **globalRectList;

	GameRenderFuncs() {
		rebase(GetVisibleTileRect, 0x1002A6B0);
		rebase(SectorListBuild, 0x10084650);
		rebase(SectorListFree, 0x10081A30);
		rebase(BuildUnk, 0x100824D0);

		rebase(ScratchbufferRelated, 0x10002530);

		rebase(unkCounter1, 0x102ABED8);
		rebase(unkFlag2, 0x1030728C);
		rebase(globalRectList, 0x10306C0C);
	}

} renderFuncs;

void __cdecl GameSystemsRenderWorld(const RenderWorldInfo &renderInfo) {
	
}

void GameSystemsRender() {
	/*
		Without this call, the ground JPGs will not be rendered.
	*/
	TigRect rect(0, 0, video->width, video->height);
	// renderFuncs.ScratchbufferRelated(rect);

	TigRect viewportSize;
	viewportSize.y = -256;
	viewportSize.width = video->width + 512;
	viewportSize.x = -256;
	viewportSize.height = video->height + 512;

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
		dirtyList.rect = TigRect(0, 0, video->width, video->height);
		dirtyList.next = nullptr;
		TigRectList *dirtyRectPtr = &dirtyList;
		renderInfo.rectList = &dirtyRectPtr;
		
		typedef void (__cdecl *DelRendFn)(const RenderWorldInfo &);
		DelRendFn delRenderFunc = temple_get<0x103072BC, DelRendFn>();
		delRenderFunc(renderInfo);

		renderFuncs.SectorListFree(sectorList);
	}

}
