#include "stdafx.h"

#include "clipping/clipping.h"

#include "tig/tig_startup.h"
#include "mapsystems.h"

MapSystems::MapSystems(TigInitializer& tig) : mGraphics(tig.GetGraphics()) {
}

MapSystems::~MapSystems() {
}

void MapSystems::LoadModule() {
	mClipping = std::make_unique<MapClipping>(mGraphics);
	mTerrain = std::make_unique<MapTerrain>();
}

void MapSystems::UnloadModule() {
	mClipping.reset();
	mTerrain.reset();
}
