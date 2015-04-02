
#include "stdafx.h"
#include "fixes.h"
#include "temple_functions.h"
#include "gamesystems.h"

typedef int(__cdecl *UiCharInit)(const GameSystemConf& conf);
static UiCharInit uiCharInit;

static int HookedUiCharInit(const GameSystemConf &conf) {
	auto copyConf = conf;

	copyConf.width = 800;
	copyConf.height = 600;

	// Char Init reads the width/height from these values but only applies it to certain parts of the
	// x/y calculation. I.e. scrollbars and the helptext are just misaligned in these cases.
	temple_set<0x103012C8>(800);
	temple_set<0x103012CC>(600);

	auto result = uiCharInit(copyConf);	

	temple_set<0x103012C8>(conf.width);
	temple_set<0x103012CC>(conf.height);

	return result;
}

class UiCharHighresFix : public TempleFix {
public:
	
	const char* name() override {
		return "Fixes higher resolution issues with the char and inventory screen";
	}

	void apply() override {
		uiCharInit = reinterpret_cast<UiCharInit>(replaceFunction(0x1014B900, HookedUiCharInit));
	}

} uiCharHighresFix;
