
#include "stdafx.h"
#include "util/fixes.h"

#include "clipping.h"
#include "../gamesystems.h"

// Clipping Geometry Hooks
static class ClippingHooks : public TempleFix {
public:
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

	gameSystems->GetClipping().Load(mapDir);
	return 1;
}

int ClippingHooks::ClippingClose() {
	if (!gameSystems) {
		return 1; // Game systems are shutting down already
	}
	auto& clipping = gameSystems->GetClipping();
	clipping.Unload();
	return 1;
}

int ClippingHooks::ClippingInit() {
	return 1;
}
