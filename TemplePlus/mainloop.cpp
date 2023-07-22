
#include "stdafx.h"

#include <graphics/device.h>
#include <graphics/textengine.h>
#include <graphics/dynamictexture.h>
#include <graphics/shaperenderer2d.h>

#include <debugui.h>

#include "mainloop.h"
#include <temple/dll.h>
#include "mainwindow.h"
#include "tig/tig_console.h"
#include "tig/tig_mouse.h"
#include "tig/tig_startup.h"
#include "gameview.h"
#include "gamesystems/gamesystems.h"
#include "ui/ui_render.h"
#include "config/config.h"
#include "tig/tig_font.h"
#include "tig/tig_mouse.h"
#include "obj.h"
#include "diag/diag.h"
#include "infrastructure/stopwatch.h"
#include "util/fixes.h"
#include "updater/updater.h"
#include "tig/tig_keyboard.h"
#include "party.h"
#include "critter.h"
#include "animgoals/anim.h"
#include "gamesystems/objects/objsystem.h"
#include "combat.h"
#include "turn_based.h"
#include "action_sequence.h"
#include "maps.h"
#include <infrastructure/keyboard.h>
#include "messages/messagequeue.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "ui/ui_mainmenu.h"
#include "ui/ui_debug.h"
#include <mod_support.h>

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

	objHndl (__cdecl *GetPCGroupMemberN)(int nIdx);

	void (__cdecl *DoMouseScrolling)();
	void (__cdecl *SetScrollDirection)(int direction);

	void (__cdecl *RenderUi)();

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

		rebase(DoMouseScrolling, 0x10001010);
		rebase(SetScrollDirection, 0x10006480);

		rebase(gameBuffersCreated, 0x102AB208);

		rebase(InGameHandleMessage, 0x10114EF0);

		rebase(RenderUi, 0x101F8D10);
	}
} mainLoop;

GameLoop::GameLoop(TigInitializer& tig, GameSystems& gameSystems, Updater &updater)
	: mTig(tig),
	  mGameSystems(gameSystems),
	  mUpdater(updater),
	  mGameRenderer(tig, *gameView->GetCamera(), mGameSystems) {

	mDiagScreen = std::make_unique<DiagScreen>(tig.GetRenderingDevice(),
		gameSystems,
		mGameRenderer);

	// Create the buffers for the scaled game view
	auto &device = tig.GetRenderingDevice();
	mSceneColor = device.CreateRenderTargetTexture(gfx::BufferFormat::A8R8G8B8, config.renderWidth, config.renderHeight, config.antialiasing);
	mSceneDepth = device.CreateRenderTargetDepthStencil(config.renderWidth, config.renderHeight, config.antialiasing);

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
	auto& gameView = mTig.GetGameView();

	// Is this a center map kind of deal?
	auto worldPos = gameView.ScreenToWorld(400, 300);
	locXY loc{ (uint32_t)(worldPos.x / INCH_PER_TILE), (uint32_t)(worldPos.z / INCH_PER_TILE) };
	mainLoop.sub_1002A580(loc);

	mainLoop.QueueFidgetAnimEvent();

	TigMsg msg;

	auto quit = false;
	while (!quit) {
		// Read user input and external system events (such as time)
		messageQueue->PollExternalEvents();			
		if (mDiagScreen->IsEnabled()) {	
			messageQueue->DebugMessages();
		}
		tig->GetDebugUI().NewFrame(); // needs to be after process_window_messages which is inside messageQueue->PollExternalEvents(), in order to process mousewheel messages

		mGameSystems.AdvanceTime();

		// This locks the cursor to our window if we are in the foreground and it's enabled
		if (!config.windowed && config.lockCursor){
			auto sceneRect = gameView.GetSceneRect();

			// Take care of roundoff issues
			// Otherwise the cursor can be 1 pixel beyond the border
			// TODO should this be generalizedinto GetSceneRect?
			// test Upper Left corner mapping for roundoff errors
			auto ul = gameView.MapToScene(sceneRect.x, sceneRect.y);
			if (ul.x < 0)
				sceneRect.x = ceil(sceneRect.x);
			if (ul.y < 0)
				sceneRect.y = ceil(sceneRect.y);

			// test Bottom Right corner mapping for roundoff errors
			auto br = gameView.MapToScene(sceneRect.x + sceneRect.z, sceneRect.y + sceneRect.w);
			if (br.x >= config.renderWidth)
				sceneRect.z = floor(sceneRect.z);
			if (br.y >= config.renderHeight)
				sceneRect.w = floor(sceneRect.w);

			tig->GetMainWindow().LockCursor(
				(int) sceneRect.x,
				(int) sceneRect.y,
				(int) sceneRect.z,
				(int) sceneRect.w
			);
		}
		else if (config.windowed && config.windowedLockCursor) {
			RECT winrect;
			GetWindowRect(tig->GetMainWindow().GetHwnd(), &winrect);
			tig->GetMainWindow().LockCursor(
				(int)winrect.left+8,
				(int)winrect.top+5,
				(int)(winrect.right - winrect.left - 18),
				(int)(winrect.bottom - winrect.top - 13)
			);
		}

		RenderFrame();

		// Why does it process msgs AFTER rendering???		
		while (messageQueue->Process(msg)) {
			if (msg.type == TigMsgType::EXIT) {
				quit = true;
				break;
			}

			// Pressing the F10 key toggles the diag screen
			if (msg.type == TigMsgType::KEYSTATECHANGE
				&& msg.arg1 == 0x44
				&& msg.arg2 == 1) {
				mDiagScreen->Toggle();
				UIShowDebug();
				if (uiSystems->GetMM().IsVisible())
				{
					tig->GetConsole().Show();
				}
			}
			

			// I have not found any place where message type 7 is queued,
			// so i removed the out of place re-rendering of the game frame

			if (!uiSystems->GetMM().IsVisible()) {
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
	
	auto& device = mTig.GetRenderingDevice();

	// Recreate the render targets if AA changed
	if (config.antialiasing != mSceneColor->IsMultiSampled()) {
		mSceneColor = device.CreateRenderTargetTexture(gfx::BufferFormat::X8R8G8B8, config.renderWidth, config.renderHeight, config.antialiasing);
		mSceneDepth = device.CreateRenderTargetDepthStencil(config.renderWidth, config.renderHeight, config.antialiasing);
	}
	
	gfx::PerfGroup perfGroup(device, "Game Loop Rendering");

	device.BeginFrame();

	device.PushRenderTarget(mSceneColor, mSceneDepth);
	device.SetCurrentCamera(mTig.GetGameView().mCamera);

	device.ClearCurrentColorTarget(XMCOLOR(0, 0, 0, 1));
	device.ClearCurrentDepthTarget();
	
	mGameRenderer.Render();

	device.BeginPerfGroup("UI");
	mainLoop.RenderUi();
	device.EndPerfGroup();

	mDiagScreen->Render();

	// Draw Version Number while in Main Menu
	if (uiSystems->GetMM().IsVisible()) {
		RenderVersion();
	}

	mouseFuncs.InvokeCursorDrawCallback();
	mouseFuncs.DrawItemUnderCursor(); // This draws dragged items

	// Reset the render target and camera
	device.PopRenderTarget();
	device.SetCurrentCamera(nullptr);
	
	// Copy from the actual render target to the back buffer and scale / position accordingly
	TigRect destRect{ 
		0, 
		0,
		(int)device.GetCurrentCamera().GetScreenWidth(),
		(int)device.GetCurrentCamera().GetScreenHeight()
	};
	TigRect srcRect{0, 0, gameView->GetWidth(), gameView->GetHeight()};
	srcRect.FitInto(destRect);

	gfx::SamplerType2d samplerType = gfx::SamplerType2d::CLAMP;
	if (!config.upscaleLinearFiltering) {
		samplerType = gfx::SamplerType2d::POINT;
	}

	tig->GetShapeRenderer2d().DrawRectangle(
		(float) srcRect.x, 
		(float)srcRect.y, 
		(float)srcRect.width, 
		(float)srcRect.height, 
		*mSceneColor,
		0xFFFFFFFF,
		samplerType
	);

	tig->GetConsole().Render();

	// Render the Debug UI
	tig->GetDebugUI().Render();

	// Render "GFade" overlay
	static auto& gfadeEnabled = temple::GetRef<BOOL>(0x10D25118);
	static auto& gfadeColor = temple::GetRef<XMCOLOR>(0x10D24A28);
	if (gfadeEnabled) {
		auto w = (float)device.GetCurrentCamera().GetScreenWidth();
		auto h = (float)device.GetCurrentCamera().GetScreenHeight();
		tig->GetShapeRenderer2d().DrawRectangle(0, 0, w, h, gfadeColor);
	}
	
	device.Present();

	device.EndPerfGroup();
}

void GameLoop::DoMouseScrolling() {

	if (config.windowed && mouseFuncs.MouseOutsideWndGet())
		return;

	static auto sysRefTime = 0;
	auto now = timeGetTime();
	if (sysRefTime && (now  < sysRefTime + 16) ){
		auto scrollButter = temple::GetRef<int>(0x102AC238);
		if (!scrollButter)
			return;
	}
	
	sysRefTime = now;

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
	int scrollMarginH = 3;
	if (config.windowed)
	{
		scrollMarginV = 7;
		scrollMarginH = 7;
	}

	auto renderWidth = config.renderWidth;
	auto renderHeight = config.renderHeight;

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

	auto &textEngine = tig->GetRenderingDevice().GetTextEngine();

	UiRenderer::PushFont(PredefinedFont::ARIAL_10);

	ColorRect textColor(0x7FFFFFFF);
	TigTextStyle style;
	style.tracking = 5;
	style.textColor = &textColor;

	auto& device = mTig.GetRenderingDevice();

	auto version = GetTemplePlusVersion();
	auto rect = UiRenderer::MeasureTextSize(version, style);
	rect.x = config.renderWidth - rect.width - 10;
	rect.y = config.renderHeight - rect.height - 10;
	
	UiRenderer::RenderText(version, rect, style);

	// Also draw the status of the auto update above the version number
	auto updateStatus = mUpdater.GetStatus();
	if (!updateStatus.empty()) {
		auto offset = rect.y;
		rect = UiRenderer::MeasureTextSize(updateStatus, style);
		rect.x = config.renderWidth - rect.width - 10;
		rect.y = offset - rect.height - 5;

		UiRenderer::RenderText(updateStatus, rect, style);
	}

	{
		auto &overridesUsed = modSupport.GetOverrides();
		for (auto i = 0; i < overridesUsed.size(); ++i) {
			auto &txt = overridesUsed[i];
			auto offset = rect.y;
			rect = UiRenderer::MeasureTextSize(txt, style);
			rect.x = config.renderWidth - rect.width - 10;
			rect.y = offset - rect.height - 5;

			UiRenderer::RenderText(txt, rect, style);
		}
	}

	UiRenderer::PopFont();
}

static class MainLoopHooks : public TempleFix {
public:


	static void NormalLmbHandleTarget(objHndl*tgt);

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
			
			// We have to manually trigger a new frame for the debug UI here since 
			// this can be called from virtually anywhere
			ImGui::NewFrame();

			return 0;
		});

		replaceFunction<void(objHndl*)>(0x101140F0, [](objHndl* tgt) {
			NormalLmbHandleTarget(tgt);
		});


		static void(__cdecl*orgScrollToPartyLeader)() = replaceFunction<void(__cdecl)()>(0x10113CE0, []() {
			auto dummy = -1;
			party.GetConsciousPartyLeader();
			orgScrollToPartyLeader();

		});

	}
} hooks;

/* 0x101140F0 */
void MainLoopHooks::NormalLmbHandleTarget(objHndl * tgt)
{
	locXY tgtLoc = LocAndOffsets::null.location;

	auto leader = party.GetConsciousPartyLeader();
	auto chosenOne = leader;
	if (!chosenOne) {
		chosenOne = party.GroupPCsGetMemberN(0);
		if (!chosenOne || !critterSys.IsDeadOrUnconscious(chosenOne))
			return;
		party.AddToCurrentlySelected(chosenOne);
	}

	auto obj = objSystem->GetObject(chosenOne);
	temple::GetRef<void(__cdecl)()>(0x100146C0)(); // FidgetAnimSchedule



	auto tgtHndl = *tgt;
	auto tgtObj = objSystem->GetObject(tgtHndl);
	auto tgtType = tgtObj->type;
	auto spellFlags = (SpellFlags)obj->GetInt32(obj_f_spell_flags);
	auto critFlags = (CritterFlag)obj->GetInt32(obj_f_critter_flags);

	if ((spellFlags & SpellFlags::SF_10000)	|| (critFlags & (CritterFlag::OCF_PARALYZED | CritterFlag::OCF_STUNNED) ) )
		return;

	if (!critterSys.isCritterCombatModeActive(chosenOne) && temple::GetRef<BOOL(__cdecl)(objHndl, void*, objHndl)>(0x10055060)(tgtHndl, nullptr, chosenOne)){
		combatSys.enterCombat(chosenOne);
	}

	if (critterSys.isCritterCombatModeActive(chosenOne))
		return;

	auto goalType = AnimGoalType::ag_anim_fidget;
	auto someFlag = 0;
	auto playConfirmationSnd = 1;
	auto onlyForPC = 0;
	auto sceneryTeleport = 0;
	auto perform = false;
	JumpPoint jmpPt;

	switch (tgtType){
	case obj_t_portal: 
		if (spellFlags & SpellFlags::SF_20000)
			return;
		goalType = ag_use_object;
		someFlag = 1;
		playConfirmationSnd = 1;
		break;
	
	case obj_t_scenery: 
		goalType = ag_use_object;
		someFlag = 1;
		playConfirmationSnd = 1;
		sceneryTeleport = tgtObj->GetInt32(obj_f_scenery_teleport_to);
		if (sceneryTeleport && maps.GetJumpPoint(sceneryTeleport, jmpPt)){
			onlyForPC = 1;
		}
		break;

	case obj_t_trap: 
		goalType = ag_move_to_tile;
		tgtLoc = tgtObj->GetLocation();
		playConfirmationSnd = 1;
		break;

	case obj_t_container: 
	case obj_t_weapon: 
	case obj_t_ammo: 
	case obj_t_armor: 
	case obj_t_money: 
	case obj_t_food: 
	case obj_t_scroll: 
	case obj_t_key: 
	case obj_t_written: 
	case obj_t_generic: 
	case obj_t_bag: 
	
		playConfirmationSnd = 1;
		perform = true;
		break;
	case obj_t_pc: 
	case obj_t_npc: 
		if (actSeqSys.SeqPickerHasTargetingType()){
			perform = true;
			break;
		}
		
		if (!critterSys.IsDeadOrUnconscious(tgtHndl) || (tgtObj->GetInt32(obj_f_spell_flags) & SpellFlags::SF_2000000)){
			if (party.IsInParty(tgtHndl))
				return;
			goalType = ag_talk;
			playConfirmationSnd = 0;
			onlyForPC = 1;
		} 
		else{
			if (spellFlags & SpellFlags::SF_20000)
				return;

			if (infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)){
				temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x100264A0)(chosenOne, tgtHndl); // moves the tgtObj to chosenOne's location if it's inventory
				return;
			}
			goalType = ag_use_container;
			someFlag = 1;
			playConfirmationSnd = 1;
			onlyForPC = 1;
		}
		break;
	case obj_t_projectile: 
	default: 
		return;
	}


	if (perform){
		if (actSeqSys.isPerforming(chosenOne))
			return;
		actSeqSys.TurnBasedStatusInit(chosenOne);
		d20Sys.GlobD20ActnInit();
		actSeqSys.ActionTypeAutomatedSelection(tgtHndl);
		d20Sys.GlobD20ActnSetTarget(tgtHndl, nullptr);
		actSeqSys.ActionAddToSeq();
		actSeqSys.sequencePerform();
		actSeqSys.SeqPickerTargetingReset();
	} 
	// do animation goal instead
	else
	{
		auto distToTgt = 100000000.0f;
		auto curSelIdx = 0u;
		auto foundOne = false;
		auto selectedClosest = objHndl::null;
		auto numSelected = party.CurrentlySelectedNum();
		for (auto i=0u; i < numSelected; i++){
			auto dude = party.GetCurrentlySelected(i);
			if (!critterSys.IsDeadOrUnconscious(dude) 
				&& ( (objects.GetType(dude) == obj_t_pc) || !onlyForPC)){
				auto dist = locSys.DistanceToObj(dude, tgtHndl);
				if (dist > distToTgt)
					continue;
				curSelIdx = i;
				distToTgt = locSys.DistanceToObj(dude, tgtHndl);
				foundOne = true;
				selectedClosest = dude;
			}
		}

		if (!foundOne)
			return;

		chosenOne = selectedClosest;
		gameSystems->GetAnim().PushForMouseTarget(chosenOne, goalType, tgtHndl, tgtLoc, objHndl::null, someFlag);


	}

	if (playConfirmationSnd && !critterSys.IsDeadOrUnconscious(chosenOne)){
		auto fellowPc = party.GetFellowPc(chosenOne);
		char text[1000];
		int soundId;
		critterSys.GetOkayVoiceLine(chosenOne, fellowPc, text, &soundId);
		critterSys.PlayCritterVoiceLine(chosenOne, fellowPc, text, soundId);
	}
}
