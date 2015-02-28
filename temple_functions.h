#ifndef TEMPLE_FUNCTIONS_H
#define TEMPLE_FUNCTIONS_H

#include "system.h"

// Contains the function definitions for stuff found in temple.dll that we may want to call or override.

typedef int (__cdecl *_temple_main)(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char *lpCommandLine, int nCmdShow);
extern _temple_main temple_main;

typedef void (__cdecl *_print_debug_message)(char *pPrintfFormat, ...);
extern _print_debug_message print_debug_message;

void init_functions();
void init_hooks();

#endif // TEMPLE_FUNCTIONS_H

