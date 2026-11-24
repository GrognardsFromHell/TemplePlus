#pragma once

#include <string>

// -1 means inherit the engine's existing alert ranges. MapList distances are tiles.
inline int ParseMapAlertDistance(const std::string &text) {
	try {
		size_t end = 0;
		auto value = std::stoi(text, &end);
		if (end == text.size() && value >= 0 && value <= 1000)
			return value;
	} catch (const std::exception &) {
	}
	return -1;
}

inline int LimitMapAlertDistance(int defaultRange, int mapLimit) {
	return mapLimit >= 0 && mapLimit < defaultRange ? mapLimit : defaultRange;
}
