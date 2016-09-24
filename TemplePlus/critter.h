#pragma once

#include "common.h"

#include <memory>
#include <temple/dll.h>
#include "dispatcher.h"

enum SkillEnum:uint32_t;

namespace gfx {
	enum class WeaponAnim;
	class EncodedAnimId;
	using AnimatedModelPtr = std::shared_ptr<class AnimatedModel>;
}

enum EquipSlot : uint32_t {
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

enum class Gender : uint32_t {
	Female = 0,
	Male = 1,
	Invalid 
};

enum class HairStyleRace {
	Human = 0,
	Dwarf,
	Elf,
	Gnome,
	HalfElf,
	HalfOrc,
	Halfling,
	Invalid 
};

enum class HairStyleSize {
	Big = 0,
	Small,
	None
};

struct HairStyle {
	HairStyleRace race;
	Gender gender;
	HairStyleSize size;
	int color;
	int style;

	HairStyle(uint32_t packed) {
		race = (HairStyleRace)(packed & 7);
		gender = (Gender)((packed >> 3) & 1);
		size = (HairStyleSize)((packed >> 10) & 3);
		style = (packed >> 4) & 7;
		color = (packed >> 7) & 7;
	}
	HairStyle() {
	}

	uint32_t Pack() const {
		return ((int)race & 7)
			| ((int)gender) & 1 << 3
			| ((int)size) & 3 << 10
			| style & 7 << 4
			| color & 7 << 7;
	}
};

struct LegacyCritterSystem : temple::AddressTable
{

	uint32_t isCritterCombatModeActive(objHndl objHnd);
	LegacyCritterSystem();

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
	Gets the current leader of the given critter (recursive in case it's an NPC charmed by an NPC). Might be 0.
	*/
	objHndl GetLeaderRecursive(objHndl critter);

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
		returns 0 if both are PCs
		returns 1 if :
		 - both are NPCs with the same leader
		 - one is the other's leader
		 - both share a faction (including factions from reputations for PCs)
	*/
	BOOL AllegianceShared(objHndl obj, objHndl obj2);

	/*
		For id = 7, this returns footstep sound ids for the critter
		on the tile it is standing on. -1 if none should be played.
	*/
	int SoundmapCritter(objHndl critter, int id);

	/*
		Kills the critter.
	*/
	void Kill(objHndl critter, objHndl killer = objHndl::null);

	/*
		Same as Kill, but applies condition "Killed By Death Effect" before killing.
	*/
	void KillByEffect(objHndl critter, objHndl killer = objHndl::null);
		
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
	void SetConcealed(objHndl critter, int concealed);
	void SetConcealedWithFollowers(objHndl obj, int newState); // same, and applies it to the critter followers too

	/*
		Third argument seems unused.
	*/
	uint32_t Resurrect(objHndl critter, ResurrectType type, int unk);

	/*
		Dominates a critter.
	*/
	uint32_t Dominate(objHndl critter, objHndl caster);

	bool IsDeadNullDestroyed(objHndl critter);

	bool IsDeadOrUnconscious(objHndl critter);

	bool IsProne(objHndl critter);

	CritterFlag GetCritterFlags(objHndl critter);
		
	bool IsMovingSilently(objHndl critter);

	void SetMovingSilently(objHndl critter, BOOL newMovingSilState);
	
	bool IsCombatModeActive(objHndl critter);

	bool IsConcealed(objHndl critter);
	
	int GetPortraitId(objHndl critter);
	
	/*
	 checks if critter can see target (i.e. target is not invisible to critter due to blindness or invisibility). 
	 Regards concealment and moving silently (including skill checks for listen/spot). 
	 Does not regard facing.
	*/
	bool CanSense(objHndl critter, objHndl tgt); 

	int GetLevel(objHndl critter);

	int SkillLevel(objHndl critter, SkillEnum skill);

	Race GetRace(objHndl critter);

	Gender GetGender(objHndl critter);

	std::string GetHairStylePreviewTexture(HairStyle style);
	std::string GetHairStyleModel(HairStyle style);
	
	/**
	 * Gets the concrete animation id for a weapon based animation using
	 * the currently equipped weapon of a critter.
	 */
	gfx::EncodedAnimId GetAnimId(objHndl critter, gfx::WeaponAnim anim);

	void UpdateModelEquipment(objHndl obj);

	/**
	 * This is called initially when the model is loaded for an object
	 * and adds NPC specific add meshes.
	 */
	void AddNpcAddMeshes(objHndl obj);

	// Create and give an item to a critter
	objHndl GiveItem(objHndl critter, int protoId);

	/*
		Takes money from a critter. If the critter is a PC, money is taken from the party instead.
		NOTE: All arguments must be *positive*.
	*/
	void TakeMoney(objHndl critter, int platinum, int gold, int silver, int copper);
	/*
		Gives money to a critter. If the critter is a PC, money is given to the party instead.
	*/
	void GiveMoney(objHndl critter, int platinum, int gold, int silver, int copper);

	/*
		Gets number of extra offhand attacks critter can do if wielding an offhand weapon (1 if no special feats)
	*/
	int NumOffhandExtraAttacks(objHndl critter);

	/*
		returns 1 if using no armor or light armor
		useful for Ranger feats that require the above
	*/
	int IsWearingLightOrNoArmor(objHndl critter);

	/*
		Checks if the given critter is dead and still holds
		lootable items.
	*/
	bool IsLootableCorpse(objHndl critter);

	/*
		Can the critter enter Barbarian rage? Currently assumes only Barbarians can TODO: update when adding other classes or check directly if it has the Barbarian Rage condition
	*/
	bool CanBarbarianRage(objHndl performer);

#pragma region Category
	MonsterCategory GetCategory(objHndl objHnd);
	uint32_t IsCategoryType(objHndl objHnd, MonsterCategory categoryType);
	uint32_t IsCategorySubtype(objHndl objHnd, MonsterSubcategoryFlag categoryType);
	uint32_t IsUndead(objHndl objHnd);
	uint32_t IsOoze(objHndl objHnd);
	uint32_t IsSubtypeFire(objHndl objHnd);
#pragma endregion

#pragma region Combat
	float GetReach(objHndl objHndl, D20ActionType actType); // reach in feet
	int GetBonusFromSizeCategory(int sizeCategory);
	int GetDamageIdx(objHndl obj, int attackIdx);
	// bonus to hit from size
	int GetCritterDamageDice(objHndl obj, int attackIdx);
	DamageType GetCritterAttackDamageType(objHndl obj, int attackIdx);
	static int GetCritterNumNaturalAttacks(objHndl obj);
	int GetCritterAttackType(objHndl obj, int attackIdx);
	int GetBaseAttackBonus(const objHndl& handle, Stat classBeingLeveld = Stat::stat_strength);
	int GetArmorClass(objHndl obj, DispIoAttackBonus *dispIo);
#pragma endregion

#pragma region Spellcasting
	int GetSpellLvlCanCast(const objHndl& handle, SpellSourceType spellSourceType, SpellReadyingType spellReadyingType);
	int GetSpellEnumsKnownByClass(const objHndl& handle, int spellClass, int* spellEnums, int capacity);
	int GetCasterLevel(objHndl obj); // returns highest caster level
	int GetCasterLevelForClass(objHndl handle, Stat classCode);
	int GetSpellListLevelExtension(objHndl handle, Stat classCode); // modifies the effective character level for the purpose of fetching spell lists
	bool IsCaster(objHndl obj);
	static int SpellNumByFieldAndClass(objHndl obj, obj_f field, uint32_t spellClassCode);
	bool HashMatchingClassForSpell(objHndl handle, uint32_t spellEnum) const; // checks if obj has a matching spell in their list (does not regard level)
	int DomainSpellNumByField(objHndl obj, obj_f field);
	bool HasDomain(objHndl handle, uint32_t domainType);
#pragma endregion 

	bool IsWarded(objHndl obj); // checks if creature is warded from melee attacks (by stuff like Meld Into Stone, Tree Shape, Otiluke's Resislient Sphere)
	bool IsSummoned(objHndl obj);
	bool IsWieldingRangedWeapon(objHndl performer);
	void GetCritterVoiceLine(objHndl obj, objHndl fellow, char *str, int* soundId);
	int PlayCritterVoiceLine(objHndl obj, objHndl fellow, char* text, int soundId);
	static int SkillBaseGet(objHndl handle, SkillEnum skill);
	static int GetNumFollowers(objHndl obj, int excludeForcedFollowers);


private:
	int GetModelRaceOffset(objHndl obj);
	void UpdateAddMeshes(objHndl obj);
	void ApplyReplacementMaterial(gfx::AnimatedModelPtr model, int mesId);

	std::string GetHairStyleFile(HairStyle style, const char *extension);

	std::unordered_map<int, std::vector<std::string>> mAddMeshes;

	const std::vector<std::string>& GetAddMeshes(int matIdx, int raceOffset);
};

extern LegacyCritterSystem critterSys;

uint32_t _isCritterCombatModeActive(objHndl objHnd);