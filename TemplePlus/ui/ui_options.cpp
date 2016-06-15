
#include "stdafx.h"

#include <temple/dll.h>
#include <infrastructure/mesparser.h>

#include "ui_options.h"
#include "ui_legacysystems.h"

enum UiOptionsTextKeys {
	// Titles of tabs in the options dialog
	TEXT_TAB_VIDEO = 0,
	TEXT_TAB_AUDIO,
	TEXT_TAB_PREF,
	TEXT_TAB_CONTROLS,

	TEXT_BTN_ACCEPT,
	TEXT_BTN_CANCEL,
	
	TEXT_DLG_HEADER,

	TEXT_SHADOW_SIMPLE = 16,
	TEXT_SHADOW_GEOMETRY,
	TEXT_SHADOW_MAPS
};

UiOptions::UiOptions(int width, int height) {

	static auto ui_options_init_adapters = temple::GetPointer<void()>(0x101177f0);
	ui_options_init_adapters();

	mText = MesFile::ParseFile("mes/options_text.mes");

	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1011b640);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Options-UI");
	}
}

UiOptions::~UiOptions() {
	auto shutdown = temple::GetPointer<void()>(0x1011b0e0);
	shutdown();
}

void UiOptions::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10117540);
	resize(&resizeArg);
}

const std::string &UiOptions::GetName() const {
	static std::string name("Options-UI");
	return name;
}
