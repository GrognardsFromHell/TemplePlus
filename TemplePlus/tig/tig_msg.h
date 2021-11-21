
#pragma once

#include <temple/dll.h>

enum class TigMsgType : uint32_t {
	MOUSE = 0,
	WIDGET = 1, // seems to be sent in response to type0 msg, also from int __cdecl sub_101F9E20(int a1, int a2) and sub_101FA410
	TMT_UNK2 = 2,
	EXIT, // may be exit game, queued on WM_CLOSE and WM_QUIT
	CHAR = 4, // arg1 is the character, in Virtual Key terms
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

enum MouseStateFlags : uint32_t
{
	MSF_LMB_CLICK = 0x1,
	MSF_LMB_DOWN = 0x2,
	MSF_LMB_RELEASED = 0x4,
	MSF_LMB_UNK = 0x8,
	MSF_RMB_CLICK = 0x10,
	MSF_RMB_DOWN = 0x20,
	MSF_RMB_RELEASED = 0x40,
	MSF_RMB_UNK = 0x80,
	MSF_MMB_CLICK = 0x100,
	MSF_MMB_DOWN = 0x200,
	MSF_MMB_RELEASED = 0x400,
	MSF_MMB_UNK = 0x800,
	MSF_POS_CHANGE = 0x1000,
	// Sent only 35ms after mouse position has stabilized
	MSF_POS_CHANGE_SLOW = 0x2000,
	MSF_SCROLLWHEEL_CHANGE = 0x4000
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
	uint32_t arg4; // button state flags for mouse events - see MouseStateFlags
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
	uint32_t buttonStateFlags; // button state flags for mouse events - see MouseStateFlags
};

struct TigKeyStateChangeMsg : TigMsgBase // type 5
{
	uint32_t key; // DINPUT KEY
	BOOL down; // false = up, true = down
	uint32_t _unused1;
	uint32_t _unused2;
};

struct TigCharMsg : TigMsgBase // type 4
{
	uint32_t charCode; // VK_* constant
	uint32_t _unused1;
	uint32_t _unused2;
	uint32_t _unused3;
};

enum class TigMsgWidgetEvent : uint32_t
{
	Clicked = 0,
	MouseReleased = 1,
	MouseReleasedAtDifferentButton = 2,
	Entered  = 3,
	Exited = 4,
	Scrolled = 5,
	RightClicked = 6 // added in Temple+
};

struct TigMsgWidget : TigMsgBase // type 1
{
	int widgetId; 
	TigMsgWidgetEvent widgetEventType; // 3 - widget entered; 4 - widget left
	uint32_t x;
	uint32_t y;
};


struct TigMsg : TigMsgBase {
	uint32_t arg1; // x for mouse events
	uint32_t arg2; // y for mouse events
	uint32_t arg3;
	uint32_t arg4; // button state flags for mouse events - see MouseStateFlags

	bool IsWidgetEvent(TigMsgWidgetEvent evtType){
		if (this->type != TigMsgType::WIDGET)
			return false;
		auto widMsg = (TigMsgWidget*)(this);
		return widMsg->widgetEventType == evtType;
	};
};

struct TigMsgGlobalKeyCallback {
	uint32_t keycode; // DirectInput constants
	void(__cdecl *callback)(uint32_t);
};
