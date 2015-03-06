#include "system.h"

#include "graphics.h"
#include "temple_functions.h"
#include "addresses.h"

#include "d3d8/d3d8.h"
#include "d3d8to9/d3d8to9.h"

/*
 * Size being cleared is 4796 byte in length
 * Start @ 0x1E74580
 */
struct VideoData
{
	HINSTANCE hinstance;
	HWND hwnd;
	IDirect3D8* d3d;
	IDirect3DDevice8* d3dDevice;
	D3DCAPS8 d3dCaps;
	char padding[124];
	DWORD unk2;
	D3DGAMMARAMP gammaRamp1;
	D3DGAMMARAMP gammaRamp2;
	float gammaRelated1; // Default = 1.0
	int gammaSupported; // Seems to be a flag 1/0
	float gammaRelated2;
	RECT screenSizeRect; // Seems to never be read from anywhere
	char junk[0x400];
	// Junk starts @ 11E75300
	// Resumes @ 11E75700
	DWORD current_bpp;
	DWORD unk3; // seems to never be read
	DWORD unk4;
	DWORD dword_11E7570C;
	D3DMATRIX dword_11E75710;
	DWORD dword_11E75750;
	DWORD dword_11E75754;
	DWORD dword_11E75758;

	DWORD width; // @ 11E7575C
	DWORD height; // @ 11E75760
	float halfWidth;
	float halfHeight;
	DWORD current_width;
	DWORD current_height;
	DWORD adapter;
	DWORD mode; // check d3d typdef for this
	D3DFORMAT adapterformat;
	DWORD current_refresh;
	IDirect3DVertexBuffer8* vertexBuffer;
	D3DMATRIX stru_11E75788;
};

VideoData* video;

_tig_init video_startup_org = 0;
int* adapter = 0;

typedef boolean (__fastcall *_init_renderstates)(Settings* settings);
_init_renderstates init_renderstates = 0;

static bool create_window(Settings* settings)
{
	LOG(info) << "Struct @ 0x" << std::hex << (DWORD)(&video->stru_11E75788);

	bool windowed = (settings->flags & 0x20) != 0;
	bool unknownFlag = (settings->flags & 0x100) != 0;

	video->hinstance = settings->hinstance;

	WNDCLASSA wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSA));
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = (WNDPROC) temple_address<0x101DE9A0>(); // tig_wndproc
	wndClass.hInstance = video->hinstance;
	wndClass.hIcon = LoadIconA(video->hinstance, "TIGIcon");
	wndClass.hCursor = LoadCursorA(0, MAKEINTRESOURCEA(IDC_ARROW));
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = "TIGClass";

	if (!RegisterClassA(&wndClass))
	{
		return false;
	}

	auto screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	auto screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

	RECT windowRect;
	HMENU menu;
	DWORD dwStyle;
	DWORD dwExStyle;

	if (!windowed)
	{
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = screenWidth;
		windowRect.bottom = screenWidth;
		menu = 0;
		dwStyle = WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		memcpy(&video->screenSizeRect, &windowRect, sizeof(RECT));
	}
	else
	{
		// Apparently this flag controls whether x,y are preset from the outside
		if (unknownFlag)
		{
			windowRect.left = settings->x;
			windowRect.top = settings->y;
			windowRect.right = settings->x + settings->width;
			windowRect.bottom = settings->y + settings->height;
		}
		else
		{
			windowRect.left = (screenWidth - settings->width) / 2;
			windowRect.top = (screenHeight - settings->height) / 2;
			windowRect.right = windowRect.left + settings->width;
			windowRect.bottom = windowRect.top + settings->height;
		}
		menu = LoadMenuA(settings->hinstance, "TIGMenu");
		dwStyle = WS_OVERLAPPEDWINDOW; //  WS_CAPTION | WS_CLIPCHILDREN | WS_SYSMENU | WS_GROUP;
		dwExStyle = 0;

		// Apparently 0x80 means window rect isn't adjusted. is this a half-implemented borderless fullscreen?
		if (!(settings->flags & 0x80))
		{
			AdjustWindowRectEx(&windowRect, dwStyle, menu != 0, 0);
			// TODO: Adjust back
			//            v13 = Rect.right - Rect.left - v1->width;
			//            LODWORD(v13) = v13 - HIDWORD(v13);
			//            v14 = Rect.bottom - Rect.top - v1->height;
			//            v3 = ((signed int)v13 >> 1) + Rect.left;
			//            v4 = ((signed int)v13 >> 1) + Rect.right;
			//            v15 = v14 / 2 + Rect.top;
			//            v5 = v14 / 2 + Rect.bottom;
			//            Rect.left += (signed int)v13 >> 1;
			//            Rect.right += (signed int)v13 >> 1;
			//            Rect.top += v14 / 2;
			//            Rect.bottom += v14 / 2;
		}
	}

	dwStyle |= WS_VISIBLE;
	video->width = settings->width;
	video->height = settings->height;

	temple_set<0x10D24E0C>(0);
	temple_set<0x10D24E10>(0);
	temple_set<0x10D24E14>(settings->width);

	const char* windowTitle;
	// Apparently is a flag that indicates a custom window title
	if (settings->flags & 0x40)
	{
		windowTitle = settings->windowTitle;
	}
	else
	{
		windowTitle = "Temple of Elemental Evil - Cirlce of Eight";
	}

	DWORD windowWidth = windowRect.right - windowRect.left;
	DWORD windowHeight = windowRect.bottom - windowRect.top;
	video->hwnd = CreateWindowExA(
		dwExStyle,
		"TIGClass",
		windowTitle,
		dwStyle,
		windowRect.left,
		windowRect.top,
		windowWidth,
		windowHeight,
		0,
		menu,
		settings->hinstance,
		0);

	if (video->hwnd)
	{
		RECT clientRect;
		GetClientRect(video->hwnd, &clientRect);
		video->current_width = clientRect.right - clientRect.left;
		video->current_height = clientRect.bottom - clientRect.top;

		// Scratchbuffer size sometimes doesn't seem to be set by ToEE itself
		temple_set<0x10307284>(video->current_width);
		temple_set<0x10307288>(video->current_height);

		return true;
	}

	return false;
}

int __cdecl video_startup(Settings* settings)
{
	memset(video, 0, 4796);

	bool windowed = (settings->flags & 0x20) != 0;

	if (windowed)
	{
		if (!settings->wndproc)
		{
			return 12;
		}
		temple_set<0x10D25C38>(settings->wndproc);
	}

	*adapter = 0;

	// create window call
	if (!create_window(settings))
	{
		return 17;
	}

	if (!init_renderstates(settings))
	{
		video->hwnd = 0;
		return 17;
	}

	video->width = settings->width;
	video->height = settings->height;
	video->halfWidth = video->width * 0.5f;
	video->halfHeight = video->height * 0.5f;

	if (settings->flags & SF_FPS)
	{
		/*
			dword_10D250EC = 1;
			dword_10D24DD8 = 8;
			dword_10D24DDC = -1;
			dword_10D24DE4 = (int)&unk_103008F4;
			dword_10D24DEC = (int)&unk_10300904;
			dword_10D24DF0 = (int)&unk_103008F4;
			dword_10D24DE8 = (int)&unk_103008F4;
			dword_10D24DB0 = 0;
			dword_10D24DB8 = 2;
			dword_10D24DBC = 0;
			dword_10D24DB4 = 5;
			*/
	}
	else
	{
		temple_set<0x10D250EC>(0);
	}

	// Seems always enabled in default config and never read
	temple_set<0x11E7570C, int>((settings->flags & 4) != 0);

	video->current_bpp = settings->bpp;

	ShowCursor(windowed); // Show cursor in windowed mode

	temple_set<0x10D250E0, int>(0);
	temple_set<0x10D250E4, int>(1);
	temple_set<0x10300914, int>(-1);
	temple_set<0x10D2511C, int>(0);

	uint32_t v3 = 0x10D24CAC;
	do
	{
		temple_set(v3, 0);
		v3 += 12;
	}
	while (v3 < 0x10D24D6C);

	v3 = 0x10D24C8C;
	do
	{
		temple_set(v3, 0);
		v3 += 8;
	}
	while (v3 < 0x10D24CAC);

	temple_set<0x10D25134>(settings->callback1);
	temple_set<0x10D25138>(settings->callback2);

	memcpy(temple_address<0x11E75840>(), settings, 0x4C);

	return 0;
}

/*
	The original function could not handle PCs with >4GB ram
*/
void get_system_memory(int* totalMem, int* availableMem)
{
	MEMORYSTATUS status;

	GlobalMemoryStatus(&status);

	// Max 1GB because of process space limitations
	*totalMem = min(1024 * 1024 * 1024, status.dwTotalPhys);
	*availableMem = min(1024 * 1024 * 1024, status.dwAvailPhys);
}

void hook_graphics()
{
	hook_directx();

	video = (VideoData*)temple_address<0x11E74580>(); // Goes until 1E7583C
	adapter = (int*)temple_address<0x11E75774>(); // actually part of video!
	init_renderstates = (_init_renderstates)temple_address<0x101DAFB0>();

	MH_CreateHook(temple_address<0x101E0750>(), get_system_memory, NULL);

	// We hook the entire video subsystem initialization function
	MH_CreateHook(temple_address<0x101DC6E0>(), video_startup, (LPVOID*)&video_startup_org);
}
