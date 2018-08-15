
#include "infrastructure/version.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(_TP_VERSION) && defined(_TP_COMMIT)
const char *buildVersion = TOSTRING(_TP_VERSION);
const char *buildCommitId = TOSTRING(_TP_COMMIT);
#else
#include <fmt/format.h>
const char *buildVersion = "UNKNOWN";
const char *buildCommitId = "UNKNOWN";
#endif

static std::string versionString;
static std::string commitIdString;

const std::string &GetTemplePlusVersion() {
	if (versionString.empty()) {
		if (strlen(buildVersion) && strlen(buildCommitId)) {
#if NDEBUG
			versionString = buildVersion;
#else
			versionString = fmt::format("{}.dbg", buildVersion);
#endif
		} else {
			versionString = "UNKNOWN VERSION";
		}
	}

	return versionString;
}

const std::string& GetTemplePlusCommitId() {
	if (commitIdString.empty()) {
		commitIdString = buildCommitId;
	}
	return commitIdString;
}
