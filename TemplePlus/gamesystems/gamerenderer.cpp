
#include "stdafx.h"

#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/shaperenderer2d.h>
#include <fonts/fonts.h>
#include <temple/aasrenderer.h>
#include <temple/dll.h>
#include <particles/render.h>
#include <particles/instances.h>
#include "ui/ui_render.h"
#include "tig/tig_font.h"
#include "particlesystems.h"

#include "tig/tig_startup.h"
#include "temple_functions.h"
#include "gamerenderer.h"
#include "gamesystems.h"
#include "clipping/clipping.h"
#include <config/config.h>
#include "mapobjrender.h"
#include "graphics/mapterrain.h"
#include "partsystemsrenderer.h"
#include "map/gmesh.h"
#include "lightningrenderer.h"
#include "ui_intgame_renderer.h"
#include "fogrenderer.h"
#include "map/sector.h"
#include "gameview.h"

using namespace gfx;
using namespace temple;
using namespace particles;

GameRenderer *gameRenderer = nullptr;

#pragma pack(push, 1)


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
  Given a rectangle in screen coordinates, calculates the rectangle in
  tile-space that
  is visible.
  */
  bool(__cdecl *GetVisibleTileRect)(const TigRect &screenRect, TileRect &tiles);

  /*
  These two functions build a linked list of all the sectors encompassed by the
  given tile
  rectangle. The list is using shared global memory from a pool and it should be
  returned
  to the pool using the second function.
  */
  SectorList *(__cdecl *SectorListBuild)(const TileRect &tiles);
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

  void (*RenderGround)(RenderWorldInfo *info);
  void (*RenderMapObj)(RenderWorldInfo *info);
  void (*PerformFogChecks)();
  void (*RenderClipping)();
  void (*RenderGMesh)();
  void (*RenderPfxLighting)();
  void (*RenderPartSys)();
  void (*RenderFogOfWar)();
  void (*RenderOcclusion)(RenderWorldInfo *info);
  void (*RenderUiRelated)(RenderWorldInfo *info); // renders Move Destinations, Character discs, Hovered objects, Boxselect, Radial Menu, and Pickers (Intgame Select)
  void (*RenderTextBubbles)(RenderWorldInfo *info);
  void (*RenderTextFloaters)(RenderWorldInfo *info);

  GameRenderFuncs() {
    rebase(GetVisibleTileRect, 0x1002A6B0);
    rebase(SectorListBuild, 0x10084650);
    rebase(SectorListFree, 0x10081A30);
    rebase(BuildUnk, 0x100824D0);

    rebase(ScratchbufferRelated, 0x10002530);

    rebase(gameDrawEnableCount, 0x102ABED8);
    rebase(unkFlag2, 0x1030728C);

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

GameRenderer::GameRenderer(TigInitializer &tig,
                           GameSystems &gameSystems)
    : mRenderingDevice(tig.GetRenderingDevice()), 
	  mGameSystems(gameSystems)      
{

	Expects(!gameRenderer);

	mAasRenderer = std::make_unique<temple::AasRenderer>(
		gameSystems.GetAAS(), 
		tig.GetRenderingDevice(),
		tig.GetShapeRenderer2d(),
		tig.GetShapeRenderer3d(),
		tig.GetMdfFactory());
	mMapObjectRenderer = std::make_unique<MapObjectRenderer>(
		gameSystems, 
		tig.GetRenderingDevice(), 
		tig.GetMdfFactory(),
		*mAasRenderer);
	mParticleSysRenderer = std::make_unique<ParticleSystemsRenderer>(
		tig.GetRenderingDevice(),
		tig.GetShapeRenderer2d(),
		gameSystems.GetAAS(),
		*mAasRenderer,
		gameSystems.GetParticleSys());
	mGmeshRenderer = std::make_unique<GMeshRenderer>(*mAasRenderer,
		*mMapObjectRenderer,
		gameSystems.GetGMesh());
	mLightningRenderer = std::make_unique<LightningRenderer>(
		tig.GetMdfFactory(),
		tig.GetRenderingDevice());
	mFogOfWarRenderer = std::make_unique<FogOfWarRenderer>(
		tig.GetMdfFactory(),
		tig.GetRenderingDevice());
	mIntgameRenderer = std::make_unique<IntgameRenderer>(
		tig.GetMdfFactory(),
		tig.GetRenderingDevice());

	gameRenderer = this;

}

GameRenderer::~GameRenderer() {
	gameRenderer = nullptr;
}

#pragma pack(push, 8)
struct PreciseSectorCol {
  int colCount;
  uint64_t tiles[4];
  uint64_t sectorIds[4];
  int startTiles[4];
  int strides[4];
  int x;
  int y;
};

struct PreciseSectorRows {
  int rowCount;
  PreciseSectorCol rows[4];
};
#pragma pack(pop)

void GameRenderer::Render() {

	gfx::PerfGroup perfGroup(mRenderingDevice, "Game Renderer");

  /*
  Without this call, the ground JPGs will not be rendered.
  */
  // TigRect rect(0, 0, graphics->GetSceneWidth(), graphics->GetSceneHeight());
  // renderFuncs.ScratchbufferRelated(rect);

  if (renderFuncs.gameDrawEnableCount <= 0) {
    return;
  }

  auto zoomFactor = gameView->GetZoom();

  TigRect viewportSize;
  viewportSize.y = -256;
  viewportSize.width = config.renderWidth / zoomFactor + 512;
  viewportSize.x = -256;
  viewportSize.height = config.renderHeight / zoomFactor + 512;

  TileRect tiles;

  if (renderFuncs.GetVisibleTileRect(viewportSize, tiles)) {
    RenderUnknown unk;
    renderFuncs.BuildUnk(tiles, unk);

    PreciseSectorRows *list = reinterpret_cast<PreciseSectorRows *>(&unk);

	auto sectorList = sectorSys.BuildSectorList(&tiles);

    RenderWorldInfo renderInfo;
    renderInfo.sectors = sectorList;
    renderInfo.unknown = &unk;
    renderInfo.tiles = &tiles;
    renderInfo.viewportSize = &viewportSize;

    // I think this maybe a 2D arcanum leftover when the map could be
    // incrementally drawn based on "dirty rects"
	
    TigRectList dirtyList;
    dirtyList.rect = TigRect(0, 0, config.renderWidth / zoomFactor , config.renderHeight / zoomFactor);
    dirtyList.next = nullptr;
    TigRectList *dirtyRectPtr = &dirtyList;
    renderInfo.rectList = &dirtyRectPtr;

    RenderWorld(&renderInfo);

	sectorSys.SectorListReturnToPool(sectorList);
  }
}

void GameRenderer::SetZoom(float zoomFactor){
	if (zoomFactor < 1.0 || zoomFactor > 2.0)
		return;
	mRenderingDevice.GetCamera().SetScale(zoomFactor);
}

void GameRenderer::RenderWorld(RenderWorldInfo *info) {
	
  if (mRenderingDevice.BeginFrame()) {
    // renderFuncs.RenderGround(info);
    mGameSystems.GetTerrain().Render();

    renderFuncs.PerformFogChecks();

    mGameSystems.GetClipping().Render();
    // renderFuncs.RenderClipping();

    mMapObjectRenderer->RenderMapObjects(
        (int)info->tiles->x1, (int)info->tiles->x2, (int)info->tiles->y1,
        (int)info->tiles->y2);
    //renderFuncs.RenderMapObj(info);
    
	mGmeshRenderer->Render();
	// renderFuncs.RenderGMesh();
    
	mLightningRenderer->Render();
	// renderFuncs.RenderPfxLighting();

	mParticleSysRenderer->Render();
    // renderFuncs.RenderPartSys();

	mFogOfWarRenderer->Render();
    // renderFuncs.RenderFogOfWar();

	mMapObjectRenderer->RenderOccludedMapObjects((int)info->tiles->x1, (int)info->tiles->x2, (int)info->tiles->y1,
		(int)info->tiles->y2);
    /*graphics->EnableLighting();
    renderFuncs.RenderOcclusion(info);
    graphics->DisableLighting();*/
	
	gfx::PerfGroup perfGroup(mRenderingDevice, "World UI");
    renderFuncs.RenderUiRelated(info);
    renderFuncs.RenderTextBubbles(info);
    renderFuncs.RenderTextFloaters(info);

    mRenderingDevice.Present();
  }
}
