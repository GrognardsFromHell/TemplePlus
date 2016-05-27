#include "stdafx.h"
#include "obj.h"
#include <temple/dll.h>
#include <temple/meshes.h>
#include "d20.h"
#include "common.h"
#include "critter.h"
#include "temple_functions.h"
#include "condition.h"
#include "obj_fieldnames.h"
#include "location.h"
#include "pathfinding.h"
#include "float_line.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "util/fixes.h"

#include "python/python_integration_obj.h"
#include <set>

Objects objects;

static_assert(temple::validate_size<CondNode, 52>::value, "Condition node structure has incorrect size.");

struct ObjectSystemAddresses : temple::AddressTable
{

	int8_t(*SectorGetElevation)(LocAndOffsets forLocation);
	int(*GetAasHandle)(objHndl obj);
	
	ObjectSystemAddresses()
	{
		rebase(SectorGetElevation, 0x100A8CB0);
		rebase(GetAasHandle, 0x10021A40);
	}
} addresses;

static_assert(temple::validate_size<ObjectId, 24>::value, "Object ID structure has incorrect size.");

#pragma region Objects implementation
		
uint32_t Objects::getInt32(objHndl obj, obj_f field) {
	return objSystem->GetObject(obj)->GetInt32(field);
	}

uint64_t Objects::getInt64(objHndl obj, obj_f field) {
	return objSystem->GetObject(obj)->GetInt64(field);
}

void Objects::SetFieldObjHnd(objHndl obj, obj_f field, objHndl value) {
	objSystem->GetObject(obj)->SetObjHndl(field, value);
	}

objHndl Objects::getObjHnd(objHndl obj, obj_f field) {
	return objSystem->GetObject(obj)->GetObjHndl(field);
	}

void Objects::setInt32(objHndl obj, obj_f field, uint32_t value) {
	objSystem->GetObject(obj)->SetInt32(field, value);
}

int32_t Objects::getArrayFieldInt32(objHndl obj, obj_f field, uint32_t index) {
	return objSystem->GetObject(obj)->GetInt32(field, index);
}

objHndl Objects::getArrayFieldObj(objHndl obj, obj_f field, uint32_t index) {
	return objSystem->GetObject(obj)->GetObjHndl(field, index);
}

gfx::AnimatedModelPtr Objects::GetAnimHandle(objHndl obj)
{
	// I think this belongs to the map_obj subsystem

	if (!obj) {
		return nullptr;
	}

	// An animation handle was already created
	auto handle = _GetInternalFieldInt32(obj, obj_f_animation_handle);
	if (handle) {
		return gameSystems->GetAAS().BorrowByHandle(handle);
	}

	// Which object to retrieve the model properties from (this indirection
	// is used for polymorph)
	auto modelSrcObj = obj;

	// If the obj is polymorphed, use the polymorph proto instead
	int polyProto = d20.d20Query(obj, DK_QUE_Polymorphed);
	if (polyProto) {
		modelSrcObj = GetProtoHandle(polyProto);
	}

	auto meshId = getInt32(modelSrcObj, obj_f_base_mesh);
	auto skeletonId = getInt32(modelSrcObj, obj_f_base_anim);
	auto idleAnimId = GetIdleAnim(obj);
	auto animParams = GetAnimParams(obj);

	// Create the model from the meshes.mes IDs and store the result in the obj
	auto model = gameSystems->GetAAS().FromIds(
		meshId, 
		skeletonId,
		idleAnimId,
		animParams,
		true // Borrowed since we store the handle in the obj
	);
	setInt32(obj, obj_f_animation_handle, model->GetHandle());

	if (IsCritter(obj)) {
		critterSys.UpdateModelEquipment(obj);
	}

	if (IsNPC(obj)) {
		critterSys.AddNpcAddMeshes(obj);
	}

	if (!IsRenderHeightSet(obj)) {
		UpdateRenderHeight(obj, *model);
	}
	if (!IsRadiusSet(obj)) {
		UpdateRadius(obj, *model);
	}

	return model;
	
}

gfx::AnimatedModelParams Objects::GetAnimParams(objHndl handle)
{
	gfx::AnimatedModelParams result;

	result.scale = objects.GetScalePercent(handle) / 100.0f;

	// Special case for equippable items
	if (IsEquipment(handle)) {
		auto itemFlags = objects.GetItemFlags(handle);
		if ((itemFlags & OIF_DRAW_WHEN_PARENTED) != 0) {
			auto parent = inventory.GetParent(handle);
			if (parent) {
				result.scale *= objects.GetScalePercent(parent) / 100.0f;

				result.attachedBoneName = inventory.GetAttachBone(handle);
				if (!result.attachedBoneName.empty()) {
					auto parentLoc = objects.GetLocationFull(parent);
					result.x = parentLoc.location.locx;
					result.y = parentLoc.location.locy;
					result.offsetX = parentLoc.off_x;
					result.offsetY = parentLoc.off_y;

					auto elevation = addresses.SectorGetElevation(parentLoc);
					auto offsetZ = objects.GetOffsetZ(parent);
					result.offsetZ = offsetZ - elevation;

					result.rotation = objects.GetRotation(parent);
					result.rotationPitch = objects.GetRotationPitch(parent);

					auto aasHandle = addresses.GetAasHandle(parent);
					if (aasHandle) {
						auto& aas = gameSystems->GetAAS();
						result.parentAnim = aas.BorrowByHandle(aasHandle);
	}
					return result;
		}
	} 
		}
	}

	auto loc = objects.GetLocationFull(handle);
	result.x = loc.location.locx;
	result.y = loc.location.locy;
	result.offsetX = loc.off_x;
	result.offsetY = loc.off_y;
	
	auto flags = objects.GetFlags(handle);
	int8_t elevation = 0; // TODO: This may be "depth" instead of elevation.
	if (!(flags & OF_NOHEIGHT)) {
		elevation = addresses.SectorGetElevation(loc);
}
	result.offsetZ = objects.GetOffsetZ(handle) - elevation;
	result.rotation = objects.GetRotation(handle);
	result.rotationPitch = objects.GetRotationPitch(handle);

	return result;
	}

void Objects::SetAnimId(objHndl obj, gfx::EncodedAnimId animId) {

	// Propagate animations to main/off hand equipment for critters
	if (IsCritter(obj)) {
		auto mainHand = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
		auto offHand = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);		
		if (!offHand) {
			offHand = critterSys.GetWornItem(obj, EquipSlot::Shield);
}

		// Apparently certain anim IDs cause weapons to disappear, 
		// possibly skill use/casting?
		int opacity = 0;
		if (animId & 0xC0000000) {
			opacity = 255;
	}

		if (mainHand) {
			FadeTo(mainHand, opacity, 10, 16, 0);
			SetAnimId(mainHand, animId);
}
		if (offHand) {
			FadeTo(offHand, opacity, 10, 16, 0);
			SetAnimId(offHand, animId);
		}
	}

	auto model = GetAnimHandle(obj);
	model->SetAnimId(animId);

}

gfx::EncodedAnimId Objects::GetIdleAnim(objHndl obj)
{
	using gfx::EncodedAnimId;

	auto idleAnimObj = obj;

	// If polymorphed, compute for the polymorph target
	auto polyProtoNum = d20.d20Query(obj, DK_QUE_Polymorphed);
	if (polyProtoNum) {
		idleAnimObj = GetProtoHandle(polyProtoNum);
	}

	auto objType = GetType(idleAnimObj);
	if (objType != obj_t_pc && objType != obj_t_npc) {
		if (objType == obj_t_portal && IsDoorOpen(obj)) {
			return EncodedAnimId(78);
}
		return EncodedAnimId(35);
	}

	if (critterSys.IsDeadNullDestroyed(obj)) {
		return EncodedAnimId(12);
	}
	else if (critterSys.IsDeadOrUnconscious(obj))
{
		return EncodedAnimId(14);
	}
	else if (critterSys.IsProne(obj))
	{
		return EncodedAnimId(1);
	}
	else if (critterSys.IsConcealed(obj))
	{
		return EncodedAnimId(33);
}
	else if (critterSys.IsMovingSilently(obj))
{
		return EncodedAnimId(44);
}
	else if (critterSys.IsCombatModeActive(obj))
{
		return critterSys.GetAnimId(idleAnimObj, gfx::WeaponAnim::CombatIdle);
}
	else
{
		return critterSys.GetAnimId(idleAnimObj, gfx::WeaponAnim::Idle);
	}
}

bool Objects::IsDoorOpen(objHndl obj)
{
	// Undetected secret doors are never open
	auto secretDoorFlags = GetSecretDoorFlags(obj);
	if (secretDoorFlags & OSDF_SECRET_DOOR 
		&& !(secretDoorFlags & OSDF_SECRET_DOOR_FOUND)) {
		return false;
	}

	auto portalFlags = GetPortalFlags(obj);
	return (portalFlags & OPF_OPEN) == OPF_OPEN;
}

Objects::Objects()
{
	pathfinding = &pathfindingSys;
	loc = &locSys;
	floats = &floatSys;
	rebase(_GetDisplayName, 0x1001F970);


	rebase(_SetFlag,	0x10020F50);
	rebase(_ClearFlag,	0x10021020);
	rebase(_GetRadius,	0x10021C40);

	rebase(_Destroy,	0x100257A0);
	rebase(_Move,		0x10025950);
	rebase(_Create,		0x10028D20);
	
	rebase(_IsPlayerControlled, 0x1002B390);

	rebase(_SecretdoorDetect,	0x10046920);
	rebase(_FadeTo,				0x1004C390);
	rebase(_GetSize,			0x1004D690);

	rebase(_AdjustReaction,		0x10053F20);
	rebase(_GetReaction,		0x10054180);

	rebase(_AiForceSpreadOut,	0x1005A640);
	
	rebase(_StatLevelGet,			0x10074800);
	rebase(_StatLevelGetBase,		0x10074CF0);
	rebase(_StatLevelSetBase,		0x10074E10);

	rebase(_HasSpellEffects,		0x10076370);

	rebase(_IsObjDeadNullDestroyed, 0x1007E650);

	rebase(_GetId, 0x1009CA40);

	rebase(_GetInternalFieldInt32,		0x1009E1D0);
	rebase(_GetInternalFieldInt64,		0x1009E2E0);
	rebase(_GetInternalFieldFloat,		0x1009E260);
	rebase(_GetInternalFieldInt32Array, 0x1009E5C0);

	rebase(_SetInternalFieldInt32,	0x100A0190);
	rebase(_SetInternalFieldFloat,	0x100A0190); // This is actually the same function as 32-bit heh


	rebase(_PortalToggleOpen,	0x100B4700);

	rebase(_TargetRandomTileNear, 0x100B99A0);
	rebase(_FindFreeSpot,		  0x100BDB50);

	rebase(_ObjectIdPrint,		0x100C2460);

	rebase(_ContainerToggleOpen, 0x1010EA00);

	rebase(_DLLFieldNames,		0x102CD840);
}

uint32_t Objects::abilityScoreLevelGet(objHndl objHnd, Stat stat, DispIO* dispIO)
{
	return objects.dispatch.DispatchGetBonus(objHnd, (DispIoBonusList*)dispIO, dispTypeAbilityScoreLevel, (D20DispatcherKey)(stat + 1));
}

float Objects::GetRadius(objHndl handle)
{
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto radiusSet = obj->GetFlags() & OF_RADIUS_SET;
	objHndl protoHandle;
	float protoRadius;
	if (radiusSet){
		auto radius = obj->GetFloat(obj_f_radius);

		if ( radius < 2000.0 && radius > 0){
			 protoHandle = obj->GetObjHndl(obj_f_prototype_handle);
			if (protoHandle){
				auto protoObj = gameSystems->GetObj().GetObject(protoHandle);
				protoRadius = protoObj->GetFloat( obj_f_radius);
				if (protoRadius > 0.0)
				{
					radius = protoRadius;
					obj->SetFloat(obj_f_radius, protoRadius);
				}
			}
			
		}
		if (radius < 2000.0 && radius > 0)
		{
			if (radius > 600)
			{
				logger->debug("Caught very large radius! Was {}", radius);
				protoHandle = obj->GetObjHndl(obj_f_prototype_handle);
				auto protoObj = gameSystems->GetObj().GetObject(protoHandle);
				protoRadius = protoObj->GetFloat(obj_f_radius);
				
				auto model = GetAnimHandle(handle);
				if (model) {
					UpdateRadius(handle, *model);
				}
				radius = obj->GetFloat(obj_f_radius);
			}
			return radius;
		}		
	}

	logger->debug("GetRadius: Radius not yet set, now calculating.");
	
	auto model = GetAnimHandle(handle);
	if (!model)
	{
		logger->warn("GetRadius: Null AAS handle!");
		protoHandle = obj->GetObjHndl(obj_f_prototype_handle);
		auto protoObj = gameSystems->GetObj().GetObject(protoHandle);
		protoRadius = protoObj->GetFloat(obj_f_radius);
		if (protoRadius > 0.0) {
			logger->debug("Returning radius from Proto: {}", protoRadius);
			return protoRadius;
		}
		logger->debug("Returning default (10.0)");
		return 10.0;
	}

	auto radius = obj->GetFloat(obj_f_radius);
	radiusSet = obj->GetFlags() & OF_RADIUS_SET; // might be changed I guess
	if (!radiusSet || abs(radius) > 2000){
		logger->debug("GetRadius: Calculating from AAS model. Initially was {}", radius);

		UpdateRadius(handle, *model);

		radius = obj->GetFloat(obj_f_radius);
		
		if (radius > 2000.0)
		{
			logger->warn("GetRadius: Huge radius calculated from AAS {}", radius);
			radius = 2000.0;
		} else if (radius <=0)
		{
			logger->warn("GetRadius: Negative radius calculated from AAS: {}. Changing to default (10.0)", radius);
			radius = 10.0;
		}
	}
	return radius;
	//return _GetRadius(handle);
	
}

int Objects::GetScalePercent(objHndl handle){

	auto obj = gameSystems->GetObj().GetObject(handle);
	auto modelScale = obj->GetInt32(obj_f_model_scale);
	

	if (IsCritter(handle)) {
		DispIoMoveSpeed dispIo;
		BonusList bonlist;
		dispIo.bonlist = &bonlist;
		bonlist.AddBonus(modelScale, 1, 102); // initial value

		auto dispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
		if (dispatcher->IsValid()){
			dispatcher->Process(dispTypeGetModelScale, DK_NONE, &dispIo);
		}
		modelScale = bonlist.GetEffectiveBonusSum();
	}

	return modelScale;

}

void Objects::SetRotation(objHndl handle, float rotation) {
	// Normalizes the rotation parameter to valid radians range
	static auto PI_2 = (float)(2.0 * M_PI);
	
	while (rotation >= PI_2) {
		rotation -= PI_2;
	}
	while (rotation < 0) {
		rotation += PI_2;
	}
	_SetInternalFieldFloat(handle, obj_f_rotation, rotation);
}

int Objects::GetScript(objHndl handle, int index) {
	auto obj = objSystem->GetObject(handle);
	return  obj->GetScript(obj_f_scripts_idx, index).scriptId;
}

void Objects::SetScript(objHndl handle, int index, int scriptId) {
	auto obj = objSystem->GetObject(handle);
	auto script = obj->GetScript(obj_f_scripts_idx, index);
	script.scriptId = scriptId;
	obj->SetScript(obj_f_scripts_idx, index, script);
}

ObjectScript Objects::GetScriptAttachment(objHndl handle, int index) {
	auto obj = objSystem->GetObject(handle);
	return  obj->GetScript(obj_f_scripts_idx, index);
}

void Objects::SetScriptAttachment(objHndl handle, int index, const ObjectScript& script) {
	auto obj = objSystem->GetObject(handle);
	obj->SetScript(obj_f_scripts_idx, index, script);
}

Dice Objects::GetHitDice(objHndl handle) {
	auto count = _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 0);
	auto sides = _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 1);
	auto modifier = _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 2);
	return Dice(count, sides, modifier);
}

// Reimplements 100801D0
int Objects::GetHitDiceNum(objHndl handle) {
	auto obj = objSystem->GetObject(handle);
	size_t result = obj->GetInt32Array(obj_f_critter_level_idx).GetSize();
	if (GetType(handle) == obj_t_npc) {
		result += _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 0);
	}
	return result;
}

int Objects::GetSize(objHndl handle) {
	// This function uses the dispatcher internally and should probably be rewritten
	//return _GetSize(handle);
	return dispatch.DispatchGetSizeCategory(handle);
}

objHndl Objects::Create(objHndl proto, locXY tile) {
	objHndl handle;
	if (_Create(proto, tile, &handle)) {
		return handle;
	} else {
		return objHndl::null;
	}
}

bool Objects::FindFreeSpot(LocAndOffsets location, float radius, LocAndOffsets& freeSpotOut) {
	return _FindFreeSpot(location, radius, freeSpotOut);
}

objHndl Objects::GetProtoHandle(int protoNumber) {
	return objSystem->GetProtoHandle(protoNumber);
}

bool Objects::AiForceSpreadOut(objHndl handle) {
	return _AiForceSpreadOut(handle, nullptr);
}

bool Objects::AiForceSpreadOut(objHndl handle, LocAndOffsets &location) {
	return _AiForceSpreadOut(handle, &location);
}

locXY Objects::TargetRandomTileNear(objHndl handle, int distance) {
	locXY result;
	_TargetRandomTileNear(handle, distance, &result);
	return result;
}

float Objects::GetRotationTowards(objHndl from, objHndl to) {
	auto locFrom = GetLocationFull(from);
	auto locTo = GetLocationFull(to);

	return AngleBetweenPoints(locFrom, locTo);
}

void Objects::FadeTo(objHndl obj, int targetOpacity, int tickTimeMs, int tickOpacityQuantum, int callbackMode) const
{
	_FadeTo(obj, targetOpacity, tickTimeMs, tickOpacityQuantum, callbackMode);
}

void Objects::Move(objHndl handle, LocAndOffsets toLocation) {
	_Move(handle, toLocation);
}

#include "combat.h"
#include "turn_based.h"

SecretDoorFlag Objects::GetSecretDoorFlags(objHndl handle) {
	return (SecretDoorFlag) _GetInternalFieldInt32(handle, obj_f_secretdoor_flags);
}

void Objects::Destroy(objHndl ObjHnd) {
	static set<objHndl> destroyed;
	std::string name = this->GetDisplayName(ObjHnd, ObjHnd);
	logger->info("Destroying {}", name);
	if (destroyed.find(ObjHnd) != destroyed.end()) {
		logger->error("Double destroying object {:x}", ObjHnd);
	}
	destroyed.insert(ObjHnd);

	auto flags = _GetInternalFieldInt32(ObjHnd, obj_f_flags);

	if (flags & OF_DESTROYED) {
		return; // Already destroyed
}

	if (!pythonObjIntegration.ExecuteObjectScript(ObjHnd, ObjHnd, ObjScriptEvent::Destroy)) {
		return; // Scripts tells us to skip it
}

	auto moveContentToLoc = temple::GetPointer<void(objHndl, BOOL)>(0x1006DB80);

	auto type = _GetInternalFieldInt32(ObjHnd, obj_f_type);
	if (type != obj_t_pc && type != obj_t_npc)
	{
		if (type >= obj_t_weapon && type <= obj_t_generic || type == obj_t_bag)
		{
			auto parentObj = inventory.GetParent(ObjHnd);
			if (parentObj)
			{
				auto loc = GetLocation(parentObj);
				inventory.ItemRemove(ObjHnd);
				auto moveObj = temple::GetPointer<void(objHndl, locXY)>(0x100252D0);
				moveObj(ObjHnd, loc);
			}
		}
		if (type == obj_t_container)
{
			moveContentToLoc(ObjHnd, 1);
		}
}
	else
	{
		auto removeFromGroups = temple::GetPointer<int(objHndl)>(0x10080DA0);
		removeFromGroups(ObjHnd);

		auto removeAiTimer = temple::GetPointer<int(objHndl)>(0x100588D0);
		removeAiTimer(ObjHnd);

		if (type == obj_t_npc)
{
			auto getDlgTarget = temple::GetPointer<objHndl(objHndl)>(0x10053CA0);
			auto cancelDialog = temple::GetPointer<void(objHndl, int)>(0x1009A5D0);

			auto v3 = getDlgTarget(ObjHnd);
			if (v3)
				cancelDialog(v3, 0);
		}
		moveContentToLoc(ObjHnd, 1);
}

	auto cancelAnims = temple::GetPointer<void(objHndl)>(0x1000C760);
	cancelAnims(ObjHnd);
	
	if (combatSys.isCombatActive())
{
		if (tbSys.turnBasedGetCurrentActor() == ObjHnd) {
			combatSys.CombatAdvanceTurn(ObjHnd);
		}
	}
	combatSys.RemoveFromInitiative(ObjHnd);
	
	
	auto removeDispatcher = temple::GetPointer<int(objHndl)>(0x1004FEE0);
	removeDispatcher(ObjHnd);

	auto updateTbUi = temple::GetPointer<void(objHndl)>(0x1014DE90);
	updateTbUi(ObjHnd);
	
	auto killRendering = temple::GetPointer<void(objHndl)>(0x10021290);
	killRendering(ObjHnd);
	
	auto aasHandle = getInt32(ObjHnd, obj_f_animation_handle);
	if (aasHandle) {
		auto freeAasModel = temple::GetPointer<void(int)>(0x10264510);
		freeAasModel(aasHandle);
		setInt32(ObjHnd, obj_f_animation_handle, 0);
	}
	
	auto v7 = GetFlags(ObjHnd);
	setInt32(ObjHnd, obj_f_flags, v7 | OF_DESTROYED);
}

ObjectId Objects::GetId(objHndl handle) {
	ObjectId result;
	_GetId(&result, handle);
	return result;
}

ObjectType Objects::GetType(objHndl obj)
{
	return static_cast<ObjectType>(getInt32(obj, obj_f_type));
}

int32_t Objects::GetHPCur(objHndl obj)
{
	return _StatLevelGet(obj, stat_hp_current);
}

bool Objects::IsPlayerControlled(objHndl obj)
{
	return _IsPlayerControlled(obj);
}

string Objects::GetDisplayName(objHndl obj, objHndl observer) {
	char name[512];
	_GetDisplayName(obj, observer, name);
	return name;
		}

bool Objects::IsStatic(objHndl handle) {

	auto type = GetType(handle);
	if (type == obj_t_projectile 
		|| type == obj_t_container 
		|| type == obj_t_pc 
		|| type == obj_t_npc 
		|| type >= obj_t_weapon && type <= obj_t_generic 
		|| type == obj_t_bag)
		return false;
	
	return (GetFlags(handle) & OF_DYNAMIC) == 0;

}

uint32_t Objects::StatLevelGet(objHndl obj, Stat stat)
{
	return _StatLevelGet(obj, stat);
		}

int Objects::StatLevelGetBase(objHndl obj, Stat stat)
{
	return _StatLevelGetBase(obj, stat);
}

int Objects::StatLevelSetBase(objHndl obj, Stat stat, int value)
{
	return _StatLevelSetBase(obj, stat, value);
}

int Objects::GetStatType(Stat stat){
	switch (stat) {
	case stat_strength:
	case stat_dexterity:
	case stat_constitution:
	case stat_intelligence:
	case stat_wisdom:
	case stat_charisma:
		return 0; // does a dispatch for ability score

	case stat_level:
	case stat_level_barbarian:
	case stat_level_bard:
	case stat_level_cleric:
	case stat_level_druid:
	case stat_level_fighter:
	case stat_level_monk:
	case stat_level_paladin:
	case stat_level_ranger:
	case stat_level_rogue:
	case stat_level_sorcerer:
	case stat_level_wizard:
		return 1;

	case stat_hp_max:
	case stat_hp_current:
	case stat_subdual_damage:
		return 2;

	case stat_race:
	case stat_gender:
	case stat_height:
	case stat_weight:
	case stat_size:
	case stat_experience:
	case stat_alignment:
	case stat_deity:
	case stat_domain_1:
	case stat_domain_2:
	case stat_ac:
	case stat_attack_bonus:
	case stat_damage_bonus:
	case stat_subrace:
	case stat_melee_attack_bonus:
	case stat_ranged_attack_bonus:
		return 3;

	case stat_money:
	case stat_money_pp:
	case stat_money_gp:
	case stat_money_ep:
	case stat_money_sp:
	case stat_money_cp:
		return 4;

	case stat_str_mod:
	case stat_dex_mod:
	case stat_con_mod:
	case stat_int_mod:
	case stat_wis_mod:
	case stat_cha_mod:
		return 5; // largely does StatLevelGet/2 - 5 with special casing for dex mod

	case stat_movement_speed:
	case stat_run_speed:
		return 6; // regards load

	case stat_load:
		return 9; // regards size & str score, comapres to carried item weight

	case stat_save_reflexes:
	case stat_save_fortitude:
	case stat_save_willpower:
		return 10; // does a dispatch for saving throw

	default:
		break;
	}
	
	if ((int)stat >= 1000 && (int)stat <= 1999){
		return 7; // feats
	}

	if ((int)stat >= 2000 && (int)stat <= 2999) {
		return 8; // seems to be unhandled?
	}

	return 12; // these are unimplemented
	//case stat_caster_level: 
	//case stat_caster_level_barbarian: 
	//case stat_caster_level_bard: 
	//case stat_caster_level_cleric: 
	//case stat_caster_level_druid: 
	//case stat_caster_level_fighter: 
	//case stat_caster_level_monk: 
	//case stat_caster_level_paladin: 
	//case stat_caster_level_ranger: 
	//case stat_caster_level_rogue: 
	//case stat_caster_level_sorcerer: 
	//case stat_caster_level_wizard: 
	//case stat_age: 
	//case stat_category: 
	//case stat_alignment_choice: 
	//
	//
	//case stat_initiative_bonus: 
	//
	//case stat_carried_weight: 

	//case stat_favored_enemies: 
	//case stat_known_spells: 
	//case stat_memorized_spells: 
	//case stat_spells_per_day: 
	//case stat_school_specialization: 
	//case stat_school_prohibited: 

	
}

const char* Objects::GetStatName(Stat stat){
	if (GetStatType(stat) == 7){
		return feats.GetFeatName((feat_enums)((int)stat - 1000));
	}

	return temple::GetRef<const char*[288]>(0x10AAE8C8)[stat]; // statMesStrings
}

Dispatcher * Objects::GetDispatcher(objHndl obj)
{
	return objSystem->GetObject(obj)->GetDispatcher();
}

void Objects::SetDispatcher(objHndl obj, uint32_t data32)
{
	objSystem->GetObject(obj)->SetDispatcher(reinterpret_cast<Dispatcher*>(data32));
}

int Objects::GetModFromStatLevel(int statLevel)
{
	return (statLevel - 10) / 2;
}

bool Objects::IsPortalOpen(objHndl obj) {
	auto sdFlags = objects.getInt32(obj, obj_f_secretdoor_flags);
	if ((sdFlags & OSDF_SECRET_DOOR) && !(sdFlags & OSDF_SECRET_DOOR_FOUND))
		return 0;
	return (objects.getInt32(obj, obj_f_portal_flags) & OPF_OPEN) != 0;
}

int Objects::GetTempId(objHndl handle) {
	return _GetInternalFieldInt32(handle, obj_f_temp_id);
}

int Objects::GetAlpha(objHndl handle) {
	return _GetInternalFieldInt32(handle, obj_f_transparency);
}

int Objects::IsCritterProne(objHndl handle){
	auto obj = gameSystems->GetObj().GetObject(handle);
	if (obj->GetFlags() & (OF_OFF | OF_DESTROYED))
		return FALSE;
	if (!obj->IsCritter())
		return FALSE;
	if (d20Sys.d20Query(handle, DK_QUE_Prone))
		return TRUE;
	return FALSE;
}
#pragma endregion

void Objects::UpdateRenderHeight(objHndl handle, gfx::AnimatedModel &model) {
	auto scale = GetScalePercent(handle);
	auto height = model.GetHeight(scale);

	SetRenderHeight(handle, height);
	SetFlag(handle, OF_HEIGHT_SET);
}

void Objects::UpdateRadius(objHndl handle, gfx::AnimatedModel &model) {
	auto scale = GetScalePercent(handle);
	auto radius = model.GetRadius(scale);

	if (radius > 0) {
		SetRadius(handle, radius);
		SetFlag(handle, OF_RADIUS_SET);
	}
}

#pragma region Hooks

// Replacements for Object functions (mainly for debugging purposes now)
class ObjectReplacements : public TempleFix {
public:

	static uint32_t _abilityScoreLevelGet(objHndl obj, Stat abScore, DispIO * dispIO)
{
	return objects.abilityScoreLevelGet(obj, abScore, dispIO);
}

	static void _destroy(objHndl obj) {
		objects.Destroy(obj);
}

	static int(*orgMove)(objHndl, LocAndOffsets);
	static int Move(objHndl obj, LocAndOffsets loc)
{
		return orgMove(obj, loc);
	};


	static int HookedGetModelScale(objHndl handle, obj_f field){
		return objects.GetScalePercent(handle);
	}

	void apply() override {

		// obj_update_render_height
		replaceFunction<void(objHndl, temple::AasHandle)>(0x10021360, [](objHndl objId, temple::AasHandle animId) {
			auto anim = gameSystems->GetAAS().BorrowByHandle(animId);
			if (anim) {
				objects.UpdateRenderHeight(objId, *anim);
			}
		});

		// obj_update_radius
		replaceFunction<void(objHndl, temple::AasHandle)>(0x10021500, [](objHndl objId, temple::AasHandle animId) {
			auto anim = gameSystems->GetAAS().BorrowByHandle(animId);
			if (anim) {
				objects.UpdateRadius(objId, *anim);
			}
		});

		replaceFunction(0x1004E7F0, _abilityScoreLevelGet);
		replaceFunction(0x100257A0, _destroy);
		//orgMove = replaceFunction(0x10025950, Move);
		//orgMoveUpdateLoc = replaceFunction(0x100C1990, MoveUpdateLoc);


		writeCall(0x10022AEF, HookedGetModelScale); 
		writeCall(0x100228A5, HookedGetModelScale); // RayCast
		writeCall(0x10023F1F, HookedGetModelScale); // Render related
		writeCall(0x10021E9F, HookedGetModelScale);
		
			
}
} objReplacements;

int(*ObjectReplacements::orgMove)(objHndl, LocAndOffsets);
#pragma endregion
