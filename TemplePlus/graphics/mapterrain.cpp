
#include "stdafx.h"
#include "mapterrain.h"
#include "maps.h"
#include <tig/tig.h>
#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>
#include <infrastructure/mesparser.h>
#include <config/config.h>
#include <util/fixes.h>
#include <gamesystems/gamesystems.h>
#include <gamesystems/timeevents.h>

TerrainSystem::TerrainSystem(gfx::RenderingDevice& device, gfx::ShapeRenderer2d& shapeRenderer)
	: mDevice(device),mShapeRenderer(shapeRenderer) {

	// Previously initialized in ground_init
	mTerrainTintRed = 1.0f;
	mTerrainTintGreen = 1.0f;
	mTerrainTintBlue = 1.0f;

}

void TerrainSystem::Render() {

	// Special dirty case for the 5000 map which has no terrain
	if (maps.GetCurrentMapId() == 5000) {
		return;
	}

	// Check the day<->night transition and stop if necessary
	if (mIsTransitioning) {
		auto timeSinceMapEntered = timeGetTime() - mTransitionStart;
		if (timeSinceMapEntered > TransitionTime) {
			mIsTransitioning = false;
		}

		mTransitionProgress = timeSinceMapEntered / (float)TransitionTime;
	}

	auto viewportWidth = config.renderWidth;
	auto viewportHeight = config.renderHeight;

	// The terrain is centered on the center tile of the map
	// This is 480,480 for all vanilla maps, but theoretically it 
	// depends on the map size specified in the map's map.prp file
	auto mapCenter = maps.GetMapCenterTile();

	// The center of the map in pixels relative to the current screen viewport
	int terrainOriginX, terrainOriginY;
	locSys.ToTranslation(mapCenter.locx, mapCenter.locy, terrainOriginX, terrainOriginY);

	// Since the origin is still pointing at the map center, shift it left/up by half
	// the terrains overall size. The Map is slightly off-center for *some* unknown
	// reason. This may be a relic from Arkanum
	terrainOriginX -= MapWidthPixels / 2 - 40;
	terrainOriginY -= MapHeightPixels / 2 - 14;

	auto startX = - terrainOriginX / TileSize;
	auto startY = - terrainOriginY / TileSize;

	for (auto y = startY; y < MapHeightTiles; ++y) {
		TigRect destRect;
		destRect.y = terrainOriginY + y * TileSize;
		destRect.height = TileSize;
		for (auto x = startX; x < MapWidthTiles; ++x) {
			destRect.x = terrainOriginX + x * TileSize;
			destRect.width = TileSize;

			// Get the correct texture for this tile
			RenderTile(x, y, destRect);

			// The next column would be out of view
			if (destRect.x + TileSize >= viewportWidth) {
				break;
			}
		}

		// The next column would be out of view
		if (destRect.y + TileSize >= viewportHeight) {
			break;
		}
	}

}

void TerrainSystem::RenderTile(int x, int y, const TigRect& destRect) {
	auto primaryMapArtId = mMapArtId;

	// Handling transition
	if (mIsTransitioning) {
		// This is flipped while we transition, since we have to draw 
		// the old map first
		if (!mIsNightTime) {
			primaryMapArtId += NightArtIdOffset;
		}
	} else {
		if (mIsNightTime) {
			primaryMapArtId += NightArtIdOffset;
		}
	}
	
	auto textureName = GetTileTexture(primaryMapArtId, x, y);
	auto texture = mDevice.GetTextures().Resolve(textureName, false);

	if (!texture->IsValid()) {
		return;
	}

	XMCOLOR color(mTerrainTintRed, mTerrainTintGreen, mTerrainTintBlue, 1);

	auto destX = (float)destRect.x;
	auto destY = (float)destRect.y;
	auto destWidth = (float)destRect.width;
	auto destHeight = (float)destRect.height;

	mShapeRenderer.DrawRectangle(destX, destY, destWidth, destHeight, texture, color);

	if (mIsTransitioning) {

		// Use the real map here
		if (mIsNightTime) {
			primaryMapArtId = mMapArtId + NightArtIdOffset;
		} else {
			primaryMapArtId = mMapArtId;
		}
		textureName = GetTileTexture(primaryMapArtId, x, y);
		texture = mDevice.GetTextures().Resolve(textureName, false);

		// Draw the "new" map over the old one with alpha that is based on 
		// the time since the transition started		
		color = XMCOLOR(mTerrainTintRed, mTerrainTintGreen, mTerrainTintBlue, mTransitionProgress);

		mShapeRenderer.DrawRectangle(destX, destY, destWidth, destHeight, texture, color);
	}

}

const std::string & TerrainSystem::GetName(void) const
{
	static std::string sName = "Terrain";
	return sName;
}

void TerrainSystem::Load(int groundArtId)
{
	if (mMapArtId == groundArtId) {
		return;
	}
	mMapArtId = groundArtId;
	mIsTransitioning = false;
	mIsNightTime = !gameSystems->GetTimeEvent().IsDaytime();
}

void TerrainSystem::LoadModule()
{
	auto groundMapping = MesFile::ParseFile("art/ground/ground.mes");
	for (auto& pair : groundMapping) {
		mTerrainDirs[pair.first] = fmt::format("art/ground/{}/", pair.second);
	}
}

void TerrainSystem::UpdateDayNight() {
	if (gameSystems->GetTimeEvent().IsDaytime()) {
		// Start the night -> day transition
		if (mIsNightTime) {
			mIsTransitioning = true;
			mIsNightTime = false;
			mTransitionStart = timeGetTime();
		}
	} else {
		// Start the day -> night transition
		if (!mIsNightTime) {
			mIsTransitioning = true;
			mIsNightTime = true;
			mTransitionStart = timeGetTime();
		}
	}
}

std::string TerrainSystem::GetTileTexture(int mapArtId, int x, int y) {

	// Find the directory that the JPEGs are kept in
	auto it = mTerrainDirs.find(mapArtId);

	if (it == mTerrainDirs.end()) {
		return std::string();
	}
	
	return fmt::format("{}{:04x}{:04x}.jpg", it->second, y, x);

}

static class GroundSystemHooks : public TempleFix {
public:
	const char* name() override {
		return "Ground System Hooks";
	}
	void apply() override {
		// ground_add_dirty_rect
		replaceFunction<void(TigRect*)>(0x1002c4d0, [](TigRect*) {
			// Simply ignored now (no dirty tracking)
		});

		// map_ground_update_day_night
		replaceFunction<void()>(0x1002d290, []() {
			gameSystems->GetTerrain().UpdateDayNight();
		});

		// The timer event init function (100616A0) calls a function in the ground system
		// creating a cyclic dependency
		writeNoops(0x10061748);
	}

} groundHooks;
