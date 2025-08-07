#pragma once
#include "dice.h"
#include "dispatcher.h"
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
//#include <gamesystem.h>

struct PoisonSpec {
	char dc;
	char immNumDie;
	char immDieType;
	char immDieBonus;
	int immediateEffect; // -7 does direct damage, -8 causes paralysis, -10 nothing

	PackedDie immSecDice;
	int immediateSecondEffect;

	PackedDie delayedDice;
	int delayedEffect;

	PackedDie delayedSecDice;
	int delayedSecondEffect;

};
//const int testSizeOfPoisonSpec = sizeof PoisonSpec; // should be 32 (0x20)

enum class PoisonEffect : int {
	None = -10,
	Unconsciousness = -9,
	Paralyze = -8,
	HPDamage = -7,
	StrPermanent = -6,
	DexPermanent = -5,
	ConPermanent = -4,
	IntPermanent = -3,
	WisPermanent = -2,
	ChaPermanent = -1,
	StrTemporary = 0,
	DexTemporary = 1,
	ConTemporary = 2,
	IntTemporary = 3,
	WisTemporary = 4,
	ChaTemporary = 5
};

namespace json11 {
	class Json;
}

class PoisonSystem {
public:
	static constexpr auto Name = "Poison";
	PoisonSystem();
	const std::string &GetName() const;//override;

	void LoadSpecsFile(const std::string &path);
	void SaveSpecsFile(const std::string & path);
	void AssignVanillaSpecs();
	void LoadSpecs(const json11::Json &jsonStyleArray);
	void AddSpec(int id, const PoisonSpec &spec);
	const PoisonSpec* GetSpec(int id);
private:
	eastl::hash_map<int, PoisonSpec> mPoisonSpecs;
};

int ApplyPoisonSecondary(DispatcherCallbackArgs args);

extern PoisonSystem* poisonSystem;
