
#include "stdafx.h"
#include "damage.h"
#include "dice.h"

static_assert(validate_size<DispIoDamage, 0x550>::value, "DispIoDamage");

static struct DamageAddresses : AddressTable {

	int (__cdecl *DoDamage)(objHndl target, objHndl src, uint32_t dmgDice, DamageType type, int attackPowerType, signed int reduction, int damageDescMesKey, int actionType);

	int (__cdecl *Heal)(objHndl target, objHndl healer, uint32_t dmgDice, D20ActionType actionType);

	void (__cdecl *HealSpell)(objHndl target, objHndl healer, uint32_t dmgDice, D20ActionType actionType, int spellId);

	int (__cdecl*HealSubdual)(objHndl target, int amount);

	int (__cdecl*DoSpellDamage)(objHndl victim, objHndl attacker, uint32_t dmgDice, DamageType type, int attackPowerType, int reduction, int damageDescMesKey, int actionType, int spellId, int flags);

	bool (__cdecl *SavingThrow)(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags);
	
	bool (__cdecl *SavingThrowSpell)(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags, int spellId);

	bool (__cdecl *ReflexSaveAndDamage)(objHndl obj, objHndl attacker, int dc, int reduction, int flags, int dicePacked, DamageType damageType, int attackPower, D20ActionType actionType, int spellId);

	DamageAddresses() {
		rebase(DoDamage, 0x100B8D70);
		rebase(Heal, 0x100B7DF0);
		rebase(HealSpell, 0x100B81D0);
		rebase(DoSpellDamage, 0x100B7F80);
		rebase(HealSubdual, 0x100B9030);
		rebase(SavingThrow, 0x100B4F20);
		rebase(SavingThrowSpell, 0x100B83C0);
		rebase(ReflexSaveAndDamage, 0x100B9500);
	}
} addresses;

Damage damage;

void Damage::DealDamage(objHndl victim, objHndl attacker, const Dice& dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType) {

	addresses.DoDamage(victim, attacker, dice.ToPacked(), type, attackPower, reduction, damageDescId, actionType);

}

void Damage::DealSpellDamage(objHndl victim, objHndl attacker, const Dice& dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType, int spellId, int flags) {

	addresses.DoSpellDamage(victim, attacker, dice.ToPacked(), type, attackPower, reduction, damageDescId, actionType, spellId, flags);

}

void Damage::Heal(objHndl target, objHndl healer, const Dice& dice, D20ActionType actionType) {
	addresses.Heal(target, healer, dice.ToPacked(), actionType);
}

void Damage::HealSpell(objHndl target, objHndl healer, const Dice& dice, D20ActionType actionType, int spellId) {
	addresses.HealSpell(target, healer, dice.ToPacked(), actionType, spellId);
}

void Damage::HealSubdual(objHndl target, int amount) {
	addresses.HealSubdual(target, amount);
}

bool Damage::SavingThrow(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags) {
	return addresses.SavingThrow(obj, attacker, dc, type, flags);
}

bool Damage::SavingThrowSpell(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags, int spellId) {
	return addresses.SavingThrow(obj, attacker, dc, type, flags);
}

bool Damage::ReflexSaveAndDamage(objHndl obj, objHndl attacker, int dc, int reduction, int flags, const Dice& dice, DamageType damageType, int attackPower, D20ActionType actionType, int spellId) {
	return addresses.ReflexSaveAndDamage(obj, attacker, dc, reduction, flags, dice.ToPacked(), damageType, attackPower, actionType, spellId);
}
