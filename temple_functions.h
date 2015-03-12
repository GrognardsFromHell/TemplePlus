
#pragma once

#include "stdafx.h"
#include "addresses.h"

// Contains the function definitions for stuff found in temple.dll that we may want to call or override.

extern "C"
{
	int __declspec(dllimport) __cdecl temple_main(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCommandLine, int nCmdShow);
}

struct TempleFuncs : AddressTable {
	void (*ProcessSystemEvents)();

	void rebase(Rebaser rebase) override {
		rebase(ProcessSystemEvents, 0x101DF440);
	}
};

extern TempleFuncs templeFuncs;

// Functions used to init subsystems

// Observed in window mode: 0x11024
// 0x4 seems to be the default (seems to be VSYNC)
// 0x20 is windowed
// 0x1000 is unknown
// 0x10000 means anti aliasing is turned on
enum StartupFlag
{
	SF_FPS = 0x1, // -fps
	SF_VSYNC = 0x4,
	SF_NOSOUND = 0x2000, // -nosound
	SF_DOUBLEBUFFER = 0x2, // -doublebuffer
	SF_ANIMCATCHUP, // -animcatchup (stored elsewhere)
	SF_ANIMDEBUG, // -animdebug (not supported)
	SF_NORANDOM,// -norandom stored in 10BDDD9C
	SF_NONMSMOUSEZ, // -nonmsmousez (stores 0 in 10300974)
	SF_MOUSEZ, // -msmousez (stores 1 in 10300974)
	SF_2680, // -2680 (stores 1 in 103072E0)
	SF_0897, // -0897 (stores 2 in 103072E0)
	SF_4637, // -4637 (stores 3 in 103072E0)
	SF_PATHLIMIT, // -pathlimitNN (stored in 102AF7C0, max seems to be 35, default 10)
	SF_SHADOW_POLY, // -shadow_poly, shadow_debug_flag = 1
	SF_SHADOW_MAP, // -shadow_map, shadow_debug_flag = 2
	SF_SHADOW_BLOBBY, // -shadow_blobby, shadow_debug_flag = 0
	SF_WINDOW = 0x20, // -window
	SF_GEOMETRY, // -geometry<X>x<Y> sets window width + height
	SF_MAXREFRESH, // -maxrefreshNNN apparently sets a framelimit, default=100
	SF_ANTIALIASING = 0x10000, // -noantialiasing inverts this
	SF_MIPMAPPING = 0x20000, // -mipmapping

	SF_FLAG_uint32_t = 0x7FFFFFFF
};

// 19 values total (guessed from memset 0 at start of main method)
struct TempleStartSettings
{
	int flags;
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	HINSTANCE hinstance;
	uint32_t unk7;
	uint32_t unk8;
	WNDPROC wndproc;
	bool (__cdecl *windowMessageFilter)(MSG *msg);
	void* someOtherFunc;
	const char* soundSystem;
	uint32_t minTexWidth;
	uint32_t minTexHeight;
	uint32_t framelimit;
	const char* windowTitle;
	uint32_t callback1;
	uint32_t callback2;
};

typedef int (__cdecl *_tig_init)(TempleStartSettings* settings);

void init_functions();
void init_hooks();
