
#pragma once

#include "util/addresses.h"
#include "tig_startup.h"
#include "ui/ui.h"

struct RectColor {
	D3DCOLOR colors[4];

	RectColor() {

	}

	RectColor(D3DCOLOR color) {
		colors[0] = color;
		colors[1] = color;
		colors[2] = color;
		colors[3] = color;
	}
};

/*
	Chroeographs a TIG loading screen. Describes the colors of the progress bar, the
	text messages that should be shown for each step and the primary splash screen image.
*/
struct LoadingSequence {
	ImgFile *image = nullptr;
	const char **messages = nullptr;
	int messagesCount = 0;
	// Moves the bar and text left
	int offsetLeft = 0;
	// Moves the bar and text up
	int offsetUp = 0;
	uint32_t barWidth = 0;
	uint32_t barHeight = 0;
	const RectColor *barBorderColor = nullptr;
	const RectColor *barBackground = nullptr;
	const RectColor *barForeground = nullptr;
	const RectColor *textColor = nullptr;

	LoadingSequence() {
	}
	~LoadingSequence() {
		delete image;
		delete[] messages;
	}

	LoadingSequence(const LoadingSequence&) = delete;
	LoadingSequence &operator =(const LoadingSequence&) = delete;
};

/*
	Functions relating to showing loading screens when the game starts or the map changes.
*/
struct LoadingScreenFuncs : AddressTable {

	/*
		Initializes the loading screen subsystem. Returns 0 on success.
		This is called automatically during the initialization of TIG.
	*/
	int (__cdecl *Init)(const TigConfig *config);

	/*
		Sets the bounds of the loading screen. Used to position the screen. Should be the screen
		width and height. Normally this should only be necessary if the screen size has changed.
	*/
	void (__cdecl *SetBounds)(int width, int height);

	/*
		Pushes a loading sequence onto the stack, which basically means it sets the current
		loading screen. NextStep can then be called to go from step to step in the loading
		sequence and redraw the frame.
		Returns 0 on success, 0x11 if too many loading screens have been pushed. The limit is 25.
		This is odd since pretty much only one loading screen is active at the same time.
	*/
	int (__cdecl *Push)(LoadingSequence *spec);

	/*
		Pops the current loading sequence. Should be called after loading is done.
	*/
	void (__cdecl *Pop)();

	/*
		Goes to the next loading step and redraws the screen.
	*/
	void (__cdecl *NextStep)();

	/*
		Draws the current loading screen. Automatically called as part of NextStep.
	*/
	void(__cdecl *Draw)();

	LoadingScreenFuncs() {
		rebase(Init, 0x101E8260);
		rebase(Draw, 0x101E7F70);
		rebase(SetBounds, 0x101E7EA0);
		rebase(Push, 0x101E8290);
		rebase(Pop, 0x101E7EC0);
		rebase(NextStep, 0x101E82E0);
	}
};

extern LoadingScreenFuncs loadingScreenFuncs;
