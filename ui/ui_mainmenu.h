
#pragma once

#include "util/addresses.h"

struct UiMainMenuFuncs : public AddressTable {

	void (__cdecl *ShowPage)(int page);

	UiMainMenuFuncs() {
		rebase(ShowPage, 0x10116500);
	}
};

extern UiMainMenuFuncs uiMainMenuFuncs;