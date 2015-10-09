
#pragma once

#include <temple/dll.h>

enum class TigMsgType : uint32_t {
	MOUSE = 0,
	TMT_UNK1 = 1, // seems to be sent in response to type0 msg, also from int __cdecl sub_101F9E20(int a1, int a2) and sub_101FA410
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

struct TigMsg {
	uint32_t createdMs;
	TigMsgType type;
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
	int(__cdecl *Process)(TigMsg *msgOut);
	void(__cdecl *Enqueue)(TigMsg *msg);
	void(__cdecl *ProcessSystemEvents)();

	TigMsgFuncs() {
		rebase(Process, 0x101DE750);
		rebase(Enqueue, 0x101DE660);
		rebase(ProcessSystemEvents, 0x101DF440);
	}
};

extern TigMsgFuncs msgFuncs;

inline void processTigMessages() {
	TigMsg msg;
	while (!msgFuncs.Process(&msg))
		;
}
