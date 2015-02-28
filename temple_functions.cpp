
#include "temple_functions.h"

_temple_main temple_main = 0;

_print_debug_message print_debug_message = 0;

void init_functions() {

    quint32 imageBase = (quint32)GetModuleHandleA("temple.dll");
    if (!imageBase) {
        qFatal("temple.dll not found in memory space");
    }

    print_debug_message = (_print_debug_message)(imageBase + 0x1E48F0);
}

/*
 *  Simply forward all logging to printf at this point.
 */
void __cdecl hooked_print_debug_message(char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
}

void init_hooks() {
    MH_CreateHook(print_debug_message, hooked_print_debug_message, NULL);
    MH_EnableHook(MH_ALL_HOOKS);
}
