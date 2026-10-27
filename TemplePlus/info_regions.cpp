#include "stdafx.h"
#include "info_regions.h"
#include <infrastructure/json11.hpp>
#include <infrastructure/vfs.h>
#include <infrastructure/location.h>
#include "gameview.h"
#include "maps.h"
#include "party.h"
#include "ui/ui_dialog.h"
#include "history.h"

namespace InfoRegions {
	struct Region {
		std::string name, text;
		std::vector<std::pair<float, float>> poly;
		float x0, y0, x1, y1;
	};
	static int sMap = -1;
	static std::vector<Region> sRegions;

	static void Load(int mapId) {
		sMap = mapId; sRegions.clear();
		auto path = fmt::format("rules/info_regions/{}.json", mapId);
		if (!vfs->FileExists(path)) return;
		std::string err;
		auto j = json11::Json::parse(vfs->ReadAsString(path), err);
		if (!err.empty()) { logger->warn("InfoRegions: {}: {}", path, err); return; }
		for (auto &r : j["regions"].array_items()) {
			Region g; g.name = r["name"].string_value(); g.text = r["text"].string_value();
			g.x0 = g.y0 = 1e9f; g.x1 = g.y1 = -1e9f;
			for (auto &p : r["poly"].array_items()) {
				float x = (float)p[0].number_value(), y = (float)p[1].number_value();
				g.poly.emplace_back(x, y);
				g.x0 = std::min(g.x0, x); g.y0 = std::min(g.y0, y); g.x1 = std::max(g.x1, x); g.y1 = std::max(g.y1, y);
			}
			if (g.poly.size() >= 3 && !g.text.empty()) sRegions.push_back(std::move(g));
		}
		logger->info("InfoRegions: map {}: {} regions", mapId, sRegions.size());
	}

	static bool Inside(const Region &g, float x, float y) {   // even-odd rule
		if (x < g.x0 || x > g.x1 || y < g.y0 || y > g.y1) return false;
		bool in = false;
		for (size_t i = 0, j = g.poly.size() - 1; i < g.poly.size(); j = i++) {
			auto &a = g.poly[i]; auto &b = g.poly[j];
			if ((a.second > y) != (b.second > y) && x < (b.first - a.first) * (y - a.second) / (b.second - a.second) + a.first)
				in = !in;
		}
		return in;
	}

	bool HandleClick(int screenX, int screenY) {
		auto mapId = maps.GetCurrentMapId();
		if (mapId != sMap) Load(mapId);
		if (sRegions.empty()) return false;
		auto loc = gameView->ScreenToTile(screenX, screenY);
		float x = loc.location.locx + 0.5f + loc.off_x / INCH_PER_TILE, y = loc.location.locy + 0.5f + loc.off_y / INCH_PER_TILE;
		for (auto &g : sRegions) {
			if (!Inside(g, x, y)) continue;
			logger->info("InfoRegions: click at {:.2f},{:.2f} hit {}", x, y, g.name);
			auto leader = party.GetConsciousPartyLeader();
			if (leader) uiDialog->ShowTextBubble(leader, objHndl::null, g.text);   // word-wrapped speech bubble, same as NPC barks
			histSys.CreateFromFreeText((g.text + "\n\n").c_str());
			return true;
		}
		return false;
	}
}
