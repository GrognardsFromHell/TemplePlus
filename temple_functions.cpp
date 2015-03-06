
#include "addresses.h"
#include "temple_functions.h"
#include "graphics.h"

void init_functions() {
	templeImageBase = (void*)GetModuleHandleA("temple.dll");
	if (!templeImageBase) {
        LOG(error) << "temple.dll not found in memory space";
    }
}

/*
 *  Simply forward all logging to printf at this point.
 */
void __cdecl hooked_print_debug_message(char *format, ...) {
	char message[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(message, 4096, format, args);
	LOG(info) << message;
}

void init_hooks() {
    LOG(info) << "Base offset for temple.dll memory is 0x" << std::hex << templeImageBase;

	temple_set<0x10EED638>(1); // Debug message enable

	MH_CreateHook(temple_address<0x101E48F0>(), hooked_print_debug_message, NULL);
    hook_graphics();
    MH_EnableHook(MH_ALL_HOOKS);
}
