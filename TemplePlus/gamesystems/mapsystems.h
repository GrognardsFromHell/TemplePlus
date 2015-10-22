
#pragma once

#include <memory>

#include <graphics/mapterrain.h>

class TigInitializer;
class MapClipping;

class MapSystems {
public:
	MapSystems(TigInitializer &tig);
	~MapSystems();

	MapClipping& GetClipping() {
		return *mClipping;
	}

	MapTerrain& GetTerrain() {
		return *mTerrain;
	}

	MapSystems(MapSystems&) = delete;
	MapSystems(MapSystems&&) = delete;
	MapSystems& operator=(MapSystems&) = delete;
	MapSystems& operator=(MapSystems&&) = delete;

private:

	std::unique_ptr<MapClipping> mClipping;
	
	std::unique_ptr<MapTerrain> mTerrain;

};
