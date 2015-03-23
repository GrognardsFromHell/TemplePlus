
#include "stdafx.h"
#include "ui.h"

GlobalPrimitive<ActiveWidgetListEntry*, 0x10EF68DC> activeWidgetAllocList;
GlobalPrimitive<Widget**, 0x10EF68E0> activeWidgets;
GlobalPrimitive<int, 0x10EF68D8> activeWidgetCount;

UiFuncs uiFuncs;
