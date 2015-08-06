

#include "stdafx.h"

#include "util/fixes.h"

static void ui_mm_show_page(int page) {
	logger->info("Showing MM page {}", page);
}

static BOOL ui_mm_is_visible() {
	logger->info("Is MM visible?");
	return FALSE;
}

static class UiScreenOverrides : public TempleFix {
	const char * name() override {
		return "UI Screens";
	}

	void apply() override {
		replaceFunction(0x10116500, ui_mm_show_page);
		replaceFunction(0x101157F0, ui_mm_is_visible);
	}

} uiScreenOverrides;
