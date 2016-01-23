
#include "stdafx.h"
#include "common.h"

int BonusList::GetEffectiveBonusSum() const {

	int result = 0;

	for (size_t i = 0; i < bonCount; ++i) {
		auto& bonus = bonusEntries[i];

		auto value = bonus.bonValue;

		// Apply bonus caps if necessary
		size_t capIdx;
		if (IsBonusCapped(i, &capIdx)) {
			auto capValue = bonCaps[capIdx].capValue;
			
			// Handles malus / bonus capping separately
			if (value > 0 && value > capValue) {
				value = capValue;
			} else if (value < 0 && value < capValue) {
				value = capValue;
			}
		}

		if (!IsBonusSuppressed(i, nullptr)) {
			result += value;
		}
	}

	if (bonFlags & 1 && result > overallCapHigh) {
		result = overallCapHigh;
	}
	if (bonFlags & 2 && result < overallCapLow) {
		result = overallCapLow;
	}

	return result;
}

bool BonusList::IsBonusSuppressed(size_t bonusIdx, size_t* suppressedByIdx) const {
	Expects(bonusIdx < bonCount);

	auto curHighest = bonusEntries[bonusIdx].bonValue;
	auto type = bonusEntries[bonusIdx].bonValue;
	auto isMalus = (curHighest <= 0);
	auto curIdx = bonusIdx;
	bool suppressed = false;

	// These bonus types stack and are therefor never suppressed
	if (type == 0 || type == 8 || type == 21) {
		return false;
	}

	for (size_t i = 0; i < bonCount; ++i) {
		auto& other = bonusEntries[bonusIdx];
		// Cannot be suppressed by itself or a bonus of another type
		if (i == bonusIdx || other.bonType != type) {
			continue;
		}
				
		// For bonuses of the same value, we use their position in the list as the tiebreaker
		// For bonuses, it's the first, for maluses it's the last
		if (isMalus)
		{
			if (other.bonValue > curHighest || (other.bonValue == curHighest && i < bonusIdx)) {
				continue;
			}
		} else {
			if (other.bonValue < curHighest || (other.bonValue == curHighest && i > bonusIdx)) {
				continue;
			}
		}
				
		suppressed = true;
		curIdx = i;
		curHighest = other.bonValue;
	}

	if (suppressed && suppressedByIdx) {
		*suppressedByIdx = curIdx;
	}

	return suppressed;

}

bool BonusList::IsBonusCapped(size_t bonusIdx, size_t* cappedByIdx) const {
	int lowestCap = 255;
	auto foundCap = false;

	auto& bonus = bonusEntries[bonusIdx];

	for (size_t i = 0; i < bonCapperCount; ++i) {

		// Caps apparently can apply to all bonus types or only specific ones
		if (bonCaps[i].bonType && bonCaps[i].bonType != bonus.bonType) {
			continue;
		}

		int capVal = abs(bonCaps[i].capValue);
		if (capVal < lowestCap) {
			lowestCap = capVal;
			if (cappedByIdx) {
				*cappedByIdx = i;
			}
			foundCap = true;
		}
		
	}

	if (!foundCap || bonus.bonValue < lowestCap) {
		return false;
	}

	return true;

}
