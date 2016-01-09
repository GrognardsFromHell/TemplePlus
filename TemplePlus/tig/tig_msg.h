
#pragma once

#include <temple/dll.h>

enum class TigMsgType : uint32_t {
	MOUSE = 0,
	WIDGET = 1, // seems to be sent in response to type0 msg, also from int __cdecl sub_101F9E20(int a1, int a2) and sub_101FA410 UiWidgetMouseHandler (0x101F9970)
	TMT_UNK2 = 2,
	EXIT, // may be exit game, queued on WM_CLOSE and WM_QUIT
	CHAR = 4,
	KEYSTATECHANGE = 5,
	/*
		Send once whenever system message are being processed.
		No arguments, just the create time is set to the time
		the messages were processed.
	*/
	UPDATE_TIME = 6,
	TMT_UNK7 = 7,
	KEYDOWN = 8
};

struct TigMsgBase
{
	uint32_t createdMs;
	TigMsgType type;
};



struct TigMsgGeneric : TigMsgBase
{
	uint32_t arg1; // x for mouse events
	uint32_t arg2; // y for mouse events
	uint32_t arg3;
	uint32_t arg4; // button state flags for mouse events
};

struct TigMouseMsg
{
	int x;
	int y;
	int mouseStateField24;
	int flags;
};

struct TigMsgMouse : TigMsgBase // type 0
{
	uint32_t x; 
	uint32_t y;
	uint32_t arg3;
	uint32_t buttonStateFlags; // button state flags for mouse events
};

struct TigMsgWidget : TigMsgBase // type 1
{
	int widgetId; 
	uint32_t widgetEventType; // 3 - widget entered; 4 - widget left
	uint32_t x;
	uint32_t y;
};


struct TigMsg : TigMsgBase {
	uint32_t arg1; // x for mouse events
	uint32_t arg2; // y for mouse events
	uint32_t arg3;
	uint32_t arg4; // button state flags for mouse events
};

struct TigMsgGlobalKeyCallback {
	uint32_t keycode; // DirectInput constants
	void(__cdecl *callback)(uint32_t);
};

struct TigMsgFuncs : temple::AddressTable {
	// Return code of 0 means a msg has been written to msgOut.
	int(__cdecl *Process)(TigMsgBase *msgOut);
	void(__cdecl *Enqueue)(TigMsgBase *msg);
	void(__cdecl *ProcessSystemEvents)();

	TigMsgFuncs() {
		rebase(Process, 0x101DE750);
		rebase(Enqueue, 0x101DE660);
		rebase(ProcessSystemEvents, 0x101DF440);
	}
} ;

extern TigMsgFuncs msgFuncs;

inline void processTigMessages() {
	TigMsg msg;
	while (!msgFuncs.Process(&msg))
		;
}
