#include "stdafx.h"
#include "mainloop.h"
#include <temple/dll.h>
#include "mainwindow.h"
#include "tig/tig_msg.h"
#include "tig/tig_mouse.h"
#include "gamesystems/gamesystems.h"
#include "graphics/graphics.h"
#include "ui/ui_render.h"
#include "util/config.h"
#include "tig/tig_font.h"
#include "obj.h"
#include "diag/diag.h"
#include <windowsx.h>

static struct MainLoop : temple::AddressTable {

	void (__cdecl *sub_1002A580)(locXY loc);

	/*
		Queues a timed event for the fidget animations if it's not already
		queued.
	*/
	bool (__cdecl *QueueFidgetAnimEvent)();

	// Not sure what these do
	int (__cdecl *sub_10113CD0)();
	bool (__cdecl *sub_10113D40)(int arg);

	bool (__cdecl *IsMainMenuVisible)();

	objHndl (__cdecl *GetPCGroupMemberN)(int nIdx);

	void (__cdecl *DoMouseScrolling)();
	void (__cdecl *SetScrollDirection)(int direction);

	void (__cdecl *RenderUi)();
	void (__cdecl *RenderMouseCursor)();

	/*
		Since the game buffers are re-created synchronously, this should
		not be necessary...
	*/
	bool* gameBuffersCreated;

	/*
		Submits a Tig message to the ingame message handler (world view basically)
	*/
	void (__cdecl *InGameHandleMessage)(TigMsg& msg);

	MainLoop() {
		rebase(sub_1002A580, 0x1002A580);
		rebase(QueueFidgetAnimEvent, 0x100146C0);

		rebase(sub_10113CD0, 0x10113CD0);
		rebase(sub_10113D40, 0x10113D40);

		rebase(GetPCGroupMemberN, 0x1002B170);
		rebase(IsMainMenuVisible, 0x101157F0);

		rebase(DoMouseScrolling, 0x10001010);
		rebase(SetScrollDirection, 0x10006480);

		rebase(gameBuffersCreated, 0x102AB208);

		rebase(InGameHandleMessage, 0x10114EF0);

		rebase(RenderUi, 0x101F8D10);
		rebase(RenderMouseCursor, 0x101DD330);
	}
} mainLoop;

GameLoop::GameLoop(MainWindow& mainWindow, GameSystems& gameSystems, Graphics& graphics)
	: mMainWindow(mainWindow), mGameSystems(gameSystems), mGameRenderer(graphics) {

	mDiagScreen = std::make_unique<DiagScreen>(graphics);

}

GameLoop::~GameLoop() {
}

/*
	This replaces the main loop that is called by temple_main. Since we already replaced
	temple_main, there is no need to hook this function in temple.dll
*/
void GameLoop::Run() {

	// Is this a center map kind of deal?
	locXY loc;
	if (!graphics->ScreenToTile(400, 300, loc)) {
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

		mGameSystems.AdvanceTime();

		// This locks the cursor to our window if we are in the foreground and it's enabled
		if (!config.windowed && config.lockCursor) {
			mMainWindow.LockCursor();
		}

		RenderFrame();

		// Why does it process msgs AFTER rendering???		
		while (!msgFuncs.Process(&msg)) {
			if (msg.type == TigMsgType::EXIT) {
				quit = true;
				break;
			}

			// Pressing the F10 key toggles the diag screen
			if (msg.type == TigMsgType::KEYSTATECHANGE
				&& msg.arg1 == 0x44
				&& msg.arg2 == 1) {
				mDiagScreen->Toggle();
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

void GameLoop::RenderFrame() {
	graphics->BeginFrame();

	auto device = graphics->device();

	// Set it as the render target
	D3DLOG(device->SetRenderTarget(0, graphics->sceneSurface()));
	D3DLOG(device->SetDepthStencilSurface(graphics->sceneDepthSurface()));

	// Clear the new render target as well
	auto clearColor = D3DCOLOR_ARGB(0, 0, 0, 0);
	D3DLOG(device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0));

	mGameRenderer.Render();
	mainLoop.RenderUi();

	mDiagScreen->Render();

	// Draw Version Number while in Main Menu
	if (mainLoop.IsMainMenuVisible()) {
		RenderVersion();
	}

	mainLoop.RenderMouseCursor(); // This calls the strange render-callback
	mouseFuncs.DrawCursor(); // This draws dragged items

	// Reset the render target
	D3DLOG(device->SetRenderTarget(0, graphics->backBuffer()));
	D3DLOG(device->SetDepthStencilSurface(graphics->backBufferDepth()));

	// Copy from the actual render target to the back buffer and scale / position accordingly
	auto destRect = graphics->sceneRect();
	device->StretchRect(
		graphics->sceneSurface(),
		nullptr,
		graphics->backBuffer(),
		&destRect,
		D3DTEXF_LINEAR
	);

	graphics->Present();
}

void GameLoop::DoMouseScrolling() {

	if (config.windowed && mouseFuncs.MouseOutsideWndGet())
		return;

	POINT mousePt = mouseFuncs.GetPos();
	POINT mmbRef = mouseFuncs.GetMmbReference();
	int scrollDir = -1;

	if (mmbRef.x != -1 && mmbRef.y != -1)
	{
		int dx = mousePt.x - mmbRef.x;
		int dy = mousePt.y - mmbRef.y;
		if ( dx*dx+ dy*dy >= 60)
		{
			if (abs(dy) > 1.70*abs(dx))  // vertical
			{
				if (dy > 0)
					scrollDir = 4;
				else
					scrollDir = 0;

			} else if (abs(dx) > 1.70*abs(dy)) // horizontal
			{
				if (dx > 0)
					scrollDir = 2;
				else
					scrollDir = 6;
			} else  // diagonal
			{
				if (dx > 0)
				{
					if (dy > 0)
						scrollDir = 3;
					else
						scrollDir = 1;
				} else
				{
					if (dy > 0)
						scrollDir = 5;
					else
						scrollDir = 7;
				}
			}
		}
	}
	if (scrollDir != -1)
	{
		SetScrollDirection(scrollDir);
		return;
	}
		


	RECT rect = graphics->sceneRect();

	mousePt.x = (int)round(mousePt.x * graphics->sceneScale());
	mousePt.y = (int)round(mousePt.y * graphics->sceneScale());

	mousePt.x += rect.left;
	mousePt.y += rect.top;


	int scrollMarginV = 2;
	int scrollMarginH = 2;
	if (config.windowed)
	{
		scrollMarginV = 7;
		scrollMarginH = 7;
	}


	if (mousePt.x < rect.left + scrollMarginH) // scroll left
	{
		if (mousePt.y < rect.top + scrollMarginV) // scroll upper left
			scrollDir = 7;
		else if (mousePt.y > rect.bottom - scrollMarginV) // scroll bottom left
			scrollDir = 5;
		else
			scrollDir = 6;
	} 
	else if (mousePt.x > rect.right - scrollMarginH) // scroll right
	{
		if (mousePt.y < rect.top + scrollMarginV) // scroll top right
			scrollDir = 1;
		else if (mousePt.y > rect.bottom - scrollMarginV) // scroll bottom right
			scrollDir = 3;
		else
			scrollDir = 2;
	}
	else // scroll vertical only
	{
		if (mousePt.y < rect.top + scrollMarginV) // scroll up
			scrollDir = 0;
		else if (mousePt.y > rect.bottom - scrollMarginV) // scroll down
			scrollDir = 4;
	}
		
	

	if (scrollDir != -1)
		SetScrollDirection(scrollDir);
	else
	{
		int * asdf = (int*) temple::GetPointer(0x1030730C);
		if (*asdf)
		{
			void(__cdecl* sub1009A5F0)() = (void(__cdecl*)())temple::GetPointer(0x1009A5F0);
			sub1009A5F0();
			*asdf = 0;
		}
	}

		
	// mainLoop.DoMouseScrolling();
}

void GameLoop::SetScrollDirection(int direction)
{
	mainLoop.SetScrollDirection(direction);
}

void GameLoop::RenderVersion() {
	UiRenderer::PushFont(PredefinedFont::ARIAL_10);

	ColorRect textColor(0x7FFFFFFF);
	TigTextStyle style;
	style.textColor = &textColor;

	auto version = GetTemplePlusVersion();
	auto rect = UiRenderer::MeasureTextSize(version, style);
	auto scsca = graphics->sceneScale();
	rect.x = graphics->GetSceneWidth() / scsca - rect.width - 10;
	rect.y = graphics->GetSceneHeight()  / scsca - rect.height - 10;

	UiRenderer::RenderText(version, rect, style);

	UiRenderer::PopFont();
}
