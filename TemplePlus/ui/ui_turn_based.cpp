
#include "stdafx.h"

#include "ui_turn_based.h"
#include <temple/dll.h>
#include <util/fixes.h>
#include "common.h"

struct UiTurnBasedAddresses : temple::AddressTable
{
	int * maxInitiativeSmallPortraits; // when this is exceeded, it switches to mini portraits
	int * maxInitiativeMiniPortraits; // at least in terms of available space in two rows; actual limit seems to be 32
	UiTurnBasedAddresses()
	{
		rebase(maxInitiativeSmallPortraits, 0x102F9AF8);
		rebase(maxInitiativeMiniPortraits, 0x102F9AFC);
	}
	
} addresses;

class UiTurnBasedReplacements: TempleFix
{
public: 

	static int(_cdecl*orgSub_101430B0)();
	static int sub_101430B0();

	void apply() override 
	{
		//orgSub_101430B0 = replaceFunction(0x101430B0, sub_101430B0);
	}
} uiTbReplacements;

int(_cdecl*UiTurnBasedReplacements::orgSub_101430B0)();

int UiTurnBasedReplacements::sub_101430B0()
{
	int result = orgSub_101430B0();
	return result;
}

//*****************************************************************************
//* TurnBased
//*****************************************************************************

UiTurnBased::UiTurnBased(int width, int height) {

	static auto ui_turnbased_init = temple::GetPointer<int()>(0x10174d70);
	ui_turnbased_init();
	/*
	LgcyWindow window(0, 0, width, height);
	strcpy(window.name, "intgame_main_window");
	window.render = temple::GetPointer<void(LgcyWidgetId)>(0x10173f70);
	window.handleMessage = temple::GetPointer<BOOL(LgcyWidgetId, TigMsg*)>(0x10174a30);

	auto widgetId = uiManager->AddWindow(window);
	mWidgetId = widgetId;
	uiLegacyManager->SendToBack(widgetId);

	mObjHndl = objHndl::null;*/
}

UiTurnBased::~UiTurnBased() {
	auto shutdown = temple::GetPointer<void()>(0x10173ab0);
	shutdown();
}
void UiTurnBased::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10173a50);
	resize(&resizeArg);
}
void UiTurnBased::Reset() {
	auto reset = temple::GetPointer<void()>(0x10173ac0);
	reset();
}
const std::string &UiTurnBased::GetName() const {
	static std::string name("TurnBased");
	return name;
}
