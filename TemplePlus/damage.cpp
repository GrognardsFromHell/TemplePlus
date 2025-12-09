
#include "stdafx.h"
#include "damage.h"
#include "dice.h"
#include "bonus.h"
#include "ai.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "critter.h"
#include "weapon.h"
#include "combat.h"
#include "history.h"
#include "float_line.h"
#include "sound.h"
#include "animgoals/anim.h"
#include "ui/ui_logbook.h"
#include "party.h"
#include <condition.h>
#include "python/python_integration_obj.h"
#include "python/python_object.h"
#include "pybind11/pybind11.h"
#include "python/python_dice.h"

namespace py = pybind11;

template <> class py::detail::type_caster<objHndl> {
public:
	bool load(handle src, bool) {
		value = PyObjHndl_AsObjHndl(src.ptr());
		success = true;
		return true;
	}

	static handle cast(const objHndl& src, return_value_policy /* policy */, handle /* parent */) {
		return PyObjHndl_Create(src);
	}

	PYBIND11_TYPE_CASTER(objHndl, _("objHndl"));
protected:
	bool success = false;
};
template <> class py::detail::type_caster<Dice> {
public:
	bool load(handle src, bool) {
		Dice dice;
		ConvertDice(src.ptr(), &dice);
		value = dice;
		success = true;
		return true;
	}

	static handle cast(const Dice& src, return_value_policy /* policy */, handle /* parent */) {
		return PyDice_FromDice(src);
	}

	PYBIND11_TYPE_CASTER(Dice, _("PyDice"));
protected:
	bool success = false;
};



static_assert(temple::validate_size<DispIoDamage, 0x550>::value, "DispIoDamage");

static struct DamageAddresses : temple::AddressTable {

	int (__cdecl *DoDamage)(objHndl target, objHndl src, uint32_t dmgDice, DamageType type, int attackPowerType, signed int reduction, int damageDescMesKey, int actionType);

	int (__cdecl *Heal)(objHndl target, objHndl healer, uint32_t dmgDice, D20ActionType actionType);

	void (__cdecl *HealSpell)(objHndl target, objHndl healer, uint32_t dmgDice, D20ActionType actionType, int spellId);

	int (__cdecl*HealSubdual)(objHndl target, int amount);

	int (__cdecl*DoSpellDamage)(objHndl victim, objHndl attacker, uint32_t dmgDice, DamageType type, int attackPowerType, int reduction, int damageDescMesKey, int actionType, int spellId, int flags);
	
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

class DamageHooks: TempleFix
{
	void apply () override{
		replaceFunction<int(__cdecl)(objHndl , objHndl , unsigned int, DamageType , 
			int , int , int , D20ActionType , int , int )>(0x100B7F80, [](objHndl victim, objHndl attacker, unsigned int nPackedDice, DamageType damType, 
			int attackPower, int reduction, int damageDescId, D20ActionType actionType, int spellId, int flags){

			Dice dice = Dice::FromPacked(nPackedDice);
			
			damage.DealSpellDamage(victim, attacker, dice, damType, attackPower, reduction, damageDescId, actionType, spellId, flags);
			return -1;
		});

		replaceFunction<int(__cdecl)(DamagePacket*, DamageType)>(0x100E1210, [](DamagePacket* pkt, DamageType damType){
			return pkt->GetOverallDamageByType(damType);
		});
		replaceFunction<int(__cdecl)(DamagePacket*)>(0x100E1360, [](DamagePacket* pkt) {
			return pkt->GetOverallDamage();
		});

		replaceFunction<int(__cdecl)(objHndl, objHndl, int, D20CAF, D20ActionType)>(0x100B7950, [](objHndl attacker, objHndl tgt, int d20Data, D20CAF flags, D20ActionType actionType) {
			return damage.DealAttackDamage(attacker, tgt, d20Data, flags, actionType);
		});

		replaceFunction<bool(__cdecl)(objHndl, objHndl, int, int, int)>(0x100B4F20, [](objHndl tgt, objHndl atk, int dc, int saveType, int flags) {
			return damage.SavingThrow(tgt, atk, dc, static_cast<SavingThrowType>(saveType), flags);
		});
	}
} damageHooks;

int DamagePacket::AddEtherealImmunity(){
	if (damModCount >= 5)
		return 0;


	MesLine line;
	line.key = 134;
	mesFuncs.GetLine_Safe(damage.damageMes, &line);
	damageFactorModifiers[damModCount].dmgFactor = 0;
	damageFactorModifiers[damModCount].type = DamageType::Unspecified;
	damageFactorModifiers[damModCount].attackPowerType = 0;
	damageFactorModifiers[damModCount].typeDescription= line.value;
	damageFactorModifiers[damModCount++].causedBy = nullptr;

	return 1;
}

int DamagePacket::AddDamageDice(uint32_t dicePacked, DamageType damType, int damageMesLine, const char* text){
	if (!damage.AddDamageDice(this, dicePacked, damType, damageMesLine))
		return 0;

	if (text){
		dice[diceCount-1].causedBy = text;
	}

	return 1;
}

BOOL DamagePacket::AddDamageBonus(int32_t damBonus, int bonType, int bonMesline, const char* causeDesc){
	bonuses.AddBonusWithDesc(damBonus, bonType, bonMesline, causeDesc);
	return 1;
}

int DamagePacket::AddPhysicalDR(int amount, int bypasserBitmask, int damageMesLine){
	return damage.AddPhysicalDR(this, amount, bypasserBitmask, (unsigned)damageMesLine);
}

int DamagePacket::AddDR(int amount, DamageType damType, int damageMesLine){

	if (this->damResCount < 5u){
		this->damageResistances[this->damResCount].damageReductionAmount = amount;
		this->damageResistances[this->damResCount].dmgFactor = 0.0f;
		this->damageResistances[this->damResCount].type = damType;
		this->damageResistances[this->damResCount].attackPowerType = D20DAP_UNSPECIFIED;
		this->damageResistances[this->damResCount].typeDescription = damage.GetMesline(damageMesLine);
		this->damageResistances[this->damResCount++].causedBy = nullptr;
		return TRUE;
	}
	return  FALSE;
}

void DamagePacket::AddAttackPower(int attackPower)
{
	this->attackPowerType |= attackPower;
}

/* 0x100E16F0 */
void DamagePacket::CalcFinalDamage(){
	for (auto i=0u; i < this->diceCount; i++){
		auto &dice = this->dice[i];
		if (dice.rolledDamage < 0){
			 
			Dice diceUnpacked = Dice::FromPacked(dice.dicePacked);
			if (this->flags & 1) // maximized
			{
				dice.rolledDamage = diceUnpacked.GetModifier() + diceUnpacked.GetCount() * diceUnpacked.GetSides();
			} else // normal
			{
				dice.rolledDamage = diceUnpacked.Roll();
			}

			if (this->flags & 2) //empowered
			{
				dice.rolledDamage = static_cast<int>(dice.rolledDamage * 1.5);
			}
		}
	}

	this->finalDamage = GetOverallDamageByType(DamageType::Unspecified);
}

/* 0x100E1210 */
int DamagePacket::GetOverallDamageByType(DamageType damType)
{
	auto damTot = (double)0.0;

	for (auto i=0u; i<this->diceCount; i++)	{
		if (damType == this->dice[i].type || damType == DamageType::Unspecified){
			damTot += this->dice[i].rolledDamage;
			if (i == 0) {
				damTot += this->bonuses.GetEffectiveBonusSum();
			}
		}
			
	}

	for (auto i=0; i<this->damModCount; i++){
		addresses.CalcDamageModFromFactor(this, &this->damageFactorModifiers[i], DamageType::Unspecified, DamageType::Unspecified);
	}

	for (auto i=0; i<this->damResCount; i++){
		addresses.CalcDamageModFromDR(this, &this->damageResistances[i], DamageType::Unspecified, DamageType::Unspecified);
	}

	for (auto i=0; i<this->damResCount; i++){
		if (damage.DamageTypeMatch(damType, this->damageResistances[i].type)){
			damTot += this->damageResistances[i].damageReduced;
		}
	}

	for (auto i = 0; i < this->damModCount; i++) {
		if (damage.DamageTypeMatch(damType, this->damageFactorModifiers[i].type)){
			damTot += this->damageFactorModifiers[i].damageReduced;
		}
	}

	if (damTot < 0.0)
		damTot = 0.0;

	return static_cast<int>(damTot);
}

int DamagePacket::GetOverallDamage()
{
	auto damTot = (double)0.0;

	for (auto i = 0u; i < this->diceCount; i++) {
		auto &dice = this->dice[i];
		if (dice.type != DamageType::Subdual) {
			damTot += dice.rolledDamage;
			if (i == 0) {
				damTot += this->bonuses.GetEffectiveBonusSum();
			}
		}

	}

	for (auto i = 0; i < this->damResCount; i++) {
		addresses.CalcDamageModFromDR(this, &this->damageResistances[i], DamageType::Unspecified, DamageType::Unspecified);
	}

	for (auto i = 0; i < this->damModCount; i++) {
		addresses.CalcDamageModFromFactor(this, &this->damageFactorModifiers[i], DamageType::Unspecified, DamageType::Unspecified);
	}

	

	for (auto i = 0; i < this->damResCount; i++) {
		auto &res = this->damageResistances[i];
		if (res.type != DamageType::Subdual){
			damTot += this->damageResistances[i].damageReduced;
		}
	}

	for (auto i = 0; i < this->damModCount; i++) {
		auto &damMod = this->damageFactorModifiers[i];
		if (damMod.type != DamageType::Subdual){
			damTot += this->damageFactorModifiers[i].damageReduced;
		}
	}

	if (damTot < 0.0)
		damTot = 0.0;

	return static_cast<int>(damTot);
}

int DamagePacket::AddModFactor(float factor, DamageType damType, int damageMesLine){

	if (this->damModCount < 5){
		this->damageFactorModifiers[this->damModCount].dmgFactor = factor;
		this->damageFactorModifiers[this->damModCount].type = damType;
		this->damageFactorModifiers[this->damModCount].attackPowerType = D20DAP_UNSPECIFIED;
		this->damageFactorModifiers[this->damModCount].typeDescription = damage.GetMesline(damageMesLine);
		this->damageFactorModifiers[this->damModCount++].causedBy = nullptr;
		return TRUE;
	}
	return FALSE;
}

BOOL DamagePacket::CriticalMultiplierApply(int multiplier)
{
	static auto critMultiplierApply = temple::GetRef<BOOL(__cdecl)(DamagePacket&, int, int)>(0x100E1640); // damagepacket, multiplier, damage.mes line
	return critMultiplierApply(*this, multiplier, 102);
}

void DamagePacket::PlayPfx(objHndl tgt)
{
	for (auto i = 0u; i < diceCount; i++) {
		temple::GetRef<void(__cdecl)(objHndl, DamageType, int)>(0x10016A90)(tgt, dice[i].type, dice[i].rolledDamage);
	}
}

DamagePacket::DamagePacket(){
	diceCount = 0;
	damResCount = 0;
	damModCount = 0;
	attackPowerType = 0;
	finalDamage = 0;
	flags = 0;
	description = nullptr;
	critHitMultiplier = 1;

}

void Damage::DealDamage(objHndl victim, objHndl attacker, const Dice& dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType) {

	addresses.DoDamage(victim, attacker, dice.ToPacked(), type, attackPower, reduction, damageDescId, actionType);

}

void Damage::DealSpellDamage(objHndl tgt, objHndl attacker, const Dice& dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType, int spellId, int flags) {

	return DealSpellDamagePython(tgt, attacker, dice, type, attackPower, reduction, damageDescId, actionType, spellId, flags);

	SpellPacketBody spPkt(spellId);
	if (!tgt)
		return;

	if (attacker && attacker != tgt && critterSys.NpcAllegianceShared(tgt, attacker))
		floatSys.FloatCombatLine(tgt, 107); // friendly fire

	aiSys.ProvokeHostility(attacker, tgt, 1, 0);

	if (critterSys.IsDeadNullDestroyed(tgt))
		return;

	DispIoDamage evtObjDam;
	evtObjDam.attackPacket.d20ActnType = actionType;
	evtObjDam.attackPacket.attacker = attacker;
	evtObjDam.attackPacket.victim = tgt;
	evtObjDam.attackPacket.dispKey = 1;
	evtObjDam.attackPacket.flags = (D20CAF)(flags | D20CAF_HIT);

	if (attacker && objects.IsCritter(attacker)){
		if (flags & D20CAF_SECONDARY_WEAPON)
			evtObjDam.attackPacket.weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponSecondary);
		else
			evtObjDam.attackPacket.weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponPrimary);

		if (evtObjDam.attackPacket.weaponUsed && objects.GetType(evtObjDam.attackPacket.weaponUsed) != obj_t_weapon)
			evtObjDam.attackPacket.weaponUsed = objHndl::null;

		evtObjDam.attackPacket.ammoItem = combatSys.CheckRangedWeaponAmmo(attacker);
	} else
	{
		evtObjDam.attackPacket.weaponUsed = objHndl::null;
		evtObjDam.attackPacket.ammoItem = objHndl::null;
	}

	if (reduction != 100){
		addresses.AddDamageModFactor(&evtObjDam.damage,  reduction * 0.01f, type, damageDescId);
	}

	evtObjDam.damage.AddDamageDice(dice.ToPacked(), type, 103);
	evtObjDam.damage.AddAttackPower(attackPower);
	auto mmData = (MetaMagicData)spPkt.metaMagicData;
	if (mmData.metaMagicEmpowerSpellCount)
		evtObjDam.damage.flags |= 2; // empowered
	if (mmData.metaMagicFlags & 1)
		evtObjDam.damage.flags |= 1; // maximized

	dispatch.DispatchSpellDamage(attacker, &evtObjDam.damage, tgt, &spPkt);

	temple::GetRef<int>(0x10BCA8AC) = 0; // is weapon damage (used in logbook for record holding)

	DamageCritter(attacker, tgt, evtObjDam);

	//addresses.DoSpellDamage(tgt, attacker, dice.ToPacked(), type, attackPower, reduction, damageDescId, actionType, spellId, flags);

}

void Damage::DealSpellDamagePython(objHndl tgt, objHndl attacker, const Dice& dice, DamageType type, int attackPower, int reduction, int damageDescId, D20ActionType actionType, int spellId, int flags, int projectileIdx, bool isWeaponlike)
{
	py::object pyDice = py::cast<Dice>( dice);
	py::tuple args = py::make_tuple(py::cast<objHndl>(tgt), py::cast<objHndl>(attacker), pyDice, static_cast<int>(type), static_cast<int>(attackPower), reduction, damageDescId, static_cast<int>(actionType), spellId, static_cast<int>(flags), projectileIdx, isWeaponlike);
	
	pythonObjIntegration.ExecuteScript("d20_combat.damage_critter", "deal_spell_damage", args.ptr());
	
}
/* 0x100B7950 */
int Damage::DealAttackDamage(objHndl attacker, objHndl tgt, int d20Data, D20CAF flags, D20ActionType actionType)
{

	auto pyResult = DealAttackDamagePython(attacker, tgt, d20Data, flags, actionType);
	return pyResult;

	aiSys.ProvokeHostility(attacker, tgt, 1, 0);

	auto tgtObj = objSystem->GetObject(tgt);
	if (critterSys.IsDeadNullDestroyed(tgt)){
		return -1;
	}

	DispIoDamage evtObjDam;
	evtObjDam.attackPacket.d20ActnType = actionType;
	evtObjDam.attackPacket.attacker = attacker;
	evtObjDam.attackPacket.victim = tgt;
	evtObjDam.attackPacket.dispKey = d20Data;
	evtObjDam.attackPacket.flags = flags;

	auto &weaponUsed = evtObjDam.attackPacket.weaponUsed;
	if (flags & D20CAF_SECONDARY_WEAPON)
		weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponSecondary);
	else
		weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponPrimary);

	if (weaponUsed && objects.GetType(weaponUsed) != obj_t_weapon){
		weaponUsed = objHndl::null;
	}

	evtObjDam.attackPacket.ammoItem = combatSys.CheckRangedWeaponAmmo(attacker);

	if ( flags & D20CAF_CONCEALMENT_MISS){
		histSys.CreateRollHistoryLineFromMesfile(11, attacker, tgt);
		floatSys.FloatCombatLine(attacker, 45); // Miss (Concealment)!
		auto soundId = inventory.GetSoundIdForItemEvent(weaponUsed, attacker, tgt, 6);
		sound.PlaySoundAtObj(soundId, attacker);
		d20Sys.d20SendSignal(attacker, DK_SIG_Attack_Made, (int)&evtObjDam, 0);
		return -1;
	}

	if (!(flags & D20CAF_HIT)) {
		floatSys.FloatCombatLine(attacker, 29);
		d20Sys.d20SendSignal(attacker, DK_SIG_Attack_Made, (int)&evtObjDam, 0);

		auto soundId = inventory.GetSoundIdForItemEvent(weaponUsed, attacker, tgt, 6);
		sound.PlaySoundAtObj(soundId, attacker);

		if (flags & D20CAF_DEFLECT_ARROWS){
			floatSys.FloatCombatLine(tgt, 5052);
			histSys.CreateRollHistoryLineFromMesfile(12, attacker, tgt);
		}

		// dodge animation
		if (!critterSys.IsDeadOrUnconscious(tgt) && !critterSys.IsProne(tgt)){
			gameSystems->GetAnim().PushDodge(attacker, tgt);
		}
		return -1;
	}

	if (tgt && attacker && combatSys.AffiliationSame(tgt, attacker) 
		&& (!tgtObj->IsNPC() || critterSys.NpcAllegianceShared(tgt, attacker))) // fixes NPC "Friendly fire" floats for factionless NPCs
	{
		floatSys.FloatCombatLine(tgt, 107); // Friendly Fire
	}

	auto isUnconsciousAlready = critterSys.IsDeadOrUnconscious(tgt);


	dispatch.DispatchDamage(attacker, &evtObjDam, dispTypeDealingDamage, DK_NONE);
	if (evtObjDam.attackPacket.flags & D20CAF_CRITICAL){

		// get extra Hit Dice and apply them
		DispIoAttackBonus evtObjCritDice;
		evtObjCritDice.attackPacket.victim = tgt;
		evtObjCritDice.attackPacket.d20ActnType = evtObjDam.attackPacket.d20ActnType;
		evtObjCritDice.attackPacket.attacker = attacker;
		evtObjCritDice.attackPacket.dispKey = d20Data;
		evtObjCritDice.attackPacket.flags = evtObjDam.attackPacket.flags;
		if (evtObjDam.attackPacket.flags & D20CAF_SECONDARY_WEAPON){
			evtObjCritDice.attackPacket.weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponSecondary);
		} else
			evtObjCritDice.attackPacket.weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponPrimary);
		if (evtObjCritDice.attackPacket.weaponUsed && objects.GetType(evtObjCritDice.attackPacket.weaponUsed) != obj_t_weapon)
			evtObjCritDice.attackPacket.weaponUsed = objHndl::null;
		evtObjCritDice.attackPacket.ammoItem = combatSys.CheckRangedWeaponAmmo(attacker);
		auto extraHitDice = dispatch.DispatchAttackBonus(attacker, objHndl::null, &evtObjCritDice, dispTypeGetCriticalHitExtraDice, DK_NONE);
		
		evtObjDam.damage.CriticalMultiplierApply(extraHitDice + 1);
		floatSys.FloatCombatLine(attacker, 12);
		
		// play sound
		auto soundId = critterSys.SoundmapCritter(tgt, 0);
		sound.PlaySoundAtObj(soundId, tgt);
		soundId = inventory.GetSoundIdForItemEvent(evtObjCritDice.attackPacket.weaponUsed, attacker, tgt, 7);
		sound.PlaySoundAtObj(soundId, attacker);

		// increase crit hits in logbook
		uiLogbook.IncreaseCritHits(attacker);
	} else
	{
		auto soundId = inventory.GetSoundIdForItemEvent(evtObjDam.attackPacket.weaponUsed, attacker, tgt, 5);
		sound.PlaySoundAtObj(soundId, attacker);
	}

	temple::GetRef<int>(0x10BCA8AC) = 1; // physical damage Flag used for logbook recording
	DamageCritter(attacker, tgt, evtObjDam);

	// play damage effect particles
	evtObjDam.damage.PlayPfx(tgt);

	d20Sys.d20SendSignal(attacker, DK_SIG_Attack_Made, (int)&evtObjDam, 0);

	// signal events
	if (!isUnconsciousAlready && critterSys.IsDeadOrUnconscious(tgt)){
		d20Sys.d20SendSignal(attacker, DK_SIG_Dropped_Enemy, (int)&evtObjDam, 0);
	}

	return evtObjDam.damage.GetOverallDamageByType(DamageType::Unspecified);
	//addresses.GetDamageTypeOverallDamage(&evtObjDam.damage, DamageType::Unspecified);
}

int Damage::DealAttackDamagePython(objHndl attacker, objHndl tgt, int d20Data, D20CAF flags, D20ActionType actionType)
{
	py::tuple args = py::make_tuple(py::cast<objHndl>(attacker), py::cast<objHndl>(tgt), d20Data, static_cast<int>(flags), static_cast<int>(actionType));

	auto pyResult = pythonObjIntegration.ExecuteScript("d20_combat.damage_critter", "deal_attack_damage", args.ptr());
	auto result = -1;
	if (PyLong_Check(pyResult)) {
		result = _PyInt_AsInt(pyResult);
	}
	Py_DECREF(pyResult);
	
	return result;
}

int Damage::DealWeaponlikeSpellDamage(objHndl tgt, objHndl attacker, const Dice & dice, DamageType type, int attackPower, int damFactor, int damageDescId, D20ActionType actionType, int spellId, D20CAF flags, int projectileIdx)
{

	DealSpellDamagePython(tgt, attacker, dice, type, attackPower, damFactor, damageDescId, actionType, spellId, flags, projectileIdx, true);
	return 0;


	SpellPacketBody spPkt(spellId);
	if (!tgt)
		return -1;

	if (attacker && attacker != tgt && critterSys.NpcAllegianceShared(tgt, attacker))
		floatSys.FloatCombatLine(tgt, 107); // friendly fire

	aiSys.ProvokeHostility(attacker, tgt, 1, 0);

	if (critterSys.IsDeadNullDestroyed(tgt))
		return -1;

	if (combatSys.IsFlankedBy(tgt, attacker))
		*(int*)&flags |= D20CAF_FLANKED;

	DispIoDamage evtObjDam;
	evtObjDam.attackPacket.d20ActnType = actionType;
	evtObjDam.attackPacket.attacker = attacker;
	evtObjDam.attackPacket.victim = tgt;
	evtObjDam.attackPacket.dispKey = projectileIdx;
	evtObjDam.attackPacket.flags = (D20CAF)(flags | D20CAF_HIT);

	if (attacker && objects.IsCritter(attacker)) {
		if (flags & D20CAF_SECONDARY_WEAPON)
			evtObjDam.attackPacket.weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponSecondary);
		else
			evtObjDam.attackPacket.weaponUsed = inventory.ItemWornAt(attacker, EquipSlot::WeaponPrimary);

		if (evtObjDam.attackPacket.weaponUsed && objects.GetType(evtObjDam.attackPacket.weaponUsed) != obj_t_weapon)
			evtObjDam.attackPacket.weaponUsed = objHndl::null;

		evtObjDam.attackPacket.ammoItem = combatSys.CheckRangedWeaponAmmo(attacker);
	}
	else
	{
		evtObjDam.attackPacket.weaponUsed = objHndl::null;
		evtObjDam.attackPacket.ammoItem = objHndl::null;
	}

	if (damFactor != 100) {
		addresses.AddDamageModFactor(&evtObjDam.damage, damFactor * 0.01f, type, damageDescId);
	}
	
	if (flags & D20CAF_CONCEALMENT_MISS) {
		histSys.CreateRollHistoryLineFromMesfile(11, attacker, tgt);
		floatSys.FloatCombatLine(attacker, 45); // Miss (Concealment)!
		// d20Sys.d20SendSignal(attacker, DK_SIG_Attack_Made, (int)&evtObjDam, 0); // casting a spell isn't considered an attack action
		return -1;
	}

	if (!(flags & D20CAF_HIT)) {
		floatSys.FloatCombatLine(attacker, 29);

		// dodge animation
		if (!critterSys.IsDeadOrUnconscious(tgt) && !critterSys.IsProne(tgt)) {
			gameSystems->GetAnim().PushDodge(attacker, tgt);
		}
		return -1;
	}

	temple::GetRef<int>(0x10BCA8AC) = 0; // is weapon damage

	// get damage dice
	evtObjDam.damage.AddDamageDice(dice.ToPacked(), type, 103);
	evtObjDam.damage.AddAttackPower(attackPower);
	auto mmData = (MetaMagicData)spPkt.metaMagicData;
	if (mmData.metaMagicEmpowerSpellCount)
		evtObjDam.damage.flags |= 2; // empowered
	if (mmData.metaMagicFlags & 1)
		evtObjDam.damage.flags |= 1; // maximized

	if (evtObjDam.attackPacket.flags & D20CAF_CRITICAL) {
		auto extraHitDice = 1;
		auto critMultiplierApply = temple::GetRef<BOOL(__cdecl)(DamagePacket&, int, int)>(0x100E1640); // damagepacket, multiplier, damage.mes line
		critMultiplierApply(evtObjDam.damage, extraHitDice + 1, 102);
		floatSys.FloatCombatLine(attacker, 12);

		// play sound
		auto soundId = critterSys.SoundmapCritter(tgt, 0);
		sound.PlaySoundAtObj(soundId, tgt);

		// increase crit hits in logbook
		uiLogbook.IncreaseCritHits(attacker);
	}

	dispatch.DispatchDamage(attacker, &evtObjDam, dispTypeDealingDamageWeaponlikeSpell, DK_NONE);
	dispatch.DispatchSpellDamage(attacker, &evtObjDam.damage, tgt, &spPkt);
	
	DamageCritter(attacker, tgt, evtObjDam);

	return -1;
}

/* 0x100B6B30 */
void Damage::DamageCritter(objHndl attacker, objHndl tgt, DispIoDamage & evtObjDam){
	//return temple::GetRef<void(__cdecl)(objHndl, objHndl, DispIoDamage&)>(0x100B6B30)(attacker, tgt, evtObjDam);
	
	return DamageCritterPython(attacker, tgt, evtObjDam);

	// replaced with Python handling, keeping this for reference
	auto tgtObj = objSystem->GetObject(tgt);
	if (!tgtObj) return;

	auto skipHitAnim = critterSys.IsDeadOrUnconscious(tgt) 
		|| (d20Sys.d20Query(tgt, DK_QUE_Prone) != 0);
	

	if (tgtObj->GetFlags() & OF_INVULNERABLE) {
		evtObjDam.damage.AddModFactor(0.0f, DamageType::Unspecified, 104);
	}

	evtObjDam.damage.CalcFinalDamage();
	dispatch.DispatchDamage(tgt, &evtObjDam, dispTypeTakingDamage, DK_NONE);
	if (evtObjDam.attackPacket.flags & D20CAF_TRAP) {
		attacker = objHndl::null;
	}
	else if (attacker != objHndl::null) {
		dispatch.DispatchDamage(attacker, &evtObjDam, dispTypeDealingDamage2, DK_NONE);
	}
	dispatch.DispatchDamage(tgt, &evtObjDam, dispTypeTakingDamage2, DK_NONE);

	if (attacker) {
		tgtObj->SetObjHndl(obj_f_last_hit_by, attacker);
	}

	auto damTot = evtObjDam.damage.GetOverallDamage();
	if (damTot < 0) damTot = 0;

	auto hpDam = critterSys.GetHpDamage(tgt);
	auto hpDamNew = hpDam + damTot;
	critterSys.SetHpDamage(tgt, hpDamNew);

	// Add roll history entry
	auto histId = histSys.RollHistoryAddType1DamageRoll(attacker, tgt, &evtObjDam.damage );
	histSys.CreateRollHistoryString(histId);

	int64_t hpChanged = -damTot;
	d20Sys.d20SendSignal(tgt, D20DispatcherKey::DK_SIG_HP_Changed, hpChanged);

	if (damTot > 0) {
		if (tgt) {
			conds.AddTo(tgt, "Damaged", { damTot, });
		}

		// bells and whistles
		if (attacker && tgt) {
			auto isWeaponDamage = temple::GetRef<BOOL>(0x10BCA8AC);
			uiLogbook.RecordHighestDamage(isWeaponDamage, damTot, attacker, tgt);
			if (attacker != tgt && critterSys.IsFriendly(attacker, tgt)) {
				auto dlgGetFriendlyFireVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int*)>(0x10037450);

				char ffText[1000]; int soundId;
				
				dlgGetFriendlyFireVoiceLine(tgt, attacker, ffText, &soundId);
				critterSys.PlayCritterVoiceLine(tgt, attacker, ffText, soundId);
				
			}
		}
	}

	auto subdualDamTot = evtObjDam.damage.GetOverallDamageByType(DamageType::Subdual);
	if (subdualDamTot < 0) subdualDamTot = 0;
	if (subdualDamTot > 0) {
		if (tgt) {
			conds.AddTo(tgt, "Damaged", { subdualDamTot ,  });
		}
	}
	auto subdualDam = critterSys.GetSubdualDamage(tgt);
	critterSys.SetSubdualDamage(tgt, subdualDam + subdualDamTot);
	d20Sys.d20SendSignal(tgt, D20DispatcherKey::DK_SIG_HP_Changed, -subdualDamTot, subdualDamTot < 0 ? -1 : 0 );

	// Float line "xxx HP"
	{
		auto floatLineColor = FloatLineColor::Red; // default
		auto colorObj = tgtObj;
		if (attacker) {
			auto attackerObj = objSystem->GetObject(attacker);
			if (attackerObj) colorObj = attackerObj;
		}
		if (colorObj->IsPC()) {
			floatLineColor = FloatLineColor::White;
		}
		else {
			if (colorObj->IsNPC()) {
				auto leader = critterSys.GetLeaderForNpc(tgt);
				if (party.IsInParty(leader)) {
					floatLineColor = FloatLineColor::Yellow;
				}
			}
		}
		if (damTot > 0 || subdualDamTot == 0) {
			constexpr int combatMesline_HP = 1;
			
			auto text = fmt::format("{} {}", damTot, combatSys.GetCombatMesLine(combatMesline_HP));
			if (text[0])
				floatSys.floatMesLine(tgt, 2, floatLineColor, text.c_str());
		}
		if (subdualDamTot > 0) {
			constexpr int combatMesline_Nonlethal = 25;

			auto text = fmt::format("{} {}", subdualDamTot, combatSys.GetCombatMesLine(combatMesline_Nonlethal));
			if (text[0])
				floatSys.floatMesLine(tgt, 2, floatLineColor, text.c_str());
		}
	}
	
	// Push hit animation
	if (attacker) {
		if (!skipHitAnim) {
			gameSystems->GetAnim().PushGoalHitByWeapon(attacker, tgt);
		}
	}

}

void Damage::DamageCritterPython(objHndl attacker, objHndl tgt, DispIoDamage& evtObjDam)
{
	py::object pyEvtObjDam = py::cast<DispIoDamage*>(&evtObjDam);
	py::object pyAttacker = py::cast(attacker);
	py::tuple args = py::make_tuple( pyAttacker, py::cast<objHndl>(tgt),pyEvtObjDam);

	auto result = pythonObjIntegration.ExecuteScript("d20_combat.damage_critter", "damage_critter", args.ptr());
	Py_DECREF(result);
}

// The logic for Fast Healing. This is a fixed amount of healing that
// happens at a regular interval. The amount applies to subdual first,
// then to lethal damage.
void Damage::FastHeal(objHndl critter, int amount) {
	if (!critter || amount <= 0) return;

	int totHeal = 0;

	auto subdual = critterSys.GetSubdualDamage(critter);
	// TODO: check for unhealable damage. Could be environmental for
	// subdual, and cursed/infernal wounds for lethal, if either is
	// implemented.
	if (subdual > 0) {
		auto sHeal = std::min(amount, subdual);
		critterSys.SetSubdualDamage(critter, subdual - sHeal);

		auto text = fmt::format("{} {}", sHeal, combatSys.GetCombatMesLine(33));
		floatSys.floatMesLine(critter, 2, FloatLineColor::LightBlue, text.c_str());

		amount -= sHeal;
		totHeal += sHeal;
	}

	auto lethal = critterSys.GetHpDamage(critter);
	if (amount > 0 && lethal > 0) {
		auto heal = std::min(amount, lethal);
		critterSys.SetHpDamage(critter, lethal - heal);

		auto text = fmt::format("{} {}", heal, combatSys.GetCombatMesLine(32));
		floatSys.floatMesLine(critter, 2, FloatLineColor::LightBlue, text.c_str());

		totHeal += heal;
	}

	if (totHeal > 0)
		d20Sys.d20SendSignal(critter, DK_SIG_HP_Changed, totHeal, 0);
}

void Damage::Heal(objHndl target, objHndl healer, const Dice& dice, D20ActionType actionType) {
	int healingBonus = 0;
	if (healer){
		healingBonus = d20Sys.D20QueryPython(healer, "Healing Bonus", 0);  //0 spell id for non-spell healing
	}
	Dice diceNew(dice.GetCount(), dice.GetSides(), dice.GetModifier() + healingBonus);
	addresses.Heal(target, healer, diceNew.ToPacked(), actionType);
}

void Damage::HealSpell(objHndl target, objHndl healer, const Dice& dice, D20ActionType actionType, int spellId) {
	int healingBonus = 0;
	if (healer) {
		healingBonus = d20Sys.D20QueryPython(healer, "Healing Bonus", spellId);  //Called with the id of the healing spell
	}
	Dice diceNew(dice.GetCount(), dice.GetSides(), dice.GetModifier() + healingBonus);
	addresses.HealSpell(target, healer, diceNew.ToPacked(), actionType, spellId);
}

void Damage::HealSubdual(objHndl target, int amount) {
	addresses.HealSubdual(target, amount);
}

bool Damage::SavingThrow(objHndl handle, objHndl attacker, int dc, SavingThrowType saveType, int flags, const BonusList *bonExtra) {
	auto obj = objSystem->GetObject(handle);
	if (!obj) {
		return false;
	}

	auto saveThrowMod = 0;

	DispIoSavingThrow evtObj;
	evtObj.obj = attacker;
	evtObj.flags = (int64_t)flags; // TODO: vanilla bug! flags input should be 64 bit (since some of the descriptor enums go beyond 32). Looks like they fixed it in the dispatcher but not this function.
	evtObj.flags |= (1ull << (D20STD_F_FINAL_ROLL-1));
	if (bonExtra) {
		evtObj.bonlist = *bonExtra;
	}
	
	// NPC special bonus from protos - moved to Global condition callback (was here in vanilla, so didn't show up on charsheet)

	dispatch.Dispatch13SavingThrow(handle, saveType, &evtObj);
	evtObj.obj = handle;
	if (attacker && !(evtObj.flags & 2)){
		saveThrowMod = dispatch.Dispatch14SavingThrowMod(attacker, saveType, &evtObj);
	}
	Dice dice(1, 20, 0);
	auto diceResult = dice.Roll();
	auto & savingThrowAlwaysRoll1 = temple::GetRef<int>(0x10BCA8B8);
	auto & savingThrowAlwaysRoll20 = temple::GetRef<int>(0x10BCA8B4);
	if (savingThrowAlwaysRoll1)
		diceResult = 1;
	else if (savingThrowAlwaysRoll20)
		diceResult = 20;
	evtObj.rollResult = diceResult;
	if (diceResult + saveThrowMod < dc || diceResult == 1){
		if (d20Sys.d20Query(handle, DK_QUE_RerollSavingThrow)){
			histSys.RollHistoryAddType3SavingThrow(handle, dc, saveType, flags, dice.ToPacked(), diceResult, &evtObj.bonlist);
			diceResult = dice.Roll();
			flags |= 1;
		}
	}

	if (diceResult == 20){
		// if (finalSaveThrowMod + 20 < dc)
			return true; // natural 20 - always succeeds
	}

	auto finalSaveThrowMod = dispatch.Dispatch44FinalSaveThrow(handle, saveType, &evtObj);
	auto histId = histSys.RollHistoryAddType3SavingThrow(handle, dc, saveType, flags, dice.ToPacked(), diceResult, &evtObj.bonlist);
	histSys.CreateRollHistoryString(histId);

	if (finalSaveThrowMod > saveThrowMod){
		// This branch replaces saving throws with a skill check, and the
		// latter do not fail or succeed automatically on any roll.
		return finalSaveThrowMod + diceResult >= dc;
	}
	else if (diceResult == 1){
		// if (finalSaveThrowMod + 1 >= dc)
			return false; // natural 1 - always fails
	}
	return saveThrowMod + diceResult >= dc;
}

bool Damage::SavingThrowSpell(objHndl obj, objHndl attacker, int dc, SavingThrowType type, int flags, int spellId, const BonusList *bonExtra) {

	auto result = false;
	SpellPacketBody spPkt(spellId);
	if (!spPkt.spellEnum)
		return false;

	SpellEntry spEntry(spPkt.spellEnum);
	auto flagsModified = flags | D20STF_SPELL_LIKE_EFFECT;
	switch (spEntry.spellSchoolEnum){
	case SpellSchools::School_Abjuration:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_ABJURATION;
		break;
	case SpellSchools::School_Conjuration:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_CONJURATION;
		break;
	case SpellSchools::School_Divination:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_DIVINATION;
		break;
	case SpellSchools::School_Enchantment:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_ENCHANTMENT;
		break;
	case SpellSchools::School_Evocation:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_EVOCATION;
		break;
	case SpellSchools::School_Illusion:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_ILLUSION;
		break;
	case SpellSchools::School_Necromancy:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_NECROMANCY;
		break;
	case SpellSchools::School_Transmutation:
		flagsModified |= D20SavingThrowFlag::D20STF_SPELL_SCHOOL_TRANSMUTATION;
		break;
	default:
		break;
	}

	for (auto i=0; i < 21; i++){ // i=0 -> 1 -> acid;  1<<13 -> 0x2000  (D20STF_SPELL_DESCRIPTOR_ACID)
		if ((spEntry.spellDescriptorBitmask & (1<<i)) == (1<<i)){
			flagsModified |= 1 << (i + 13);
		}
	}

	// fix for spell descriptor parsing that mapped "fortitude" to value 3
	if (type == (SavingThrowType)3){
		type = SavingThrowType::Fortitude;
	}

	BonusList bonlist;

	// Gets a DC bonus based on the target of the spell
	dispatch.DispatchTargetSpellDCBonus(attacker, obj, &bonlist, &spPkt);

	int nDCBonus = bonlist.GetEffectiveBonusSum();

	result = damage.SavingThrow(obj, attacker, dc+ nDCBonus, type, flagsModified, bonExtra);
	spPkt.savingThrowResult = result;
	if (!spPkt.UpdateSpellsCastRegistry()){
		logger->debug("SavingThrowSpell: Unable to save spell pkt");
		return false;
	}
	spPkt.UpdatePySpell();
	return result;
	// return addresses.SavingThrowSpell(obj, attacker, dc, type, flags, spellId);
}

bool Damage::ReflexSaveAndDamage(objHndl obj, objHndl attacker, int dc, int reduction, int flags, const Dice& dice, DamageType damageType, int attackPower, D20ActionType actionType, int spellId) {
	SpellPacketBody spPkt(spellId);
	BonusList bonlist;

	// Gets a DC bonus based on the target of the spell
	dispatch.DispatchTargetSpellDCBonus(attacker, obj, &bonlist, &spPkt);

	int nDCBonus = bonlist.GetEffectiveBonusSum();

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


int Damage::AddPhysicalDR(DamagePacket* damPkt, int DRAmount, int bypasserBitmask, unsigned int damageMesLine)
{
	if (damPkt->damResCount < 5u)
	{
		damPkt->damageResistances[damPkt->damResCount].damageReductionAmount = DRAmount;
		damPkt->damageResistances[damPkt->damResCount].dmgFactor = 0;
		damPkt->damageResistances[damPkt->damResCount].type = DamageType::SlashingAndBludgeoningAndPiercing;
		damPkt->damageResistances[damPkt->damResCount].attackPowerType = bypasserBitmask;
		damPkt->damageResistances[damPkt->damResCount].typeDescription = damage.GetMesline(damageMesLine);
		damPkt->damageResistances[damPkt->damResCount++].causedBy = 0;
		return TRUE;
	}
	return  FALSE;
	
}

const char* Damage::GetMesline(unsigned lineId){

	MesLine line(lineId);
	auto res = mesFuncs.GetLine(*addresses.damageMes, &line);
	if (!res){
		mesFuncs.GetLine_Safe(damageMes, &line);
	}
		

	return line.value;
}

int Damage::AddDamageDice(DamagePacket* dmgPkt, int dicePacked, DamageType damType, unsigned int damageMesLine){
	if (dmgPkt->diceCount >= 5)
		return FALSE;

	const char* line = damage.GetMesline(damageMesLine);

	auto _damType = damType;
	if (damType == DamageType::Unspecified)	{
		if (dmgPkt->diceCount > 0)
			_damType = dmgPkt->dice[0].type;
	}
	auto diceCount = dmgPkt->diceCount;
	dmgPkt->dice[dmgPkt->diceCount++] = DamageDice(dicePacked, _damType, line);

	return TRUE;
	//return addresses.AddDamageDice(dmgPkt, dicePacked, damType, damageMesLine);
}

int Damage::AddDamageDiceWithDescr(DamagePacket* dmgPkt, int dicePacked, DamageType damType, unsigned int damageMesLine, char* descr)
{
	if (AddDamageDice(dmgPkt, dicePacked, damType, damageMesLine) == 1)
	{
		dmgPkt->dice[dmgPkt->diceCount - 1].causedBy = descr;
		return 1;
	}
	return 0;
}

BOOL Damage::DamageTypeMatch(DamageType reduction, DamageType attackType){

	if (attackType == DamageType::Subdual) {
		if (reduction != DamageType::Subdual)
			return FALSE;
	}
	else if (attackType == DamageType::Unspecified)
		return TRUE;

	if (reduction == DamageType::Unspecified || attackType == reduction)
		return TRUE;

	switch (attackType){
	case DamageType::BludgeoningAndPiercing:
		if (reduction == DamageType::Bludgeoning || reduction == DamageType::Piercing)
			return TRUE;
		break;
	case DamageType::PiercingAndSlashing:
		if (reduction == DamageType::Piercing || reduction == DamageType::Slashing)
			return TRUE;
		break;
	case DamageType::SlashingAndBludgeoning:
		if (reduction == DamageType::Slashing || reduction == DamageType::Bludgeoning)
			return TRUE;
		break;
	case DamageType::SlashingAndBludgeoningAndPiercing:
		if (reduction == DamageType::Bludgeoning || reduction == DamageType::Slashing
			|| reduction == DamageType::Piercing || reduction == DamageType::BludgeoningAndPiercing
			|| reduction == DamageType::PiercingAndSlashing || reduction == DamageType::SlashingAndBludgeoning
			|| reduction == DamageType::Subdual)
			return TRUE;
		break;
	default:
		return FALSE;

	}

	return FALSE;
}

Dice Damage::ModifyDamageDiceForSize(Dice &base, int steps) {
	if (steps == 0) return base;

	int sides = base.GetSides();
	int count = base.GetCount();
	int mod = base.GetModifier();

	bool inc = steps > 0;

	for (int i = inc ? steps : -steps; i > 0; i--) {
		if (inc) {
			// weapon size increasing
			switch (sides)
			{
			case 1: sides = 2; continue;
			case 2: sides = 3; continue;
			case 3: sides = 4; continue;
			case 4: sides = 6; continue;
			case 6:
				if (count == 1) sides = 8;
				else if (count <= 3) count++;
				else count += 2;
				continue;
			case 8:
				if (count == 1) { count = 2; sides = 6; }
				else if (count <= 3) count++;
				else if (count <= 6) count += 2;
				else count += 4;
				continue;
			case 10: count *= 2; sides = 8; continue;
			case 12:
				sides = 6;
				if (count == 1) count = 3;
				else count = (count+1)*2;
				continue;
			default: break;
			}
		} else {
			// weapon size decreasing
			switch (sides)
			{
			case 2: sides = 1; continue;
			case 3: sides = 2; continue;
			case 10: sides = 8; continue;
			case 12: sides = 10; continue;
			case 4:
				if (count == 1) sides = 3;
				else { count--; sides = 6; }
				continue;
			case 6:
				if (count == 1) sides = 4;
				else if (count == 2) { count = 1; sides = 10; }
				else if (count <= 4) count--;
				else count -= 2;
				continue;
			case 8:
				if (count <= 2) sides = 6;
				else if (count <= 4) count--;
				else if (count <= 8) count -= 2;
				else count -= 4;
				continue;
			default: break;
			}
		}
	}

	return Dice(count, sides, mod);
}

Damage::Damage(){
	damageMes = 0;
	// damageMes = addresses.damageMes;
}

void Damage::Init(){
	mesFuncs.Open("tpmes\\damage.mes", &damageMes);
}

void Damage::Exit() const
{
	mesFuncs.Close(damageMes);
}

