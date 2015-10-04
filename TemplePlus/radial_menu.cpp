#include "stdafx.h"
#include "radial_menu.h"
#include "dispatcher.h"
#include "obj.h"
#include "util/fixes.h"

temple::GlobalPrimitive<uint32_t, 0x1026C67B> menuArg;
RadialFuncs radialFuncs;


struct RadialMenuReplacements : TempleFix
{
	const char* name() override {
		return "Radial Menu Replacements";
	}

	void apply() override {
		replaceFunction(0x1004D1F0, _RadialMenuUpdate);

	}
};

RadialMenuReplacements radMenuReplace;


void _RadialMenuUpdate(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1)
	{
		if (objects.IsPlayerControlled(objHnd))
		{
			objects.dispatch.DispatcherProcessor(dispatcher, dispTypeRadialMenuEntry, 0, nullptr);
			radialFuncs.sub_100F0A70(objHnd);
		}
	}
}

void RadialMenuStructInit(RadialMenuStruct * radmenu)
{
	memset(radmenu, 0, sizeof(RadialMenuStruct));
	radmenu->field0 = menuArg.ptr();
	radmenu->field38 = radialFuncs.D20ASthg_sub_100F0110;
	radmenu->field20 = -1;
}