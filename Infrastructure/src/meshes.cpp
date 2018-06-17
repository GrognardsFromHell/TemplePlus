
#include "infrastructure/meshes.h"
#include <fmt/format.h>

using namespace gfx;

static const char* sBardInstrumentTypeNames[] = {
	"bard_flute",
	"bard_drum",
	"bard_mandolin",
	"bard_trumpet",
	"bard_harp",
	"bard_lute",
	"bard_pipes",
	"bard_recorder"
};

static const char* sWeaponTypeNames[] = {
	"unarmed",
	"dagger",
	"sword",
	"mace",
	"hammer",
	"axe",
	"club",
	"battleaxe",
	"greatsword",
	"greataxe",
	"greathammer",
	"spear",
	"staff",
	"polearm",
	"bow",
	"crossbow",
	"sling",
	"shield",
	"flail",
	"chain",
	"2hflail",
	"shuriken",
	"monk"
};

static const char* sWeaponAnimNames[] = {
	"none",
	"rattack",
	"rattack2",
	"rattack3",
	"lattack",
	"lattack2",
	"lattack3",
	"walk",
	"run",
	"idle",
	"fhit",
	"fhit2",
	"fhit3",
	"lhit",
	"lhit2",
	"lhit3",
	"rhit",
	"rhit2",
	"rhit3",
	"bhit",
	"bhit2",
	"bhit3",
	"rcriticalswing",
	"lcriticalswing",
	"fidget",
	"fidget2",
	"fidget3",
	"sneak",
	"panic",
	"rcombatstart",
	"lcombatstart",
	"combatidle",
	"combatfidget",
	"special1",
	"special2",
	"special3",
	"fdodge",
	"rdodge",
	"ldodge",
	"bdodge",
	"rthrow",
	"lthrow",
	"lsnatch",
	"rsnatch",
	"lturn",
	"rturn"
};

static const char* sNormalAnimNames[] = {
	"falldown",
	"prone_idle",
	"prone_fidget",
	"getup",
	"magichands",
	"picklock",
	"picklock_concentrated",
	"examine",
	"throw",
	"death",
	"death2",
	"death3",
	"dead_idle",
	"dead_fidget",
	"death_prone_idle",
	"death_prone_fidget",
	"abjuration_casting",
	"abjuration_conjuring",
	"conjuration_casting",
	"conjuration_conjuring",
	"divination_casting",
	"divination_conjuring",
	"enchantment_casting",
	"enchantment_conjuring",
	"evocation_casting",
	"evocation_conjuring",
	"illusion_casting",
	"illusion_conjuring",
	"necromancy_casting",
	"necromancy_conjuring",
	"transmutation_casting",
	"transmutation_conjuring",
	"conceal",
	"conceal_idle",
	"unconceal",
	"item_idle",
	"item_fidget",
	"open",
	"close",
	"skill_animal_empathy",
	"skill_disable_device",
	"skill_heal",
	"skill_heal_concentrated",
	"skill_hide",
	"skill_hide_idle",
	"skill_hide_fidget",
	"skill_unhide",
	"skill_pickpocket",
	"skill_search",
	"skill_spot",
	"feat_track",
	"trip",
	"bullrush",
	"flurry",
	"kistrike",
	"tumble",
	"special1",
	"special2",
	"special3",
	"special4",
	"throw",
	"wand_abjuration_casting",
	"wand_abjuration_conjuring",
	"wand_conjuration_casting",
	"wand_conjuration_conjuring",
	"wand_divination_casting",
	"wand_divination_conjuring",
	"wand_enchantment_casting",
	"wand_enchantment_conjuring",
	"wand_evocation_casting",
	"wand_evocation_conjuring",
	"wand_illusion_casting",
	"wand_illusion_conjuring",
	"wand_necromancy_casting",
	"wand_necromancy_conjuring",
	"wand_transmutation_casting",
	"wand_transmutation_conjuring",
	"skill_barbarian_rage",
	"open_idle"
};

static const char* GetBardInstrumentTypeName(BardInstrumentType instrumentType) {
	return sBardInstrumentTypeNames[(int)instrumentType];
}

static const char* GetNormalAnimTypeName(NormalAnimType animType) {
	auto idx = (int)animType;
	if (idx >= 79) {
		idx = 0;
	}
	return sNormalAnimNames[idx];
}

static const char* GetWeaponAnimName(WeaponAnim weaponAnim) {
	return sWeaponAnimNames[(int)weaponAnim];
}

static const char* GetWeaponTypeName(WeaponAnimType weaponAnimType) {
	return sWeaponTypeNames[(int)weaponAnimType];
}

bool EncodedAnimId::IsConjuireAnimation() const
{
	if (IsSpecialAnim()) {
		return false;
	}

	auto normalAnim = GetNormalAnimType();

	switch (normalAnim) {
	case NormalAnimType::AbjurationConjuring:
	case NormalAnimType::ConjurationConjuring:
	case NormalAnimType::DivinationConjuring:
	case NormalAnimType::EnchantmentConjuring:
	case NormalAnimType::EvocationConjuring:
	case NormalAnimType::IllusionConjuring:
	case NormalAnimType::NecromancyConjuring:
	case NormalAnimType::TransmutationConjuring:
	case NormalAnimType::WandAbjurationConjuring:
	case NormalAnimType::WandConjurationConjuring:
	case NormalAnimType::WandDivinationConjuring:
	case NormalAnimType::WandEnchantmentConjuring:
	case NormalAnimType::WandEvocationConjuring:
	case NormalAnimType::WandIllusionConjuring:
	case NormalAnimType::WandNecromancyConjuring:
	case NormalAnimType::WandTransmutationConjuring:
		return true;
	default:
		return false;
	}
}

std::string gfx::EncodedAnimId::GetName() const {

	static std::string sUnknown = "<unknown>";

	if (IsWeaponAnim()) {
		return fmt::format("{}_{}_{}",
		                   GetWeaponTypeName(GetWeaponLeftHand()),
		                   GetWeaponTypeName(GetWeaponRightHand()),
		                   GetWeaponAnimName(GetWeaponAnim())
		);
	}

	if (IsBardInstrumentAnim()) {
		return GetBardInstrumentTypeName(GetBardInstrumentType());
	}

	return GetNormalAnimTypeName(GetNormalAnimType());

}

