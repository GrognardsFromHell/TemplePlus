
#include "stdafx.h"
#include "ui_systems.h"
#include "ui_mainmenu.h"
#include "util/fixes.h"

static class UiMainMenuWrapper : public TempleFix {
public:

	void apply() override {
		// ui_mm_is_visible
		replaceFunction<BOOL()>(0x101157f0, []() {
			return uiSystems->GetMM().IsVisible() ? TRUE : FALSE;
		});

		// ui_mm_hide
		replaceFunction<void()>(0x10116220, []() {
			uiSystems->GetMM().Hide();
		});

		// ui_mm_render_logo
		replaceFunction<void()>(0x10115820, []() {
			// TODO: This is stupid, this just renders the game logo as if the main menu was open
		});

		// ui_mm_show_page
		replaceFunction<void(int)>(0x10116500, [](int page) {
			uiSystems->GetMM().Show((MainMenuPage)page);
		});

	}

} wrapper;
