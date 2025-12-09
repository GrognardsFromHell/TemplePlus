
#pragma once

#include "dispatcher.h"
#include "common.h"
#include "tig/tig_mes.h"

class Dice;

struct DamageDice { // see 100E03F0 AddDamageDice
	int dicePacked;
	DamageType type;
	int rolledDamage;
	const char *typeDescription;
	const char * causedBy; // e.g. item name
	DamageDice():dicePacked(0), type(DamageType::Unspecified), rolledDamage(-1), typeDescription(nullptr),causedBy(nullptr)	{}
	DamageDice(int DicePacked, DamageType DamType, const char* TypeDescr):DamageDice()	{
		dicePacked = DicePacked;
		type = DamType;
		typeDescription = TypeDescr;
	};
};

struct DamageReduction {
	int damageReductionAmount;
	float dmgFactor;
	DamageType type;
	int attackPowerType; // see D20AttackPower; if an attack has an overlapping attackPowerType (result of an & operation), the damage reduction will NOT apply; DamageReductions with attackPowerType = 1 will ALWAYS apply 
	const char *typeDescription;
	const char * causedBy; // e.g. an item name
	int damageReduced; // e.g. from CalcDamageModFromFactor 0x100E0E00 
};

struct DamagePacket {
	char* description;
	int critHitMultiplier; // 1 by default; gets set on an actual crit hit (and may be modified by various things)
	DamageDice dice[5];
	uint32_t diceCount;
	DamageReduction damageResistances[5];
	int damResCount;
	DamageReduction damageFactorModifiers[5]; // may also be used for vulnerabilities (e.g. Condition Monster Subtype Fire does this for Cold Damage)
	int damModCount;
	BonusList bonuses;
	int attackPowerType; // see D20DAP
	int finalDamage;
	int flags; // 1 - Maximized (takes max value of damage dice) ; 2 - Empowered (1.5x on rolls)
	// int field51c; // looks like a mistake... see 0x10047C80

	int AddEtherealImmunity();
	int AddDamageDice(uint32_t dicePacked, DamageType damType, int damageMesLine, const char* description = nullptr);
	BOOL AddDamageBonus(int32_t damBonus, int bonType, int bonMesline, const char* causeDesc = nullptr);
	int AddPhysicalDR(int amount, int bypasserBitmask, int damageMesLine);
	int AddDR(int amount, DamageType damType, int damageMesLine);
	void AddAttackPower(int attackPower);
	void CalcFinalDamage(); // calcualtes the finalDamage field
	int GetOverallDamageByType(DamageType damType);
	int GetOverallDamage();
	int AddModFactor(float factor, DamageType damType, int damageMesLine);
	BOOL CriticalMultiplierApply(int multiplier);
	void PlayPfx(objHndl target);
	DamagePacket();
	
};

#pragma pack(push, 1)
struct DispIoDamage : DispIO { // Io type 4
	int padding;
	AttackPacket attackPacket;
	DamagePacket damage;
	int padding2;

	DispIoDamage() {
		dispIOType = dispIOTypeDamage;
		attackPacket.d20ActnType = D20A_NONE;
	}
};
#pragma pack(pop)

class Damage {
	friend class D20System;
	friend struct DamagePacket;
public:
	
	MesHandle damageMes;

	void DealDamageFullUnk(objHndl victim, objHndl attacker, 
		const Dice &dice, 
		DamageType type = DamageType::Unspecified, 
		int attackPower = 0, 
		D20ActionType actionType = D20A_NONE) {
		// 100% of damage is dealt (no saving throw)
		// line 103 of damage.mes says: "Unknown"
		DealDamage(victim, attacker, dice, type, attackPower, 100, 103, actionType);
	}

	void DealSpellDamageFullUnk(objHndl victim, objHndl attacker, const Dice &dice, DamageType type, int attackPower, D20ActionType actionType, int spellId, int flags) {
		DealSpellDamage(victim, attacker, dice, type, attackPower, 100, 103, actionType, spellId, flags);
	}

	void DealDamage(objHndl victim, objHndl attacker, const Dice &dice, DamageType type, int attackPower, int damFactor, int damageDescId, D20ActionType actionType);
	void DealSpellDamage(objHndl victim, objHndl attacker, const Dice &dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType,
		int spellId, int flags);

	

	/*
		deals damage from a successful weapon attack.
		Return value is used by Coup De Grace action.
	*/
	int DealAttackDamage(objHndl attacker, objHndl tgt, int d20Data, D20CAF flags, D20ActionType actionType);
	
	/*
	    used for spells that have an attack roll
	*/
	int DealWeaponlikeSpellDamage(objHndl tgt, objHndl attacker, const Dice &dice, DamageType type, int attackPower, int damFactor, int damageDescId, D20ActionType actionType, int spellId, D20CAF flags, int projectileIdx = 0);

	void DamageCritter(objHndl attacker, objHndl tgt, DispIoDamage & evtObjDam);


	void FastHeal(objHndl critter, int amount);

	void Heal(objHndl target, objHndl healer, const Dice &dice, D20ActionType actionType);
	
	void HealSpell(objHndl target, objHndl healer, const Dice &dice, D20ActionType actionType, int spellId);

	void HealSubdual(objHndl target, int amount);

	// See D20SavingThrowFlags for flags
	bool SavingThrow(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags, const BonusList *bonExtra = nullptr);

	// Save against a spell
	bool SavingThrowSpell(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags, int spellId, const BonusList *bonExtra = nullptr);

	/*
		Deals damage that can be reduced by a successful reflex save.
	*/
	bool ReflexSaveAndDamage(objHndl obj, objHndl attacker, int dc, int reduction, int flags, const Dice &dice, DamageType damageType, int attackPower, D20ActionType actionType, int spellId);

	void DamagePacketInit(DamagePacket * dmgPkt);
	int AddDamageBonusWithDescr(DamagePacket* damage, int damBonus, int bonType, int bonusMesLine, char* desc);
	int AddPhysicalDR(DamagePacket *damPkt, int DRAmount, int bypasserBitmask, unsigned int damageMesLine);
	const char* GetMesline(unsigned damageMesLine);
	int AddDamageDice(DamagePacket *dmgPkt, int dicePacked, DamageType damType, unsigned int damageMesLine);
	int AddDamageDiceWithDescr(DamagePacket *dmgPkt, int dicePacked, DamageType damType, unsigned int damageMesLine, char* descr);
	BOOL DamageTypeMatch(DamageType reduction, DamageType attackType);

	Dice ModifyDamageDiceForSize(Dice &base, int steps);

	Damage();
private:
	void Init();
	void Exit() const;

	void DamageCritterPython(objHndl attacker, objHndl tgt, DispIoDamage& evtObjDam);
	int DealAttackDamagePython(objHndl attacker, objHndl tgt, int d20Data, D20CAF flags, D20ActionType actionType);
	void DealSpellDamagePython(objHndl tgt, objHndl attacker, const Dice& dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType, int spellId, int flags, int projectileIdx = 1, bool isWeaponlike = false);
};

extern Damage damage;
