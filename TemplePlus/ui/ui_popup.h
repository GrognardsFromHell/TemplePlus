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

	void Reset();
};

const int testSizeofPromptPacket = sizeof(UiPromptPacket); // hsould be 180 (0xB4)

struct UiPromptListEntry {
	int flags; // 1 - execute callback after reseting the prompt (otherwise does so before)
	int isActive;
	LgcyWindow * wnd;
	LgcyButton * btns[3];
	UiPromptPacket prompt;
	void ResetWnd();
};


class UiPopup
{
	friend class UiPopupReplacement;
public:

	/*
	  buttonTextType: 0 for Okay / Cancel, 1 for Yes/No
	  haven't seen the second callback actually used, might be debug?
	*/
	int VanillaPopupShow(const char *bodyText, const char *title, int buttonTextType, int(__cdecl *callback)(int), int(__cdecl *optionalCallback)(int)); 

	int FindPopupBtnIdx(int widId);
	void ExecuteCallback(int popupIdx, int btnIdx);
	
	UiPromptListEntry & GetCurPopup();

protected:
	BOOL UiPopupMsg(int widId, TigMsg* msg);
	BOOL UiPopupWndMsg(int widId, TigMsg* msg);
	int GetCurrentPopupIdx();
	void SetCurrentPopupIdx(int popupIdx);
	UiPromptListEntry & GetPopupByType(int popupType);
};