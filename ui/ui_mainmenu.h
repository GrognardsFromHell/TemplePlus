
#pragma once

#include <temple/dll.h>

struct UiMainMenuFuncs : temple::AddressTable {

	void (__cdecl *ShowPage)(int page);

	UiMainMenuFuncs() {
		rebase(ShowPage, 0x10116500);
	}
};

extern UiMainMenuFuncs uiMainMenuFuncs;
