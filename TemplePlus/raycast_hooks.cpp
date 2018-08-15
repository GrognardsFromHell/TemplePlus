

#include "stdafx.h"
#include "raycast.h"
#include "util/fixes.h"

static class RaycastHooks : public TempleFix {
public:

	void apply() override {

		// screen_raycast
		replaceFunction<int(int, int, objHndl *, GameRaycastFlags)>(0x10022360, [](int x, int y, objHndl *handleOut, GameRaycastFlags flags) {
			return PickObjectOnScreen(x, y, handleOut, flags) ? 1 : 0;
		});

	}

} hooks;
