#pragma once
#include "common.h"
#include <tig/tig_msg.h>


struct LgcyWindow;
struct LgcyButton;

class UiSlider
{
	friend class SliderHooks;

	int GetSliderChosenAmount();
	void SliderHide();
	void SliderCallbackExecute(int amount);
};