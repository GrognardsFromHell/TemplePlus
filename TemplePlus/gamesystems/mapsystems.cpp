#include "stdafx.h"

#include <graphics/graphics.h>

#include "clipping/clipping.h"

#include "tig/tig_startup.h"
#include "mapsystems.h"

MapSystems::MapSystems(TigInitializer& tig) {

	auto& graphics = tig.GetGraphics();

	mClipping = std::make_unique<MapClipping>(graphics);

	// mTerrain = std::make_unique<MapTerrain>();

}

MapSystems::~MapSystems() {
}
