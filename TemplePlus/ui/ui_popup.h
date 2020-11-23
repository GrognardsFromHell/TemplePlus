#pragma once
#include "common.h"
#include <tig/tig_msg.h>


struct LgcyWindow;
struct LgcyButton;
struct UiPromptPacket {
	int idx;
	void * unk4;
	const char* bodyText;
	TigRect textRect;
	const char* wndTitle;
	int image;
	int btnNormalTexture;
	int btn2NormalTexture;
	int btn3NormalTexture;
	int btnHoverTexture;
	int btn2HoverTexture;
	int btn3HoverTexture;
	int btnPressedTexture;
	int btn2PressedTexture;
	int btn3PressedTexture;
	int btnDisabledTexture;
	int btn2DisabledTexture;
	int btn3DisabledTexture;
	int(__cdecl* onPopupShow)();
	void(__cdecl* onPopupHide)();
	void(__cdecl* callback)(int popupBtnIdx);
	TigRect wndRect;
	TigRect okRect;
	const char* okBtnText;
	TigRect cancelRect;
	const char* cancelBtnText;
	TigRect textEntryRect;
	int unkA8;
	int unkAC;
	int unkB0;

	UiPromptPacket();
	int Show(int promptIdx, int flag);
	void Reset();
};

const int testSizeofPromptPacket = sizeof(UiPromptPacket); // should be 180 (0xB4)