
#include "infrastructure/meshes.h"

// True if the weapon has to fallback on both hands
static bool IsWeapon2hFallback(gfx::WeaponAnimType weaponType) {
	switch (weaponType) {
	case gfx::WeaponAnimType::Greatsword:
	case gfx::WeaponAnimType::Greataxe:
	case gfx::WeaponAnimType::Greathammer:
	case gfx::WeaponAnimType::Spear:
	case gfx::WeaponAnimType::Staff:
	case gfx::WeaponAnimType::Polearm:
	case gfx::WeaponAnimType::Bow:
	case gfx::WeaponAnimType::Crossbow:
	case gfx::WeaponAnimType::Chain:
	case gfx::WeaponAnimType::TwoHandedFlail:
	case gfx::WeaponAnimType::Shuriken:
	case gfx::WeaponAnimType::Monk:
		return true;
	default:
		return false;
	}
}

static gfx::WeaponAnimType GetWeaponFallback(gfx::WeaponAnimType weaponType) {
	switch (weaponType) {
		// Oddly enough these are not marked as having substitutes, so i wonder
		// if they are actually used...
	case gfx::WeaponAnimType::Mace:
	case gfx::WeaponAnimType::Hammer:
	case gfx::WeaponAnimType::Axe:
	case gfx::WeaponAnimType::Club:
	case gfx::WeaponAnimType::Battleaxe:
		return gfx::WeaponAnimType::Sword;

	case gfx::WeaponAnimType::Greataxe:
	case gfx::WeaponAnimType::Greathammer:
		return gfx::WeaponAnimType::Greatsword;

	case gfx::WeaponAnimType::Shield:
		return gfx::WeaponAnimType::Unarmed;

	case gfx::WeaponAnimType::Flail:
		return gfx::WeaponAnimType::Sword;

	case gfx::WeaponAnimType::TwoHandedFlail:
		return gfx::WeaponAnimType::Polearm;

	case gfx::WeaponAnimType::Shuriken:
	case gfx::WeaponAnimType::Monk:
		return gfx::WeaponAnimType::Unarmed;

	default:
		return weaponType;
	}
}

static gfx::WeaponAnim GetWeaponAnimFallback(gfx::WeaponAnim weaponAnim) {
	switch (weaponAnim) {
	case gfx::WeaponAnim::RightAttack2:
		return gfx::WeaponAnim::RightAttack;
	case gfx::WeaponAnim::RightAttack3:
		return gfx::WeaponAnim::RightAttack2;
	case gfx::WeaponAnim::LeftAttack2:
		return gfx::WeaponAnim::LeftAttack;
	case gfx::WeaponAnim::LeftAttack3:
		return gfx::WeaponAnim::LeftAttack2;
	case gfx::WeaponAnim::Run:
		return gfx::WeaponAnim::Walk;
	case gfx::WeaponAnim::FrontHit:
		return gfx::WeaponAnim::RightAttack;
	case gfx::WeaponAnim::FrontHit2:
		return gfx::WeaponAnim::FrontHit;
	case gfx::WeaponAnim::FrontHit3:
		return gfx::WeaponAnim::FrontHit2;
	case gfx::WeaponAnim::LeftHit:
		return gfx::WeaponAnim::FrontHit;
	case gfx::WeaponAnim::LeftHit2:
		return gfx::WeaponAnim::LeftHit;
	case gfx::WeaponAnim::LeftHit3:
		return gfx::WeaponAnim::LeftHit2;
	case gfx::WeaponAnim::RightHit:
		return gfx::WeaponAnim::FrontHit;
	case gfx::WeaponAnim::RightHit2:
		return gfx::WeaponAnim::RightHit;
	case gfx::WeaponAnim::RightHit3:
		return gfx::WeaponAnim::RightHit2;
	case gfx::WeaponAnim::BackHit:
		return gfx::WeaponAnim::FrontHit;
	case gfx::WeaponAnim::BackHit2:
		return gfx::WeaponAnim::BackHit;
	case gfx::WeaponAnim::BackHit3:
		return gfx::WeaponAnim::BackHit2;
	case gfx::WeaponAnim::RightCriticalSwing:
		return gfx::WeaponAnim::RightAttack;
	case gfx::WeaponAnim::LeftCriticalSwing:
		return gfx::WeaponAnim::LeftAttack;
	case gfx::WeaponAnim::Fidget2:
		return gfx::WeaponAnim::Fidget;
	case gfx::WeaponAnim::Fidget3:
		return gfx::WeaponAnim::Fidget2;
	case gfx::WeaponAnim::Sneak:
		return gfx::WeaponAnim::Walk;
	case gfx::WeaponAnim::Panic:
		return gfx::WeaponAnim::Fidget;
	case gfx::WeaponAnim::RightCombatStart:
	case gfx::WeaponAnim::LeftCombatStart:
		return gfx::WeaponAnim::CombatFidget;
	case gfx::WeaponAnim::CombatIdle:
		return gfx::WeaponAnim::Idle;
	case gfx::WeaponAnim::CombatFidget:
		return gfx::WeaponAnim::Fidget;
	case gfx::WeaponAnim::Special1:
	case gfx::WeaponAnim::Special2:
	case gfx::WeaponAnim::Special3:
	case gfx::WeaponAnim::FrontDodge:
		return gfx::WeaponAnim::RightAttack;
	case gfx::WeaponAnim::RightDodge:
	case gfx::WeaponAnim::LeftDodge:
	case gfx::WeaponAnim::BackDodge:
		return gfx::WeaponAnim::FrontDodge;
	case gfx::WeaponAnim::RightThrow:
		return gfx::WeaponAnim::RightAttack;
	case gfx::WeaponAnim::LeftThrow:
		return gfx::WeaponAnim::LeftAttack;
	case gfx::WeaponAnim::LeftSnatch:
		return gfx::WeaponAnim::RightAttack;
	case gfx::WeaponAnim::RightSnatch:
		return gfx::WeaponAnim::LeftAttack;
	default:
		return gfx::WeaponAnim::None;
	}
}

static bool GetNormalAnimFallback(gfx::NormalAnimType anim, gfx::EncodedAnimId& animId) {
	switch (anim) {
	case gfx::NormalAnimType::ProneIdle:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::DeadIdle);
		return true;
	case gfx::NormalAnimType::ProneFidget:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ProneIdle);
		return true;
	case gfx::NormalAnimType::Getup:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::CombatFidget);
		return true;
	case gfx::NormalAnimType::Magichands:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::RightAttack);
		return true;
	case gfx::NormalAnimType::Picklock:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Magichands);
		return true;
	case gfx::NormalAnimType::PicklockConcentrated:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Picklock);
		return true;
	case gfx::NormalAnimType::Examine:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Magichands);
		return true;
	case gfx::NormalAnimType::Throw:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::Death:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::Death2:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Death);
		return true;
	case gfx::NormalAnimType::Death3:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Death2);
		return true;
	case gfx::NormalAnimType::DeadIdle:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::DeadFidget:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::DeadIdle);
		return true;
	case gfx::NormalAnimType::DeathProneIdle:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::DeadIdle);
		return true;
	case gfx::NormalAnimType::DeathProneFidget:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::DeathProneIdle);
		return true;
	case gfx::NormalAnimType::AbjurationCasting:
	case gfx::NormalAnimType::AbjurationConjuring:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::RightAttack);
		return true;
	case gfx::NormalAnimType::ConjurationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::ConjurationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::DivinationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::DivinationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::EnchantmentCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::EnchantmentConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::EvocationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::EvocationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::IllusionCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::IllusionConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::NecromancyCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::NecromancyConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::TransmutationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::TransmutationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::Conceal:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::RightAttack);
		return true;
	case gfx::NormalAnimType::ConcealIdle:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::Idle);
		return true;
	case gfx::NormalAnimType::Unconceal:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Getup);
		return true;
	case gfx::NormalAnimType::ItemFidget:
	case gfx::NormalAnimType::Open:
	case gfx::NormalAnimType::Close:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::SkillAnimalEmpathy:
	case gfx::NormalAnimType::SkillDisableDevice:
	case gfx::NormalAnimType::SkillHeal:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Magichands);
		return true;
	case gfx::NormalAnimType::SkillHealConcentrated:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::SkillHeal);
		return true;
	case gfx::NormalAnimType::SkillHide:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::SkillHideIdle:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::Idle);
		return true;
	case gfx::NormalAnimType::SkillHideFidget:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::CombatFidget);
		return true;
	case gfx::NormalAnimType::SkillUnhide:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::SkillPickpocket:
	case gfx::NormalAnimType::SkillSearch:
	case gfx::NormalAnimType::SkillSpot:
	case gfx::NormalAnimType::FeatTrack:
	case gfx::NormalAnimType::Trip:
	case gfx::NormalAnimType::Bullrush:
	case gfx::NormalAnimType::Flurry:
	case gfx::NormalAnimType::Kistrike:
	case gfx::NormalAnimType::Tumble:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::Magichands);
		return true;
	case gfx::NormalAnimType::Special1:
	case gfx::NormalAnimType::Special2:
	case gfx::NormalAnimType::Special3:
	case gfx::NormalAnimType::Special4:
	case gfx::NormalAnimType::Throw2:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	case gfx::NormalAnimType::WandAbjurationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandAbjurationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::AbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandConjurationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandConjurationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandDivinationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandDivinationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandEnchantmentCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandEnchantmentConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandEvocationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandEvocationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandIllusionCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandIllusionConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandNecromancyCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandNecromancyConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::WandTransmutationCasting:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationCasting);
		return true;
	case gfx::NormalAnimType::WandTransmutationConjuring:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::WandAbjurationConjuring);
		return true;
	case gfx::NormalAnimType::SkillBarbarianRage:
		animId = gfx::EncodedAnimId(gfx::WeaponAnim::RightAttack);
		return true;
	case gfx::NormalAnimType::OpenIdle:
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
		return true;
	default:
		return false;
	}

}

static bool GetFallbackAnim(gfx::EncodedAnimId& animId) {

	if (animId.IsBardInstrumentAnim()) {
		// There are no fallback animations for bard instruments.
		// Apparently there was a table, but no instrument had a fallback in there
		return false;
	}

	if (animId.IsWeaponAnim()) {

		auto leftHand = animId.GetWeaponLeftHand();
		auto rightHand = animId.GetWeaponRightHand();

		// Possibly the weapons in either hand need to fallback together
		if (IsWeapon2hFallback(leftHand) || IsWeapon2hFallback(rightHand)) {
			auto leftHandFallback = GetWeaponFallback(leftHand);
			auto rightHandFallback = GetWeaponFallback(rightHand);

			if (leftHandFallback != leftHand && rightHandFallback != rightHand) {
				animId = gfx::EncodedAnimId(animId.GetWeaponAnim(), leftHandFallback, rightHandFallback);
				return true;
			}
		}
		else {
			auto rightHandFallback = GetWeaponFallback(rightHand);
			if (rightHandFallback != rightHand) {
				animId = gfx::EncodedAnimId(animId.GetWeaponAnim(), leftHand, rightHandFallback);
				return true;
			}

			auto leftHandFallback = GetWeaponFallback(leftHand);
			if (leftHandFallback != leftHand) {
				animId = gfx::EncodedAnimId(animId.GetWeaponAnim(), leftHandFallback, rightHand);
				return true;
			}
		}

		auto weaponAnimFallback = GetWeaponAnimFallback(animId.GetWeaponAnim());
		if (weaponAnimFallback == gfx::WeaponAnim::None) {
			return false;
		}

		animId = gfx::EncodedAnimId(weaponAnimFallback, leftHand, rightHand);
		return true;
	}

	// Normal animations can fall back to weapon animations, so it's not just a lookup table here
	return GetNormalAnimFallback(animId.GetNormalAnimType(), animId);
}

bool gfx::EncodedAnimId::ToFallback() {
	return GetFallbackAnim(*this);
}
