#pragma once


struct TooltipStyle;

class UiTooltips {
public:
	TooltipStyle& GetStyle(int idx) const;
	const char* GetTooltipString(int line) const; // gets line from tooltip_ui_strings.mes
};

extern UiTooltips tooltips;