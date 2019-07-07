#include "stdafx.h"
#include "util/fixes.h"
#include <temple/dll.h>
#include "poison.h"
#include "dispatcher.h"
#include "condition.h"
#include "damage.h"
#include "d20.h"
#include "float_line.h"
#include "history.h"
#include <infrastructure/json11.hpp>
#include <infrastructure/vfs.h>
#include "gamesystems/gamesystems.h"
#include "animgoals/anim.h"

PoisonSystem* poisonSystem = nullptr;

static class PoisonFixes : public TempleFix
{
public:
	void apply() override;
	static int PoisonedOnBeginRound(DispatcherCallbackArgs args);
	static int PoisonedOnAdd(DispatcherCallbackArgs args);

	static const PoisonSpec* GetPoisonSpec(int poisonId);

} poisonFixes;


void PoisonFixes::apply() {
	replaceFunction(0x100EA040, PoisonedOnBeginRound);
	replaceFunction(0x100E9E50, PoisonedOnAdd);
}

int PoisonFixes::PoisonedOnBeginRound(DispatcherCallbackArgs args) {
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);

	auto poisonId = args.GetCondArg(0);
	auto pspec = GetPoisonSpec(poisonId);
	if (pspec == nullptr) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// decrement duration
	auto dur = args.GetCondArg(1);
	auto durRem = dur - (int)dispIo->data1;
	if (durRem >= 0) {
		args.SetCondArg(1, durRem);
		return 0;
	}

	// make saving throw
	auto dc = pspec->dc;
	if (dc <= 0) {
		dc = args.GetCondArg(2);
	}
	if (dc < 0 || dc > 100) // failsafe
		dc = 15;

	if (pspec->delayedEffect == -10) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// success - remove condition
	if (damage.SavingThrow(args.objHndCaller, objHndl::null, dc, SavingThrowType::Fortitude, D20STF_POISON)) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// failure

	// check delay poison
	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Delay Poison"), 0)) {
		floatSys.FloatSpellLine(args.objHndCaller, 20033, FloatLineColor::White);
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	if (pspec->delayedEffect == -8) // paralyze
	{
		auto rollResParalyzedRounds = Dice(2, 6, 0).Roll() * 10; // x10 due to minutes, not rounds
		conds.AddTo(args.objHndCaller, "Paralyzed", { rollResParalyzedRounds, 0, 0 });
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}
	if (pspec->delayedEffect == -7) // HP damage
	{
		auto dice = Dice(pspec->delayedDice.count, pspec->delayedDice.sides, pspec->delayedDice.bonus);
		damage.DealDamage(args.objHndCaller, objHndl::null, dice, DamageType::Poison, 1, 100, 0, D20ActionType::D20A_NONE);
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	if (pspec->delayedEffect == -9)
	{
		conds.AddTo(args.objHndCaller, "Unconscious", { });
		gameSystems->GetAnim().PushAnimate(args.objHndCaller, 64);
		floatSys.FloatCombatLine(args.objHndCaller, 17); // Unconscious!
		histSys.CreateRollHistoryLineFromMesfile(16, args.objHndCaller, objHndl::null); // [ACTOR] falls ~unconscious~[TAG_UNCONSCIOUS]!
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}


	floatSys.FloatCombatLine(args.objHndCaller, 56);
	histSys.CreateRollHistoryLineFromMesfile(21, args.objHndCaller, objHndl::null); // "X takes poison damage!"
	floatSys.FloatCombatLine(args.objHndCaller, 96);

	auto rollRes = Dice(pspec->delayedDice.count, pspec->delayedDice.sides, pspec->delayedDice.bonus).Roll();
	conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec->delayedEffect + (pspec->delayedEffect < 0 ? 6 : 0), rollRes });

	if (pspec->delayedSecondEffect != -10) {
		rollRes = Dice(pspec->delayedSecDice.count, pspec->delayedSecDice.sides, pspec->delayedSecDice.bonus).Roll();
		conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec->delayedSecondEffect + (pspec->delayedSecondEffect < 0 ? 6 : 0), rollRes });

	}

	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
}

int PoisonFixes::PoisonedOnAdd(DispatcherCallbackArgs args) {

	auto poisonId = args.GetCondArg(0);
	auto pspec = GetPoisonSpec(poisonId);
	if (pspec == nullptr) { 
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0; 
	}

	args.SetCondArg(1, 10); // set initial 10 rounds countdown for secondary damage
	int immEffect = pspec->immediateEffect;

	if (immEffect == -10)
	{
		return 0; // do nothing
	}

	auto dc = pspec->dc;
	if (dc <= 0) {
		dc = args.GetCondArg(2);
	}

	histSys.CreateRollHistoryLineFromMesfile(20, args.objHndCaller, objHndl::null); // [ACTOR] is ~poisoned~[TAG_POISON]!
	floatSys.FloatCombatLine(args.objHndCaller, 55); // Poisoned!
	if (damage.SavingThrow(args.objHndCaller, objHndl::null, dc, SavingThrowType::Fortitude, D20STF_POISON))
	{
		return 0;
	}

	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Delay Poison"), 0)) {
		floatSys.FloatSpellLine(args.objHndCaller, 20033, FloatLineColor::White); // Effects delayed due to Delay Poison!
		return 0;
	}

	if (immEffect == -8) // paralyze
	{
		auto rollResParalyzedRounds = Dice(2, 6, 0).Roll() * 10; // x10 due to minutes, not rounds
		conds.AddTo(args.objHndCaller, "Paralyzed", { rollResParalyzedRounds, 0, 0 });
		return 0;
	}
	if (immEffect == -7) // HP damage
	{
		auto dice = Dice(pspec->immNumDie, pspec->immDieType, pspec->immDieBonus);
		damage.DealDamage(args.objHndCaller, objHndl::null, dice, DamageType::Poison, 1, 100, 0, D20ActionType::D20A_NONE);
		return 0;
	}

	if (immEffect == -9)
	{
		conds.AddTo(args.objHndCaller, "Unconscious", { });
		gameSystems->GetAnim().PushAnimate(args.objHndCaller, 64);
		floatSys.FloatCombatLine(args.objHndCaller, 17); // Unconscious!
		histSys.CreateRollHistoryLineFromMesfile(16, args.objHndCaller, objHndl::null); // [ACTOR] falls ~unconscious~[TAG_UNCONSCIOUS]!
		return 0; 
	}

	auto rollRes = Dice(pspec->immNumDie, pspec->immDieType, pspec->immDieBonus).Roll();
	conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec->immediateEffect + (pspec->immediateEffect < 0 ? 6 : 0), rollRes });

	if (pspec->immediateSecondEffect != -10) {
		rollRes = Dice(pspec->immSecDice.count, pspec->immSecDice.sides, pspec->immSecDice.bonus).Roll();
		conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec->immediateSecondEffect + (pspec->immediateSecondEffect < 0 ? 6 : 0), rollRes });

	}
	floatSys.FloatCombatLine(args.objHndCaller, 96); // Ability Loss
	return 0;
}

const PoisonSpec * PoisonFixes::GetPoisonSpec(int poisonId)
{
	if (!poisonSystem) {
		if ((poisonId >= 0) && (poisonId < 36))
		{
			auto &poisonSpecs = temple::GetRef<PoisonSpec[36]>(0x1028C080);
			return &poisonSpecs[poisonId];
		}
		return nullptr;
	}
	return poisonSystem->GetSpec(poisonId);
}


PoisonSystem::PoisonSystem()
{
	Expects(!poisonSystem);
	poisonSystem = this;

	AssignVanillaSpecs();
	LoadSpecsFile("data/mes/poisons.json");
}

PoisonSystem::~PoisonSystem()
{
	if (poisonSystem == this)
	{
		poisonSystem = nullptr;
	}
}

const std::string & PoisonSystem::GetName() const
{
	static std::string name("Poison");
	return name;
}

void PoisonSystem::LoadSpecsFile(const std::string & path)
{
	std::string error;
	if (!vfs->FileExists(path)) return;
	json11::Json json = json.parse(vfs->ReadAsString(path), error);

	if (json.is_null()) {
		throw TempleException("Unable to parse text styles from {}: {}", path, error);
	}

	if (!json.is_array()) {
		throw TempleException("Text style files must start with an array at the root");
	}

	LoadSpecs(json);
}

void PoisonSystem::AssignVanillaSpecs()
{
	auto &poisonSpecs0 = temple::GetRef<PoisonSpec[36]>(0x1028C080);

	for (size_t i = 0; i < 36; i++)
	{
		AddSpec(i, poisonSpecs0[i]);
	}
}

void PoisonSystem::LoadSpecs(const json11::Json & jsonStyleArray)
{
	for (auto &item : jsonStyleArray.array_items()) {
		if (!item.is_object()) {
			logger->warn("Skipping text style that is not an object.");
			continue;
		}

		auto idNode = item["id"];
		if (!idNode.is_number()) {
			logger->warn("Skipping text style that is missing 'id' attribute.");
			continue;
		}
		int id = (uint8_t)item["id"].int_value();

		PoisonSpec spec = {};

		if (item["dc"].is_number()) {
			spec.dc = (uint8_t)item["dc"].int_value();
		}
		{
			if (item["immediateEffect"].is_number()) {
				spec.immediateEffect = item["immediateEffect"].int_value();
			}

			if (item["immNumDie"].is_number()) {
				spec.immNumDie = (uint8_t)item["immNumDie"].int_value();
			}

			if (item["immDieType"].is_number()) {
				spec.immDieType = (uint8_t)item["immDieType"].int_value();
			}

			if (item["immDieBonus"].is_number()) {
				spec.immDieBonus = (uint8_t)item["immDieBonus"].int_value();
			}
		}

		{
			if (item["immediateSecondEffect"].is_number()) {
				spec.immediateSecondEffect = item["immediateSecondEffect"].int_value();
			}

			if (item["immSecDice"].is_object()) {
				auto &subitem = item.object_items().at("immSecDice");

				if (subitem["bonus"].is_number()) {
					spec.immSecDice.bonus = (uint8_t)subitem["bonus"].int_value();
				}

				if (subitem["count"].is_number()) {
					spec.immSecDice.count = (uint8_t)subitem["count"].int_value();
				}

				if (subitem["sides"].is_number()) {
					spec.immSecDice.sides = (uint8_t)subitem["sides"].int_value();
				}
			}
		}

		{
			if (item["delayedEffect"].is_number()) {
				spec.delayedEffect = item["delayedEffect"].int_value();
			}

			if (item["delayedDice"].is_object()) {
				auto &subitem = item.object_items().at("delayedDice");

				if (subitem["bonus"].is_number()) {
					spec.delayedDice.bonus = (uint8_t)subitem["bonus"].int_value();
				}

				if (subitem["count"].is_number()) {
					spec.delayedDice.count = (uint8_t)subitem["count"].int_value();
				}

				if (subitem["sides"].is_number()) {
					spec.delayedDice.sides = (uint8_t)subitem["sides"].int_value();
				}
			}
		}

		{
			if (item["delayedSecondEffect"].is_number()) {
				spec.delayedSecondEffect = item["delayedSecondEffect"].int_value();
			}

			if (item["delayedSecDice"].is_object()) {
				auto &subitem = item.object_items().at("delayedSecDice");

				if (subitem["bonus"].is_number()) {
					spec.delayedSecDice.bonus = (uint8_t)subitem["bonus"].int_value();
				}

				if (subitem["count"].is_number()) {
					spec.delayedSecDice.count = (uint8_t)subitem["count"].int_value();
				}

				if (subitem["sides"].is_number()) {
					spec.delayedSecDice.sides = (uint8_t)subitem["sides"].int_value();
				}
			}
		}

		AddSpec(id, spec);
	}
}

void PoisonSystem::AddSpec(int id, const PoisonSpec &spec)
{
	auto it = mPoisonSpecs.find(id);
	if (it != mPoisonSpecs.end()) {
		mPoisonSpecs.erase(id);
	}
	mPoisonSpecs.emplace(id, std::move(spec));
}

const PoisonSpec * PoisonSystem::GetSpec(int id)
{
	auto it = mPoisonSpecs.find(id);
	if (it != mPoisonSpecs.end()) {
		return &it->second;
	}
	return nullptr;
}
