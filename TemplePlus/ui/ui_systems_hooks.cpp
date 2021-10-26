
#include "stdafx.h"

#include <util/fixes.h>

#include "ui_system.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"

static class UiSystemHooks : public TempleFix {
public:

	virtual void apply() {

		// gameuilib_reset
		replaceFunction<void()>(0x10115270, []() {
			uiSystems->Reset();
		});

		// call_ui_resize_screen
		replaceFunction<void(UiResizeArgs*)>(0x1009a5c0, [](UiResizeArgs *args) {
			uiSystems->ResizeViewport(args->rect2.width, args->rect2.height);
		});
		
		// HookUiOptions();

		// Camp
		replaceFunction<BOOL(int)>(0x1010EF00, [](int hoursToRest){
			return uiSystems->GetCamping().Camp(hoursToRest);
		});

		// SetHoursToRestUntilHealed
		replaceFunction<void()>(0x1012EC70, []() {
			uiSystems->GetCamping().SetTimeUntilHealed();
		});
		
		/*static BOOL(__cdecl*orgSetViewport)(int) = replaceFunction<BOOL(int)>(0x10115040, [](int viewport)->BOOL {
				logger->trace("SetViewport: {}", viewport);
				logger->trace("Viewport idx before: {}", temple::GetRef<int>(0x10BD3B44));
				auto result = orgSetViewport(viewport);
				logger->trace("Viewport idx after: {}", temple::GetRef<int>(0x10BD3B44));
				return result;
			});*/
	}

private:

	/*void HookUiOptions() {
		// ui_options_hide
		replaceFunction<void()>(0x10117780, []() {
			uiSystems->GetOptions().Hide();
		});
		// ui_options_is_visible
		replaceFunction<BOOL()>(0x101177d0, []() {
			return uiSystems->GetOptions().IsVisible() ? TRUE : FALSE;
		});
		// ui_options_show
		replaceFunction<void(int)>(0x10119d20, [](BOOL fromMainMenu) {
			uiSystems->GetOptions().Show(fromMainMenu == TRUE);
		});
	}*/

} hooks;
