#pragma once
#include "common.h"
#include <tig/tig_msg.h>


struct WidgetType1;
struct WidgetType2;
struct UiPromptPacket {
	int idx;
	void * unk4;
	const char* bodyText;
	TigRect textRect;
	const char* wndTitle;
	int image;
	int texture0;
	int texture1;
	int texture2;
	int btnHoverTexture;
	int texture4;
	int texture5;
	int btnPressedTexture;
	int texture7;
	int texture8;
	int btnDisabledTexture;
	int texture10;
	int field50;
	int(__cdecl* someCallback)();
	void(__cdecl* renderFuncMaybe)();
	void(__cdecl* callback)(int);
	TigRect wndRect;
	TigRect okRect;
	const char* okBtnText;
	TigRect cancelRect;
	const char* cancelBtnText;
	TigRect textEntryRect;
	int unkA8;
	int unkAC;
	int unkB0;
};

const int testSizeofPromptPacket = sizeof(UiPromptPacket); // hsould be 180 (0xB4)

struct UiPromptListEntry {
	int flags; // 1 - execute callback after reseting the prompt (otherwise does so before)
	int isActive;
	WidgetType1 * wnd;
	WidgetType2 * btn1;
	WidgetType2 * btn2;
	WidgetType2 * btn3;
	UiPromptPacket prompt;
};

class UiPopup
{
	friend class UiPopupReplacement;
public:



	int FindPopupBtnIdx(int widId);
	void ExecuteCallback(int popupIdx, int btnIdx);
	
	UiPromptListEntry & GetCurPopup();

protected:
	BOOL UiPopupMsg(int widId, TigMsg* msg);
	BOOL UiPopupWndMsg(int widId, TigMsg* msg);
	int GetCurrentPopupIdx();
};