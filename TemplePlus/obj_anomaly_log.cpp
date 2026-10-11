#include "stdafx.h"
#include "obj_anomaly_log.h"
#include "anomaly_notices.h"
#include "obj.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/mapsystem.h"
#include "gamesystems/timeevents.h"
#include <infrastructure/version.h>
#include <infrastructure/meshes.h>
#include <infrastructure/mesparser.h>
#include <infrastructure/vfs.h>
#include <infrastructure/stringutil.h>
#include <tio/tio.h>
#include <map>
#include <vector>
#include <cmath>
#include <cfloat>
#include <ctime>

ObjAnomalyLog objAnomalyLog;

static const char *ANOMALY_FILE = "Save\\Current\\tp_obj_anomalies.txt";
static const char *FORMAT_HEADER = "# Temple+ object radius/height anomaly log v1";

namespace {
	// Ordered field list for the on-disk format (tab separated key=value)
	struct Entry {
		std::map<std::string, std::string> fields;
		float worst = 0.0f;
		int count = 0;
	};

	std::map<std::string, Entry> sEntries; // key: guid|kind
	int sDropped = 0;
	std::unordered_map<int, std::string> sMeshes;
	bool sMeshesLoaded = false;

	float Severity(float v) {
		return std::isfinite(v) ? std::fabs(v) : FLT_MAX;
	}

	std::string FloatStr(float v) {
		if (std::isnan(v)) return "nan";
		if (std::isinf(v)) return v > 0 ? "inf" : "-inf";
		return fmt::format("{:.3f}", v);
	}

	float ParseFloat(const std::string &s) {
		if (s == "nan") return NAN;
		if (s == "inf") return INFINITY;
		if (s == "-inf") return -INFINITY;
		return (float)atof(s.c_str());
	}

	std::string Sanitize(std::string s) {
		for (auto &ch : s) {
			if (ch == '\t' || ch == '\n' || ch == '\r') ch = ' ';
		}
		return s;
	}

	std::string Now() {
		auto t = time(nullptr);
		tm lt;
		localtime_s(&lt, &t);
		char buf[32];
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &lt);
		return buf;
	}

	std::string MeshName(int id) {
		if (!sMeshesLoaded) {
			sMeshesLoaded = true;
			try {
				for (auto &it : MesFile::ParseFile("art\\meshes\\meshes.mes")) sMeshes[it.first] = it.second;
			}
			catch (...) {}
		}
		auto it = sMeshes.find(id);
		return it == sMeshes.end() ? fmt::format("#{}", id) : it->second;
	}

	const char *ORDER[] = { "guid", "kind", "worst", "count", "first_value", "fixed_to", "first_seen", "last_seen",
		"site", "last_site", "proto", "name", "mesh", "skeleton", "scale", "anim_id", "map", "loc", "game_time", "version" };
}

void ObjAnomalyLog::Report(objHndl handle, const char *kind, const char *site, float badValue, float fixedTo,
	gfx::AnimatedModel *model)
{
	if (!handle) return;
	auto obj = objSystem->GetObject(handle);
	if (!obj) return;

	auto guid = obj->id.ToString();
	auto key = guid + "|" + kind;
	auto now = Now();

	auto it = sEntries.find(key);
	if (it != sEntries.end()) {
		auto &e = it->second;
		e.count++;
		e.fields["count"] = fmt::format("{}", e.count);
		e.fields["last_seen"] = now;
		e.fields["last_site"] = site;
		if (Severity(badValue) > Severity(e.worst)) {
			e.worst = badValue;
			e.fields["worst"] = FloatStr(badValue);
		}
		anomalyNotices.Report(kind, guid, // dedupes: only bumps the count if already shown
			fmt::format("{} {} on {} (proto {})", kind, e.fields["worst"], e.fields["name"], e.fields["proto"]),
			fmt::format("site={} (seen {} times)", site, e.count));
		return; // already recorded: no log spam
	}

	if (sEntries.size() >= MAX_ENTRIES) {
		// Replace the least severe entry if this one is worse
		auto least = sEntries.begin();
		for (auto i = sEntries.begin(); i != sEntries.end(); ++i) {
			if (Severity(i->second.worst) < Severity(least->second.worst)) least = i;
		}
		if (Severity(least->second.worst) >= Severity(badValue)) {
			sDropped++;
			return;
		}
		sEntries.erase(least);
		sDropped++;
	}

	Entry e;
	e.worst = badValue;
	e.count = 1;
	auto &f = e.fields;
	f["guid"] = guid;
	f["kind"] = kind;
	f["worst"] = FloatStr(badValue);
	f["count"] = "1";
	f["first_value"] = FloatStr(badValue);
	f["fixed_to"] = std::isnan(fixedTo) ? "-" : FloatStr(fixedTo);
	f["first_seen"] = now;
	f["last_seen"] = now;
	f["site"] = site;
	f["last_site"] = site;
	f["proto"] = fmt::format("{}", objSystem->GetProtoId(handle));
	f["name"] = Sanitize(objects.GetDisplayName(handle, handle));
	auto meshId = obj->GetInt32(obj_f_base_mesh);
	auto skelId = obj->GetInt32(obj_f_base_anim);
	f["mesh"] = Sanitize(MeshName(meshId)) + ".skm";
	f["skeleton"] = Sanitize(MeshName(skelId)) + ".ska";
	f["scale"] = fmt::format("{}", objects.GetScalePercent(handle));
	f["anim_id"] = model ? fmt::format("{}", (int)model->GetAnimId()) : "-";
	f["map"] = fmt::format("{}", gameSystems->GetMap().GetCurrentMapId());
	auto loc = objects.GetLocationFull(handle);
	f["loc"] = fmt::format("{},{} ({:.1f},{:.1f})", loc.location.locx, loc.location.locy, loc.off_x, loc.off_y);
	auto gt = gameSystems->GetTimeEvent().GetTime();
	f["game_time"] = Sanitize(gameSystems->GetTimeEvent().FormatTime(gt));
	f["version"] = GetTemplePlusVersion();

	logger->warn("ObjAnomaly: new {} anomaly {} (fixed to {}) on {} [{}] proto {} mesh {} scale {} map {} at {} via {}",
		kind, f["worst"], f["fixed_to"], f["name"], guid, f["proto"], f["mesh"], f["scale"], f["map"], f["loc"], site);

	anomalyNotices.Report(kind, guid,
		fmt::format("{} {} on {} (proto {})", kind, f["worst"], f["name"], f["proto"]),
		fmt::format("site={} fixed_to={} mesh={} scale={} map={} loc={}", site, f["fixed_to"], f["mesh"], f["scale"], f["map"], f["loc"]));
	sEntries[key] = std::move(e);
}

void ObjAnomalyLog::SetFixedTo(objHndl handle, const char *kind, float fixedTo) {
	if (!handle) return;
	auto obj = objSystem->GetObject(handle);
	if (!obj) return;
	auto it = sEntries.find(obj->id.ToString() + "|" + kind);
	if (it != sEntries.end()) it->second.fields["fixed_to"] = FloatStr(fixedTo);
}

void ObjAnomalyLog::Clear() {
	sEntries.clear();
	sDropped = 0;
}

size_t ObjAnomalyLog::Count() const {
	return sEntries.size();
}

std::string ObjAnomalyLog::Dump() const {
	std::string out = FORMAT_HEADER;
	out += fmt::format("\n# entries: {}, dropped (cap {}): {}\n", sEntries.size(), MAX_ENTRIES, sDropped);
	for (auto &it : sEntries) {
		bool first = true;
		for (auto key : ORDER) {
			auto f = it.second.fields.find(key);
			if (f == it.second.fields.end()) continue;
			if (!first) out += "\t";
			out += fmt::format("{}={}", key, f->second);
			first = false;
		}
		out += "\n";
	}
	return out;
}

void ObjAnomalyLog::SaveToCurrent() {
	if (sEntries.empty() && !sDropped) {
		if (vfs->FileExists(ANOMALY_FILE)) tio_remove(ANOMALY_FILE);
		return;
	}
	auto text = Dump();
	auto fh = tio_fopen(ANOMALY_FILE, "wt");
	if (!fh) {
		logger->error("ObjAnomaly: unable to write {}", ANOMALY_FILE);
		return;
	}
	tio_fwrite(text.data(), 1, text.size(), fh);
	tio_fclose(fh);
}

void ObjAnomalyLog::LoadFromCurrent() {
	Clear();
	if (!vfs->FileExists(ANOMALY_FILE)) return;
	std::string text;
	try {
		text = vfs->ReadAsString(ANOMALY_FILE);
	}
	catch (...) {
		logger->warn("ObjAnomaly: unable to read {}", ANOMALY_FILE);
		return;
	}
	for (auto &line : split(text, '\n', true)) {
		if (line.empty()) continue;
		if (line[0] == '#') {
			auto pos = line.find("dropped (cap");
			if (pos != std::string::npos) {
				auto colon = line.find(':', pos);
				if (colon != std::string::npos) sDropped = atoi(line.c_str() + colon + 1);
			}
			continue;
		}
		Entry e;
		for (auto &part : split(line, '\t')) {
			auto eq = part.find('=');
			if (eq == std::string::npos) continue;
			e.fields[part.substr(0, eq)] = part.substr(eq + 1);
		}
		if (!e.fields.count("guid") || !e.fields.count("kind")) {
			logger->warn("ObjAnomaly: skipping malformed line in {}", ANOMALY_FILE);
			continue;
		}
		e.worst = ParseFloat(e.fields["worst"]);
		e.count = atoi(e.fields["count"].c_str());
		if (sEntries.size() < MAX_ENTRIES)
			sEntries[e.fields["guid"] + "|" + e.fields["kind"]] = std::move(e);
	}
	if (!sEntries.empty())
		logger->info("ObjAnomaly: loaded {} radius/height anomaly entries from the save", sEntries.size());
}
