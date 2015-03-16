
#pragma once

#include "addresses.h"

struct Widget {
	uint32_t type;
	uint32_t size;
	int parentId;
	int widgetId;
	char name[64];
	uint32_t widgetFlags;
	int x;
	int y;
	int xrelated;
	int yrelated;
	uint32_t width;
	uint32_t height;
	uint32_t field_6c;
	uint32_t field_70;
	uint32_t render; // Function pointer
	uint32_t handleMessage; // Function pointer
};

/*
	Type: 1
	Size: 660
	Examples: mm_ui.c:1057
*/
struct WidgetType1 : public Widget {
	int children[128];
	uint32_t field_27c;
	uint32_t childrenCount;
	int windowId;
	uint32_t field_288;
	uint32_t field_28c;
	uint32_t field_290;
};

/*
	Type: 2
	Size: 188
	Examples: charmap_ui.c:203, options_ui.c:1342
*/
struct WidgetType2 : public Widget {
};

/*
	Type: 3
	Size: 176
	Only Example: wft\wft_scrollbar.c:138
*/
struct WidgetType3 : public Widget {
};

struct ActiveWidgetListEntry {
	Widget *widget;
	const char *sourceFilename;
	uint32_t sourceLine;
	ActiveWidgetListEntry *next;
};

/*
	Tracks all active widgets with information about where they come frome.
*/
extern GlobalPrimitive<ActiveWidgetListEntry*, 0x10EF68DC> activeWidgetAllocList;

/*
	The list of all active widgets
*/
extern GlobalPrimitive<Widget**, 0x10EF68E0> activeWidgets;
extern GlobalPrimitive<int, 0x10EF68D8> activeWidgetCount;
