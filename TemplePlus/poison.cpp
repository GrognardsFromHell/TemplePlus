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
#include <fstream>
#include <gamesystems\gamesystems.h>
#include <combat.h>
#include <gamesystems\d20\d20stats.h>
#include "config\config.h"

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

	// decrement duration
	auto dur = args.GetCondArg(1);
	auto durRem = dur - (int)dispIo->data1;
	if (durRem >= 0) {
		args.SetCondArg(1, durRem);
		return 0;
	}

	// check delay poison
	auto delay = conds.GetByName("sp-Delay Poison");
	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Critter_Has_Condition, delay, 0)) {
		floatSys.FloatSpellLine(args.objHndCaller, 20033, FloatLineColor::White);

		// add re-apply condition
		if (config.stricterRulesEnforcement) {
			auto pid = args.GetCondArg(0);
			auto dc = args.GetCondArg(2);
			conds.AddTo(args.objHndCaller, "Delayed Poison", { pid, 1, dc });
		}
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}


	return ApplyPoisonSecondary(args);
}

int ApplyPoisonSecondary(DispatcherCallbackArgs args) {
	auto poisonId = args.GetCondArg(0);
	auto pspec = poisonFixes.GetPoisonSpec(poisonId);
	if (pspec == nullptr) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// make saving throw
	auto dc = pspec->dc;
	if (dc <= 0) {
		dc = args.GetCondArg(2);
	}
	if (dc < 0 || dc > 100) // failsafe
		dc = 15;

	if (pspec->delayedEffect == (int)PoisonEffect::None) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// success - remove condition
	if (damage.SavingThrow(args.objHndCaller, objHndl::null, dc, SavingThrowType::Fortitude, D20STF_POISON)) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// failure

	if (pspec->delayedEffect == (int)PoisonEffect::Paralyze) // paralyze
	{
		auto rollResParalyzedRounds = Dice(2, 6, 0).Roll() * 10; // x10 due to minutes, not rounds
		conds.AddTo(args.objHndCaller, "Paralyzed", { rollResParalyzedRounds, 0, 0 });
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}
	if (pspec->delayedEffect == (int)PoisonEffect::HPDamage) // HP damage
	{
		auto dice = Dice(pspec->delayedDice.count, pspec->delayedDice.sides, pspec->delayedDice.bonus);
		damage.DealDamage(args.objHndCaller, objHndl::null, dice, DamageType::Poison, 1, 100, 0, D20ActionType::D20A_NONE);
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	if (pspec->delayedEffect == (int)PoisonEffect::Unconsciousness)
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

	if (pspec->delayedSecondEffect != (int)PoisonEffect::None) {
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

	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Delay Poison"), 0)) {
		floatSys.FloatSpellLine(args.objHndCaller, 20033, FloatLineColor::White); // Effects delayed due to Delay Poison!

		// If strict rules, remove this condition and add one that will re-apply the
		// poison when Delay Poison runs out.
		if (config.stricterRulesEnforcement) {
			auto dc = args.GetCondArg(2);
			conds.AddTo(args.objHndCaller, "Delayed Poison", { poisonId, 0, dc });
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		}
		return 0;
	}

	if (immEffect == (int)PoisonEffect::None)
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

	if (immEffect == (int)PoisonEffect::Paralyze) // paralyze
	{
		auto rollResParalyzedRounds = Dice(2, 6, 0).Roll() * 10; // x10 due to minutes, not rounds
		conds.AddTo(args.objHndCaller, "Paralyzed", { rollResParalyzedRounds, 0, 0 });
		return 0;
	}
	if (immEffect == (int)PoisonEffect::HPDamage) // HP damage
	{
		auto dice = Dice(pspec->immNumDie, pspec->immDieType, pspec->immDieBonus);
		damage.DealDamage(args.objHndCaller, objHndl::null, dice, DamageType::Poison, 1, 100, 0, D20ActionType::D20A_NONE);
		return 0;
	}

	if (immEffect == (int)PoisonEffect::Unconsciousness)
	{
		conds.AddTo(args.objHndCaller, "Unconscious", { });
		gameSystems->GetAnim().PushAnimate(args.objHndCaller, 64);
		floatSys.FloatCombatLine(args.objHndCaller, 17); // Unconscious!
		histSys.CreateRollHistoryLineFromMesfile(16, args.objHndCaller, objHndl::null); // [ACTOR] falls ~unconscious~[TAG_UNCONSCIOUS]!
		return 0; 
	}

	auto rollRes = Dice(pspec->immNumDie, pspec->immDieType, pspec->immDieBonus).Roll();
	conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec->immediateEffect + (pspec->immediateEffect < 0 ? 6 : 0), rollRes });
	{
		Stat stat = (Stat)abs(pspec->immediateEffect);
		auto statName = d20Stats.GetStatShortName(stat);
		histSys.CreateFromFreeText(fmt::format("{} takes {} {} damage from poison!\n", description.getDisplayName(args.objHndCaller), rollRes, statName).c_str());
	}

	if (pspec->immediateSecondEffect != (int)PoisonEffect::None) {
		rollRes = Dice(pspec->immSecDice.count, pspec->immSecDice.sides, pspec->immSecDice.bonus).Roll();
		conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec->immediateSecondEffect + (pspec->immediateSecondEffect < 0 ? 6 : 0), rollRes });

	}
	floatSys.FloatCombatLine(args.objHndCaller, 96); // Ability Loss
	return 0;
}

const PoisonSpec * PoisonFixes::GetPoisonSpec(int poisonId)
{
	return gameSystems->GetPoison().GetSpec(poisonId);
}

PoisonSystem::PoisonSystem()
{
	// json will not override already loaded specs
	if (!config.preferPoisonSpecFile) {
		AssignVanillaSpecs();
	}
	//SaveSpecsFile("d:\\temp\\vpoisons.json");
	LoadSpecsFile("rules/poisons.json");
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
		throw TempleException("Unable to parse poison.json from {}: {}", path, error);
	}

	if (!json.is_array()) {
		throw TempleException("poison.json must start with an array at the root");
	}

	LoadSpecs(json);
}

void PoisonSystem::SaveSpecsFile(const std::string & path)
{
	std::ofstream dump(path, std::fstream::trunc);
	if (!dump.is_open()) return;
	std::vector<json11::Json> items;
	size_t len = mPoisonSpecs.size();
	for (size_t i = 0; i < len; i++) {
		auto& item = mPoisonSpecs[i];
		json11::Json::object o = { 
			{"id", (INT32)i},
			{"dc", (INT32)item.dc},
			{"immediateEffect", item.immediateEffect},
			{"comment", _GetCombatMesLine(i + 300)}
		};
		if (item.immNumDie != 0)
			o.emplace("immNumDie", (INT32)item.immNumDie);
		if (item.immDieType != 0)
			o.emplace("immDieType", (INT32)item.immDieType);
		if (item.immDieBonus != 0)
			o.emplace("immDieBonus", (INT32)item.immDieBonus);

		if (item.immediateSecondEffect != -10)
		{
			o.emplace("immediateSecondEffect", item.immediateSecondEffect);
			if ((item.immSecDice.bonus != 0) || (item.immSecDice.sides != 0) || (item.immSecDice.count != 0)){
				json11::Json::object so = {};
				if (item.immSecDice.bonus != 0)
					so.emplace("bonus", (INT32)item.immSecDice.bonus);
				if (item.immSecDice.count != 0)
					so.emplace("count", (INT32)item.immSecDice.count);
				if (item.immSecDice.sides != 0)
					so.emplace("sides", (INT32)item.immSecDice.sides);
				o.emplace("immSecDice", so);
			}
		}

		if (item.delayedEffect != -10)
		{
			o.emplace("delayedEffect", item.delayedEffect);
			if ((item.delayedDice.bonus != 0) || (item.delayedDice.sides != 0) || (item.delayedDice.count != 0)) {
				json11::Json::object so = {};
				if (item.delayedDice.bonus != 0)
					so.emplace("bonus", (INT32)item.delayedDice.bonus);
				if (item.delayedDice.count != 0)
					so.emplace("count", (INT32)item.delayedDice.count);
				if (item.delayedDice.sides != 0)
					so.emplace("sides", (INT32)item.delayedDice.sides);
				o.emplace("delayedDice", so);
			}
		}

		if (item.delayedSecondEffect != -10)
		{
			o.emplace("delayedSecondEffect", item.delayedSecondEffect);
			if ((item.delayedSecDice.bonus != 0) || (item.delayedSecDice.sides != 0) || (item.delayedSecDice.count != 0)) {
				json11::Json::object so = {};
				if (item.delayedSecDice.bonus != 0)
					so.emplace("bonus", (INT32)item.delayedSecDice.bonus);
				if (item.delayedSecDice.count != 0)
					so.emplace("count", (INT32)item.delayedSecDice.count);
				if (item.delayedSecDice.sides != 0)
					so.emplace("sides", (INT32)item.delayedSecDice.sides);
				o.emplace("delayedSecDice", so);
			}
		}

		items.push_back(o);
	}
	dump << json11::Json(items).dump();
	dump.close();
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
			logger->warn("Skipping poison that is not an object.");
			continue;
		}

		auto idNode = item["id"];
		if (!idNode.is_number()) {
			logger->warn("Skipping poison that is missing 'id' attribute.");
			continue;
		}
		int id = (uint8_t)item["id"].int_value();

		PoisonSpec spec = {};
		spec.immediateEffect = -10;
		spec.immediateSecondEffect = -10;
		spec.delayedEffect = -10;
		spec.delayedSecondEffect = -10;

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
				spec.immDieBonus = (int8_t)item["immDieBonus"].int_value();
			}
		}

		{
			if (item["immediateSecondEffect"].is_number()) {
				spec.immediateSecondEffect = item["immediateSecondEffect"].int_value();
			}

			if (item["immSecDice"].is_object()) {
				auto &subitem = item.object_items().at("immSecDice");

				if (subitem["bonus"].is_number()) {
					spec.immSecDice.bonus = (int8_t)subitem["bonus"].int_value();
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
					spec.delayedDice.bonus = (int8_t)subitem["bonus"].int_value();
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
					spec.delayedSecDice.bonus = (int8_t)subitem["bonus"].int_value();
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
