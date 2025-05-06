#pragma once

#include <temple/dll.h>
#include "tig/tig_mes.h"


/*
	Maximum distance for NPCs to execute the "EnterCombat" function (18 tiles)
	Unfortunately increasing this only makes you bump into the pathfinding
	limitation
*/
#define COMBAT_ACTIVATION_DISTANCE 42.5 
#include "critter.h"

enum FloatLineColor : uint32_t;
struct AiTactic;
struct D20Actn;
uint32_t Combat_GetMesfileIdx_CombatMes();

struct LegacyCombatSystem : temple::AddressTable {
	MesHandle* combatMesfileIdx;
	MesHandle combatMesNew;
	GroupArray * groupInitiativeList;

	char * GetCombatMesLine(int line) const;
	void FloatCombatLine(objHndl obj, int line);
	void FloatCombatLine(objHndl obj, int line, FloatLineColor floatColor);
	void FloatTextBubble(objHndl handle, int combatMesLine);

	int IsWithinReach(objHndl attacker, objHndl target);
	
	BOOL CanMeleeTargetAtLoc(objHndl obj, objHndl target, LocAndOffsets* loc); // checks if obj is able to hit target if the TARGET is at loc	
	BOOL CanMeleeTargetAtLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* loc);
	BOOL CanMeleeTargetFromLoc(objHndl obj, objHndl target, LocAndOffsets* objLoc); // checks if OBJ is able to hit TARGET if OBJ is at loc
	bool CanMeleeTargetFromLocRegardItem(objHndl obj, objHndl weapon, objHndl target, LocAndOffsets* objLoc);
	BOOL CanMeleeTarget(objHndl obj, objHndl target);
	BOOL CanMeleeTargetRegardWeapon(objHndl obj, objHndl weapon, objHndl target);
	int GetThreateningCrittersAtLoc(objHndl obj, LocAndOffsets* loc, objHndl threateners[40]);
	objHndl CheckRangedWeaponAmmo(objHndl obj); // checks if the ammo slot item matches a wielded weapon (primary or secondary), and if so, returns it
	bool AmmoMatchesItemAtSlot(objHndl obj, EquipSlot equipSlot);
	bool NeedsToReload(objHndl obj);
	objHndl * GetHostileCombatantList(objHndl obj, int* count); // gets a list from the combat initiative
	std::vector<objHndl> GetHostileCombatantList(objHndl handle); // gets a list from the combat initiative
	void GetEnemyListInRange(objHndl obj, float rangeFeet, std::vector<objHndl> & enemies);
	bool HasLineOfAttack(objHndl obj, objHndl target); // can shoot or attack target (i.e. target isn't behind a wall or sthg)
	// can shoot or attack target (i.e. target isn't behind a wall or sthg)
	bool HasLineOfAttackFromPosition(LocAndOffsets fromPosition, objHndl target);
	objHndl CreateProjectileAndThrow(locXY origin, int projectileProto, LocAndOffsets tgtLoc, int missX, int missY, objHndl thrower, objHndl tgt);


	void AddToInitiativeWithinRect(objHndl objHndl) const; // adds critters to combat initiative around obj
	void EndTurn();
	void CombatSubturnEnd();
	void Subturn();
	void TurnProcessAi(objHndl obj);
	BOOL StartCombat(objHndl combatInitiator, int setToFirstInitiativeFlag); // setToFirstInitiativeFlag - will set combatInitiator to start of initiative list if true
	void TurnStart2( int initiativeIdx);
	void CombatAdvanceTurn(objHndl obj);
	BOOL IsBrawlInProgress();
	void CritterExitCombatMode(objHndl handle);
	bool CombatEnd(); // ends combat mode
	

	uint32_t* combatModeActive;
	bool forceEndedCombatNow; // to prevent loops where combat is entered and exited at the same time event
	bool isCombatActive();
	bool IsAutoAttack();
	bool AllCombatantsFarFromParty();
	bool AllPcsUnconscious(); // true if all party PCs are unconscious
	uint32_t IsCloseToParty(objHndl objHnd);
	/*
	// in vanilla, checks if obj are both in the party or both NOT in the party; kinda used like IsFriendly in the code, so be careful!
	TODO: make this take into account "innocents" or friendly factions
	*/
	BOOL AffiliationSame(objHndl obj, objHndl obj2); 
	/*
		retrieves a list of enemies that can melee attack obj; return val is that number of such enemies
	*/
	int GetEnemiesCanMelee(objHndl obj, objHndl* canMeleeList);
	std::vector<objHndl> GetEnemiesCanMelee(objHndl handle);

	objHndl GetWeapon(AttackPacket* attackPacket);
	static bool IsUnarmed(objHndl handle);
	bool DisarmCheck(objHndl attacker, objHndl defender, D20Actn* d20a);
	bool SunderCheck(objHndl attacker, objHndl defender, D20Actn* d20a);
	uint32_t UseItem(objHndl performer, objHndl item, objHndl target);
	/*
		flags:
		0x1  - target is not friendly (for PC obj, this includes charmed targets)
		0x2  - target is friendly
		0x8  - filter Q_AI_Fireball_OK
		0x10 - exclude self
		0x20 - check OF_INVULNERABLE
	*/
	int GetClosestEnemy(objHndl obj, LocAndOffsets* locOut, objHndl * objOut, float* distOut, int flags);
	int GetInitiativeListLength();
	objHndl GetInitiativeListMember(int n);
	int GetClosestEnemy(AiTactic * aiTac, int selectionType);
	int (__cdecl* IsFlankedBy)(objHndl victim, objHndl attacker);
	/*
		Use for the non-lethal brawl.
	*/
	void Brawl(objHndl a, objHndl b);
	void (__cdecl *_Brawl)(objHndl a, objHndl b);
	void enterCombat(objHndl objHnd, bool regardDistance = true);
	void AddToInitiative(objHndl critter);
	void (__cdecl *RemoveFromInitiative)(objHndl critter);

	int (__cdecl *GetInitiative)(objHndl critter);
	void (__cdecl *SetInitiative)(objHndl critter, int initiative);
	int (__cdecl*_GetClosestEnemy)(objHndl obj, LocAndOffsets* locOut, objHndl * objOut, float* distOut, int flags);

	int MiscCheckRoll(objHndl obj, BonusList* bonlist, int modifier, int dc, const char* text, int *rollHistId);
	/*
		The To-Hit calculation. Sets the flags in the D20Action D20CAF_HIT and D20CAF_CRITICAL among other things.
	*/
	void ToHitProcessing(D20Actn &d20a);
	void ToHitProcessingPython(D20Actn &d20a);
	bool TripCheck(objHndl handle, objHndl target);

	int GetCombatRoundCount();
	LegacyCombatSystem() {
		rebase(combatModeActive, 0x10AA8418);
		rebase(combatMesfileIdx, 0x10AA8408);
		rebase(groupInitiativeList, 0x10BCAC78);

		rebase(_enterCombat,0x100631E0);
		
		rebase(IsFlankedBy, 0x100B9200);
		rebase(_GetInitiativeListLength, 0x100DEDA0);
		rebase(_GetInitiativeListMember, 0x100DEDF0);
		
		rebase(RemoveFromInitiative, 0x100DF530);
		rebase(GetInitiative, 0x100DEDB0);
		rebase(SetInitiative, 0x100DF2E0);
		rebase(_GetClosestEnemy, 0x100E2B80);
		rebase(_Brawl, 0x100EBD40);
	}

private:
	void (__cdecl *_enterCombat)(objHndl objHnd);
	int(__cdecl* _GetInitiativeListLength)();
	objHndl(__cdecl*_GetInitiativeListMember)(int n);

};

extern LegacyCombatSystem combatSys;


uint32_t _isCombatActive();
uint32_t _IsCloseToParty(objHndl objHnd);
char * _GetCombatMesLine(int line);
uint64_t __cdecl _GetCleveVictim(objHndl objHnd);
