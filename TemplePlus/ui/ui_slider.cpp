
#include "stdafx.h"
#include <util/fixes.h>
#include "common.h"
#include <config/config.h>
#include <temple/dll.h>
#include "ui_slider.h"
#include "platform/windows.h"


UiSlider uiSlider;

static class SliderHooks : public TempleFix
{
public: 

	void apply() override 
	{

		static BOOL(__cdecl* orgSliderWndMsg)(int, TigMsg* ) = replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10161EC0, [](int widId, TigMsg* msg){
			if (msg->type == TigMsgType::CHAR){
				auto keyPress = msg->arg1;
				if (keyPress == VK_RETURN){
					uiSlider.SliderCallbackExecute(uiSlider.GetSliderChosenAmount());
					uiSlider.SliderHide();
					return TRUE;
				}
				
			}
			return orgSliderWndMsg(widId, msg);
		});

		// if is Co8, adapt the button positions to the graphical modifications made by Co8
		if (temple::Dll::GetInstance().HasCo8Hooks())
		{

			// ui_slider
			int writeval;
			writeval = 328 + 73;
			write(0x102FB6D8, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102FB6DC, &writeval, sizeof(int));

			writeval = 452 + 73;
			write(0x102FB6E8, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102FB6EC, &writeval, sizeof(int));



			// radial menu slider accept
			writeval = 328 + 73;
			write(0x102F9568, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102F956C, &writeval, sizeof(int));
			// decline
			writeval = 452 + 73;
			write(0x102F9578, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102F957C, &writeval, sizeof(int));
		}
	}
} hooks;

int UiSlider::GetSliderChosenAmount(){
	return temple::GetRef<int>(0x10BF007C);
}

void UiSlider::SliderHide(){
	temple::GetRef<void(__cdecl)()>(0x10161160)();
}

void UiSlider::SliderCallbackExecute(int amount){

	auto cb = temple::GetRef<void(__cdecl*)(int)>(0x10BF0364);
	if (cb != nullptr)
		cb(amount);
}
