#include "../TemplePlus/gamesystems/map_alert.h"
#include <cassert>

int main() {
	assert(ParseMapAlertDistance("0") == 0);
	assert(ParseMapAlertDistance("10") == 10);
	assert(ParseMapAlertDistance("1000") == 1000);
	for (auto invalid : {"", "-1", "10 feet", "2.5", "1001", "999999999999999999999"})
		assert(ParseMapAlertDistance(invalid) == -1);
	for (int original : {0, 1, 10, 24, 40}) {
		assert(LimitMapAlertDistance(original, -1) == original);
		assert(LimitMapAlertDistance(original, 1000) == original);
		assert(LimitMapAlertDistance(original, 0) == 0);
	}
	assert(LimitMapAlertDistance(24, 10) == 10);
	assert(LimitMapAlertDistance(4, 10) == 4);
}
