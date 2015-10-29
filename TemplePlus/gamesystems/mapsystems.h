#pragma once

#include <memory>

#include <graphics/mapterrain.h>

#include "gamesystem.h"

class TigInitializer;
class MapClipping;

class MapSystems : public GameSystem, public ModuleAwareGameSystem {
public:

	static constexpr const char* Name = "Map Systems";

	MapSystems(TigInitializer& tig);
	~MapSystems();

	bool IsClippingLoaded() {
		return !!mClipping;
	}

	MapClipping& GetClipping() {
		return *mClipping;
	}

	MapTerrain& GetTerrain() {
		return *mTerrain;
	}
	
	const std::string& GetName() const override {
		static std::string sName = Name;
		return sName;
	}
	
	void LoadModule() override;
	void UnloadModule() override;

	MapSystems(MapSystems&) = delete;
	MapSystems(MapSystems&&) = delete;
	MapSystems& operator=(MapSystems&) = delete;
	MapSystems& operator=(MapSystems&&) = delete;

private:

	Graphics &mGraphics;

	std::unique_ptr<MapClipping> mClipping;
	std::unique_ptr<MapTerrain> mTerrain;

};
