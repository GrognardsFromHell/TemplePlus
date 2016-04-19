
#include "stdafx.h"
#include "util/fixes.h"
#include "config/config.h"
#include "gamesystems/gamesystems.h"
#include <temple/dll.h>

typedef int(__cdecl *UiCharInit)(const GameSystemConf& conf);
static UiCharInit uiCharInit;

static int HookedUiCharInit(const GameSystemConf &conf) {
	auto copyConf = conf;

	copyConf.width = 800;
	copyConf.height = 600;

	// Char Init reads the width/height from these values but only applies it to certain parts of the
	// x/y calculation. I.e. scrollbars and the helptext are just misaligned in these cases.
	auto& vidWidth = temple::GetRef<0x103012C8, int>();
	auto& vidHeight = temple::GetRef<0x103012CC, int>();
	vidWidth = 800;
	vidHeight = 600;

	auto result = uiCharInit(copyConf);	

	vidWidth = conf.width;
	vidHeight = conf.height;

	return result;
}

// Fixes higher resolution issues with the char and inventory screen
class UiCharHighresFix : public TempleFix {
public:
	
	void apply() override {
		if (!config.engineEnhancements) {
			return;
		}

		uiCharInit = reinterpret_cast<UiCharInit>(replaceFunction(0x1014B900, HookedUiCharInit));
	}

} uiCharHighresFix;
