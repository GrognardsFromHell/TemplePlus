

#include "stdafx.h"

#include "util/fixes.h"
#include "templeplus.h"
#include "ui/js/js_interop.h"

static void ui_mm_show_page(int page) {
	logger->info("Showing MM page {}", page);

	JsInterop::CallAsync("ui_mm_show_page", page);
}

static BOOL ui_mm_is_visible() {
	logger->info("Is MM visible?");

	auto visible = JsInterop::Call<bool>("ui_mm_is_visible");

	return visible ? TRUE : FALSE;
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
