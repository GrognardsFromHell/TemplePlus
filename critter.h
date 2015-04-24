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

struct CritterSystem : AddressTable
{

	uint32_t isCritterCombatModeActive(objHndl objHnd);
	CritterSystem();

	int GetLootBehaviour(objHndl npc);
	void SetLootBehaviour(objHndl npc, int behaviour);

	bool HasMet(objHndl pc, objHndl npc);

	/*
		Unk Flag could mean -> Add their NPC followers to the group as well
	*/
	bool AddFollower(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);

	/*
		Unk Flag could mean -> Remove their NPC followers to the group as well
	*/
	bool RemoveFollower(objHndl npc, int unkFlag);

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
	bool IsFriendly(objHndl, objHndl);

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
};

extern CritterSystem critterSys;

uint32_t _isCritterCombatModeActive(objHndl objHnd);