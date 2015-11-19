#include "stdafx.h"

#include "clipping/clipping.h"

#include "tig/tig_startup.h"
#include "mapsystems.h"

MapSystems::MapSystems(TigInitializer& tig) : mTig(tig) {
}

MapSystems::~MapSystems() {
}

void MapSystems::LoadModule() {
	mClipping = std::make_unique<MapClipping>(mTig.GetRenderingDevice());
	mTerrain = std::make_unique<MapTerrain>(mTig.GetRenderingDevice(),
		mTig.GetShapeRenderer2d());
}

void MapSystems::UnloadModule() {
	mClipping.reset();
	mTerrain.reset();
}
