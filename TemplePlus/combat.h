#pragma once

#include <temple/dll.h>
#include "tig/tig_mes.h"


/*
	Maximum distance for NPCs to execute the "EnterCombat" function
	Unfortunately increasing this only makes you bump into the pathfinding
	limitation
*/
#define COMBAT_ACTIVATION_DISTANCE 42.5 
#include "critter.h"

struct AiTactic;
struct D20Actn;
uint32_t Combat_GetMesfileIdx_CombatMes();

struct CombatSystem : temple::AddressTable {
	MesHandle* combatMesfileIdx;
	MesHandle combatMesNew;
	GroupArray * groupInitiativeList;

	char * GetCombatMesLine(int line);
	int IsWithinReach(objHndl attacker, objHndl target);
	BOOL CanMeleeTargetAtLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* loc);
	BOOL CanMeleeTargetAtLoc(objHndl obj, objHndl target, LocAndOffsets* loc);
	BOOL CanMeleeTarget(objHndl obj, objHndl target);
	BOOL CanMeleeTargetRegardWeapon(objHndl obj, objHndl weapon, objHndl target);
	int GetThreateningCrittersAtLoc(objHndl obj, LocAndOffsets* loc, objHndl threateners[40]);
	objHndl CheckRangedWeaponAmmo(objHndl obj); // checks if the ammo slot item matches a wielded weapon (primary or secondary), and if so, returns it
	bool AmmoMatchesItemAtSlot(objHndl obj, EquipSlot equipSlot);
	
	uint32_t* combatModeActive;
	bool isCombatActive();
	uint32_t IsCloseToParty(objHndl objHnd);
	/*
	// in vanilla, checks if obj are both in the party or both NOT in the party; kinda used like IsFriendly in the code, so be careful!
	TODO: make this take into account "innocents" or friendly factions
	*/
	BOOL AffiliationSame(objHndl obj, objHndl obj2); 
	/*
		retrieves a list of enemies that the obj can melee with; return val is that number of such enemies
	*/
	int GetEnemiesCanMelee(objHndl obj, objHndl* canMeleeList);
	objHndl GetWeapon(AttackPacket* attackPacket);
	bool DisarmCheck(objHndl attacker, objHndl defender, D20Actn* d20a);
	bool SunderCheck(objHndl attacker, objHndl defender, D20Actn* d20a);
	int GetClosestEnemy(objHndl obj, LocAndOffsets* locOut, objHndl * objOut, float* distOut, int flags);
	int GetInitiativeListLength();
	objHndl GetInitiativeListMember(int n);
	int GetClosestEnemy(AiTactic * aiTac, int selectionType);
	int (__cdecl* IsFlankedBy)(objHndl victim, objHndl attacker);
	/*
		Use for the non-lethal brawl.
	*/
	void (__cdecl *Brawl)(objHndl a, objHndl b);
	void enterCombat(objHndl objHnd);

	void (__cdecl *AddToInitiative)(objHndl critter);
	void (__cdecl *RemoveFromInitiative)(objHndl critter);

	int (__cdecl *GetInitiative)(objHndl critter);
	void (__cdecl *SetInitiative)(objHndl critter, int initiative);
	int (__cdecl*_GetClosestEnemy)(objHndl obj, LocAndOffsets* locOut, objHndl * objOut, float* distOut, int flags);

	CombatSystem() {
		rebase(combatModeActive, 0x10AA8418);
		rebase(combatMesfileIdx, 0x10AA8408);
		rebase(groupInitiativeList, 0x10BCAC78);

		rebase(_enterCombat,0x100631E0);
		
		rebase(IsFlankedBy, 0x100B9200);
		rebase(_GetInitiativeListLength, 0x100DEDA0);
		rebase(_GetInitiativeListMember, 0x100DEDF0);
		rebase(AddToInitiative, 0x100DF1E0);
		rebase(RemoveFromInitiative, 0x100DF530);
		rebase(GetInitiative, 0x100DEDB0);
		rebase(SetInitiative, 0x100DF2E0);
		rebase(_GetClosestEnemy, 0x100E2B80);
		rebase(Brawl, 0x100EBD40);
	}

private:
	void (__cdecl *_enterCombat)(objHndl objHnd);
	int(__cdecl* _GetInitiativeListLength)();
	objHndl(__cdecl*_GetInitiativeListMember)(int n);

};

extern CombatSystem combatSys;


uint32_t _isCombatActive();
uint32_t _IsCloseToParty(objHndl objHnd);
char * _GetCombatMesLine(int line);