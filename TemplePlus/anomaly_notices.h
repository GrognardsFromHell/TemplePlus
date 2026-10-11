#pragma once
#include <string>

/*
	Generic in-game "anomaly notice" for playtesters.
	Any subsystem can report something abnormal; the player gets ONE compact,
	semi-transparent heads-up (deduplicated by kind+key, with a count) asking them
	to save and send the save. Dismissed by clicking it; auto-archived on map change,
	save load or new game. Archived notices can be reopened (tpdp.show_anomaly_archive()).
	Controlled by the ini setting anomalyNotices (reporting/logging always happens).
	C++:    anomalyNotices.Report("radius", guid, "Bad radius on Oak Tree", "value=99999 ...");
	Python: tpdp.report_anomaly(kind, key, message, details="")
*/
class AnomalyNotices {
public:
	// Returns true if this kind+key was new (first report this session)
	bool Report(const std::string &kind, const std::string &key, const std::string &message, const std::string &details = "");
	void Dismiss();        // archive all active notices and hide the overlay
	void ShowArchive();    // reopen the overlay with the archived notices
	std::string GetArchiveText() const;
	void OnGameReset();    // load / new game: archive
	int ActiveCount() const;
};

extern AnomalyNotices anomalyNotices;
