
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"
#include "bonus.h"
#include "feat.h"
#include "d20.h"

static void NormalizeAxis(float& offset, uint32_t &tilePos) {
	auto tiles = (int) (offset / INCH_PER_TILE);
	if (tiles != 0) {
		tilePos += tiles;
		offset -= tiles * INCH_PER_TILE;
	}
}

void LocAndOffsets::Normalize() {
	NormalizeAxis(off_x, location.locx);
	NormalizeAxis(off_y, location.locy);
}

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
	auto type = bonusEntries[bonusIdx].bonType;
	auto isMalus = (curHighest <= 0);
	auto curIdx = bonusIdx;
	bool suppressed = false;

	// These bonus types stack and are therefor never suppressed
	if (type == 0 || type == 8 || type == 21) {
		return false;
	}

	for (size_t i = 0; i < bonCount; ++i) {
		auto& other = bonusEntries[i];
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

int BonusList::AddBonusWithDesc(int value, int bonType, int mesline, char* descr)
{
	if (AddBonus(value, bonType, mesline ))
	{
		bonusEntries[bonCount - 1].bonusDescr = descr;
		return 1;
	}
	return 0;
}

int BonusList::AddBonusFromFeat(int value, int bonType, int mesline, feat_enums feat){
	auto featName = feats.GetFeatName(feat);
	return AddBonusWithDesc(value, bonType, mesline, featName);
}

BOOL BonusList::ZeroBonusSetMeslineNum(int mesline)
{
	if (zeroBonusCount >= 10)
		return FALSE;
	
	zeroBonusReasonMesLine[zeroBonusCount++] = mesline;
	return TRUE;
}

int BonusList::AddCap(int capType, int capValue, uint32_t bonMesLineNum){
	
	MesLine mesLine;
	if (bonCapperCount >= 10) {
		auto breakPointDummy = 1;
		if (bonCapperCount >= BonusListMax) return 0;// bug? there's only 10 slots (this is the original code!)
	}

	mesLine.key = bonMesLineNum;
	if (bonMesLineNum >= 335)
		mesFuncs.GetLine_Safe(bonusSys.bonusMesNew, &mesLine);
	else
		mesFuncs.GetLine_Safe(*bonusSys.bonusMesHandle, &mesLine);
	bonCaps[bonCapperCount].capValue = capValue;
	bonCaps[bonCapperCount].bonType = capType;
	bonCaps[bonCapperCount].bonCapperString = (char*)mesLine.value;
	bonCaps[bonCapperCount++].bonCapDescr = nullptr;
	return 1;
	
}

int BonusList::AddCapWithDescr(int capType, int capValue, uint32_t bonMesLineNum, char* capDescr){

	if (AddCap(capType, capValue, bonMesLineNum) == 1){
		bonCaps[bonCapperCount - 1].bonCapDescr = capDescr;
		return 1;
	}
	return 0;
}

objHndl AttackPacket::GetWeaponUsed() const
{
	if ( !(flags& D20CAF_TOUCH_ATTACK) ||  (flags & D20CAF_THROWN_GRENADE) )
	{
		return weaponUsed;
	}
	return 0i64;
}

bool AttackPacket::IsOffhandAttack()
{
	if (flags & D20CAF_SECONDARY_WEAPON) {
		return true;
	}

	if (dispKey == ATTACK_CODE_OFFHAND + 2 )
		return true;

	return false;
}

AttackPacket::AttackPacket(): field_1C(0) {
	flags = D20CAF_NONE;
	victim = 0i64;
	attacker = 0i64;
	weaponUsed = 0i64;
	ammoItem = 0i64;
	d20ActnType = D20A_STANDARD_ATTACK;
	dispKey = DK_NONE;
}

PointNode::PointNode(){
	absX = 0;
	absY = 0;
	absZ = 0;
}

PointNode::PointNode(float x, float y, float z){
	absX = x;
	absZ = z;
	absY = y;
}

int BonusList::AddBonus(int value, int bonType, int mesline)
{
	return bonusSys.bonusAddToBonusList(this, value, bonType, mesline);
}
