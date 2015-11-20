#include "stdafx.h"
#include "mainloop.h"
#include <temple/dll.h>
#include "mainwindow.h"
#include "tig/tig_msg.h"
#include "tig/tig_mouse.h"
#include "tig/tig_startup.h"
#include "gamesystems/gamesystems.h"
#include "ui/ui_render.h"
#include "util/config.h"
#include "tig/tig_font.h"
#include "obj.h"
#include "diag/diag.h"
#include <graphics/device.h>
#include "../Infrastructure/include/infrastructure/stopwatch.h"

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

		rebase(gameBuffersCreated, 0x102AB208);

		rebase(InGameHandleMessage, 0x10114EF0);

		rebase(RenderUi, 0x101F8D10);
		rebase(RenderMouseCursor, 0x101DD330);
	}
} mainLoop;

GameLoop::GameLoop(TigInitializer& tig, GameSystems& gameSystems)
	: mTig(tig),
	  mGameSystems(gameSystems),
	  mGameRenderer(tig, mGameSystems) {

	mDiagScreen = std::make_unique<DiagScreen>(tig.GetRenderingDevice());
}

GameLoop::~GameLoop() {
}

/*
	This replaces the main loop that is called by temple_main. Since we already replaced
	temple_main, there is no need to hook this function in temple.dll
*/
void GameLoop::Run() {
	auto& camera = mTig.GetRenderingDevice().GetCamera();
	// Is this a center map kind of deal?
	auto worldPos = camera.ScreenToWorld(400, 300);
	locXY loc{ (uint32_t)(worldPos.x / INCH_PER_TILE), (uint32_t)(worldPos.z / INCH_PER_TILE) };
	mainLoop.sub_1002A580(loc);

	mainLoop.QueueFidgetAnimEvent();
	
	TigMsg msg;
	auto quit = false;
	static int fpsCounter = 0;
	static int timeElapsed = 0;
	while (!quit) {

		Stopwatch sw1;

		// Read user input and external system events (such as time)
		msgFuncs.ProcessSystemEvents();
		while (!*mainLoop.gameBuffersCreated) {
			Sleep(250);
			msgFuncs.ProcessSystemEvents();
		}

		mGameSystems.AdvanceTime();

		// This locks the cursor to our window if we are in the foreground and it's enabled
		if (!config.windowed && config.lockCursor) {
			tig->GetMainWindow().LockCursor();
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
			timeElapsed+= sw1.GetElapsedMs();
			if (fpsCounter++ >= 100)
			{
				fpsCounter = 0;
				logger->info("Time per frame: {}ms ,  FPS: {}", timeElapsed / 100, 1000*100/(timeElapsed) );
				timeElapsed = 0;
			}
		}

		
	}

}

void GameLoop::RenderFrame() {

	auto& device = tig->GetRenderingDevice();

	device.BeginFrame();

	auto d3dDevice = device.GetDevice();

	// Set it as the render target
	D3DLOG(d3dDevice->SetRenderTarget(0, device.GetRenderSurface()));
	D3DLOG(d3dDevice->SetDepthStencilSurface(device.GetRenderDepthStencilSurface()));

	// Clear the new render target as well
	auto clearColor = D3DCOLOR_ARGB(0, 0, 0, 0);
	D3DLOG(d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0));

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
	D3DLOG(d3dDevice->SetRenderTarget(0, device.GetBackBuffer()));
	D3DLOG(d3dDevice->SetDepthStencilSurface(device.GetBackBufferDepthStencil()));

	// Copy from the actual render target to the back buffer and scale / position accordingly
	RECT destRect{ 0, 0, device.GetRenderWidth(), device.GetRenderHeight() };
	d3dDevice->StretchRect(
		device.GetRenderSurface(),
		nullptr,
		device.GetBackBuffer(),
		&destRect,
		D3DTEXF_LINEAR
	);

	device.Present();
}

void GameLoop::DoMouseScrolling() {
	// TODO: This would be the place to implement better scrolling in windowed mode
	mainLoop.DoMouseScrolling();
}

void GameLoop::RenderVersion() {
	UiRenderer::PushFont(PredefinedFont::ARIAL_10);

	ColorRect textColor(0x7FFFFFFF);
	TigTextStyle style;
	style.flags = 0x0800;
	style.field10 = 25;
	style.textColor = &textColor;
	
	auto& device = mTig.GetRenderingDevice();

	auto version = GetTemplePlusVersion();
	auto rect = UiRenderer::MeasureTextSize(version, style);
	rect.x = device.GetRenderWidth() - rect.width - 10 - 250;
	rect.y = device.GetRenderHeight() - rect.height - 10 - 250 ;

	UiRenderer::RenderText(version, rect, style);

	UiRenderer::PopFont();
}
