#pragma once
#include "common.h"
#include <tig/tig_msg.h>


struct WidgetType1;
struct WidgetType2;

class UiSlider
{
	friend class SliderHooks;

	int GetSliderChosenAmount();
	void SliderHide();
	void SliderCallbackExecute(int amount);
};