
#include "stdafx.h"
#include "mapterrain.h"
#include "maps.h"
#include <tig/tig.h>
#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>
#include <infrastructure/mesparser.h>
#include <config/config.h>

static struct TerrainSystemAddresses : temple::AddressTable {	
	int *mapArtId;
	int *daytimeFlag;
	uint32_t *timeMapEntered;

	float *terrainTintRed;
	float *terrainTintGreen;
	float *terrainTintBlue;

	TerrainSystemAddresses() {
		rebase(mapArtId, 0x1080FA58);
		rebase(daytimeFlag, 0x1080AED8);
		rebase(timeMapEntered, 0x1080ABB0);

		rebase(terrainTintRed, 0x11E69574);
		rebase(terrainTintGreen, 0x11E69570);
		rebase(terrainTintBlue, 0x11E69564);
	}
} addresses;

TerrainSystem::TerrainSystem(gfx::RenderingDevice& device, gfx::ShapeRenderer2d& shapeRenderer)
	: mDevice(device),mShapeRenderer(shapeRenderer) {
}

void TerrainSystem::Render() {

	// Special dirty case for the 5000 map which has no terrain
	if (maps.GetCurrentMapId() == 5000) {
		return;
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
	auto mapArtId = *addresses.mapArtId;
	auto primaryMapArtId = mapArtId;

	auto isNightTime = ((*addresses.daytimeFlag) & 1) == 1;

	// Handling transition
	auto isTransitioning = ((*addresses.daytimeFlag) & 2) == 2;	
	uint32_t timeSinceMapEntered = 0;
	if (isTransitioning) {
		timeSinceMapEntered = timeGetTime() - *addresses.timeMapEntered;
		if (timeSinceMapEntered > TransitionTime) {
			isTransitioning = false;
			*addresses.daytimeFlag &= 1;
		}

		// This is flipped while we transition, since we have to draw 
		// the old map first
		if (!isNightTime) {
			primaryMapArtId += NightArtIdOffset;
		}
	} else {
		if (isNightTime) {
			primaryMapArtId += NightArtIdOffset;
		}
	}
	
	auto textureName = GetTileTexture(primaryMapArtId, x, y);
	auto texture = mDevice.GetTextures().Resolve(textureName, false);

	if (!texture->IsValid()) {
		return;
	}

	XMCOLOR color(*addresses.terrainTintRed,
		*addresses.terrainTintGreen,
		*addresses.terrainTintBlue,
		1);

	auto destX = (float)destRect.x;
	auto destY = (float)destRect.y;
	auto destWidth = (float)destRect.width;
	auto destHeight = (float)destRect.height;

	mShapeRenderer.DrawRectangle(destX, destY, destWidth, destHeight, texture, color);

	if (isTransitioning) {

		// Use the real map here
		if (isNightTime) {
			primaryMapArtId = mapArtId + NightArtIdOffset;
		} else {
			primaryMapArtId = mapArtId;
		}
		textureName = GetTileTexture(primaryMapArtId, x, y);
		texture = mDevice.GetTextures().Resolve(textureName, false);

		// Draw the "new" map over the old one with alpha that is based on 
		// the time since the transition started
		color = XMCOLOR(*addresses.terrainTintRed,
			*addresses.terrainTintGreen,
			*addresses.terrainTintBlue,
			timeSinceMapEntered / (float) TransitionTime);

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
	static auto map_open_load_terrain = temple::GetPointer<void(int artId)>(0x1002d710);
	map_open_load_terrain(groundArtId);
}

void TerrainSystem::LoadModule()
{
	auto groundMapping = MesFile::ParseFile("art/ground/ground.mes");
	for (auto& pair : groundMapping) {
		mTerrainDirs[pair.first] = fmt::format("art/ground/{}/", pair.second);
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
