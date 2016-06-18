
#include "stdafx.h"

#include <util/fixes.h>

#include "ui_system.h"
#include "ui_systems.h"

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

	}

} hooks;
