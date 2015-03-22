
#pragma once

#include "addresses.h"

struct UiMainMenuFuncs : public AddressTable {

	void (__cdecl *ShowPage)(int page);

	void rebase(Rebaser rebase) override {
		rebase(ShowPage, 0x10116500);
	}
};

extern UiMainMenuFuncs uiMainMenuFuncs;