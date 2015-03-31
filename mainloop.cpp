
#include "stdafx.h"
#include "mainloop.h"
#include "addresses.h"
#include "temple_functions.h"
#include "tig_msg.h"
#include "gamesystems.h"
#include "graphics.h"

static struct MainLoop : AddressTable {
	
	bool (__cdecl *sub_100290C0)(int64_t a, int64_t b, void *pOut);
	void (__cdecl *sub_1002A580)(locXY loc);

	/*
		Queues a timed event for the fidget animations if it's not already
		queued.
	*/
	bool (__cdecl *QueueFidgetAnimEvent)();

	// Not sure what these do
	int(__cdecl *sub_10113CD0)();
	bool(__cdecl *sub_10113D40)(int arg);

	bool (__cdecl *IsMainMenuVisible)();

	objHndl(__cdecl *GetPCGroupMemberN)(int nIdx);

	void (__cdecl *DoMouseScrolling)();

	/*
		Since the game buffers are re-created synchronously, this should
		not be necessary...
	*/
	bool *gameBuffersCreated;

	/*
		Submits a Tig message to the ingame message handler (world view basically)
	*/
	void (__cdecl *InGameHandleMessage)(TigMsg &msg);
	
	MainLoop() {
		rebase(sub_100290C0, 0x100290C0);
		rebase(sub_1002A580, 0x1002A580);
		rebase(QueueFidgetAnimEvent, 0x100146C0);

		rebase(sub_10113CD0, 0x10113CD0);
		rebase(sub_10113D40, 0x10113D40);

		rebase(GetPCGroupMemberN, 0x1002B170);
		rebase(IsMainMenuVisible, 0x101157F0);

		rebase(DoMouseScrolling, 0x10001010);

		rebase(gameBuffersCreated, 0x102AB208);
		
		rebase(InGameHandleMessage, 0x10114EF0);
	}
} mainLoop;

static void DoMouseScrolling();
static void RenderFrame();

/*
	This replaces the main loop that is called by temple_main. Since we already replaced
	temple_main, there is no need to hook this function in temple.dll
*/
void RunMainLoop() {
	
	// Is this a center map kind of deal?
	locXY loc;
	if (!mainLoop.sub_100290C0(400, 300, &loc)) {
		throw TempleException("Initial call to unknown main loop function failed!");
	}
	mainLoop.sub_1002A580(loc);

	mainLoop.QueueFidgetAnimEvent();

	TigMsg msg;
	auto quit = false;
	while (!quit) {
		
		// Read user input and external system events (such as time)
		msgFuncs.ProcessSystemEvents();
		while (!*mainLoop.gameBuffersCreated) {
			Sleep(250);
			msgFuncs.ProcessSystemEvents();
		}

		gameSystemFuncs.AdvanceTime();

		RenderFrame();
				
		// Why does it process msgs AFTER rendering???		
		while (!msgFuncs.Process(&msg)) {
			if (msg.type == TigMsgType::EXIT) {
				quit = true;
				break;
			}

			// I have not found any place where message type 7 is queued,
			// so i removed the out of place re-rendering of the game frame

			if (!mainLoop.IsMainMenuVisible()) {
				mainLoop.InGameHandleMessage(msg);
			}

			auto unk = mainLoop.sub_10113CD0();
			if (mainLoop.sub_10113D40(unk)) {
				DoMouseScrolling();
			}
		}	
	}

}

static struct RenderFuncs : AddressTable {

	void (__cdecl *RenderUi)();
	void (__cdecl *RenderMouseCursor)();
	void (__cdecl *RenderGameSystems)();

	RenderFuncs() {
		rebase(RenderUi, 0x101F8D10);
		rebase(RenderMouseCursor, 0x101DD330);
		rebase(RenderGameSystems, 0x10002650);
	}
} renderFuncs;

// TODO: hook this?
static void RenderFrame() {
	graphics.BeginFrame();

	renderFuncs.RenderGameSystems();
	renderFuncs.RenderUi();
	renderFuncs.RenderMouseCursor();
	
	graphics.Present();
}

void DoMouseScrolling() {
	// TODO: This would be the place to implement better scrolling in windowed mode
	mainLoop.DoMouseScrolling();
}
