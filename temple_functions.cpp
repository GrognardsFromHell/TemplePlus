
#include "stdafx.h"
#include "addresses.h"
#include "temple_functions.h"
#include "graphics.h"
#include "tig_mouse.h"

TempleFuncs templeFuncs;

void init_functions()
{
	templeImageBase = static_cast<void*>(GetModuleHandleA("temple.dll"));
	if (!templeImageBase)
	{
		LOG(error) << "temple.dll not found in memory space";
	}

	AddressInitializer::performRebase();
}

/*
 *  Simply forward all logging to printf at this point.
 */
void __cdecl hooked_print_debug_message(char* format, ...)
{
	static char buffer[32 * 1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	int len = strlen(buffer) - 1;
	// Strip trailing newlines
	while (len > 0 && (buffer[len] == '\n' || buffer[len] == '\r' || buffer[len] == ' '))
	{
		buffer[len] = 0;
		--len;
	}
	if (buffer[0] == 0)
	{
		return; // Trimmed completely
	}

	LOG(info) << buffer;
}

void init_hooks()
{
	LOG(info) << format("Base offset for temple.dll memory is 0x%x") % templeImageBase;

	temple_set<0x10EED638>(1); // Debug message enable
	MH_CreateHook(temple_address<0x101E48F0>(), hooked_print_debug_message, NULL);

	hook_graphics();
	hook_mouse();

	MH_EnableHook(MH_ALL_HOOKS);
}
