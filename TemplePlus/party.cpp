#include "stdafx.h"
#include "common.h"
#include "party.h"
#include "config/config.h"
#include "util/fixes.h"
#include "critter.h"
#include "obj.h"
#include "location.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "condition.h"
#include "combat.h"
#include "turn_based.h"

struct LegacyPartySystemAddresses : temple::AddressTable
{
	GroupArray * groupNpcFollowers;
	GroupArray * groupCurrentlySelected;  
	GroupArray * groupAiFollowers; 
	GroupArray * groupPcs;
	GroupArray * groupList; // the entire party (PCs, NPCs and AI followers)

	void(__cdecl * AddToCurrentlySelected)(objHndl objHnd);
	void(*ArraySort)(void* arr, size_t arrSize, size_t elementSize, int(*sortFunc)(void*, void*));

	int * partyMoney; // size 4 array containing: CP, SP, GP, PP

	LegacyPartySystemAddresses()
	{
		rebase(AddToCurrentlySelected, 0x1002B560);
		rebase(ArraySort, 0x10254750);
		rebase(groupNpcFollowers, 0x11E71BE0);
		rebase(groupCurrentlySelected, 0x11E71D00);
		rebase(groupAiFollowers, 0x11E71E20);
		rebase(groupPcs , 0x11E71F40);
		rebase(groupList, 0x11E721E0);
		rebase(partyMoney, 0x1080AB60);
	}
	
	
} addresses;

LegacyPartySystem party;

class LegacyPartySystemHacks : TempleFix
{
public: 
	void SetMaxPCs(char maxPCs);

	void apply() override 
	{
		SetMaxPCs((char)config.maxPCs);
		replaceFunction(0x1002BBE0, AddToPcGroup);
		replaceFunction(0x1002BC40, AddToNpcGroup);

		// TriggersFearfulResponse
		static BOOL(__cdecl *orgFearfulResponse)(objHndl) = replaceFunction<BOOL(__cdecl)(objHndl)>(0x10080720, [](objHndl handle){
			if (config.tolerantNpcs)
				return FALSE;
			return orgFearfulResponse(handle);
		});

		replaceFunction<objHndl()>(0x1002BE60, [](){
			return party.GetConsciousPartyLeader();
		});
	}
} partyHacks;

void LegacyPartySystemHacks::SetMaxPCs(char maxPCs)
{
	char * maxPCsBuffer = &maxPCs;
	char maxNPCs = 8 - maxPCs;
	char * maxNPCsBuffer = &maxNPCs;
	//write(0x1002BBED + 2, maxPCsBuffer , 1); // replaced with AddToPcGroup
	write(0x1002BC4D + 2, maxNPCsBuffer, 1);
	// write(0x100B0185 + 2, maxNPCsBuffer, 1); // python follower_atmax - deprecated by python reimplementation
	
}

void LegacyPartySystem::SetMaxPCs(char maxPCs)
{
	partyHacks.SetMaxPCs(maxPCs);
}

void LegacyPartySystem::GroupArraySort(GroupArray* groupArray)
{
	if (groupArray->sortFunc)
	{
		addresses.ArraySort(groupArray, groupArray->GroupSize, sizeof(objHndl), groupArray->sortFunc);
	}
}

int LegacyPartySystem::GetPartyAlignment()
{
	return temple::GetRef<int>(0x1080ABA4);
}

int LegacyPartySystem::GetMoney() const
{
	return temple::GetRef<int(__cdecl)()>(0x1002B750)();
}

void LegacyPartySystem::GiveMoneyFromItem(const objHndl& item){
	auto itemObj = objSystem->GetObject(item);
	if (!itemObj){
		return;
	}
	auto moneyAmt = itemObj->GetInt32(obj_f_money_quantity);
	auto moneyType = itemObj->GetInt32(obj_f_money_type);
	if (moneyType < 4 && moneyType >= 0){
		addresses.partyMoney[moneyType] += moneyAmt;
	}
}

void LegacyPartySystem::ApplyConditionAround(const objHndl& obj, double range, const char* condName, const objHndl& obj2){
	auto groupLen = GroupListGetLen();
	for (auto i = 0u; i < groupLen; i++){
		auto partyMem = GroupListGetMemberN(i);
		if (!partyMem)
			continue;
		if (locSys.DistanceToObj(obj, partyMem) > range)
			continue;
		std::vector<int> args({ (int)obj2.GetHandleLower(), (int)obj2.GetHandleUpper() });
		auto cond = conds.GetByName(condName);
		for (auto i = 2u; i < cond->numArgs; i++)
			args.push_back(0);
		conds.AddTo(partyMem, condName, args);	
	}
}

uint32_t LegacyPartySystem::GetLivingPartyMemberCount()
{
	auto N = GroupListGetLen();
	auto result = 0u;
	for (auto i=0; i < N; i++){
		auto partyMem = GroupListGetMemberN(i);
		if (!critterSys.IsDeadNullDestroyed(partyMem))
			result++;
	}
	return result;
}

bool LegacyPartySystem::IsInParty(objHndl critter){
	if (!critter)
		return false;
	return party.ObjIsInGroupArray(addresses.groupList, critter);
}

/* 0x1002BBE0 */
uint32_t LegacyPartySystem::AddToPCGroup(objHndl objHnd)
{
	auto npcFollowers = GroupNPCFollowersLen();
	auto pcs = GroupPCsLen();

	if ( pcs < config.maxPCs
		|| config.maxPCsFlexible && (npcFollowers + pcs < PARTY_SIZE_MAX))
	{
		auto addOk = ObjAddToGroupArray(addresses.groupPcs, objHnd);
		if (addOk)
		{
			addOk = ObjAddToGroupArray(addresses.groupList, objHnd);
			if (addOk)
			{
				AddToCurrentlySelected(objHnd);
			}
		}
		return addOk;
	}
	return 0;
}

uint32_t LegacyPartySystem::AddToNpcGroup(objHndl objHnd)
{
	auto npcFollowers = GroupNPCFollowersLen();
	if (npcFollowers >= 5) return 0;
	
	auto pcs = GroupPCsLen();

	if (npcFollowers < PARTY_SIZE_MAX - config.maxPCs
		|| config.maxPCsFlexible && (npcFollowers + pcs < PARTY_SIZE_MAX))
	{
		auto v2 = ObjAddToGroupArray(addresses.groupNpcFollowers, objHnd);
		if (v2)
		{
			v2 = ObjAddToGroupArray(addresses.groupList, objHnd);
			if (v2)
			{
				AddToCurrentlySelected(objHnd);
			}
		}
		return v2;
	}
	return 0;
}

void LegacyPartySystem::AddToCurrentlySelected(objHndl obj, bool forceAdd)
{
	if (forceAdd || ObjIsInGroupArray(addresses.groupList, obj))
		ObjAddToGroupArray(addresses.groupCurrentlySelected, obj);
}

void LegacyPartySystem::GroupArrayClearMembers(GroupArray* groupArray)
{
	groupArray->GroupSize = 0;
	memset(groupArray->GroupMembers, 0, sizeof(groupArray->GroupMembers));
}

void LegacyPartySystem::CurrentlySelectedClear()
{
	GroupArrayClearMembers(addresses.groupCurrentlySelected);
}

uint32_t LegacyPartySystem::CurrentlySelectedNum()
{
	return addresses.groupCurrentlySelected->GroupSize;
}

objHndl LegacyPartySystem::GetCurrentlySelected(int n){

	if (n > (int)CurrentlySelectedNum() - 1)
		return objHndl::null;

	return addresses.groupCurrentlySelected->GroupMembers[n];
}

objHndl LegacyPartySystem::GetLeader()
{
	objHndl leader = GroupListGetMemberN(0);
	if (objects.IsNPC(leader))
	{
		leader = GetConsciousPartyLeader();
	}
	return leader;
}

objHndl LegacyPartySystem::GetConsciousPartyLeader(){

	auto selectedCount = CurrentlySelectedNum();
	for (auto i=0u; i < selectedCount; i++){
		auto dude = GetCurrentlySelected(i);
		if (!dude) continue;
		if (!critterSys.IsDeadOrUnconscious(dude))
			return dude;
	}
	

	/* added fix in case the leader is not currently selected and is in combat 
		This fixes issue with Fear'ed characters fucking up things in TB combat, because while running away they weren't selected. 
		Symptoms included causing an incorrect radial menu to appear for the fear'd character.
	*/
	if (combatSys.isCombatActive()){
		auto curActor = tbSys.turnBasedGetCurrentActor();
		if (IsInParty(curActor) && !critterSys.IsDeadOrUnconscious(curActor))
			return curActor;
	}


	auto partySize = GroupListGetLen();
	for (auto i=0u; i < partySize; i++){
		auto dude = GroupListGetMemberN(i);
		if (!dude)continue;
		if (!critterSys.IsDeadOrUnconscious(dude))
			return dude;
	}

	// still none found:
	if (partySize)
		return GroupListGetMemberN(0);

	return objHndl::null;
}

objHndl LegacyPartySystem::PartyMemberWithHighestSkill(SkillEnum skillEnum)
{
	auto highestSkillMemberGet = temple::GetRef<objHndl(__cdecl)(SkillEnum)>(0x1002BD50);
	return highestSkillMemberGet(skillEnum);
}

int LegacyPartySystem::MoneyAdj(int plat, int gold, int silver, int copper)
{
	auto partyMoneyAdj = temple::GetRef<int(__cdecl)(int, int, int, int)>(0x1002B7D0);
	return partyMoneyAdj(plat, gold, silver, copper);
}

void LegacyPartySystem::DebitMoney(int plat, int gold, int silver, int copper){
	temple::GetRef<void(int, int, int, int)>(0x1002C020)(plat, gold, silver, copper);
}

uint32_t AddToPcGroup(objHndl objHnd)
{
	return party.AddToPCGroup(objHnd);
}

uint32_t AddToNpcGroup(objHndl objHnd)
{
	return party.AddToNpcGroup(objHnd);
}