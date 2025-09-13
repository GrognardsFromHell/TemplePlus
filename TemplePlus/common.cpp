
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"
#include "bonus.h"
#include "feat.h"
#include "d20.h"
#include <infrastructure/elfhash.h>
#include <map>

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

void BonusList::Reset()
{
	memset(this, 0, sizeof(BonusList));
	/*this->bonCount = 0;
	this->bonCapperCount = 0;
	this->zeroBonusCount = 0;
	this->bonFlags = 0;*/
	this->overallCapHigh.bonValue = 0x7fffFFFF;
	this->overallCapLow.bonValue = 0x80000001;
}

int BonusList::GetEffectiveBonusSum() const {

	int result = 0;

	for (size_t i = 0; i < bonCount; ++i) {
		// need to wait for the rest of the sum first
		if (IsPenaltyCappedPositive(i)) continue;

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

	for (size_t i = 0; i < bonCount; ++i) {
		// nothing in this loop can reduce result below 1
		if (result <= 1) break;

		if (!IsPenaltyCappedPositive(i)) continue;
		if (IsBonusSuppressed(i, nullptr)) continue;

		auto& bonus = bonusEntries[i];
		auto value = bonus.bonValue;

		size_t capIdx;
		if (IsBonusCapped(i, &capIdx)) {
			auto capValue = bonCaps[capIdx].capValue;
			if (value > 0 && value > capValue) {
				value = capValue;
			} else if (value < 0 && value < capValue) {
				value = capValue;
			}
		}

		result += value;
		if (result < 1) result = 1;
	}

	if (bonFlags & 1 && result > overallCapHigh.bonValue) {
		result = overallCapHigh.bonValue;
	}
	if (bonFlags & 2 && result < overallCapLow.bonValue) {
		result = overallCapLow.bonValue;
	}

	return result;
}

int BonusList::GetBaseScaled(float factor) const
{
	int initial = 0;
	int exponent = 0;

	for (size_t i = 0; i < bonCount; ++i) {
		auto& bonus = bonusEntries[i];
		auto  value = bonus.bonValue;

		if (IsBonusSuppressed(i, nullptr)) continue;

		size_t capIdx;
		if (IsBonusCapped(i, &capIdx)) {
			auto capValue = bonCaps[capIdx].capValue;

			if (value > 0 && value > capValue) {
				value = capValue;
			} else if (value < 0 && value < capValue) {
				value = capValue;
			}
		}

		if (1 == bonus.bonType) {
			initial = value;
		} else {
			exponent += value;
		}
	}

	if (exponent == 0) return initial;

	float scaled = static_cast<float>(initial);
	if (exponent < 0) {
		for (int i = 0; i > exponent; i--) {
			scaled /= factor;
		}
	} else {
		for (int i = 0; i < exponent; i++) {
			scaled *= factor;
		}
	}

	return static_cast<int>(scaled);
}

/* 0x100E6680 */
int BonusList::GetHighestBonus() const
{
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
			}
			else if (value < 0 && value < capValue) {
				value = capValue;
			}
		}

		if (!IsBonusSuppressed(i, nullptr) && result < value) {
			result = value;
		}
	}

	if (bonFlags & 1 && result > overallCapHigh.bonValue) {
		result = overallCapHigh.bonValue;
	}
	if (bonFlags & 2 && result < overallCapLow.bonValue) {
		result = overallCapLow.bonValue;
	}

	return result;
}

int BonusList::GetLargestPenalty() const
{
	int result = 0;

	for (size_t i = 0; i < bonCount; ++i) {
		auto& bonus = bonusEntries[i];

		auto value = bonus.bonValue;

		if (value >= 0) continue;

		size_t capIdx;
		if (IsPenaltyCapped(i, &capIdx)) {
			auto capValue = bonCaps[capIdx].capValue;

			if (value < capValue) {
				value = capValue;
			}
		}

		if (!IsBonusSuppressed(i, nullptr) && result > value) {
			result = value;
		}
	}

	if (bonFlags & 2 && result < overallCapLow.bonValue) {
		result = overallCapLow.bonValue;
	}

	return result;
}

void BonusList::Simplify()
{
	int presult = 0;

	for (size_t i = 0; i < bonCount; ++i) {
		if (IsPenaltyCappedPositive(i)) continue;

		auto& bonus = bonusEntries[i];
		auto value = bonus.bonValue;

		size_t capIdx;
		if (IsBonusCapped(i, &capIdx)) {
			auto capValue = bonCaps[capIdx].capValue;

			if (value > 0 && value > capValue) {
				value = capValue;
			} else if (value < 0 && value < capValue) {
				value = capValue;
			}
		}

		if (!IsBonusSuppressed(i, nullptr)) {
			presult += value;
		}
	}

	for (size_t i = 0; i < bonCount; ++i) {
		if (!IsPenaltyCappedPositive(i)) continue;

		auto& bonus = bonusEntries[i];
		auto value = bonus.bonValue;

		if (IsBonusSuppressed(i, nullptr)) {
			bonus.bonValue = 0;
			continue;
		}

		size_t capIdx;
		if (IsBonusCapped(i, &capIdx)) {
			auto capValue = bonCaps[capIdx].capValue;
			
			if (value > 0 && value > capValue) {
				value = capValue;
			} else if (value < 0 && value < capValue) {
				value = capValue;
			}
		}

		if (presult <= -value) {
			value = 1 - presult;
			if (value > 0) value = 0;
			bonus.bonValue = value;
			auto newMes = fmt::format("{} (penalty capped)", bonus.bonusMesString);
			bonus.bonusMesString = bonusSys.CacheCustomText(newMes);
		}
		presult += value;
	}
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
	if (bonus.bonValue < 0)
		return IsPenaltyCapped(bonusIdx, cappedByIdx);

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

bool BonusList::IsPenaltyCapped(size_t bonusIdx, size_t* cappedByIdx) const
{
	// Added Temple+: penalty caps are specified by the negative of the bonus type
	// Penalty capping bonus type = 0 is not supported (no actual use cases AFAIK)

	int restrictiveCap = -255;
	auto foundCap = false;

	auto& bonus = bonusEntries[bonusIdx];
	auto penaltyValue = bonus.bonValue;

	if (penaltyValue >= 0) {
		return false;
	}

	for (size_t i = 0; i < bonCapperCount; ++i) {

		
		auto penaltyCapType = -bonCaps[i].bonType; 
		if (penaltyCapType <= 0) // penalty caps are specified by strictly negative bonus type
			continue;
		if (penaltyCapType != bonus.bonType) {
			continue;
		}

		int capVal = bonCaps[i].capValue;
		if (capVal > restrictiveCap) {
			restrictiveCap = capVal;
			if (cappedByIdx) {
				*cappedByIdx = i;
			}
			foundCap = true;
		}

	}

	if (!foundCap) {
		return false;
	}

	return penaltyValue < restrictiveCap;
}

bool BonusList::IsPenaltyCappedPositive(size_t bonusIdx) const
{
	auto & bonus = bonusEntries[bonusIdx];
	bool hasFlag = !!(bonus.bonType & PenaltyCapPositive);

	return bonus.bonValue < 0 && hasFlag;
}

int BonusList::AddBonusWithDesc(int value, int bonType, int mesline, const char* descr)
{
	if (AddBonus(value, bonType, mesline ))
	{
		bonusEntries[bonCount - 1].bonusDescr = (char*) descr;
		return 1;
	}
	return 0;
}

int BonusList::AddBonusWithDesc(int value, int bonType, std::string & text, char * descr){
	if (AddBonus(value, bonType, text)){
		this->bonusEntries[this->bonCount - 1].bonusDescr = descr;
		return TRUE;
	}
	return FALSE;
}

int BonusList::AddBonusFromFeat(int value, int bonType, int mesline, feat_enums feat){
	auto featName = feats.GetFeatName(feat);
	return AddBonusWithDesc(value, bonType, mesline, featName);
}

int BonusList::AddBonusFromFeat(int value, int bonType, int mesline, std::string & feat)
{
	return AddBonusFromFeat(value, bonType, mesline, (feat_enums)ElfHash::Hash(feat));
}

int BonusList::ModifyBonus(int value, int bonType, int meslineIdentifier){
	MesLine line(meslineIdentifier);	
	bonusSys.GetBonusMesLine(line);
	
	for (auto i=0u; i< bonCount; i++){
		auto &entry = bonusEntries[i];

		if (entry.bonType == bonType){

			if (meslineIdentifier && line.value == entry.bonusMesString)	{
				entry.bonValue += value;
			}
			
			if (meslineIdentifier)
				break;
		}
	}
	return 0;
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
		if (bonCapperCount >= 10) return 0;
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

int BonusList::AddCapWithDescr(int capType, int capValue, uint32_t bonMesLineNum, const char* capDescr){

	if (AddCap(capType, capValue, bonMesLineNum) == 1){
		bonCaps[bonCapperCount - 1].bonCapDescr = capDescr;
		return 1;
	}
	return 0;
}

int BonusList::AddCapWithCustomDescr(int capType, int capValue, uint32_t bonMesLineNum, std::string & textArg)
{
	auto textId = ElfHash::Hash(textArg);
	auto textCache = bonusSys.customBonusStrings.find(textId);
	if (textCache == bonusSys.customBonusStrings.end()) {
		bonusSys.customBonusStrings[textId] = textArg;
	}
	if (AddCap(capType, capValue, bonMesLineNum) == 1) {
		bonCaps[bonCapperCount - 1].bonCapDescr = bonusSys.customBonusStrings[textId].c_str();
		return 1;
	}
	return 0;
}





BOOL BonusList::SetOverallCap(int newBonFlags, int newCap, int newCapType, int newCapMesLineNum, const char *capDescr) {
	if (!newBonFlags)
		return FALSE;

	auto bonMesString = GetBonusMesLine(newCapMesLineNum);

	if (newBonFlags & 1 && (overallCapHigh.bonValue > newCap || newBonFlags & 4)) {
		overallCapHigh.bonValue = newCap;
		overallCapHigh.bonType = newCapType;
		overallCapHigh.bonusMesString = bonMesString;
		overallCapHigh.bonusDescr = capDescr;
	}
	if (newBonFlags & 2 && (overallCapLow.bonValue < newCap || newBonFlags & 4)) {
		overallCapLow.bonValue = newCap;
		overallCapLow.bonType = newCapType;
		overallCapLow.bonusMesString = bonMesString;
		overallCapLow.bonusDescr = capDescr;
	}

	bonFlags |= newBonFlags;
	return TRUE;
}

objHndl AttackPacket::GetWeaponUsed() const
{
	if ( !(flags& D20CAF_TOUCH_ATTACK) ||  (flags & D20CAF_THROWN_GRENADE) )
	{
		return weaponUsed;
	}
	return objHndl::null;
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

int BonusList::AddBonus(int value, int bonType, int mesline){
	return bonusSys.bonusAddToBonusList(this, value, bonType, mesline);
}

BOOL BonusList::AddBonus(int value, int bonType, std::string & textArg){
	if (this->bonCount >= BonusListMax) return FALSE;

	auto mesText = bonusSys.CacheCustomText(textArg);

	this->bonusEntries[this->bonCount].bonValue = value;
	this->bonusEntries[this->bonCount].bonType = bonType;
	this->bonusEntries[this->bonCount].bonusMesString = mesText;
	this->bonusEntries[this->bonCount++].bonusDescr = nullptr;
	return TRUE;
}


const char* BonusList::GetBonusMesLine(int lineNum) {
	MesLine mesLine(lineNum);

	if (lineNum >= 335)
		mesFuncs.GetLine_Safe(bonusSys.bonusMesNew, &mesLine);
	else
		mesFuncs.GetLine_Safe(*bonusSys.bonusMesHandle, &mesLine);

	return mesLine.value;
}

void GroupArray::Clear() {
	this->GroupSize = 0;
	memset(this->GroupMembers, 0, sizeof(this->GroupMembers));
}

void GroupArray::Reset(){
	Clear();
	this->sortFunc = nullptr;
}

inline std::ostream& operator<<(std::ostream& os, const ScreenDirections& direc)
{
	switch (direc)
	{
	case ScreenDirections::Top:
		return os << "U";
	case ScreenDirections::TopRight:
		return os << "UR";
	case ScreenDirections::Right:
		return os << "R";
	case ScreenDirections::BottomRight:
		return os << "DR";
	case ScreenDirections::Bottom:
		return os << "D";
	case ScreenDirections::BottomLeft:
		return os << "DL";
	case ScreenDirections::Left:
		return os << "L";
	case ScreenDirections::TopLeft:
		return os << "UL";
	default:
		return os << "NA";
	}
}
