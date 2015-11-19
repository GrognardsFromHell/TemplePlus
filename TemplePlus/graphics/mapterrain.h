#pragma once

struct TigRect;

namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
}

class MapTerrain {
public:

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

	MapTerrain(gfx::RenderingDevice &device,
		gfx::ShapeRenderer2d &shapeRenderer);

	void Render();

	void RenderTile(int x, int y, const TigRect& destRect);

	MapTerrain(MapTerrain&) = delete;
	MapTerrain(MapTerrain&&) = delete;
	MapTerrain& operator=(MapTerrain&) = delete;
	MapTerrain& operator=(MapTerrain&&) = delete;

private:
	gfx::RenderingDevice& mDevice;
	gfx::ShapeRenderer2d& mShapeRenderer;

	std::unordered_map<int, std::string> mTerrainDirs;

	std::string GetTileTexture(int mapArtId, int x, int y);

};
