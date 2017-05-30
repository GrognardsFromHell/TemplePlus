
#include "stdafx.h"
#include "util/fixes.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"


class UiUtilityBarHooks : TempleFix
{
	void apply() override{
		
		static int (__cdecl*orgUtilityBarShow)() = replaceFunction<int(__cdecl)()>(0x1010EE80, [](){
			
			uiSystems->GetDM().ShowButton();
			return orgUtilityBarShow();
		});

		static int(__cdecl*orgUtilityBarHide)() = replaceFunction<int(__cdecl)()>(0x1010EEC0, []() {	
			uiSystems->GetDM().HideButton();
			return orgUtilityBarHide();
		});

	}
} uiUtilityBarHooks;
