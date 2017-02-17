
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// This is required to get "new style" common dialogs like message boxes
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Used to reserve space for loading temple.dll at a fixed address
constexpr DWORD defaultBaseAddr = 0x10000000;
constexpr DWORD defaultImageSize = 0x1EB717E;

using Startup = int(void*);

static void mystrcat(TCHAR *out, TCHAR *in) {
	int offset = lstrlen(out);
	int size = lstrlen(in);
	for (int i = 0; i < size; i++) {
		out[offset + i] = in[i];
	}
	offset += size;
	out[offset] = 0;
}

static void ShowStartupError(TCHAR *message) {
	LPTSTR lpMsgBuf;
	LPTSTR lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen(lpMsgBuf) + lstrlen(message) + 3) * sizeof(TCHAR));

	mystrcat(lpDisplayBuf, message);
	mystrcat(lpDisplayBuf, L": ");
	mystrcat(lpDisplayBuf, lpMsgBuf);

	LocalFree(lpMsgBuf);

	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Startup Error"), MB_OK | MB_ICONERROR);

	LocalFree(lpDisplayBuf);
	ExitProcess(-1);
}

int __stdcall WinMainCRTStartup() {

	// Reserve the space for temple.dll so no other loaded DLLs will conflict with it
	auto reservedMem = VirtualAlloc(reinterpret_cast<void*>(defaultBaseAddr),
		defaultImageSize,
		MEM_RESERVE,
		PAGE_NOACCESS);

	if (!reservedMem) {
		ShowStartupError(L"Unable to reserve memory for loading temple.dll at a fixed base address");
		return -1;
	}

	// Now load TemplePlus.dll which may have maaany dependencies
	auto tpDll = LoadLibrary(L"TemplePlus.dll");
	
	if (!tpDll) {
		ShowStartupError(L"Unable to load TemplePlus.dll");	
		return -1;
	}

	auto startup = (Startup*) GetProcAddress(tpDll, "Startup");

	if (!startup) {
		MessageBox(nullptr, L"Unable to locate Startup function in TemplePlus.dll", L"Startup Error", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
		return -1;
	}

	int result = startup(reservedMem);
	ExitProcess(result);
	return result;
}
