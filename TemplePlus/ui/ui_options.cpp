
#include "stdafx.h"

#include <temple/dll.h>
#include <infrastructure/mesparser.h>

#include "ui_options.h"
#include "ui_legacysystems.h"
#include "gamesystems/gamesystems.h"
#include "anim.h"
#include "ui_systems.h"

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
}

UiOptions::~UiOptions() {
}

void UiOptions::ResizeViewport(const UiResizeArgs& resizeArg) {
}

void UiOptions::Show(bool fromMainMenu) {
	mFromMainMenu = fromMainMenu;
	if (!mVisible) {
		gameSystems->GetAnim().PushDisableFidget();
	}
	uiSystems->GetUtilityBar().HideOpenedWindows(true);
	mVisible = true;
	if (mFromMainMenu) {
		uiSystems->GetUtilityBar().Hide();
	}
}

void UiOptions::Hide() {
	mVisible = false;
	
}

const std::string &UiOptions::GetName() const {
	static std::string name("Options-UI");
	return name;
}
