#include "stdafx.h"
#include "diag.h"
#include "tig/tig_font.h"
#include <ui/ui_render.h>
#include <config/config.h>
#include <pathfinding.h>
#include <description.h>
#include <animgoals/anim.h>
#include <graphics/device.h>
#include <graphics/textures.h>

#include "../gamesystems/gamesystems.h"
#include "../gamesystems/legacysystems.h"
#include "../gamesystems/gamerenderer.h"
#include "../gamesystems/mapobjrender.h"
#include "../gamesystems/partsystemsrenderer.h"
#include "../gamesystems/clipping/clipping.h"
#include <tig/tig_mouse.h>
#include "location.h"
#include <action_sequence.h>

using namespace gfx;

class DiagScreen::Impl {
public:

	Impl() : textColor(0xDFFFFFFF) {
		style.textColor = &textColor;
		style.tracking = 4;
	}

	ColorRect textColor;
	TigTextStyle style;

};

DiagScreen::DiagScreen(RenderingDevice& device,
	GameSystems &gameSystems,
	GameRenderer &gameRenderer) 
	  : mImpl(std::make_unique<Impl>()), 
		mDevice(device), 
		mGameSystems(gameSystems),
		mGameRenderer(gameRenderer) {
	if (diagScreen == nullptr) {
		diagScreen = this;
	}
}

DiagScreen::~DiagScreen() {
	if (diagScreen == this) {
		diagScreen = nullptr;
	}
}

void DiagScreen::Render() {

	if (!mEnabled) {
		return;
	}

	UiRenderer::PushFont(PredefinedFont::ARIAL_10);

	auto& textureManager = mDevice.GetTextures();

	auto loaded = textureManager.GetLoaded();
	auto registered = textureManager.GetRegistered();
	std::vector<std::string> lines;

	POINT mousePt = mouseFuncs.GetPos();
	LocAndOffsets mouseLoc;
	locSys.GetLocFromScreenLocPrecise(mousePt.x, mousePt.y, mouseLoc);
	lines.push_back(fmt::format("#Mouse"));
	lines.push_back(fmt::format("x: {} y: {}", mousePt.x, mousePt.y));
	lines.push_back(fmt::format("Location: {}", mouseLoc));
	auto dragSelect = temple::GetRef<int>(0x10BD3B60);
	auto lmbClicked = temple::GetRef<int>(0x10BD3B5C);
	lines.push_back(fmt::format("Drag Select: {} LMB clicked: {}", dragSelect, lmbClicked));

	lines.push_back(fmt::format("#Textures"));
	lines.push_back(fmt::format("{} of {} loaded", loaded, registered));
	lines.push_back(fmt::format("Memory Budget: {}", FormatMemSize(textureManager.GetMemoryBudget())));
	lines.push_back(fmt::format("Used (est.): {}", FormatMemSize(textureManager.GetUsageEstimate())));

	auto& particleRenderer = mGameRenderer.GetParticleSysRenderer();
	lines.push_back(fmt::format("#Particle Systems"));
	lines.push_back(fmt::format("{} of {} rendered", particleRenderer.GetRenderedLastFrame(), 
		particleRenderer.GetTotalLastFrame()));
	lines.push_back(fmt::format("Avg Render Time: {} ms", particleRenderer.GetRenderTimeAvg()));

	auto& mapObjRenderer = mGameRenderer.GetMapObjectRenderer();
	lines.push_back(fmt::format("# Map Objects"));
	lines.push_back(fmt::format("{} of {} rendered", mapObjRenderer.GetRenderedLastFrame(),
		mapObjRenderer.GetTotalLastFrame()));
	lines.push_back(fmt::format("AnimSlots used: {}", gameSystems->GetAnim().GetSlotUsedCount()));
	
	auto& clipping = mGameSystems.GetClipping();
	lines.push_back(fmt::format("# Clipping Objects"));
	lines.push_back(fmt::format("{} of {} rendered", clipping.GetRenderered(),
		clipping.GetTotal()));
	
	lines.push_back(fmt::format("\n#Intgame"));
	lines.push_back(fmt::format("uiIntgameWidgetEnteredForGameplay: {}", temple::GetRef<int>(0x102FC644)));
	lines.push_back(fmt::format("uiIntgameWidgetEnteredForRender: {}", temple::GetRef<int>(0x102FC640)));
	auto intgameActor = temple::GetRef<objHndl>(0x10C04118);
	lines.push_back(fmt::format("intgameActor: {}", intgameActor));
	auto intgameFocus = temple::GetRef<objHndl>(0x10BE60D8);
	lines.push_back(fmt::format("intgameFocusObj: {}", intgameFocus));
	auto intgameTarget = temple::GetRef<objHndl>(0x10C040E8);
	lines.push_back(fmt::format("uiIntgameObjFromRaycast: {}", intgameTarget));
	

	lines.push_back(fmt::format("\n#Action Sequence"));
	lines.push_back(fmt::format("seqPickerD20ActnType: {}", *actSeqSys.seqPickerD20ActnType));
	auto curSeq = *actSeqSys.actSeqCur;
	lines.push_back(fmt::format("curSeq: {}", (void*)curSeq ));
	if (curSeq) {
		lines.push_back(fmt::format("\tperformer: {}", curSeq->performer));
		lines.push_back(fmt::format("\tflags: {}", curSeq->seqOccupied));
		lines.push_back(fmt::format("\taction idx: {}/{}", curSeq->d20aCurIdx, curSeq->d20ActArrayNum));
		for (auto i = 0; i < curSeq->d20aCurIdx; ++i) {
			lines.push_back(fmt::format("\taction[{}]: {}", i, curSeq->d20ActArray[i].d20ActType));
		}
	}
	lines.push_back(fmt::format("Greybar countdown: AI {} PC {}", 
		gameSystems->GetVagrant().GetAiGreybarResetCountdownValue(), 
		gameSystems->GetVagrant().GetPcGreybarResetCountdownValue()));
	
	if (config.pathfindingDebugMode)
	{
		lines.push_back(fmt::format("#Pathfinding"));

		lines.push_back(fmt::format("Active Picker Idx: {}", temple::GetRef<int>(0x102F920C)));
		lines.push_back(fmt::format("uiIntgameAcquireByRaycastOn: {}", temple::GetRef<int>(0x10C0410C)));
		auto uiIntgameSelectionConfirmed = temple::GetRef<int>(0x10C04110);
		if (uiIntgameSelectionConfirmed)
		{
			int dummy = 1;
		}
		lines.push_back(fmt::format("uiIntgameSelectionConfirmed: {}", uiIntgameSelectionConfirmed));
		lines.push_back(fmt::format("uiIntgameWaypointMode: {}", temple::GetRef<int>(0x10C04114)));
		
		
		objHndl uiIntgameTargetObjFromPortraits;
		
		uiIntgameTargetObjFromPortraits = temple::GetRef<objHndl>(0x10C04120);
		if (uiIntgameTargetObjFromPortraits)
		{
			lines.push_back(fmt::format("uiIntgameTargetObjFromPortraits: {}", uiIntgameTargetObjFromPortraits));
		}
		
			
		auto uiIntgamePathPreviewDestLoc = temple::GetRef<LocAndOffsets>(0x10C040F8);
		lines.push_back(fmt::format("uiIntgameWaypointLoc: {}", uiIntgamePathPreviewDestLoc));


		lines.push_back(fmt::format("State: {}", pathfindingSys.pdbgGotPath));
		if (pathfindingSys.pdbgMover)
			lines.push_back(fmt::format("Mover Obj: {}", pathfindingSys.pdbgMover));
		if (pathfindingSys.pdbgTargetObj)
			lines.push_back(fmt::format("Target Obj: {}", pathfindingSys.pdbgTargetObj));
		lines.push_back(fmt::format("From: {}", pathfindingSys.pdbgFrom));
		lines.push_back(fmt::format("To: {}", pathfindingSys.pdbgTo));
		if (pathfindingSys.pdbgUsingNodes && !pathfindingSys.pdbgAbortedSansNodes)
		{
			lines.push_back(fmt::format("Using {} pathnodes", pathfindingSys.pdbgNodeNum));
		} else
		{
			if (pathfindingSys.pdbgAbortedSansNodes)
			{

				lines.push_back(fmt::format("Using {} direction steps, failed because of {}, then tried {} pathnodes", pathfindingSys.pdbgDirectionsCount, pathfindingSys.pdbgShortRangeError, pathfindingSys.pdbgNodeNum));
			} else
			{
				lines.push_back(fmt::format("Using {} direction steps", pathfindingSys.pdbgDirectionsCount));
			}
		}
	}

	TigRect rect;
	rect.x = 25;
	rect.y = 25;
	rect.width = 400;

	for (auto& line : lines) {
		bool bold = false;
		if (!line.empty() && line.front() == '#') {
			line.erase(0, 1);
			bold = true;
			UiRenderer::PushFont(PredefinedFont::ARIAL_BOLD_10);
		}

		auto measuredSize = UiRenderer::MeasureTextSize(line, mImpl->style, 400);

		UiRenderer::RenderText(line, rect, mImpl->style);

		rect.y += measuredSize.height;

		if (bold) {
			UiRenderer::PopFont();
		}
	}

	UiRenderer::PopFont();

}

void DiagScreen::Toggle() {
	mEnabled = !mEnabled;
}

std::string DiagScreen::FormatMemSize(size_t memory) {

	if (memory < 1024) {
		return fmt::format("{} bytes");
	}
	
	if (memory < 1024 * 1024) {
		return fmt::format("{:.1f} KB", memory / 1024.0f);
	}
	
	return fmt::format("{:.1f} MB", memory / (1024.f * 1024.f));

}

DiagScreen* diagScreen = nullptr;
