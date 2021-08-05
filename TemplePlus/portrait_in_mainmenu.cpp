
#include "stdafx.h"

#include "graphics/video_hooks.h"
#include "util/fixes.h"
#include "obj.h"

static void(__cdecl *ObjRenderInUi)(objHndl objId, int x, int y, float rotation, float scale);

// fixes displaying 3d character models in the partypool while in the main menu
class MainMenuPartyPoolFix : public TempleFix, temple::AddressTable {
public:
	MainMenuPartyPoolFix() {
		rebase(ObjRenderInUi, 0x100243B0);
	}
	
	void apply() override {		
		redirectCall(0x1019DF6C, &hookedRenderFunc);
	}

	static void __cdecl hookedRenderFunc(objHndl objId, int x, int y, float rotation, float scale) {
		// Fix projection matrix on shop map
		auto currentScale = video->viewParams.scale;
		video->viewParams.scale = 1;
		videoFuncs.updateProjMatrices(&video->viewParams);

		ObjRenderInUi(objId, x, y, rotation, scale);

		video->viewParams.scale = currentScale;
		videoFuncs.updateProjMatrices(&video->viewParams);
	}

} mainMenuPartyPoolFix;
