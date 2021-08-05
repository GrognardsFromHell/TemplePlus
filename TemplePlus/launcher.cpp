
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

// This is required to get "new style" common dialogs like message boxes
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int LaunchGame(HINSTANCE hInstance, LPSTR lpCmdLine, void *reservedTempleArea,
               size_t reservedTempleAreaSize);

class Launcher {
public:
    Launcher() noexcept {
        reservedArea = VirtualAlloc((void *) (RESERVED_TEMPLE_AREA_START),
                                    RESERVED_TEMPLE_AREA_SIZE,
                                    MEM_RESERVE,
                                    PAGE_NOACCESS);
    }

    int launch(HINSTANCE hInstance, PSTR lpCmdLine) {
        return LaunchGame(hInstance, lpCmdLine, reservedArea, RESERVED_TEMPLE_AREA_SIZE);
    }

private:
    static constexpr int RESERVED_TEMPLE_AREA_START = 0x10000000;
    static constexpr int RESERVED_TEMPLE_AREA_SIZE = 0x1EB717E;

    // Reserve memory for temple.dll before anything else is loaded
    LPVOID reservedArea = nullptr;
};

// This MUST be defined on the top-level to be called as early as possible in the
// CRT startup process.
static Launcher launcher;

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    return launcher.launch(hInstance, lpCmdLine);
}
