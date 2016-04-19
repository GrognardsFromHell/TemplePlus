#include "stdafx.h"

#include <temple/dll.h>
#include "config/config.h"
#include "util/fixes.h"

struct CharCreationAddresses : temple::AddressTable {
	uint32_t* pointBuyPointsRemaining;

	CharCreationAddresses() {
		rebase(pointBuyPointsRemaining, 0x10C453F4);
	}

} addresses;

static class CharCreationSystem : TempleFix {
public:
	void apply() override;
} fix;

void CharCreationSystem::apply() {
	auto pointsMaxPrint = static_cast<uint8_t>(config.pointBuyPoints);
	write(0x1018B575 + 1, &pointsMaxPrint, sizeof(uint8_t));
	write(0x1018B5FD + 2, &pointsMaxPrint, sizeof(uint8_t));
	write(0x1018B52E + 6, &pointsMaxPrint, sizeof(uint8_t));
	write(0x1018B9EF + 1, &pointsMaxPrint, sizeof(uint8_t));
}
