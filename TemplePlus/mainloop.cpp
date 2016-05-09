#include "stdafx.h"
#include "mainloop.h"
#include <temple/dll.h>
#include "mainwindow.h"
#include "tig/tig_msg.h"
#include "tig/tig_mouse.h"
#include "tig/tig_startup.h"
#include "gamesystems/gamesystems.h"
#include "ui/ui_render.h"
#include "config/config.h"
#include "tig/tig_font.h"
#include "obj.h"
#include "diag/diag.h"
#include <graphics/device.h>
#include "infrastructure/stopwatch.h"
#include <graphics/shaperenderer2d.h>
#include "util/fixes.h"
#include "updater/updater.h"
#include <dinput.h>

static GameLoop *gameLoop = nullptr;

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

GameLoop::GameLoop(TigInitializer& tig, GameSystems& gameSystems, Updater &updater)
	: mTig(tig),
	  mGameSystems(gameSystems),
	  mUpdater(updater),
	  mGameRenderer(tig, mGameSystems) {

	mDiagScreen = std::make_unique<DiagScreen>(tig.GetRenderingDevice(),
		gameSystems,
		mGameRenderer);

	if (!gameLoop) {
		gameLoop = this;
	}
}

GameLoop::~GameLoop() {
	if (gameLoop == this) {
		gameLoop = nullptr;
	}
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
	static int timeElapsed = 0, timeElapsed2=0;
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
		Stopwatch sw2;

		RenderFrame();

		timeElapsed2 += sw2.GetElapsedMs();

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
				/*if (msg.type == TigMsgType::KEYSTATECHANGE || 
					msg.type == TigMsgType::CHAR)
				{
					int dummy = 1;
				}
				else*/
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
				if (timeElapsed > 0) {
				//	logger->info("Time per frame: {}ms ,  FPS: {}", timeElapsed / 100, 1000 * 100 / (timeElapsed));
				}
				if (timeElapsed2 > 0) {
				//	logger->info("Time per frame - Render: {}ms ,  Rendering FPS: {}", timeElapsed2 / 100, 1000 * 100 / (timeElapsed2));
				}
				timeElapsed = 0;
				timeElapsed2 = 0;
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
	auto &sceneRect = device.GetSceneRect();
	RECT destRect;
	destRect.left = (int) sceneRect.x;
	destRect.top = (int) sceneRect.y;
	destRect.right = (int)(sceneRect.x + sceneRect.z);
	destRect.bottom = (int)(sceneRect.y + sceneRect.w);
	d3dDevice->StretchRect(
		device.GetRenderSurface(),
		nullptr,
		device.GetBackBuffer(),
		&destRect,
		D3DTEXF_LINEAR
	);

	// Render "GFade" overlay
	static auto& gfadeEnabled = temple::GetRef<BOOL>(0x10D25118);
	static auto& gfadeColor = temple::GetRef<XMCOLOR>(0x10D24A28);
	if (gfadeEnabled) {
		auto w = (float) device.GetRenderWidth();
		auto h = (float) device.GetRenderHeight();
		tig->GetShapeRenderer2d().DrawRectangle(0, 0, w, h, gfadeColor);
	}

	device.Present();
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
	
	int scrollMarginV = 2;
	int scrollMarginH = 2;
	if (config.windowed)
	{
		scrollMarginV = 7;
		scrollMarginH = 7;
	}

	auto renderWidth = mTig.GetRenderingDevice().GetRenderWidth();
	auto renderHeight = mTig.GetRenderingDevice().GetRenderHeight();

	if (mousePt.x <= scrollMarginH) // scroll left
	{
		if (mousePt.y <= scrollMarginV) // scroll upper left
			scrollDir = 7;
		else if (mousePt.y >= renderHeight - scrollMarginV) // scroll bottom left
			scrollDir = 5;
		else
			scrollDir = 6;
	} 
	else if (mousePt.x >= renderWidth - scrollMarginH) // scroll right
	{
		if (mousePt.y <= scrollMarginV) // scroll top right
			scrollDir = 1;
		else if (mousePt.y >= renderHeight - scrollMarginV) // scroll bottom right
			scrollDir = 3;
		else
			scrollDir = 2;
	}
	else // scroll vertical only
	{
		if (mousePt.y <= scrollMarginV) // scroll up
			scrollDir = 0;
		else if (mousePt.y >= renderHeight - scrollMarginV) // scroll down
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
	style.tracking = 5;
	style.textColor = &textColor;

	auto& device = mTig.GetRenderingDevice();

	auto version = GetTemplePlusVersion();
	auto rect = UiRenderer::MeasureTextSize(version, style);
	rect.x = device.GetRenderWidth() - rect.width - 10;
	rect.y = device.GetRenderHeight() - rect.height - 10;

	UiRenderer::RenderText(version, rect, style);

	// Also draw the status of the auto update above the version number
	auto updateStatus = mUpdater.GetStatus();
	if (!updateStatus.empty()) {
		auto offset = rect.y;
		rect = UiRenderer::MeasureTextSize(updateStatus, style);
		rect.x = device.GetRenderWidth() - rect.width - 10;
		rect.y = offset - rect.height - 5;

		UiRenderer::RenderText(updateStatus, rect, style);
	}

	UiRenderer::PopFont();
}

static class MainLoopHooks : public TempleFix {
public:

	void apply() override {

		/*
			This is called in some places manually to fade out the game screen.
		*/
		// tig_window_render_frame
		replaceFunction<int(uint32_t)>(0x101ded80, [](uint32_t worldRenderFunc) {
			if (!gameLoop) {
				return 0;
			}

			gameLoop->RenderFrame();
			return 0;
		});
		/*
			The normal (non-combat) LMB handler function
		*/
		static void(__cdecl*orgNormalLmbHandler)(TigMsg*) = replaceFunction<void(TigMsg*)>(0x10114AF0, [](TigMsg* msg)
		{
			//logger->debug("NormalLmbHandler: LMB released; args:  {} , {} , {} , {}", msg->arg1, msg->arg2, msg->arg3, msg->arg4);
			orgNormalLmbHandler(msg);
			//logger->debug("NormalLmbHandler: success.");
		});

		static void(__cdecl*orgIngameMsgHandler)(TigMsg*) = replaceFunction<void(TigMsg*)>(0x10114EF0, [](TigMsg* msg)
		{
			if (msg->type == TigMsgType::KEYSTATECHANGE || msg->type == TigMsgType::CHAR || msg->type == TigMsgType::KEYDOWN){
				int dummy = 1;
				if (msg->arg1 == DIK_HOME)	{
					int asd = 1;
					VK_HOME;
				}
			}
			//logger->debug("NormalLmbHandler: LMB released; args:  {} , {} , {} , {}", msg->arg1, msg->arg2, msg->arg3, msg->arg4);
			orgIngameMsgHandler(msg);
			//logger->debug("NormalLmbHandler: success.");
		});


	}
} hooks;
