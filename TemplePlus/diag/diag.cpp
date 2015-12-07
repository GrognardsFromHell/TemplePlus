#include "stdafx.h"
#include "diag.h"
#include "tig/tig_font.h"
#include <ui/ui_render.h>
#include <graphics/device.h>
#include <graphics/textures.h>

#include "../gamesystems/gamesystems.h"
#include "../gamesystems/gamerenderer.h"
#include "../gamesystems/mapobjrender.h"
#include "../gamesystems/mapsystems.h"
#include "../gamesystems/partsystemsrenderer.h"
#include "../gamesystems/clipping/clipping.h"

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

	auto& clipping = mGameSystems.GetMapSystems().GetClipping();
	lines.push_back(fmt::format("# Clipping Objects"));
	lines.push_back(fmt::format("{} of {} rendered", clipping.GetRenderered(),
		clipping.GetTotal()));

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
