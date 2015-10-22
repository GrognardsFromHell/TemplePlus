
#include "stdafx.h"
#include "util/fixes.h"

#include "clipping.h"
#include "../gamesystems.h"
#include <gamesystems/mapsystems.h>

static class ClippingHooks : public TempleFix {
public:
	
	const char* name() override {
		return "Clipping Geometry Hooks";
	}

	void apply() override;

	static int ClippingLoad(const char *mapDir, const char *mapSaveDir);
	static int ClippingClose();
	static int ClippingInit();

} fix;

void ClippingHooks::apply() {
	replaceFunction(0x100A4040, ClippingLoad);
	replaceFunction(0x100A4C50, ClippingClose);
	replaceFunction(0x100A3DF0, ClippingInit);
}

int ClippingHooks::ClippingLoad(const char* mapDir, const char* mapSaveDir) {

	gameSystems->GetMapSystems().GetClipping().Load(mapDir);
	return 1;
}

int ClippingHooks::ClippingClose() {
	gameSystems->GetMapSystems().GetClipping().Unload();
	return 1;
}

int ClippingHooks::ClippingInit() {
	return 1;
}
