#pragma once

#include "../gamesystems/gamesystem.h"
#include <temple/dll.h>

struct TigRect;

namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
}

class TerrainSystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "Terrain";

	/**
	 * Both width and height of a single tile in pixels.
	 */
	static constexpr auto TileSize = 256;

	/**
	 * Max. dimensions of a map in tiles.
	 */
	static constexpr auto MapWidthTiles = 66;
	static constexpr auto MapHeightTiles = 71;

	/**
	* Max. dimensions of a map in pixels.
	*/
	static constexpr auto MapWidthPixels = MapWidthTiles * TileSize;
	static constexpr auto MapHeightPixels = MapHeightTiles * TileSize;

	// Time in MS that we fade between the day/night versions upon transitioning
	static constexpr auto TransitionTime = 2000;

	// Offset for the map art ids for nighttime ids
	static constexpr auto NightArtIdOffset = 1000;

	TerrainSystem(gfx::RenderingDevice &device,
		gfx::ShapeRenderer2d &shapeRenderer);

	void Render();

	void RenderTile(int x, int y, const TigRect& destRect);
	
	const std::string &GetName(void) const override;

	void Load(int groundArtId);
	
	void LoadModule() override;

	TerrainSystem(TerrainSystem&) = delete;
	TerrainSystem(TerrainSystem&&) = delete;
	TerrainSystem& operator=(TerrainSystem&) = delete;
	TerrainSystem& operator=(TerrainSystem&&) = delete;
	
	void UpdateDayNight();
private:
	gfx::RenderingDevice& mDevice;
	gfx::ShapeRenderer2d& mShapeRenderer;

	int mMapArtId = 0;
	
	// Tracking for the day/night transition
	bool mIsNightTime = false;
	bool mIsTransitioning = false;
	uint32_t mTransitionStart = 0;
	float mTransitionProgress = 0;

	// Terrain colors are managed by the daylight system
	float &mTerrainTintRed = temple::GetRef<float>(0x11E69574);
	float &mTerrainTintGreen = temple::GetRef<float>(0x11E69570);
	float &mTerrainTintBlue = temple::GetRef<float>(0x11E69564);

	std::unordered_map<int, std::string> mTerrainDirs;

	std::string GetTileTexture(int mapArtId, int x, int y);

};
