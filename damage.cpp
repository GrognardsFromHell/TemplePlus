
#include "stdafx.h"
#include "damage.h"
#include "dice.h"
#include "bonus.h"

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

	void(__cdecl * DamagePacketInit)(DamagePacket *dmgPkt);
	int (__cdecl *AddDamageDice)(DamagePacket *dmgPkt, int dicePacked, DamageType damType, unsigned int damageMesLine);


	int (__cdecl *AddAttackPowerType)(DamagePacket *dmgPkt, int attackPowerType); // allows you to bypass DR with the same attackPowerType bitmask
	int (__cdecl *AddDamageBonus)(DamagePacket *damPkt, int bonValue, int bonType, int bonMesLineNum, char *bonDescr);
	int (__cdecl *AddPhysicalDR)(DamagePacket *damPkt, int DRAmount, int bypasserBitmask, unsigned int damageMesLine); // DR vs. normal physical attack
	int (__cdecl *AddDamageModFactor)(DamagePacket *damage, float dmgFactor, DamageType damType, unsigned int damageMesLine); // use this to grant immunities / vulnerabilities by setting the factor to 0.0 or 2.0
	int (__cdecl *AddIncorporealImmunity)(DamagePacket *dmgPkt);
	int (__cdecl *AddDR)(DamagePacket *damPkt, int DRAmount, DamageType damType, int damageMesLine);
	int (__cdecl *AddDamResistanceWithCause)(DamagePacket *a1, int DRAmount, DamageType damType, unsigned int damageMesLine, const char *causedBy);
	int (__cdecl *EnsureMinimumDamage1)(DamagePacket *dmgPkt); // if overall damage is 0 or less, gives it a bonus so it's exactly 1
	void (__cdecl *SetMaximizedFlag)(DamagePacket *dmgPkt);
	void (__cdecl *SetEmpoweredFlag)(DamagePacket *);
	int (__cdecl *CalcDamageModFromDR)(DamagePacket *damPkt, DamageReduction *damReduc, DamageType attackDamType, DamageType attackDamType2); // sets the damageReduced field in damReduc
	void(__cdecl *CalcDamageModFromFactor)(DamagePacket *damPkt, DamageReduction *damReduc, DamageType attackDamType, DamageType attackDamType2); // same, but for "factor" type DamageReduction (i.e. it doesn't consider damResAmount)
	int (__cdecl *GetDamageTypeOverallDamage)(DamagePacket *damPkt, DamageType damType);
	int(__cdecl *GetOverallDamage)(DamagePacket *damagePacket);
	void(__cdecl *CalcFinalDamage)(DamagePacket *damPkt);
	MesHandle *damageMes;

	DamageAddresses() {

		rebase(damageMes, 0x102E3B30);

		rebase(DoDamage, 0x100B8D70);
		rebase(Heal, 0x100B7DF0);
		rebase(HealSpell, 0x100B81D0);
		rebase(DoSpellDamage, 0x100B7F80);
		rebase(HealSubdual, 0x100B9030);
		rebase(SavingThrow, 0x100B4F20);
		rebase(SavingThrowSpell, 0x100B83C0);
		rebase(ReflexSaveAndDamage, 0x100B9500);

		rebase(DamagePacketInit, 0x100E0390);
		rebase(AddDamageDice, 0x100E03F0);

		rebase(AddAttackPowerType, 0x100E0520);
		rebase(AddDamageBonus, 0x100E05E0);
		rebase(AddPhysicalDR, 0x100E0610);
		rebase(AddDamageModFactor, 0x100E06D0);
		rebase(AddIncorporealImmunity ,0x100E0780);
		rebase(AddDR, 0x100E0830);
		rebase(AddDamResistanceWithCause, 0x100E08F0);
		rebase(EnsureMinimumDamage1, 0x100E09B0);
		rebase(SetMaximizedFlag, 0x100E0A50);
		rebase(SetEmpoweredFlag, 0x100E0A60);
		rebase(CalcDamageModFromDR, 0x100E0C90);
		rebase(CalcDamageModFromFactor, 0x100E0E00);
		rebase(GetDamageTypeOverallDamage, 0X100E1210);
		rebase(GetOverallDamage, 0x100E1360);
		rebase(CalcFinalDamage, 0x100E16F0);

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
	return addresses.SavingThrowSpell(obj, attacker, dc, type, flags, spellId);
}

bool Damage::ReflexSaveAndDamage(objHndl obj, objHndl attacker, int dc, int reduction, int flags, const Dice& dice, DamageType damageType, int attackPower, D20ActionType actionType, int spellId) {
	return addresses.ReflexSaveAndDamage(obj, attacker, dc, reduction, flags, dice.ToPacked(), damageType, attackPower, actionType, spellId);
}

void Damage::DamagePacketInit(DamagePacket* dmgPkt)
{
	dmgPkt->diceCount=0;
	dmgPkt->damResCount=0;
	dmgPkt->damModCount=0;
	dmgPkt->attackPowerType=0;
	dmgPkt->finalDamage=0;
	dmgPkt->flags=0;
	dmgPkt->description=0;
	dmgPkt->critHitMultiplier=1;
	bonusSys.initBonusList(&dmgPkt->bonuses);
}

int Damage::AddDamageBonusWithDescr(DamagePacket* damage, int damBonus, int bonType, int bonusMesLine, char* desc)
{
	bonusSys.bonusAddToBonusListWithDescr(&damage->bonuses, damBonus, bonType, bonusMesLine, desc );
	return 1;
}

int Damage::AddPhysicalDR(DamagePacket* damPkt, int DRAmount, int bypasserBitmask, unsigned damageMesLine)
{
	MesLine mesLine; 

	if (damPkt->damResCount < 5u)
	{
		mesLine.key = damageMesLine;
		mesFuncs.GetLine_Safe(*damageMes, &mesLine);
		damPkt->damageResistances[damPkt->damResCount].damageReductionAmount = DRAmount;
		damPkt->damageResistances[damPkt->damResCount].dmgFactor = 0;
		damPkt->damageResistances[damPkt->damResCount].type = DamageType::SlashingAndBludgeoningAndPiercing;
		damPkt->damageResistances[damPkt->damResCount].bypasserBitmask = bypasserBitmask;
		damPkt->damageResistances[damPkt->damResCount].typeDescription = mesLine.value;
		damPkt->damageResistances[damPkt->damResCount++].causedBy = 0;
		return 1;
	}
	return  0;
	
}

int Damage::AddDamageDice(DamagePacket* dmgPkt, int dicePacked, DamageType damType, unsigned damageMesLine)
{
	return addresses.AddDamageDice(dmgPkt, dicePacked, damType, damageMesLine);
}

int Damage::AddDamageDiceWithDescr(DamagePacket* dmgPkt, int dicePacked, DamageType damType, unsigned damageMesLine, char* descr)
{
	if (AddDamageDice(dmgPkt, dicePacked, damType, damageMesLine) == 1)
	{
		dmgPkt->dice[dmgPkt->diceCount - 1].causedBy = descr;
		return 1;
	}
	return 0;
}

Damage::Damage()
{
	damageMes = addresses.damageMes;
}