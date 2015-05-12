#pragma once
#include "stdafx.h"
#include "common.h"

enum class EquipSlot : uint32_t {
	Helmet = 0,
	Necklace = 1,
	Gloves = 2,
	WeaponPrimary = 3,
	WeaponSecondary = 4,
	Armor = 5,
	RingPrimary = 6,
	RingSecondary = 7,
	Boots = 8,
	Ammo = 9,
	Cloak = 10,
	Shield = 11,
	Robes = 12,
	Bracers = 13,
	BardicItem = 14,
	Lockpicks = 15,
	Count = 16
};

enum class StandPointType : uint32_t {
	Day = 0,
	Night = 1,
	Scout = 2
};

struct StandPoint {
	uint32_t mapId;
	int _pad1;
	LocAndOffsets location;
	int jumpPointId;
	int _pad2;
}; 

enum class ResurrectType : uint32_t {
	RaiseDead = 0,
	Resurrect = 1,
	ResurrectTrue = 2,
	CuthbertResurrect = 3
};

struct CritterSystem : AddressTable
{

	uint32_t isCritterCombatModeActive(objHndl objHnd);
	CritterSystem();

	uint32_t IsPC(objHndl);
	int GetLootBehaviour(objHndl npc);
	void SetLootBehaviour(objHndl npc, int behaviour);

	uint32_t HasMet(objHndl pc, objHndl npc);

	/*
		Unk Flag could mean -> Add their NPC followers to the group as well
	*/
	uint32_t AddFollower(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);

	/*
		Unk Flag could mean -> Remove their NPC followers to the group as well
	*/
	uint32_t RemoveFollower(objHndl npc, int unkFlag);

	/*
		Gets the current leader of the given critter. Might be 0.
	*/
	objHndl GetLeader(objHndl critter);

	/*
		Checks for line of sight betwen a critter and a target obj.
		Returns 0 if there are no obstacles.
	*/
	int HasLineOfSight(objHndl critter, objHndl target);

	/*
		Gets an item worn at the given equipment slot.
	*/
	objHndl GetWornItem(objHndl handle, EquipSlot slot);

	void Attack(objHndl target, objHndl attacker, int n1, int n2);

	/*
		Checks if the given critter is friendly towards the target.
	*/
	uint32_t IsFriendly(objHndl, objHndl);

	/*
		For id = 7, this returns footstep sound ids for the critter
		on the tile it is standing on. -1 if none should be played.
	*/
	int SoundmapCritter(objHndl critter, int id);

	/*
		Kills the critter.
	*/
	void Kill(objHndl critter, objHndl killer = 0);

	/*
		Same as Kill, but applies condition "Killed By Death Effect" before killing.
	*/
	void KillByEffect(objHndl critter, objHndl killer = 0);
		
	/*
		Changes one of the standpoints for a critter.
	*/
	void SetStandPoint(objHndl critter, StandPointType type, const StandPoint &standpoint);

	/*
		Gets one of the standpoints for a critter.
	*/
	StandPoint GetStandPoint(objHndl critter, StandPointType type);

	/*
		Sets the overall subdual damage received by the critter.
	*/
	void SetSubdualDamage(objHndl critter, int damage);

	/*
		Awards XP to the given critter.
	*/
	void AwardXp(objHndl critter, int xpAwarded);

	/*
		Perform the special Balor death logic.
	*/
	void BalorDeath(objHndl npc);

	/*
		Changes whether the given critter is concealed or not.	
	*/
	void SetConcealed(objHndl critter, bool concealed);

	/*
		Third argument seems unused.
	*/
	uint32_t Resurrect(objHndl critter, ResurrectType type, int unk);

	/*
		Dominates a critter.
	*/
	uint32_t Dominate(objHndl critter, objHndl caster);

	uint32_t IsDeadOrUnconscious(objHndl critter);
	
	int GetPortraitId(objHndl critter);

	int GetLevel(objHndl critter);

	Race GetRace(objHndl critter);

	// Create and give an item to a critter
	objHndl GiveItem(objHndl critter, int protoId);

#pragma region Category
	MonsterCategory GetCategory(objHndl objHnd);
	uint32_t IsCategoryType(objHndl objHnd, MonsterCategory categoryType);
	uint32_t IsCategorySubtype(objHndl objHnd, MonsterCategory categoryType);
	uint32_t IsUndead(objHndl objHnd);
	uint32_t IsOoze(objHndl objHnd);
	uint32_t IsSubtypeFire(objHndl objHnd);
#pragma endregion

};

extern CritterSystem critterSys;

uint32_t _isCritterCombatModeActive(objHndl objHnd);