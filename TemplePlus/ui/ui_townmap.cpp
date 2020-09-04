#include "stdafx.h"
#include <temple/dll.h>
#include <util/fixes.h>
#include <graphics/math.h>
#include "graphics/render_hooks.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/mapsystem.h>
#include <graphics/dynamictexture.h>
#include <graphics/device.h>
#include "ui_townmap.h"
#include "combat.h"
#include "ui_systems.h"
#include "ui_worldmap.h"
#include "ui_legacysystems.h"
#include "animgoals/anim.h"
#include "gamesystems/timeevents.h"
#include "location.h"
#include "gameview.h"
#include "tig/tig_loadingscreen.h"
#include "maps.h"
#include "tig/tig_msg.h"
#include "tig/tig_keyboard.h"

struct TownmapWidgets
{
	LgcyWindow* mainWnd;
	LgcyButton *worldmapBtn;
	LgcyButton *currentMapBtn;
	LgcyButton *mapViewBtn;
	LgcyButton *partyMembers[32];
	LgcyButton *infoFlags[20];
	LgcyButton *exitBtn;
	LgcyWindow *selectionWnd;
	LgcyButton **acquiredMapBtns;
	LgcyButton *centerOnPartyBtn;
	LgcyButton *placeInfoBtn;
	LgcyButton *delInfoBtn;
	LgcyButton *zoomToolBtn;
};

UiTownmap::UiTownmap(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1012e1c0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Townmap-UI");
	}
}
UiTownmap::~UiTownmap() {
	auto shutdown = temple::GetPointer<void()>(0x1012bbe0);
	shutdown();
}
void UiTownmap::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10128450);
	resize(&resizeArg);
}
void UiTownmap::Reset() {
	auto reset = temple::GetPointer<void()>(0x1012bb40);
	reset();
}
bool UiTownmap::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x10128650);
	return save(file) == 1;
}
bool UiTownmap::LoadGame(const UiSaveFile &save) {
	auto load = temple::GetPointer<int(const UiSaveFile*)>(0x101288f0);
	return load(&save) == 1;
}
const std::string &UiTownmap::GetName() const {
	static std::string name("Townmap-UI");
	return name;
}

bool UiTownmap::IsAvailable(){
	return mAvailable != FALSE;
}

void UiTownmap::Show(){
	static auto uiHideVarious = temple::GetRef<void(__cdecl)(int)>(0x101156B0);
	uiHideVarious(1);
	if (combatSys.isCombatActive())
		return;
	if (!uiSystems->GetRandomEncounter().HasPermissionToExit() 
		&&	uiSystems->GetWorldmap().NeedToClearEncounterMap()){
		uiSystems->GetRandomEncounter().ShowExitWnd();
		return;
	}

	if (!IsAvailable()){
		return;
	}
		
	if (!IsVisible()){
		gameSystems->GetTimeEvent().PushDisableAdvance();
	}

	mVisible = 1;
	auto lgcyWidgets = temple::GetRef<TownmapWidgets*>(0x10BE0D4C);
	uiManager->SetHidden(lgcyWidgets->mainWnd->widgetId, false);
	uiManager->BringToFront(lgcyWidgets->mainWnd->widgetId);

	uiManager->SetHidden(lgcyWidgets->selectionWnd->widgetId, false);
	uiManager->BringToFront(lgcyWidgets->selectionWnd->widgetId);

	if (!mPositionsInited){
		PositionsInit();
		mPositionsInited = 1;
	}

	auto mapId = gameSystems->GetMap().GetCurrentMapId();
	if (gameSystems->GetMap().IsRandomEncounterMap(mapId)){
		Hide();
		uiSystems->GetWorldmap().Show(3);
		return;
	}

	if (mCurrentlyDisplayedMapIdx != mapId - 4999){
		// todo
		//SetCurrentlyDisplayedMap(mapId);
		CenterOnParty();
	}
	memset(mMapsVisited, 0, sizeof mMapsVisited);
	auto count = 0;
	for (auto &mapId : gameSystems->GetMap().GetVisitedMaps()) {
		if (count >= 200) {
			break;
		}
		mMapsVisited[count++] = mapId;
	}
}

void UiTownmap::Hide()
{
}

void UiTownmap::PositionsInit(){
	auto N_maps = gameSystems->GetMap().GetHighestMapId();
	auto& device = tig->GetRenderingDevice();

	for (auto i = 0; i < N_maps; i++){
		auto &pos = mPositions[i];
		auto startLoc = gameSystems->GetMap().GetStartPos(i + 5000);	
		pos = device.GetCamera().TileToWorld( startLoc );
	}
}

void UiTownmap::CenterOnParty(){

}

class TownmapRenderer {
	friend class TownmapHooks;
public:
	TownmapRenderer();
	~TownmapRenderer();

	void Render(TigRect srcRect, TigRect destRect);

private:
	int* mTextureIdsSmall = temple::GetPointer<int>(0x11E69A60); // 81 entries (9x9)
	int* mTextureIdsBig = temple::GetPointer<int>(0x11E69580); // 306 entries (17x18)
	BOOL& mFogOfWar = temple::GetRef<BOOL>(0x11E69568);
	uint8_t* mFogData = temple::GetPointer<uint8_t>(0x11E69BC0);
	uint8_t* mFogUnexploredData = temple::GetPointer<uint8_t>(0x11E61560);
	uint32_t& mTownmapMapId = temple::GetRef<uint32_t>(0x11E6956C);

	gfx::DynamicTexturePtr mFogTexture;
};

static TownmapRenderer* sRenderer = nullptr;

TownmapRenderer::TownmapRenderer() {
	if (!sRenderer) {
		sRenderer = this;
	}

	mFogTexture = gfx::renderingDevice->CreateDynamicTexture(gfx::BufferFormat::A8R8G8B8, 256, 256);
}

TownmapRenderer::~TownmapRenderer() {
	if (sRenderer == this) {
		sRenderer = nullptr;
	}
}

template <int cols, int rows, int tileDimension, typename TRenderer, typename... TArgs>
static void RenderTownmapTiles(const TigRect& srcRect, const TigRect& destRect, float widthScale, float heightScale, TArgs ... args) {
	auto rightmostTile = (srcRect.x + srcRect.width) / tileDimension;
	if (rightmostTile >= cols) {
		rightmostTile = cols;
	}

	auto bottommostTile = (srcRect.y + srcRect.height) / tileDimension;
	if (bottommostTile >= rows) {
		bottommostTile = rows;
	}
	auto startTileX = srcRect.x / tileDimension;
	if (srcRect.x / tileDimension < 0) {
		startTileX = 0;
	}
	auto startTileY = srcRect.y / tileDimension;
	if (srcRect.y / tileDimension < 0) {
		startTileY = 0;
	}

	for (auto tileY = startTileY; tileY <= bottommostTile; ++tileY) {
		auto curSrcY = tileY * tileDimension;

		for (auto tileX = startTileX; tileX <= rightmostTile; ++tileX) {
			auto curSrcX = tileX * tileDimension;
			auto clippedSrcX = tileX * (float) tileDimension;
			auto clippedSrcWidth = clippedSrcX + (float) tileDimension;
			if (clippedSrcX < (float)srcRect.x) {
				clippedSrcX = (float)srcRect.x;
			}
			auto srcRectRight = (float)(srcRect.x + srcRect.width);
			if (clippedSrcWidth > srcRectRight) {
				clippedSrcWidth = srcRectRight;
			}
			clippedSrcWidth = clippedSrcWidth - clippedSrcX;

			auto clippedSrcY = tileY * (float) tileDimension;
			auto clippedSrcHeight = clippedSrcY + (float) tileDimension;
			if (clippedSrcY < (float)srcRect.y) {
				clippedSrcY = (float)srcRect.y;
			}
			auto srcRectBottom = (float)(srcRect.y + srcRect.height);
			if (clippedSrcHeight > srcRectBottom)
				clippedSrcHeight = srcRectBottom;
			clippedSrcHeight -= clippedSrcY;

			TigRect tileDestRect;
			tileDestRect.x = destRect.x + (int)((clippedSrcX - srcRect.x) * widthScale);
			tileDestRect.y = destRect.y + (int)((clippedSrcY - srcRect.y) * heightScale);
			tileDestRect.width = destRect.x + (int)((clippedSrcX + clippedSrcWidth - srcRect.x) * widthScale) - tileDestRect.x;
			tileDestRect.height = destRect.y + (int)((clippedSrcY + clippedSrcHeight - srcRect.y) * heightScale) - tileDestRect.y;

			// Real size of a townmap tile is 256x256, and it is stretched up to full size (1024x1024)
			constexpr auto tileScale = 256.0f / tileDimension;
			clippedSrcX = (clippedSrcX - curSrcX) * tileScale;
			clippedSrcY = (clippedSrcY - curSrcY) * tileScale;
			clippedSrcWidth *= tileScale;
			clippedSrcHeight *= tileScale;

			if (clippedSrcWidth && clippedSrcHeight && tileDestRect.width && tileDestRect.height) {
				TRenderer::Render(tileX, tileY,
				                  clippedSrcX, clippedSrcY, clippedSrcWidth, clippedSrcHeight,
				                  tileDestRect,
				                  args...);
			}
		}
	}
}

template <int cols>
struct RenderTile {
	static void Render(int tileX, int tileY,
	                   float clippedSrcX, float clippedSrcY,
	                   float clippedSrcWidth, float clippedSrcHeight,
	                   const TigRect& tileDestRect,
	                   int* textureIds) {
		Render2dArgs arg;
		arg.textureId = textureIds[tileX + cols * tileY];
		arg.flags = Render2dArgs::FLAG_FLOATSRCRECT;
		TigRect srcRectClipped;
		srcRectClipped.x = *reinterpret_cast<int*>(&clippedSrcX);
		srcRectClipped.y = *reinterpret_cast<int*>(&clippedSrcY);
		srcRectClipped.width = *reinterpret_cast<int*>(&clippedSrcWidth);
		srcRectClipped.height = *reinterpret_cast<int*>(&clippedSrcHeight);
		arg.srcRect = &srcRectClipped;
		arg.destRect = &tileDestRect;
		arg.vertexColors = nullptr;
		arg.vertexZ = 0;
		RenderHooks::TextureRender2d(&arg);
	}
};

struct RenderFog {
	static void Render(int tileX, int tileY,
	                   float clippedSrcX, float clippedSrcY,
	                   float clippedSrcWidth, float clippedSrcHeight,
	                   const TigRect& tileDestRect,
	                   uint8_t* fogData,
	                   gfx::DynamicTexture& fogTexture) {

		static uint32_t sPixelData[256 * 256];

		auto pixelPtr = &sPixelData[0];
		auto fogPtr = &fogData[8193 * (tileX + 2 * tileY) + 1];

		for (auto y = 0; y < 256; ++y) {
			for (auto chunk = 0; chunk < 32; chunk++) {
				auto fogByte = *fogPtr;
				for (auto bitmask = 1; bitmask < 0x100; bitmask <<= 1) {
					*pixelPtr = (fogByte & bitmask) == 0 ? 0xFF000000 : 0;
					pixelPtr++;
				}
				++fogPtr;
			}
		}

		fogTexture.Update<uint32_t>({&sPixelData[0], 256 * 256});

		Render2dArgs arg;
		arg.flags = Render2dArgs::FLAG_FLOATSRCRECT | Render2dArgs::FLAG_BUFFERTEXTURE;
		arg.texBuffer = (TigBuffer*) &fogTexture;
		TigRect srcRectClipped;
		srcRectClipped.x = *reinterpret_cast<int*>(&clippedSrcX);
		srcRectClipped.y = *reinterpret_cast<int*>(&clippedSrcY);
		srcRectClipped.width = *reinterpret_cast<int*>(&clippedSrcWidth);
		srcRectClipped.height = *reinterpret_cast<int*>(&clippedSrcHeight);
		arg.srcRect = &srcRectClipped;
		arg.destRect = &tileDestRect;
		arg.vertexColors = nullptr;
		arg.vertexZ = 0;
		RenderHooks::TextureRender2d(&arg);
	}
};

void TownmapRenderer::Render(TigRect srcRect, TigRect destRect) {

	// This seems to be the origin of the townmap, basically
	srcRect.x += 8448;
	srcRect.y -= 6000;
	auto widthScale = (float)destRect.width / (float)srcRect.width;
	auto heightScale = (float)destRect.height / (float)srcRect.height;

	// Render using the small / big grid size depending on zoom level
	if (widthScale <= 1 / 6.f) {
		RenderTownmapTiles<9, 9, 2048, RenderTile<9>>(srcRect, destRect, widthScale, heightScale, mTextureIdsSmall);
	} else {
		RenderTownmapTiles<17, 18, 1024, RenderTile<17>>(srcRect, destRect, widthScale, heightScale, mTextureIdsBig);
	}

	if (mFogOfWar) {
		if (gameSystems->GetMap().GetCurrentMapId() == mTownmapMapId) {
			memcpy(mFogData, mFogUnexploredData, 0x8004);
		}
		RenderTownmapTiles<2, 2, 10240, RenderFog>(srcRect, destRect, widthScale, heightScale, mFogData, *mFogTexture);
	}

	/*
	if (mFogOfWar)
		{
			v104 = get_fogcol();
			if (gameSystems->GetMap().GetCurrentMapId() == ui_townmap_map_id) {
				qmemcpy(&townmap_fog_unexplored_data, fog_unexplored_data, 0x8004u);
			}
			v54 = srcRect.x;
			v55 = srcRect.x + srcRect.width;
			v56 = v55 / 10240;
			rightmostTile = v55 / 10240;
			if (v55 / 10240 >= 2)
			{
				v56 = 2;
				rightmostTile = 2;
			}
			v57 = srcRect.y;
			v58 = 0x66666667i64 * (v57 + srcRect.height);
			v59 = (HIDWORD(v58) >> 31) + (SHIDWORD(v58) >> 12);
			bottommostTile = (HIDWORD(v58) >> 31) + (SHIDWORD(v58) >> 12);
			if (v59 >= 2)
			{
				v59 = 2;
				bottommostTile = 2;
			}
			v60 = v54 / 10240;
			startTileX = v54 / 10240;
			if (v54 / 10240 < 0)
			{
				v60 = 0;
				startTileX = 0;
			}
			v61 = v57 / 10240;
			if (v57 / 10240 < 0)
				v61 = 0;
			curTileY = v61;
			if (v61 <= v59)
			{
				curSrcY = 10240 * v61;
				do
				{
					curTileX_ = v60;
					if (v60 <= v56)
					{
						srcRect.height = 10240 * v60;
						v102 = (float)curTileY * 10240.0;
						*(float *)&v100 = (float)curSrcY;
						while (1)
						{
							v62 = srcRect.x;
							clippedSrcY = v102;
							clippedSrcX = (float)curTileX_ * 10240.0;
							clippedSrcWidth = clippedSrcX + 10240.0;
							v63 = v102 + 10240.0;
							v90 = (float)v62;
							if ((clippedSrcX < (float)v90) | __UNORDERED__(clippedSrcX, v90))
								clippedSrcX = v90;
							v64 = srcRect.y;
							v91 = (float)srcRect.y;
							if ((clippedSrcY < (float)v91) | __UNORDERED__(clippedSrcY, v91))
								clippedSrcY = v91;
							LODWORD(v97) = v62 + srcRect.width;
							v65 = (float)SLODWORD(v97);
							if (clippedSrcWidth > v65)
								clippedSrcWidth = v65;
							LODWORD(v97) = v64 + srcRect.height;
							v86 = (float)SLODWORD(v97);
							if (v63 > v86)
								v63 = v86;
							v66 = destRect.x;
							clippedSrcWidth = clippedSrcWidth - clippedSrcX;
							v67 = v63 - clippedSrcY;
							v68 = (unsigned __int64)((clippedSrcX - v90) * widthScale);
							v69 = destRect.y;
							v70 = v66 + v68;
							destRect.x = v66 + v68;
							v71 = (unsigned __int64)((clippedSrcY - v91) * heightScale);
							v72 = v69 + v71;
							destRect.y = v69 + v71;
							v73 = (unsigned __int64)((clippedSrcWidth + clippedSrcX - v90) * widthScale);
							v74 = v66 + v73 - v70;
							destRect.width = v66 + v73 - v70;
							v75 = (unsigned __int64)((clippedSrcY + v67 - v91) * heightScale);
							v76 = destRect.y + v75 - v72;
							destRect.height = destRect.y + v75 - v72;
							clippedSrcX = (clippedSrcX - (float)srcRect.height) * 0.025;
							clippedSrcY = (clippedSrcY - *(float *)&v100) * 0.025;
							v77 = clippedSrcWidth * 0.025;
							clippedSrcWidth = v77;
							clippedSrcHeight = v67 * 0.025;
							if (!((v77 == 0.0) | __UNORDERED__(v77, 0.0))
								&& !((clippedSrcHeight == 0.0) | __UNORDERED__(clippedSrcHeight, 0.0))
								&& v74
								&& v76)
							{
								if (tig_buffer_lock_texture(townmap_fog_buffer))
									return;
								if (tig_buffer_copy_props(townmap_fog_buffer, &infoOut))
								{
									tig_buffer_unlock_texture(townmap_fog_buffer);
									return;
								}
								v78 = (char *)infoOut.textureData;
								v79 = v104;
								v80 = (char *)&unk_11E69BC1 + 8193 * (curTileX_ + 2 * curTileY);
								LODWORD(v97) = 4 * (infoOut.lockedTexturePitch / 4 - 256);
								texIdRowOffset = 256;
								do
								{
									v81 = 32;
									do
									{
										v82 = (unsigned __int8)*v80;
										v83 = 1;
										do
										{
											*(_DWORD *)v78 = (v82 & v83) == 0 ? v79 : 0;
											v83 *= 2;
											v78 += 4;
										} while (v83 < 256);
										++v80;
										--v81;
									} while (v81);
									v78 += LODWORD(v97);
									--texIdRowOffset;
								} while (texIdRowOffset);
								tig_buffer_unlock_texture(townmap_fog_buffer);
								arg.texbuffer = townmap_fog_buffer;
								arg.flags = 0x180;
								arg.srcRect = (tio_rect *)&srcRecta;
								arg.destRect = &destRect;
								arg.vertexColors = 0;
								LODWORD(arg.vertexZ) = 1325400064;
								ui_draw_element(&arg);
							}
							v35 = __OFSUB__(curTileX_ + 1, rightmostTile);
							v33 = curTileX_ + 1 == rightmostTile;
							v34 = curTileX_++ + 1 - rightmostTile < 0;
							srcRect.height += 10240;
							if (!((unsigned __int8)(v34 ^ v35) | v33))
							{
								v60 = startTileX;
								v59 = bottommostTile;
								v56 = rightmostTile;
								break;
							}
						}
					}
					v35 = __OFSUB__(curTileY + 1, v59);
					v33 = curTileY + 1 == v59;
					v34 = curTileY++ + 1 - v59 < 0;
					curSrcY += 10240;
				} while ((unsigned __int8)(v34 ^ v35) | v33);
			}
		} */
}

/**
 * Hook for replacing the existing render function.
 */

static class TownmapHooks : public TempleFix {
public:

	void apply() override {

		// UiTownmapMainWndMsg - irregular behavior on 'ESC' key handling (would cause crash)
		replaceFunction<BOOL(int, TigMsg&)>(0x1012C2D0, [](int widId, TigMsg& msg)->BOOL {
			if (msg.type == TigMsgType::KEYSTATECHANGE) {
				auto &keystt = (TigKeyStateChangeMsg&)msg;
				if (keystt.key == DIK_ESCAPE && keystt.down == 0) {
					static auto hideUiTownmap = temple::GetRef<void()>(0x1012BCB0);
					hideUiTownmap();
					return TRUE; // fixes crash issue and brings in line with other UI message handlers
				}	
			}
			if (msg.type == TigMsgType::CHAR || msg.type == TigMsgType::MOUSE || msg.type == TigMsgType::WIDGET) {
				return TRUE;
			}
			return FALSE;
			});
		

		// ui_render_townmap_ui_0
		replaceFunction<void(TigRect*, TigRect*)>(0x1002c750, [](TigRect* srcRect, TigRect* dstRct) {
			                                          static TownmapRenderer sRendererInst;

			                                          if (!sRenderer) {
				                                          return;
			                                          }

			                                          sRenderer->Render(*srcRect, *dstRct);
		                                          });
	}

} townmapHooks;

