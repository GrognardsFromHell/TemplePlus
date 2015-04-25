
#pragma once

#include "dispatcher.h"
#include "common.h"

class Dice;

struct DamageDice {
	const char *typeDescription;
	int field4;
	int dicePacked;
	DamageType type;
	int rolledDamage;
};

struct DamageReduction {
	int field0;
	float dmgFactor;
	DamageType type;
	int fieldc;
	const char *typeDescription;
	int field14;
	int field18;
};

struct DamagePacket {
	DamageDice dice[5];
	int field64;
	int field68;
	int diceCount;
	char unk[140];
	int unkCount;
	DamageReduction reductions[5];
	int reductionsCount;
	BonusList bonuses;
	int attackPowerType;
	int field50c;
	int flags;
	int field51c;
};

#pragma pack(push, 1)
struct DispIoDamage : DispIO {
	int padding;
	objHndl src;
	objHndl victim;
	D20ActionType actionType;
	int field1c;
	int flags;
	int field24;
	objHndl weaponUsed;
	objHndl anotherItem;
	DamagePacket damage;

	DispIoDamage() {
		dispIOType = dispIOTypeDamage;
	}
};
#pragma pack(pop)

class Damage {
public:
		
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

	void DealDamage(objHndl victim, objHndl attacker, const Dice &dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType);
	void DealSpellDamage(objHndl victim, objHndl attacker, const Dice &dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType,
		int spellId, int flags);

	void Heal(objHndl target, objHndl healer, const Dice &dice, D20ActionType actionType);
	
	void HealSpell(objHndl target, objHndl healer, const Dice &dice, D20ActionType actionType, int spellId);

	void HealSubdual(objHndl target, int amount);

	// See D20SavingThrowFlags for flags
	bool SavingThrow(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags);

	// Save against a spell
	bool SavingThrowSpell(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags, int spellId);

	/*
		Deals damage that can be reduced by a successful reflex save.
	*/
	bool ReflexSaveAndDamage(objHndl obj, objHndl attacker, int dc, int reduction, int flags, const Dice &dice, DamageType damageType, int attackPower, D20ActionType actionType, int spellId);

};

extern Damage damage;
