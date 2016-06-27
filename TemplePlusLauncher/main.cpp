
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// This is required to get "new style" common dialogs like message boxes
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int __declspec(dllimport) temple_main(HINSTANCE hInstance, LPSTR lpCmdLine, void *reservedDllMem);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	// The following code will cause temple.dll to be rebased
	/*VirtualAlloc(reinterpret_cast<void*>(0x10000000),
	1,
	MEM_RESERVE,
	PAGE_NOACCESS);*/

	/**
	 * This is needed (sadly) to prevent temple.dll from being rebased to a different loading
	 * offset. If we do not pre-allocate this area of memory, the OS might load other DLLs
	 * to the same region and cause temple.dll to be loaded somewhere else.
	 * Since some of the older DLL hacks failed to update the relocation tables, they will crash
	 * if the DLL is loaded at another offset.
	 */
	auto reservedMem = VirtualAlloc(reinterpret_cast<void*>(0x10000000),
		0x1EB717E,
		MEM_RESERVE,
		PAGE_NOACCESS);

	return temple_main(hInstance, lpCmdLine, reservedMem);

}
