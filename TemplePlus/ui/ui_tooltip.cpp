#include "stdafx.h"
#include "ui.h"
#include "ui_tooltip.h"


UiTooltips tooltips;

TooltipStyle& UiTooltips::GetStyle(int idx) const {
	auto& ttStyles = temple::GetRef<TooltipStyle*>(0x10BDE3C8);
	return ttStyles[idx];
}

const char* UiTooltips::GetTooltipString(int line) const
{
	MesLine mesline;
	mesline.key = line;
	auto mesHnd = temple::GetRef<MesHandle>(0x10BDDE60);
	mesFuncs.GetLine_Safe(mesHnd, &mesline);
	return mesline.value;
}
