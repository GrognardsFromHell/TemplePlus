#include "stdafx.h"
#include "anomaly_notices.h"
#include "ui/widgets/widgets.h"
#include "ui/widgets/widget_content.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "tig/tig_msg.h"
#include "tig/tig_startup.h"
#include <graphics/shaperenderer2d.h>
#include <config/config.h>
#include <ctime>
#include <deque>
#include <set>

AnomalyNotices anomalyNotices;

namespace {
	struct Notice {
		std::string kind, key, message, details, firstSeen;
		int count = 1;
		int mapId = 0;
	};

	constexpr size_t MAX_TRACKED_KEYS = 4096;
	constexpr size_t MAX_ARCHIVE = 200;
	constexpr int WIDTH = 520;
	constexpr int LINES = 4;
	constexpr int LINE_H = 18;

	std::vector<Notice> sActive;      // unacknowledged, deduped by kind+key
	std::deque<Notice> sArchive;      // acknowledged
	std::set<std::string> sSeenKeys;  // session-wide dedupe for logging
	bool sShowingArchive = false;
	int sShownMapId = -1;

	std::string Now() {
		auto t = time(nullptr); tm lt; localtime_s(&lt, &t);
		char buf[32]; strftime(buf, sizeof(buf), "%H:%M:%S", &lt);
		return buf;
	}

	class NoticeWidget : public WidgetContainer {
	public:
		NoticeWidget() : WidgetContainer(WIDTH, 12 + LINES * LINE_H) {
			for (int i = 0; i < LINES; i++) {
				auto t = std::make_unique<WidgetText>("", i == 0 ? "priory-title" : "default");
				t->SetY(6 + i * LINE_H);
				t->SetX(10);
				t->SetFixedWidth(WIDTH - 20);
				mLines[i] = t.get();
				AddContent(std::move(t));
			}
		}
		void SetLine(int i, const std::string &text) { mLines[i]->SetText(text); }

		void Render() override {
			if (!IsVisible()) return;
			// Auto-archive when the player moves on to another map
			if (!sShowingArchive && gameSystems && sShownMapId != -1
				&& gameSystems->GetMap().GetCurrentMapId() != sShownMapId) {
				anomalyNotices.Dismiss();
				return;
			}
			auto area = GetContentArea();
			tig->GetShapeRenderer2d().DrawRectangle((float)area.x, (float)area.y, (float)area.width, (float)area.height, 0xA0200000);
			tig->GetShapeRenderer2d().DrawRectangleOutline({ (float)area.x, (float)area.y },
				{ (float)(area.x + area.width - 1), (float)(area.y + area.height - 1) }, XMCOLOR(0xFFE0A040));
			WidgetContainer::Render();
		}
		bool HandleMouseMessage(const TigMouseMsg &msg) override {
			if (msg.flags & MSF_LMB_RELEASED) {
				anomalyNotices.Dismiss();
			}
			return true;
		}
	private:
		WidgetText *mLines[LINES] = {};
	};

	std::unique_ptr<NoticeWidget> sWidget;

	NoticeWidget *GetWidget() {
		if (!sWidget) {
			sWidget = std::make_unique<NoticeWidget>();
			sWidget->Hide();
		}
		sWidget->SetPos(std::max(0, config.renderWidth - WIDTH - 10), 10);
		return sWidget.get();
	}

	std::string Clip(const std::string &s, size_t n) {
		return s.size() <= n ? s : s.substr(0, n - 3) + "...";
	}

	void Refresh() {
		if (!config.anomalyNotices || !uiManager) return;
		auto w = GetWidget();
		if (sShowingArchive) {
			w->SetLine(0, fmt::format("Temple+ anomaly archive ({} entr{})", sArchive.size(), sArchive.size() == 1 ? "y" : "ies"));
			for (int i = 1; i < LINES; i++) {
				if ((size_t)i <= sArchive.size()) {
					auto &n = sArchive[sArchive.size() - i];
					w->SetLine(i, Clip(fmt::format("{} {} x{}: {}", n.firstSeen, n.kind, n.count, n.message), 80));
				}
				else w->SetLine(i, i == 1 ? "(empty)" : "");
			}
		}
		else {
			if (sActive.empty()) { w->Hide(); return; }
			int total = 0;
			for (auto &n : sActive) total += n.count;
			auto &last = sActive.back();
			w->SetLine(0, fmt::format("Temple+ detected something abnormal ({} issue{}, {} event{})",
				sActive.size(), sActive.size() == 1 ? "" : "s", total, total == 1 ? "" : "s"));
			w->SetLine(1, Clip("Latest: " + last.message, 80));
			w->SetLine(2, "Please save now and send that save (plus TemplePlus.log) to the developers.");
			w->SetLine(3, "Click to dismiss. Reopen: tpdp.show_anomaly_archive() in the console.");
		}
		w->Show();
		w->BringToFront();
	}

	void ArchiveActive() {
		for (auto &n : sActive) {
			sArchive.push_back(n);
			if (sArchive.size() > MAX_ARCHIVE) sArchive.pop_front();
		}
		sActive.clear();
	}
}

bool AnomalyNotices::Report(const std::string &kind, const std::string &key, const std::string &message, const std::string &details)
{
	auto fullKey = kind + "|" + key;
	for (auto &n : sActive) {
		if (n.kind == kind && n.key == key) {
			n.count++;
			Refresh(); // only the count changes; no new log line
			return false;
		}
	}
	bool isNew = sSeenKeys.find(fullKey) == sSeenKeys.end();
	if (isNew) {
		if (sSeenKeys.size() < MAX_TRACKED_KEYS) sSeenKeys.insert(fullKey);
		logger->warn("Anomaly [{}] {}: {}{}{}", kind, key, message, details.empty() ? "" : " | ", details);
	}
	else {
		// Already archived this session: count it in the archive, don't nag the player again
		for (auto &n : sArchive) {
			if (n.kind == kind && n.key == key) { n.count++; return false; }
		}
	}

	Notice n;
	n.kind = kind; n.key = key; n.message = message; n.details = details;
	n.firstSeen = Now();
	n.mapId = gameSystems ? gameSystems->GetMap().GetCurrentMapId() : 0;
	sActive.push_back(n);
	if (sActive.size() > 64) { sArchive.push_back(sActive.front()); sActive.erase(sActive.begin()); }
	sShowingArchive = false;
	sShownMapId = n.mapId;
	Refresh();
	return true;
}

void AnomalyNotices::Dismiss() {
	ArchiveActive();
	sShowingArchive = false;
	sShownMapId = -1;
	if (sWidget) sWidget->Hide();
}

void AnomalyNotices::ShowArchive() {
	ArchiveActive();
	sShowingArchive = true;
	sShownMapId = -1;
	Refresh();
}

std::string AnomalyNotices::GetArchiveText() const {
	std::string out;
	for (auto &n : sArchive)
		out += fmt::format("{}\t{}\t{}\tx{}\tmap {}\t{}\t{}\n", n.firstSeen, n.kind, n.key, n.count, n.mapId, n.message, n.details);
	for (auto &n : sActive)
		out += fmt::format("{}\t{}\t{}\tx{}\tmap {}\t{}\t{} (active)\n", n.firstSeen, n.kind, n.key, n.count, n.mapId, n.message, n.details);
	return out;
}

void AnomalyNotices::OnGameReset() {
	Dismiss();
}

int AnomalyNotices::ActiveCount() const {
	return (int)sActive.size();
}
