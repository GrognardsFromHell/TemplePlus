#include "stdafx.h"
#include "diag.h"
#include "tig/tig_font.h"
#include <graphics/graphics.h>
#include <graphics/textures.h>
#include <ui/ui_render.h>
#include <util/config.h>
#include <pathfinding.h>
#include <description.h>

class DiagScreen::Impl {
public:

	Impl() : textColor(0xDFFFFFFF) {
		style.textColor = &textColor;
		style.tracking = 4;
	}

	ColorRect textColor;
	TigTextStyle style;

};

DiagScreen::DiagScreen(Graphics& g) : mImpl(std::make_unique<Impl>()), mGraphics(g) {
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

	auto textureManager = static_cast<TextureManager*>(gfx::textureManager);

	auto loaded = textureManager->GetLoaded();
	auto registered = textureManager->GetRegistered();
	std::vector<std::string> lines;

	lines.push_back(fmt::format("#Textures"));
	lines.push_back(fmt::format("{} of {} loaded", loaded, registered));
	lines.push_back(fmt::format("Memory Budget: {}", FormatMemSize(textureManager->GetMemoryBudget())));
	lines.push_back(fmt::format("Used (est.): {}", FormatMemSize(textureManager->GetUsageEstimate())));
	if (config.pathfindingDebugMode)
	{
		lines.push_back(fmt::format("#Pathfinding"));
		lines.push_back(fmt::format("State: {}", pathfindingSys.pdbgGotPath));
		if (pathfindingSys.pdbgMover)
			lines.push_back(fmt::format("Mover Obj: {}", description.getDisplayName( pathfindingSys.pdbgMover)));
		if (pathfindingSys.pdbgTargetObj)
			lines.push_back(fmt::format("Target Obj: {}", description.getDisplayName(pathfindingSys.pdbgTargetObj)));
		lines.push_back(fmt::format("From: {}", pathfindingSys.pdbgFrom));
		lines.push_back(fmt::format("To: {}", pathfindingSys.pdbgTo));
		if (pathfindingSys.pdbgUsingNodes && !pathfindingSys.pdbgAbortedSansNodes)
		{
			lines.push_back(fmt::format("Using {} pathnodes", pathfindingSys.pdbgNodeNum));
		} else
		{
			if (pathfindingSys.pdbgAbortedSansNodes)
			{
				lines.push_back(fmt::format("Using {} direction steps, failed, then tried {} pathnodes", pathfindingSys.pdbgDirectionsCount, pathfindingSys.pdbgNodeNum));
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
