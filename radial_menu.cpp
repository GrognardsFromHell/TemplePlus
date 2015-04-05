#include "stdafx.h"
#include "radial_menu.h"

GlobalPrimitive<uint32_t, 0x1026C67B> menuArg;
RadialFuncs radialFuncs;

void RadialMenuStructInit(RadialMenuStruct * radmenu)
{
	memset(radmenu, 0, sizeof(RadialMenuStruct));
	radmenu->field0 = menuArg.ptr();
	radmenu->field38 = radialFuncs.D20ASthg_sub_100F0110;
	radmenu->field20 = -1;
}