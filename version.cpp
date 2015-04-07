
#include "stdafx.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(_TP_VERSION) && defined(_TP_COMMIT)
const char *buildVersion = TOSTRING(_TP_VERSION);
const char *buildCommitId = TOSTRING(_TP_COMMIT);
#else
const char *buildVersion = "UNKNOWN";
const char *buildCommitId = "UNKNOWN";
#endif

static string versionString;
static string commitIdString;

const string &GetTemplePlusVersion() {
	if (versionString.empty()) {
		if (strlen(buildVersion) && strlen(buildCommitId)) {
#if NDEBUG
			versionString = buildVersion;
#else
			versionString = format("{}.dbg", buildVersion);
#endif
		} else {
			versionString = "UNKNOWN VERSION";
		}
	}

	return versionString;
}

const string& GetTemplePlusCommitId() {
	if (commitIdString.empty()) {
		commitIdString = buildCommitId;
	}
	return commitIdString;
}
