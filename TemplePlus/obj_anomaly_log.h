#pragma once

#include <string>
#include "obj_structs.h"

namespace gfx { class AnimatedModel; }

/*
	Persistent log of aberrant object radius / render height values.
	Entries are keyed by object GUID + kind (radius/height), so each object is recorded once
	(with hit count, first/last seen and the worst value). Stored inside the save game as
	Save\Current\tp_obj_anomalies.txt, so it travels with any save a player sends.
	Only invoked on the (rare) bad-value branches; no cost when values are sane.
*/
class ObjAnomalyLog {
public:
	// Thresholds
	static constexpr float RADIUS_MAX = 600.0f;   // the old "Caught very large radius" workaround threshold
	static constexpr float HEIGHT_MAX = 1000.0f;  // Zuggtmoy (largest vanilla critter) is ~381
	static constexpr size_t MAX_ENTRIES = 256;

	static bool IsBadRadius(float r) { return !(r >= 0.0f && r <= RADIUS_MAX); } // also true for NaN
	static bool IsBadHeight(float h) { return !(h >= 0.0f && h <= HEIGHT_MAX); }

	// kind: "radius" or "height"; site: calling function; fixedTo: value used/stored instead (NaN if none)
	void Report(objHndl handle, const char *kind, const char *site, float badValue, float fixedTo,
		gfx::AnimatedModel *model = nullptr);

	// Records the value an anomaly was recalculated to, when only known after the report
	void SetFixedTo(objHndl handle, const char *kind, float fixedTo);
	void Clear();
	void SaveToCurrent();   // writes Save\Current\tp_obj_anomalies.txt (or removes it when empty)
	void LoadFromCurrent(); // reads it back (absent = empty, e.g. old / vanilla saves)
	std::string Dump() const;
	size_t Count() const;
};

extern ObjAnomalyLog objAnomalyLog;
