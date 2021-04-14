#include "stdafx.h"
#include "common.h"
#include "history.h"
#include "damage.h"
#include "tig/tig_mes.h"
#include "util/fixes.h"
#include <gamesystems/d20/d20_help.h>
#include "party.h"
#include "description.h"
#include "combat.h"
#include <string.h>
#include "spell.h"

#define HISTORY_ARRAY_SIZE 150

struct HistoryEntry{
	uint32_t histId;
	uint32_t histType;
	objHndl obj;
	ObjectId objId;
	char objDescr[2000];
	objHndl obj2;
	ObjectId obj2Id;
	char obj2Descr[2000];
	uint32_t prevId;
	uint32_t nextId;
};

struct HistoryEntryType0 : HistoryEntry{
	int rollResult;
	int unk;
	BonusList bonlist;
	int rollIdType7;
	int overallBonus;
	D20CAF d20Caf;
	int pad[99];
};

struct HistoryEntryType1 : HistoryEntry
{
	DamagePacket dmgPkt;

	HistoryEntryType1(objHndl attacker, objHndl tgt, DamagePacket* dmg);
};


struct HistoryEntryType2 : HistoryEntry
{
	uint32_t dicePacked;
	int32_t rollResult;
	uint32_t skillIdx;
	uint32_t dc;
	BonusList bonlist;
	uint32_t pad[100];

	HistoryEntryType2(objHndl obj, uint32_t dicePacked, int rollResult, int dc, int skillIdx, BonusList* Bonlist);
};

struct HistoryEntryType3 : HistoryEntry
{
	uint32_t dicePacked;
	int32_t rollResult;
	uint32_t dc;
	SavingThrowType saveType;
	uint32_t saveFlags;
	BonusList bonlist;
	uint32_t pad[99];

	HistoryEntryType3(objHndl obj, uint32_t dicePacked, int rollResult, int dc, SavingThrowType saveType, int flags, BonusList* Bonlist);
};

struct HistoryEntryType4 : HistoryEntry {
	uint32_t dicePacked;
	int rollResult;
	int dc;
	int saveType;
	const char* text;
	BonusList bonlist;
	int pad[99];

	HistoryEntryType4(objHndl obj, int dicePacked, int rollResult, int dc, const char* text, BonusList* Bonlist);
};

struct HistoryEntryType5 : HistoryEntry { // used for rolls vs a percentage, like concealment, or stuff like blink spell
	int failureChance;
	int rollResult;
	int combatMesFailureReason; // description for the cause of the roll (e.g. 
	int combatMesResult; // descrition for the result
	int combatMesTitle;
	
	int pad[321];

	HistoryEntryType5(objHndl Obj, objHndl Target, int FailureChance, int combatMesFailureReason, int RollResult, int combatMesResult, int combatMesTitle);
};

HistoryEntryType4::HistoryEntryType4(objHndl Obj, int DicePacked, int RollResult, int Dc, const char* Text, BonusList *Bonlist) {
	histType = 4;
	obj = Obj;
	obj2 = 0i64;
	dicePacked = DicePacked;
	dc = Dc;
	rollResult = RollResult;
	text = Text;
	bonlist = *Bonlist;
}

HistoryEntryType5::HistoryEntryType5(objHndl Obj, objHndl Target, int FailureChance, int CombatMesFailureReason, int RollResult, int CombatMesResult, int CombatMesTitle){
	histType = 5;
	obj = Obj;
	obj2 = Target;
	failureChance = FailureChance;
	rollResult = RollResult;
	combatMesFailureReason = CombatMesFailureReason;
	combatMesTitle = CombatMesTitle;
	combatMesResult = CombatMesResult;
}

struct HistoryEntryType7: HistoryEntry{
	int line;
	int unk; // always 0??
	BonusList bonlist;
};


struct HistoryArrayEntry{
	uint32_t field0;
	uint32_t time; // time when entry is added to array
	union
	{
		HistoryEntryType1 base;
		HistoryEntryType1 t1;
		HistoryEntryType2 t2;
		HistoryEntryType3 t3;
		HistoryEntryType4 t4;
		HistoryEntryType5 t5;
	}entry;
};



struct HistorySystemAddresses : temple::AddressTable
{

	int32_t *rollSerialNumber; 
	MesHandle * rollUiMesHandle;
	HistoryArrayEntry * histArray;
	int (__cdecl *RollHistoryType1Add)(objHndl objHnd, objHndl objHnd2, DamagePacket *damPkt);
	int(__cdecl *RollHistoryType2Add)(objHndl objHnd, objHndl objHnd2, int skillIdx, int dicePacked, int rollResult, int DC, BonusList *bonlist);
	int (__cdecl *RollHistoryType3Add)(objHndl obj, int DC, int saveType, int flags, int dicePacked, int rollResult, BonusList *bonListIn);
	int (__cdecl *RollHistoryAddType6OpposedCheck)(objHndl attacker, objHndl defender, int attackerRoll, int defenderRoll, BonusList* attackerBonlist, BonusList* defenderBonlist, int combatMesLineTitle, int combatMesLineResult, int flag);
	int (__cdecl*CreateRollHistoryString)(int histId);
	int(__cdecl*CreateRollHistoryLineFromMesfile)(int historyMesLine, objHndl obj, objHndl obj2);
	void(*CreateFromFreeText)(const char* text);
	HistorySystemAddresses()
	{

		rebase(RollHistoryType1Add, 0x10047C80);
		rebase(RollHistoryType2Add, 0x10047CF0);
		rebase(RollHistoryType3Add, 0x10047D90);
		rebase(RollHistoryAddType6OpposedCheck, 0x10047F70);
		rebase(CreateRollHistoryString, 0x100DFFF0);
		rebase(rollUiMesHandle, 0x102B0168);
		rebase(rollSerialNumber, 0x102B016C);
		rebase(histArray, 0x109DDA20);
		rebase(CreateFromFreeText, 0x100DFFC0);
		rebase(CreateRollHistoryLineFromMesfile, 0x100E01F0);
		
	}

	
} addresses;


class HistSysReplacements : public TempleFix
{

public:
	void apply() override {

		static void(__cdecl*orgParseHistoryEntry)(int, D20RollHistoryEntry *) = replaceFunction<void(__cdecl)(int, D20RollHistoryEntry *)>(0x1019CE60, [](int histId, D20RollHistoryEntry *rh) {
			auto hist = histSys.HistoryFind(histId);
			if (!hist)
				return;
			if (hist->histType != 3) {
				orgParseHistoryEntry(histId, rh);
				return;
			}

			// saving throw
			auto hist3 = (HistoryEntryType3*)hist;
			//if normal roll
			if (hist3->rollResult != 20 && hist3->rollResult != 1) {
				orgParseHistoryEntry(histId, rh);
				return;
			}

			
			rh->Clear();
			/*
			auto observer = party.GetConsciousPartyLeader();
			auto descr = description.getDisplayName(hist->obj, observer);
			histSys.GetRollUiString();
			if (hist3->rollResult == 1) {
				
			}*/
			
		});

		// Print to D20 Roll console
		static void(__cdecl*orgPrintHistoryEntryToD20Console)(int, char *) = replaceFunction<void(__cdecl)(int, char *)>(0x10048960, [](int histId, char *textOut) {
			auto hist = histSys.HistoryFind(histId);
			if (!hist)
				return;

			if (hist->histType != 3){
				return orgPrintHistoryEntryToD20Console(histId, textOut);
			}

			auto hist3 = (HistoryEntryType3*)hist;
			if (hist3->rollResult != 20 && hist3->rollResult != 1)
				return 	 orgPrintHistoryEntryToD20Console(histId, textOut);

			auto observer = party.GetConsciousPartyLeader();
			auto descr = description.getDisplayName(hist->obj, observer);
			auto saveTypeString = combatSys.GetCombatMesLine( (int)(hist3->saveType) + 500);
			auto text = fmt::format("{} {} {} {} - {}", descr, histSys.GetRollUiString(26), saveTypeString, histSys.GetRollUiString(18), histSys.GetRollUiString(20 + (hist3->rollResult != 20) ));
			if (hist3->rollResult == 20){
				text.append(fmt::format(" (natural 20)\n\n"));
			} else
			{
				text.append(fmt::format(" (natural 1)\n\n"));
			}

			sprintf(textOut, "%s", text.c_str());
			textOut[text.size()] = 0;
		});

	}
} historySysReplacements;

#pragma region History System Implementation
HistorySystem histSys;


int HistorySystem::RollHistoryAddType6OpposedCheck(objHndl attacker, objHndl defender, int attackerRoll, int defenderRoll, BonusList* attackerBonlist, BonusList* defenderBonlist, int combatMesLineTitle, int combatMesLineResult, int flag)
{
	return addresses.RollHistoryAddType6OpposedCheck(attacker, defender, attackerRoll, defenderRoll, attackerBonlist, defenderBonlist, combatMesLineTitle, combatMesLineResult, flag);
}

int HistorySystem::CreateRollHistoryString(int histId)
{
	return addresses.CreateRollHistoryString(histId);
}

HistorySystem::HistorySystem()
{
	rebase(RollHistoryAdd, 0x10047430);
}

int HistorySystem::CreateRollHistoryLineFromMesfile(int historyMesLine, objHndl obj, objHndl obj2)
{
	return addresses.CreateRollHistoryLineFromMesfile(historyMesLine, obj, obj2);
}

int HistorySystem::RollHistoryAddType1DamageRoll(objHndl attacker, objHndl tgt, DamagePacket* dmg)
{
	HistoryEntryType1 hist(attacker, tgt, dmg);

	auto id = RollHistoryAdd(&hist);
	AppendHistoryId(id);
	return id;
}

int HistorySystem::RollHistoryType2Add(objHndl obj, uint32_t dicePacked, int rollResult, int dc, int skillIdx, BonusList* bonlist)
{
	HistoryEntryType2 hist(obj, dicePacked, rollResult, dc, skillIdx, bonlist);
	auto id = RollHistoryAdd(&hist);
	AppendHistoryId(id);
	return id;
}

int HistorySystem::RollHistoryType3Add(objHndl handle, int dc, SavingThrowType saveType, int flags, uint32_t dicePacked, int d20RollRes, BonusList * bonlist)
{
	HistoryEntryType3 hist(handle, dicePacked, d20RollRes, dc, saveType, flags,bonlist);
	auto id = RollHistoryAdd(&hist);
	AppendHistoryId(id);
	return id;
}

int HistorySystem::RollHistoryType4Add(objHndl obj, int dc, const char* text, uint32_t dicePacked, int d20RollRes, BonusList* bonlist){
	HistoryEntryType4 hist(obj, dicePacked,d20RollRes, dc, text, bonlist);
	auto id = RollHistoryAdd(&hist);
	AppendHistoryId(id);
	return id;
}

int HistorySystem::RollHistoryType7Add(objHndl obj, BonusList* bonlist, int line, int unk){
	HistoryEntryType7 hist;
	hist.histType = 7;
	hist.obj = obj;
	hist.obj2 = 0i64;
	hist.bonlist = *bonlist;
	hist.line = line;
	hist.unk = unk;
	return RollHistoryAdd(&hist);
}

int HistorySystem::RollHistoryType0Add(int RollResult, int CritHitRoll, objHndl Obj, objHndl Obj2, BonusList* Bonlist, BonusList* Bonlist2, D20CAF flags){
	HistoryEntryType0 hist;
	hist.rollResult = RollResult;
	hist.obj = Obj;
	hist.obj2 = Obj2;
	hist.histType = 0;
	hist.unk = CritHitRoll;
	hist.bonlist = *Bonlist;
	hist.d20Caf = flags;
	hist.rollIdType7 = RollHistoryType7Add(Obj2, Bonlist2, 33, 0);
	hist.overallBonus = Bonlist2->GetEffectiveBonusSum();
	auto id = RollHistoryAdd(&hist);
	AppendHistoryId(id);
	return id;
}

int HistorySystem::RollHistoryType5Add(objHndl obj, objHndl tgt, int failChance, int combatMesFailureReason, int rollResult, int combatMesResult, int combatMesTitle){
	HistoryEntryType5 hist(obj, tgt, failChance, combatMesFailureReason, rollResult, combatMesResult, combatMesTitle);
	auto id = RollHistoryAdd(&hist);
	AppendHistoryId(id);
	return id;
}

void HistorySystem::CreateFromFreeText(const char*text)
{
	addresses.CreateFromFreeText(text);
}

void HistorySystem::PrintSpellCast(objHndl caster, int spEnum){
	auto historyFormat = histSys.GetHistoryMesLine(49); // [ACTOR] casts [SPELL NAME]!
	const char* actorName = nullptr;
	
	if (caster){
		actorName = description.getDisplayName(caster, party.GroupPCsGetMemberN(0)); // Get a PC to identify the caster (regards HasMet status)
	}

	auto spellHelpId = spellSys.GetSpellEnumTAG(spEnum);
	auto spellName = spellSys.GetSpellMesline(spEnum);

	char txtBuffer[512];
	char stringOut[512];
	sprintf(txtBuffer, "~%s~[%s]", spellName, spellHelpId);
	auto strGenerator = temple::GetRef<void(__cdecl)(char*, const char*, const char*, const char*, const char*)>(0x100E00B0);
	strGenerator(stringOut, historyFormat, actorName, nullptr, txtBuffer);
	
	GetD20RollHistoryConsole().CreateFromString(stringOut);
}

HistoryEntry* HistorySystem::HistoryFind(int histId){
	auto histArray = addresses.histArray;
	for (auto i = 0; i < HISTORY_ARRAY_SIZE; i++){
		if (histArray[i].entry.base.histId == histId)
			return &histArray[i].entry.base;
	}
	return nullptr;
}

void HistorySystem::AppendHistoryId(int histId){
	auto histArray = addresses.histArray;
	auto &lastHistId = temple::GetRef<int>(0x109DDA18);
	auto idx = 0;
	for ( ; idx < HISTORY_ARRAY_SIZE; idx++){
		if (histArray[idx].entry.base.histId == histId)
			break;
	}
	if (idx >= HISTORY_ARRAY_SIZE){
		// originally did some naughty stuff here, I think I'll leave it out...
	} 
	else{
		auto &hist = histArray[idx].entry;
		hist.base.prevId = lastHistId;
		hist.base.nextId = 0;

		if (lastHistId)	{

			HistoryEntry *lastHist = HistoryFind(lastHistId);
			
			if (lastHist)
				lastHist->nextId = histId;
		}
		lastHistId = histId;
	}
}
void HistorySystem::ParseHistoryEntry(int histId, D20RollHistoryEntry * rh)
{
	//todo
}
const char * HistorySystem::GetRollUiString(int lineId)
{
	MesLine line(lineId);
	mesFuncs.GetLine_Safe(*addresses.rollUiMesHandle, &line);
	return line.value;
}
const char * HistorySystem::GetHistoryMesLine(int lineId){
	MesLine line(lineId);
	mesFuncs.GetLine_Safe(temple::GetRef<MesHandle>(0x10BCAD9C), &line);
	return line.value;
}
D20RollHistoryEntry & HistorySystem::GetD20RollHistoryConsole(){
	return temple::GetRef<D20RollHistoryEntry>(0x11868F80);
}
#pragma endregion

#pragma region Replacements


#pragma endregion



const auto TestSizeOfHistoryEntryType0 = sizeof(HistoryEntryType0); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType1 = sizeof(HistoryEntryType1); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType2 = sizeof(HistoryEntryType2); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType3 = sizeof(HistoryEntryType3); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType4 = sizeof(HistoryEntryType4); // should be 5384 (0x1508)
const auto TestSizeOfHistoryEntryType5 = sizeof(HistoryEntryType5); // should be 5384 (0x1508)
const auto TestSizeOfHistoryArrayEntry = sizeof(HistoryArrayEntry); // should be 5392 (0x1510)

HistoryEntryType3::HistoryEntryType3(objHndl handle, uint32_t dicePacked, int rollResult, int dc, SavingThrowType saveType, int flags, BonusList * bonlistIn)
{
	this->histType = 3;
	this->obj = handle;
	this->obj2 = objHndl::null;
	this->dc = dc;
	this->saveType = saveType;
	this->saveFlags = flags;
	this->dicePacked = dicePacked;
	this->rollResult = rollResult;
	this->bonlist = *bonlistIn;
}

HistoryEntryType2::HistoryEntryType2(objHndl obj, uint32_t dicePacked, int rollResult, int dc, int skillIdx, BonusList* Bonlist)
{
	this->histType = 2;
	this->obj = obj;
	this->obj2 = objHndl::null;
	this->dc = dc;
	this->skillIdx = skillIdx;
	this->dicePacked = dicePacked;
	this->rollResult = rollResult;
	this->bonlist = *Bonlist;
}

HistoryEntryType1::HistoryEntryType1(objHndl attacker, objHndl tgt, DamagePacket* dmg)
{
	this->histType = 1;
	this->obj = attacker;
	this->obj2 = tgt;
	memcpy(&this->dmgPkt, dmg, sizeof(DamagePacket));
}
