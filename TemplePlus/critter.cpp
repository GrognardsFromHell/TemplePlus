#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"
#include "inventory.h"
#include "float_line.h"
#include "condition.h"
#include "d20.h"
#include "particles.h"
#include "config/config.h"
#include "python/python_integration_obj.h"
#include "python/python_object.h"
#include "tig/tig_startup.h"
#include "util/fixes.h"
#include "gamesystems/particlesystems.h"
#include <graphics/mdfmaterials.h>
#include <infrastructure/meshes.h>
#include <infrastructure/vfs.h>
#include <temple/meshes.h>
#include <infrastructure/mesparser.h>
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "ai.h"
#include "party.h"
#include "ui\ui.h"
#include "temple_functions.h"
#include "combat.h"

static struct CritterAddresses : temple::AddressTable {

	uint32_t (__cdecl *HasMet)(objHndl critter, objHndl otherCritter);
	uint32_t (__cdecl *AddFollower)(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower);
	uint32_t (__cdecl *RemoveFollower)(objHndl npc, int unkFlag);
	objHndl (__cdecl *GetLeader)(objHndl critter);
	objHndl(__cdecl *GetLeaderRecursive)(objHndl critter);
	int (__cdecl *HasLineOfSight)(objHndl critter, objHndl target);
	void (__cdecl *Attack)(objHndl target, objHndl attacker, int n1, int n2);
	uint32_t (__cdecl *IsFriendly)(objHndl pc, objHndl npc);
	int (__cdecl *SoundmapCritter)(objHndl critter, int id);
	void (__cdecl *KillByEffect)(objHndl critter, objHndl killer);
	void (__cdecl *Kill)(objHndl critter, objHndl killer);

	void (__cdecl *GetStandpoint)(objHndl, StandPointType, StandPoint *);
	void (__cdecl *SetStandpoint)(objHndl, StandPointType, const StandPoint *);
	void (__cdecl *SetSubdualDamage)(objHndl critter, int damage);

	void (__cdecl *BalorDeath)(objHndl critter);
	void (__cdecl *SetConcealed)(objHndl critter, int concealed);

	uint32_t(__cdecl *Resurrect)(objHndl critter, ResurrectType type, int unk);

	uint32_t(__cdecl *IsDeadOrUnconscious)(objHndl critter);

	objHndl (__cdecl *GiveItem)(objHndl critter, int protoId);

	void(__cdecl *TakeMoney)(objHndl critter, int platinum, int gold, int silver, int copper);
	void(__cdecl *GiveMoney)(objHndl critter, int platinum, int gold, int silver, int copper);

	int (*GetWeaponAnim)(objHndl critter, objHndl primaryWeapon, objHndl secondary, gfx::WeaponAnim animId);

	void(__cdecl*GetCritterVoiceLine)(objHndl obj, objHndl fellow, char* str, int* soundId);
	int (__cdecl*PlayCritterVoiceLine)(objHndl obj, objHndl fellow, char* str, int soundId);
	CritterAddresses() {
		rebase(PlayCritterVoiceLine, 0x10036120);
		rebase(GetCritterVoiceLine, 0x100373C0);
		rebase(HasMet, 0x10053CD0);
		rebase(AddFollower, 0x100812F0);
		rebase(RemoveFollower, 0x10080FD0);
		rebase(GetLeader, 0x1007EA70);
		rebase(GetLeaderRecursive, 0x10080430);
		rebase(HasLineOfSight, 0x10059470);
		rebase(Attack, 0x1005E8D0);
		rebase(IsFriendly, 0x10080E00);
		rebase(SoundmapCritter, 0x1006DEF0);
		rebase(Kill, 0x100B8A00);
		rebase(KillByEffect, 0x100B9460);
		rebase(GetStandpoint, 0x100BA890);
		rebase(SetStandpoint, 0x100BA8F0);
		rebase(SetSubdualDamage, 0x1001DB10);
		rebase(BalorDeath, 0x100F66F0);
		rebase(SetConcealed, 0x10080670);
		rebase(Resurrect, 0x100809C0);
		rebase(IsDeadOrUnconscious, 0x100803E0);
		rebase(GiveItem, 0x1006CC30);

		rebase(GiveMoney, 0x1007F960);
		rebase(TakeMoney, 0x1007FA40);
		rebase(GetWeaponAnim, 0x10020B60);
	}

	
} addresses;

class CritterReplacements : public TempleFix
{
	public: const char* name() override { 
		return "Critter System" "Function Replacements";
	} 
	void ShowExactHpForNPCs();

	void apply() override 
	{
		logger->info("Replacing Critter System functions");
		replaceFunction(0x10062720, _isCritterCombatModeActive); 
		ShowExactHpForNPCs();
	}
} critterReplacements;

void CritterReplacements::ShowExactHpForNPCs()
{
	if (config.showExactHPforNPCs)
	{
		char displayExactHpForNPCsChar = 0;
		char * displayExactHpForNPCsBuffer = &displayExactHpForNPCsChar;
		write(0x1012441B + 2, displayExactHpForNPCsBuffer, 1);
	}
}

#pragma region Critter System Implementation

struct LegacyCritterSystem critterSys;

uint32_t LegacyCritterSystem::isCritterCombatModeActive(objHndl objHnd)
{
	return (objects.getInt32(objHnd, obj_f_critter_flags) & OCF_COMBAT_MODE_ACTIVE) != 0;
}

LegacyCritterSystem::LegacyCritterSystem()
{
}

uint32_t LegacyCritterSystem::IsPC(objHndl objHnd)
{
	return objects.getInt32(objHnd, obj_f_type) == obj_t_pc;
}
#pragma endregion

int LegacyCritterSystem::GetLootBehaviour(objHndl npc) {
	return objects.getInt32(npc, obj_f_npc_pad_i_3) & 0xF;
}

void LegacyCritterSystem::SetLootBehaviour(objHndl npc, int behaviour) {
	objects.setInt32(npc, obj_f_npc_pad_i_3, behaviour & 0xF);
}

uint32_t LegacyCritterSystem::HasMet(objHndl critter, objHndl otherCritter) {
	return addresses.HasMet(critter, otherCritter);
}

uint32_t LegacyCritterSystem::AddFollower(objHndl npc, objHndl pc, int unkFlag, bool asAiFollower) {
	return addresses.AddFollower(npc, pc, unkFlag, asAiFollower);
}

uint32_t LegacyCritterSystem::RemoveFollower(objHndl npc, int forceFollower) {
	return addresses.RemoveFollower(npc, forceFollower);
}

objHndl LegacyCritterSystem::GetLeader(objHndl critter) {
	return addresses.GetLeader(critter);
}

objHndl LegacyCritterSystem::GetLeaderRecursive(objHndl critter)
{
	return addresses.GetLeaderRecursive(critter);
}

int LegacyCritterSystem::HasLineOfSight(objHndl critter, objHndl target) {
	return addresses.HasLineOfSight(critter, target);
}

objHndl LegacyCritterSystem::GetWornItem(objHndl handle, EquipSlot slot) {
	return inventory.ItemWornAt(handle, (uint32_t) slot);
}

void LegacyCritterSystem::Attack(objHndl target, objHndl attacker, int n1, int flags) {
	// flags:
	// 0x1 - if 1, will adjust reaction by -10 regardless of the hostility threshold
	// 0x2 - 
	// 0x4

	if (!target || !attacker)
		return;
	if (attacker == target)
		return;
	auto attackerObj = gameSystems->GetObj().GetObject(attacker);
	auto tgtObj = gameSystems->GetObj().GetObject(target);
	if (objects.IsCritter(attacker))
	{
		auto critFlags = attackerObj->GetInt32(obj_f_critter_flags2);
		if (critFlags & OCF2_NIGH_INVULNERABLE)
			return;
		if (critterSys.IsDeadNullDestroyed(attacker)) return;

	}
	auto objFlags = attackerObj->GetFlags();
	if (objFlags& (OF_OFF | OF_DONTDRAW | OF_INVULNERABLE))
		return;
	auto attackerName = attackerObj->GetInt32(obj_f_name);

	if (attackerName == 6719) { // who is this hardcoded mofo???
		return;
	}

	if (combatSys.IsBrawlInProgress())
	{
			return;
	}
		

	if (attackerObj->IsPC() && tgtObj->IsPC())
		return;


	if (tgtObj->IsPC() && attackerObj->IsNPC() || tgtObj->IsNPC()){
		if (flags & 4)	{
			auto tgtLeader = critterSys.GetLeader(target);
			if (tgtLeader)	{
				auto ai_1005E2B0 = temple::GetRef<void(__cdecl)(objHndl, objHndl, objHndl, int, int, int)>(0x1005E2B0);
				ai_1005E2B0(target, tgtLeader, attacker, 1, 0, 1);
			}
		} 
		else
		{
			auto flag2 = flags & 2;
			if (!flag2)	{
				// do ai_1005E2B0 for the critter followers
				auto ai_1005E830 = temple::GetRef<void(__cdecl)(objHndl, objHndl, int, int)>(0x1005E830);
				ai_1005E830(target, attacker, 1, flags &1);
			}
			if (attackerObj->IsCritter()){

				// UnConceal
				auto attackerLeader = critterSys.GetLeaderRecursive(attacker);
				auto toUnconceal = attackerLeader;
				if (!attackerLeader){
					toUnconceal = attacker;
					attackerLeader = attacker;
				}
				if (critterSys.IsConcealed(attackerLeader))	{
					critterSys.SetConcealedWithFollowers(toUnconceal, 0);
				}

				// unset ONF_KOS_OVERRIDE
				if (attackerObj->IsNPC()){
					auto npcFlags = attackerObj->GetNPCFlags();
					npcFlags &= ~(ONF_KOS_OVERRIDE);
					attackerObj->SetNPCFlags(npcFlags);
				}

				// Unknown
				if (!flag2)	{
					if (attackerObj->IsPC())	{
						auto sub_10057790 = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x10057790);
						sub_10057790(attacker, target);
					}
					if (!(flags&1))	{
						auto sub_1005D890 = temple::GetRef<void(__cdecl)(objHndl, objHndl, int)>(0x1005D890);
						sub_1005D890(attacker, target, n1);
					}
				}

				if (attackerObj->IsNPC()){

					auto leader = critterSys.GetLeader(attacker);

					// set "who hit me last"
					if ( !(flags&1) && target != leader){
						attackerObj->SetObjHndl(obj_f_npc_who_hit_me_last, target);
					}

					auto aiShitlistAddWithFollowers = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x1005CCA0);
					aiShitlistAddWithFollowers(attacker, target);
					if (tgtObj->IsNPC()) {
						aiShitlistAddWithFollowers(target, attacker);
					}
					else if (tgtObj->IsPC()){
					
						auto aiPar = aiSys.GetAiParams(attacker);
						
						if (leader == target)	{
							auto npcRefuseFollowingCheck = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10058A30);
							if (npcRefuseFollowingCheck(attacker, target) && critterSys.RemoveFollower(attacker, 0))
							{
								ui.UpdatePartyUi();
								auto npcFlags = attackerObj->GetInt32(obj_f_npc_flags);
								attackerObj->SetNPCFlags(npcFlags | ONF_JILTED);
							}
							else if (templeFuncs.RNG(1,3) == 1 && !critterSys.IsDeadOrUnconscious(attacker))
							{
								auto uiDlgSoundPlayer = temple::GetRef<void(__cdecl*)(objHndl, objHndl, char*, int)>(0x10AA73B0);
								if (uiDlgSoundPlayer)
								{
									char ffText[1000]; int soundId;
									auto getFriendlyFireVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int*)>(0x10037450);
									getFriendlyFireVoiceLine(attacker, target, ffText, &soundId);
									uiDlgSoundPlayer(attacker, target, ffText, soundId);
								}
							}
						} 
						else if (flags &1 )
						{
							objects.AdjustReaction(attacker, target, -10);
						} else
						{
							auto curReaction = objects.GetReaction(attacker, target);
							if (curReaction > aiPar.hostilityThreshold)
								objects.AdjustReaction(attacker, target, aiPar.hostilityThreshold - curReaction);
						}
					}

					if ( !(flags&1) || !critterSys.AllegianceShared(target, attacker) && target != leader)
					{
						aiSys.FightStatusProcess(attacker, target);
					}
				}
			}
		}
		return;
	}

	if (attackerObj->IsNPC()){
		aiSys.UpdateAiFlags(attacker, AIFS_FIGHTING, target, nullptr);
		return;
	}

	//addresses.Attack(target, attacker, n1, flags);
}

uint32_t LegacyCritterSystem::IsFriendly(objHndl pc, objHndl npc) {
	return addresses.IsFriendly(pc, npc);
}

BOOL LegacyCritterSystem::AllegianceShared(objHndl obj, objHndl obj2)
{
	auto allegShared = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10080A70);
	return allegShared(obj, obj2);
}

int LegacyCritterSystem::SoundmapCritter(objHndl critter, int id) {
	return addresses.SoundmapCritter(critter, id);
}

void LegacyCritterSystem::Kill(objHndl critter, objHndl killer) {
	return addresses.Kill(critter, killer);
}

void LegacyCritterSystem::KillByEffect(objHndl critter, objHndl killer) {
	return addresses.KillByEffect(critter, killer);
}

static_assert(temple::validate_size<StandPoint, 0x20>::value, "Invalid size");

void LegacyCritterSystem::SetStandPoint(objHndl critter, StandPointType type, const StandPoint& standpoint) {
	addresses.SetStandpoint(critter, type, &standpoint);
}

StandPoint LegacyCritterSystem::GetStandPoint(objHndl critter, StandPointType type) {
	StandPoint result;
	addresses.GetStandpoint(critter, type, &result);
	return result;
}

void LegacyCritterSystem::SetSubdualDamage(objHndl critter, int damage) {
	addresses.SetSubdualDamage(critter, damage);
}

void LegacyCritterSystem::AwardXp(objHndl critter, int xpAwarded) {
	auto xp = objects.getInt32(critter, obj_f_critter_experience);
	xp += xpAwarded;
	d20Sys.d20SendSignal(critter, DK_SIG_Experience_Awarded, xp, 0);
	objects.setInt32(critter, obj_f_critter_experience, xp);
}

void LegacyCritterSystem::BalorDeath(objHndl npc) {
	addresses.BalorDeath(npc);
}

void LegacyCritterSystem::SetConcealed(objHndl critter, int concealed) {
	addresses.SetConcealed(critter, concealed);
}

void LegacyCritterSystem::SetConcealedWithFollowers(objHndl obj, int newState)
{
	auto setConcWithFollowers = temple::GetRef<void(__cdecl)(objHndl, int)>(0x10080670);
	setConcWithFollowers(obj, newState);
}

uint32_t LegacyCritterSystem::Resurrect(objHndl critter, ResurrectType type, int unk) {
	return addresses.Resurrect(critter, type, unk);
}

uint32_t LegacyCritterSystem::Dominate(objHndl critter, objHndl caster) {

	
	floatSys.FloatSpellLine(critter, 20018, FloatLineColor::Red);  // Float a "charmed!" line above the critter

	vector<int> args(3);

	args[0] = gameSystems->GetParticleSys().CreateAtObj("sp-Dominate Person", critter);
	args[1] = (caster >> 32) & 0xFFFFFFFF;
	args[2] = caster & 0xFFFFFFFF;

	auto cond = conds.GetByName("Dominate");
	return conds.AddTo(critter, cond, args);
}

bool LegacyCritterSystem::IsDeadNullDestroyed(objHndl critter)
{
	if (!critter) {
		return true;
	}

	auto flags = objects.GetFlags(critter);
	if (flags & OF_DESTROYED) {
		return true;
	}

	return objects.GetHPCur(critter) <= -10;
}

bool LegacyCritterSystem::IsDeadOrUnconscious(objHndl critter) {
	if (IsDeadNullDestroyed(critter)) {
		return true;
	}
	return objects.d20.d20Query(critter, DK_QUE_Unconscious) != 0;
}

bool LegacyCritterSystem::IsProne(objHndl critter)
{
	return objects.d20.d20Query(critter, DK_QUE_Prone) != 0;
}

CritterFlag LegacyCritterSystem::GetCritterFlags(objHndl critter)
{
	return (CritterFlag) objects.getInt32(critter, obj_f_critter_flags);
}

bool LegacyCritterSystem::IsMovingSilently(objHndl critter)
{
	auto flags = GetCritterFlags(critter);
	return (flags & OCF_MOVING_SILENTLY) == OCF_MOVING_SILENTLY;
}

bool LegacyCritterSystem::IsCombatModeActive(objHndl critter)
{
	auto flags = GetCritterFlags(critter);
	return (flags & OCF_COMBAT_MODE_ACTIVE) == OCF_COMBAT_MODE_ACTIVE;
}

bool LegacyCritterSystem::IsConcealed(objHndl critter)
{
	auto flags = GetCritterFlags(critter);
	return (flags & OCF_IS_CONCEALED) == OCF_IS_CONCEALED;
}

int LegacyCritterSystem::GetPortraitId(objHndl leader) {
	return objects.getInt32(leader, obj_f_critter_portrait);
}

int LegacyCritterSystem::GetLevel(objHndl critter) {
	return objects.StatLevelGet(critter, stat_level);
}

Race LegacyCritterSystem::GetRace(objHndl critter) {
	return (Race)objects.StatLevelGet(critter, stat_race);
}

Gender LegacyCritterSystem::GetGender(objHndl critter) {
	return (Gender)objects.StatLevelGet(critter, stat_gender);
}

std::string LegacyCritterSystem::GetHairStylePreviewTexture(HairStyle style)
{
	return GetHairStyleFile(style, "tga");
}

std::string LegacyCritterSystem::GetHairStyleModel(HairStyle style)
{
	return GetHairStyleFile(style, "skm");
}

gfx::EncodedAnimId LegacyCritterSystem::GetAnimId(objHndl critter, gfx::WeaponAnim anim)
{
	auto weaponPrim = GetWornItem(critter, EquipSlot::WeaponPrimary);
	auto weaponSec = GetWornItem(critter, EquipSlot::WeaponSecondary);
	if (!weaponSec) {
		weaponSec = GetWornItem(critter, EquipSlot::Shield);
	}

	int rawAnimId = addresses.GetWeaponAnim(critter, weaponPrim, weaponSec, anim);
	return gfx::EncodedAnimId(rawAnimId);
}

int LegacyCritterSystem::GetModelRaceOffset(objHndl obj)
{
	// Meshes above 1000 are monsters, they dont get a creature type
	auto meshId = objects.getInt32(obj, obj_f_base_mesh);
	if (meshId >= 1000) {
		return -1;
	}

	auto race = GetRace(obj);
	auto gender = GetGender(obj);
	bool isMale = (gender == Gender::Male);

	/*
		The following table comes from materials.mes, where
		the offsets into the materials and addmesh table are listed.
	*/
	int result;
	switch (race)
	{
	case race_human:
		result = (isMale ? 0 : 1);
		break;
	case race_elf:
		result = (isMale ? 2 : 3);
		break;
	case race_halforc:
		result = (isMale ? 4 : 5);
		break;
	case race_dwarf:
		result = (isMale ? 6 : 7);
		break;
	case race_gnome:
		result = (isMale ? 8 : 9);
		break;
	case race_halfelf:
		result = (isMale ? 10 : 11);
		break;
	case race_halfling:
		result = (isMale ? 12 : 13);
		break;
	default:
		result = 0;
		break;
	}

	return result;
}

void LegacyCritterSystem::UpdateAddMeshes(objHndl obj)
{

	auto raceOffset = GetModelRaceOffset(obj);

	// For monsters, normal addmeshes are not processed
	if (raceOffset == -1) {
		return;
	}

	auto model(objects.GetAnimHandle(obj));

	if (!model) {
		return;
	}

	// Reset all existing add meshes
	model->ClearAddMeshes();

	// Do not process add meshes if the user is polymorphed
	if (objects.d20.d20Query(obj, DK_QUE_Polymorphed)) {
		return;
	}

	// Adjust the hair style size based on the worn helmet
	auto helmet = GetWornItem(obj, EquipSlot::Helmet);

	// This seems to be the helmet type (0 = small, 2 = large)
	int helmetType = 0;
	if (helmet) {
		auto wearFlags = objects.GetItemWearFlags(helmet);
		if (wearFlags & OIF_WEAR_HELMET) {
			helmetType = (objects.getInt32(helmet, obj_f_armor_flags) >> 2) & 3;
		}
	}

	auto robes = GetWornItem(obj, EquipSlot::Robes);

	for (int slotId = 0; slotId < (int) EquipSlot::Count; ++slotId) {
		auto slot = (EquipSlot)slotId;
		auto item = GetWornItem(obj, slot);
		if (!item) {
			continue;
		}

		// Armor is hidden by a robe
		if (robes && slot == EquipSlot::Armor) {
			continue;
		}

		// Addmesh / Material index of the item
		auto matIdx = objects.getInt32(item, obj_f_item_material_slot);
		if (matIdx == -1) {
			continue; // No assigned addmesh or material replacement
		}
		
		auto& addMeshes(GetAddMeshes(matIdx, raceOffset));

		if (!addMeshes.empty()) {
			model->AddAddMesh(addMeshes[0]);
		}

		// Only add the helmet part if there's no real helmet
		if (helmetType == 0 && addMeshes.size() >= 2) {
			model->AddAddMesh(addMeshes[1]);
			helmetType = 2; // The addmesh counts as a large helmet
		}

	}

	// Add the hair model
	auto packedHairStyle = objects.getInt32(obj, obj_f_critter_hair_style);
	HairStyle hairStyle{ packedHairStyle };

	// Modify the hair style size based on the used helmet, 
	// since it might cover it
	if (helmetType == 2) {
		// Large helmets remove most of the hair
		hairStyle.size = HairStyleSize::None;
	} else if (helmetType == 1) {
		// Smaller helmets only part of it
		hairStyle.size = HairStyleSize::Small;
	} else {
		hairStyle.size = HairStyleSize::Big;
	}

	auto hairModel{GetHairStyleModel(hairStyle)};
	if (!hairModel.empty()) {
		model->AddAddMesh(hairModel);
	}
}

const std::vector<std::string>& LegacyCritterSystem::GetAddMeshes(int matIdx, int raceOffset) {

	static std::vector<std::string> sEmptyAddMeshes;
	
	// Lazily load the addmesh rules
	if (mAddMeshes.empty()) {
		auto mapping(MesFile::ParseFile("rules\\addmesh.mes"));
		for (auto &entry : mapping) {
			mAddMeshes[entry.first] = split(entry.second, ';', true);
		}
	}

	auto it = mAddMeshes.find(matIdx + raceOffset);
	if (it != mAddMeshes.end()) {
		return it->second;
	}

	return sEmptyAddMeshes;
}

void LegacyCritterSystem::ApplyReplacementMaterial(gfx::AnimatedModelPtr model, int mesId)
{
	auto& replacementSet = tig->GetMdfFactory().GetReplacementSet(mesId);
	for (auto& entry : replacementSet) {
		model->AddReplacementMaterial(entry.first, entry.second);
	}
}

static const char *GetHairStyleRaceName(HairStyleRace race) {
	switch (race) {
	case HairStyleRace::Human:
		return "hu";
	case HairStyleRace::Dwarf:
		return "dw";
	case HairStyleRace::Elf:
		return "el";
	case HairStyleRace::Gnome:
		return "gn";
	case HairStyleRace::HalfElf:
		return "he";
	case HairStyleRace::HalfOrc:
		return "ho";
	case HairStyleRace::Halfling:
		return "hl";
	default:
		throw TempleException("Unsupported hair style race: {}", (int)race);
	}
}

static const char *GetHairStyleSizeName(HairStyleSize size) {
	switch (size) {
	case HairStyleSize::Big:
		return "big";
	case HairStyleSize::Small:
		return "small";
	case HairStyleSize::None:
		return "none";
	default:
		throw TempleException("Unsupported hair style size: {}", (int)size);
	}
}

// Gets the race that should be fallen back to if there's 
// no model for the given race. If it returns race again
// there's no more fallback
static HairStyleRace GetHairStyleFallbackRace(HairStyleRace race) {
	switch (race) {
	case HairStyleRace::Human:
		return HairStyleRace::Human;
	case HairStyleRace::Dwarf:
		return HairStyleRace::Dwarf;
	case HairStyleRace::Elf:
		return HairStyleRace::Human;
	case HairStyleRace::Gnome:
		return HairStyleRace::Human;
	case HairStyleRace::HalfElf:
		return HairStyleRace::Human;
	case HairStyleRace::HalfOrc:
		return HairStyleRace::HalfOrc;
	case HairStyleRace::Halfling:
		return HairStyleRace::Human;
	default:
		throw TempleException("Unsupported hair style race: {}", (int)race);
	}
}

static HairStyleSize GetHairStyleFallbackSize(HairStyleSize size) {
	switch (size) {
	case HairStyleSize::Big:
		return HairStyleSize::Small;
	case HairStyleSize::Small:
		return HairStyleSize::None;
	case HairStyleSize::None:
		return HairStyleSize::None;
	default:
		throw TempleException("Unsupported hair style size: {}", (int)size);
	}
}

std::string LegacyCritterSystem::GetHairStyleFile(HairStyle style, const char * extension)
{
	const char *genderShortName;
	const char *genderDir;
	if (style.gender == Gender::Male) {
		genderShortName = "m";
		genderDir = "male";
	} else {
		genderShortName = "f";
		genderDir = "female";
	}
	
	// These will be modified if a fallback is needed, so copy them here
	auto race = style.race;
	auto size = style.size;
	auto styleNr = style.style;

	while (true)
	{
		while (true)
		{
			while (true)
			{
				auto filename(fmt::format("art\\meshes\\hair\\{}\\s{}\\{}_{}_s{}_c{}_{}.{}",
					genderDir,
					style.style,
					GetHairStyleRaceName(race),
					genderShortName,
					style.style,
					style.color,
					GetHairStyleSizeName(size),
					extension
					));
				
				if (vfs->FileExists(filename))
					return filename;

				// Fall back to a compatible race, if no model for this 
				// race exists
				auto fallbackRace = GetHairStyleFallbackRace(race);
				if (fallbackRace == race) {
					break;
				}
				race = fallbackRace;
			}

			// Fall back to a smaller size of the model
			auto fallbackSize = GetHairStyleFallbackSize(size);
			if (fallbackSize == size) {
				break;
			}
			size = fallbackSize;
		}

		// Fall back to style 5 if no other options exist
		if (styleNr == 5)
			break;
		styleNr = 5;
	}

	return std::string();
}

void LegacyCritterSystem::UpdateModelEquipment(objHndl obj)
{

	UpdateAddMeshes(obj);
	auto raceOffset = GetModelRaceOffset(obj);
	if (raceOffset == -1) {
		return;
	}

	auto model = objects.GetAnimHandle(obj);
	if (!model) {
		return; // No animation really present
	}

	// This is a bit shit but since AAS will just splice the
	// add meshes into the list of model parts, 
	// we have to reset the render buffers
	gameSystems->GetAAS().InvalidateBuffers(model->GetHandle());

	// Apply the naked replacement materials for
	// equipment slots that support them
	ApplyReplacementMaterial(model, 0 + raceOffset); // Helmet
	ApplyReplacementMaterial(model, 100 + raceOffset); // Cloak
	ApplyReplacementMaterial(model, 200 + raceOffset); // Gloves
	ApplyReplacementMaterial(model, 500 + raceOffset); // Armor
	ApplyReplacementMaterial(model, 800 + raceOffset); // Boots
	
	// Now apply it for the actual equipment
	for (uint32_t slotId = 0; slotId < (uint32_t)EquipSlot::Count; ++slotId) {
		auto slot = (EquipSlot) slotId;
		auto item = GetWornItem(obj, slot);
		if (item) {
			auto materialSlot = objects.getInt32(item, obj_f_item_material_slot);
			if (materialSlot != -1) {
				ApplyReplacementMaterial(model, materialSlot + raceOffset);
			}
		}
	}
}

void LegacyCritterSystem::AddNpcAddMeshes(objHndl obj)
{

	auto id = objects.getInt32(obj, obj_f_npc_add_mesh);
	auto model = objects.GetAnimHandle(obj);

	for (auto &addMesh : GetAddMeshes(id, 0)) {
		model->AddAddMesh(addMesh);
	}

}

objHndl LegacyCritterSystem::GiveItem(objHndl critter, int protoId) {
	return addresses.GiveItem(critter, protoId);
}

void LegacyCritterSystem::TakeMoney(objHndl critter, int platinum, int gold, int silver, int copper)
{
	addresses.TakeMoney(critter, platinum, gold, silver, copper);
}

void LegacyCritterSystem::GiveMoney(objHndl critter, int platinum, int gold, int silver, int copper)
{
	addresses.GiveMoney(critter, platinum, gold, silver, copper);
}

int LegacyCritterSystem::NumOffhandExtraAttacks(objHndl critter)
{
	if (feats.HasFeatCount(critter, FEAT_GREATER_TWO_WEAPON_FIGHTING)
		|| feats.HasFeatCountByClass(critter, FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, (Stat)0, 0))
	{
		return 3;
	}

	if (feats.HasFeatCount(critter, FEAT_IMPROVED_TWO_WEAPON_FIGHTING)
		|| feats.HasFeatCountByClass(critter, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, (Stat)0,0))
		return 2;

	return 1;
}

int LegacyCritterSystem::IsWearingLightOrNoArmor(objHndl critter)
{
	objHndl armor = inventory.ItemWornAt(critter, 5);
	if (armor)
	{
		int armorFlags = objects.getInt32(armor, obj_f_armor_flags);
		if (inventory.GetArmorType(armorFlags) != ARMOR_TYPE_NONE	&& inventory.GetArmorType(armorFlags) != ARMOR_TYPE_LIGHT)
		{
			return 0;
		}
	}
	return 1;
}

bool LegacyCritterSystem::IsLootableCorpse(objHndl critter)
{	
	if (!objects.IsCritter(critter)) {
		return false;
	}

	if (!critterSys.IsDeadNullDestroyed(critter)) {
		return false; // It's still alive
	}

	// Find any item in the critters inventory that would be considered lootable
	auto invenCount = objects.getInt32(critter, obj_f_critter_inventory_num);
	for (size_t i = 0; i < invenCount; ++i) {
		auto item = objects.getArrayFieldObj(critter, obj_f_critter_inventory_list_idx, i);
		auto invLocation = objects.GetItemInventoryLocation(item);
		if (invLocation >= 200 && invLocation <= 216) {
			continue; // Currently equipped on the corpse
		}

		auto itemFlags = objects.GetItemFlags(item);
		if (itemFlags & OIF_NO_LOOT) {
			continue; // Flagged as unlootable
		}

		return true; // Found an item that is lootable
	}

	return false;
}

bool LegacyCritterSystem::CanBarbarianRage(objHndl obj)
{
	if (objects.StatLevelGet(obj, stat_level_barbarian) <= 0)
		return false;
	
	auto isRagedAlready = d20Sys.d20Query(obj, DK_QUE_Barbarian_Raged);
	if (isRagedAlready)
		return false;
	auto isFatigued = d20Sys.d20Query(obj, DK_QUE_Barbarian_Fatigued);
	if (isFatigued)
		return false;
	return true;
}

MonsterCategory LegacyCritterSystem::GetCategory(objHndl objHnd)
{
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
			return (MonsterCategory)(monCat & 0xFFFFFFFF);
		}
	}
	return mc_type_monstrous_humanoid; // default - so they have at least a weapons proficiency
}

uint32_t LegacyCritterSystem::IsCategoryType(objHndl objHnd, MonsterCategory categoryType){
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
			return (monCat & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

uint32_t LegacyCritterSystem::IsCategorySubtype(objHndl objHnd, MonsterCategory categoryType){
	if (objHnd != 0) {
		if (objects.IsCritter(objHnd)) {
			auto monCat = objects.getInt64(objHnd, obj_f_critter_monster_category);
			MonsterCategory moncatSubtype = static_cast<MonsterCategory>(monCat >> 32);
			return (moncatSubtype & categoryType) == categoryType;
		}
	}
	return 0;
}

uint32_t LegacyCritterSystem::IsUndead(objHndl objHnd){
	return IsCategoryType(objHnd, mc_type_undead);
}

uint32_t LegacyCritterSystem::IsOoze(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_ooze);
}

uint32_t LegacyCritterSystem::IsSubtypeFire(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtype_fire);
}

float LegacyCritterSystem::GetReach(objHndl obj, D20ActionType actType) 
{
	float naturalReach = (float) objects.getInt32(obj, obj_f_critter_reach);
	/*
	__asm{
		fild naturalReach;
		fstp naturalReach;
	}
	*/
	if (naturalReach < 0.01)
		naturalReach = 5.0;
	if (actType != D20A_TOUCH_ATTACK)
	{
		objHndl weapon = inventory.GetItemAtInvIdx(obj, 203);
		if (weapon)
		{
			auto weapType = objects.GetWeaponType(weapon);
			switch (weapType)
			{
			case wt_glaive:
			case wt_guisarme:
			case wt_longspear:
			case wt_ranseur:
			case wt_spike_chain:
				return naturalReach + 3.0f; // +5.0 - 2.0
			default:
				return naturalReach - 2.0f;
			}
				
		}
	}
	return naturalReach - 2.0f;
}

int LegacyCritterSystem::GetBonusFromSizeCategory(int sizeCategory)
{
	int result = sizeCategory;
	switch (sizeCategory)
	{
	case 1:
		result = 8;
		break;
	case 2:
		result = 4;
		break;
	case 3:
		result = 2;
		break;
	case 4:
		result = 1;
		break;
	case 5:
		result = 0;
		break;
	case 6:
		result = -1;
		break;
	case 7:
		result = -2;
		break;
	case 8:
		result = -4;
		break;
	case 9:
		result = -8;
		break;
	default:
		result = sizeCategory;
		break;
	}
	return result;
}

int LegacyCritterSystem::GetDamageIdx(objHndl obj, int attackIdx)
{
	int n =0;
	for (int i = 0; i < 4; i++)
	{
		n += objects.getArrayFieldInt32(obj, obj_f_critter_attacks_idx, i);
		if (n > attackIdx)
			return i;
	}
	return 0;
}

int LegacyCritterSystem::GetCritterDamageDice(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_critter_damage_idx, damageIdx);

}

DamageType LegacyCritterSystem::GetCritterAttackDamageType(objHndl obj, int attackIdx)
{
	DamageType damType[7];
	damType[0] = DamageType::SlashingAndBludgeoningAndPiercing;
	damType[1] = DamageType::PiercingAndSlashing;
	damType[2] = DamageType::PiercingAndSlashing;
	damType[3] = DamageType::Piercing;
	damType[4] = DamageType::Bludgeoning;
	damType[5] = DamageType::Bludgeoning;
	damType[6] = DamageType::Piercing;
	int damageIdx = GetDamageIdx(obj, attackIdx);
	int x = objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
	if (x > 6 || x < 0)
		return DamageType::Bludgeoning;
	return damType[x];
}

int LegacyCritterSystem::GetCritterAttackType(objHndl obj, int attackIdx)
{
	int damageIdx = GetDamageIdx(obj, attackIdx);
	return objects.getArrayFieldInt32(obj, obj_f_attack_types_idx, damageIdx);
}

bool LegacyCritterSystem::IsWarded(objHndl obj)
{
	auto args = PyTuple_New(1);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(obj));
	

	auto result = pythonObjIntegration.ExecuteScript("combat", "IsWarded", args);
	int isWarded = PyInt_AsLong(result);
	Py_DECREF(result);
	Py_DECREF(args);
	
	return isWarded != 0;
}

bool LegacyCritterSystem::IsSummoned(objHndl obj)
{
	// TODO
	return 0;
}

int LegacyCritterSystem::GetCasterLevel(objHndl obj)
{
	int result = 0;
	for (int i = 0; i < NUM_CLASSES; i++)
	{
		if (d20ClassSys.IsCastingClass(d20ClassSys.classEnums[i])  )
		{
			result += objects.StatLevelGet(obj, d20ClassSys.classEnums[i]);
		}
	}
	return result;
}

bool LegacyCritterSystem::IsCaster(objHndl obj)
{
	return GetCasterLevel(obj) > 0;
}

bool LegacyCritterSystem::IsWieldingRangedWeapon(objHndl obj)
{
	auto weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (!weapon)
	{
		weapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	}
	if (!weapon)
		return 0;
	return (objects.getInt32(weapon, obj_f_weapon_flags) & OWF_RANGED_WEAPON ) != 0;
}

void LegacyCritterSystem::GetCritterVoiceLine(objHndl obj, objHndl fellow, char* text, int* soundId)
{
	addresses.GetCritterVoiceLine(obj, fellow, text, soundId);
}

int LegacyCritterSystem::PlayCritterVoiceLine(objHndl obj, objHndl fellow, char* text, int soundId)
{
	return addresses.PlayCritterVoiceLine(obj, fellow, text, soundId);
}

bool LegacyCritterSystem::HashMatchingClassForSpell(objHndl handle, uint32_t spellEnum) const
{
	return temple::GetRef<BOOL(__cdecl)(objHndl, uint32_t)>(0x10075DA0)(handle, spellEnum);
}

int LegacyCritterSystem::SkillBaseGet(objHndl handle, SkillEnum skill)
{
	if (!handle)
		return 0;
	return gameSystems->GetObj().GetObject(handle)->GetInt32Array(obj_f_critter_skill_idx)[ skill] / 2;
}

int LegacyCritterSystem::SpellNumByFieldAndClass(objHndl obj, obj_f field, uint32_t spellClassCode)
{
	auto objBody = gameSystems->GetObj().GetObject(obj);
	auto spellArray = objBody->GetSpellArray(field);
	int spellArrayNum = spellArray.GetSize();
	
	int numSpells = 0;
	for (int i = 0; i < spellArrayNum; i++){
		auto spArrayClassCode = spellArray[i].classCode;
		if (spArrayClassCode == spellClassCode)
			numSpells++;
	}
	return numSpells;

}

int LegacyCritterSystem::DomainSpellNumByField(objHndl obj, obj_f field)
{
	auto objBody = gameSystems->GetObj().GetObject(obj);
	auto spellArray = objBody->GetSpellArray(field);
	int spellArrayNum = spellArray.GetSize();

	int numSpells = 0;
	for (int i = 0; i < spellArrayNum; i++) {
		if (spellSys.isDomainSpell(spellArray[i].classCode ))
			numSpells++;
	}
	return numSpells;
}

int LegacyCritterSystem::GetNumFollowers(objHndl obj, int excludeForcedFollowers)
{
	auto objBod = objSystem->GetObject(obj);
	auto followerArray = objBod->GetObjectIdArray(obj_f_critter_follower_idx);
	auto followersNum = followerArray.GetSize();
	if (excludeForcedFollowers)
	{
		auto orgNum = followersNum;
		for (int i = 0; i < orgNum; i++)
		{
			auto follower = followerArray[i];
			auto followerNpcFlags = objects.getInt32(follower, obj_f_npc_flags);
			if (followerNpcFlags & NpcFlag::ONF_FORCED_FOLLOWER)
				followersNum--;
		}
	}
	return followersNum;
}
#pragma region Critter Hooks
uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 
