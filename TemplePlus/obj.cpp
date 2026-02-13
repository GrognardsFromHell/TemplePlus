#include "stdafx.h"
#include "obj.h"
#include "ai.h"
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
#include "gamesystems/mapsystem.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/legacymapsystems.h"
#include "animgoals/anim.h"

#include <infrastructure/vfs.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include "util/fixes.h"
#include <gamesystems/d20/d20stats.h>
#include "python/python_integration_obj.h"
#include <set>
#include <config/config.h>
#include "gamesystems/timeevents.h"
#include "gamesystems/legacysystems.h"
#include "gamesystems/objfade.h"
#include "party.h"
#include "d20_race.h"

Objects objects;

static_assert(temple::validate_size<CondNode, 52>::value, "Condition node structure has incorrect size.");

struct ObjectSystemAddresses : temple::AddressTable
{

	int(*GetAasHandle)(objHndl obj);
	
	ObjectSystemAddresses()
	{
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


void Objects::SetFieldString(objHndl obj, obj_f field, char* value) {
	objSystem->GetObject(obj)->SetString(field, value);
}

const char* Objects::getString(objHndl obj, obj_f field) {
	return objSystem->GetObject(obj)->GetString(field);
}

void Objects::setInt32(objHndl obj, obj_f field, uint32_t value) {
	objSystem->GetObject(obj)->SetInt32(field, value);
}

int32_t Objects::getArrayFieldInt32(objHndl obj, obj_f field, uint32_t index) {
	return objSystem->GetObject(obj)->GetInt32(field, index);
}

int32_t Objects::getArrayFieldInt32Size(objHndl obj, obj_f field) {
	return objSystem->GetObject(obj)->GetInt32Size(field);
}

int64_t Objects::getArrayFieldInt64(objHndl obj, obj_f field, uint32_t index) {
	return objSystem->GetObject(obj)->GetInt64(field, index);
}

int32_t Objects::getArrayFieldInt64Size(objHndl obj, obj_f field) {
	return objSystem->GetObject(obj)->GetInt64Size(field);
}


objHndl Objects::getArrayFieldObj(objHndl obj, obj_f field, uint32_t index) {
	return objSystem->GetObject(obj)->GetObjHndl(field, index);
}

int Objects::GetAasHandle(objHndl handle){
	auto model(GetAnimHandle(handle));
	if (model) {
		return model->GetHandle();
	}
	return 0;
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
	int polyProto = d20Sys.d20Query(obj, DK_QUE_Polymorphed);
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

					auto depth = gameSystems->GetHeight().GetDepth(parentLoc);
					auto offsetZ = objects.GetOffsetZ(parent);
					result.offsetZ = offsetZ - depth;

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
	int8_t depth = 0;
	if (!(flags & OF_NOHEIGHT)) {
		depth = gameSystems->GetHeight().GetDepth(loc);
	}
	result.offsetZ = objects.GetOffsetZ(handle) - depth;
	result.rotation = objects.GetRotation(handle);
	result.rotationPitch = objects.GetRotationPitch(handle);

	return result;
	}

void Objects::ClearAnim(objHndl handle)
{
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto animHandle = obj->GetInt32(obj_f_animation_handle);
	if (animHandle) {
		gameSystems->GetAAS().FreeHandle(animHandle);
		obj->SetInt32(obj_f_animation_handle, 0);
	}
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
		if (animId.IsSpecialAnim()) {
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

bool Objects::HasAnimId(objHndl obj, gfx::EncodedAnimId animId)
{
	auto model = GetAnimHandle(obj);
	return model->HasAnim(animId);
}

gfx::EncodedAnimId Objects::GetIdleAnim(objHndl obj)
{
	using gfx::EncodedAnimId;

	auto idleAnimObj = obj;

	// If polymorphed, compute for the polymorph target
	auto polyProtoNum = d20Sys.d20Query(obj, DK_QUE_Polymorphed);
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
	// rebase(_GetRadius,	0x10021C40);

	rebase(_Destroy,	0x100257A0);
	rebase(_Move,		0x10025950);
	rebase(_Create,		0x10028D20);
	
	rebase(_IsPlayerControlled, 0x1002B390);

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

/* 0x1004E7F0 */
uint32_t Objects::abilityScoreLevelGet(objHndl objHnd, Stat stat, DispIO* dispIO)
{
	auto dispIo = (DispIoBonusList*)dispIO;
	auto result =
		objects.dispatch.Dispatch10AbilityScoreLevelGet(objHnd, stat, dispIo);
	if (config.npcStatBoost > 0 && objects.IsNPC(objHnd) && !party.IsInParty(objHnd) ) 
	{
		if (result >= 3 || stat != Stat::stat_intelligence) { // so it doesn't change behaviors such as animal intelligence
			result += config.npcStatBoost;
		}
	}
	return result;
}

float AdjustRadiusSize(float baseRadius, int baseSize, int curSize)
{
	if (baseSize == curSize) return baseRadius;

	float radius = baseRadius;

	// TODO: audit this w/r/t common radii for actual creature sizes.
	//
	// Note that the ToEE settings are on the small side. Standard medium is 25,
	// and that is multiplied by 0.75 to get the radius in inches (I believe). So
	// most mediums occupy just over a 3ft. diameter circle of actual space.
	if (curSize >= baseSize) {
		for(int i = baseSize; i < curSize; i++) {
			if (i == 4) radius += 5.0; // small -> medium
			else if (i == 3) radius = 20.0; // tiny -> small
			else if (5 <= i && i <= 7) radius += 25.0; // add typical medium radius
			else if (i > 7) radius += 50.0; // double medium radius for >=colossal
			// else leave radius alone for tiny and below
		}
	} else {
		for (int i = baseSize; i > curSize; i--) {
			if (i == 4) radius = 12.0; // small -> tiny
			else if (i == 5) radius -= 5.0; // medium -> small
			else if (i <= 8) radius -= 25.0; // < colossal
			else if (i > 8) radius -= 50.0; // >= colossal
			// else leave radius alone
		}
		// sanity check due to seemingly very low radii in protos
		if (curSize > 3 && radius < 18.0) radius = baseRadius;
	}

	return radius;
}

float Objects::GetRadius(objHndl handle, bool base)
{
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto radiusSet = obj->GetFlags() & OF_RADIUS_SET;
	objHndl protoHandle;
	float protoRadius;

	int baseSize = GetSize(handle, true);
	int curSize = GetSize(handle, false);

	if (radiusSet){
		auto radius = obj->GetFloat(obj_f_radius);

		if ( radius < 2000.0 && radius > 0){
			protoHandle = obj->GetObjHndl(obj_f_prototype_handle);
			if (protoHandle){
				auto protoObj = gameSystems->GetObj().GetObject(protoHandle);
				protoRadius = protoObj->GetFloat(obj_f_radius);
				if (protoRadius > 0.0)
				{
					radius = protoRadius;
					obj->SetFloat(obj_f_radius, protoRadius);
				}
			}

			if (!base) {
				radius = AdjustRadiusSize(radius, baseSize, curSize);
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

	logger->debug("GetRadius: {} Radius not yet set, now calculating.", handle);
	
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
		dispIo.factor = 1.8;
		bonlist.AddBonus(modelScale, 1, 102); // initial value

		auto dispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
		if (dispatcher->IsValid()){
			dispatcher->Process(dispTypeGetModelScale, DK_NONE, &dispIo);
		}
		modelScale = bonlist.GetBaseScaled(dispIo.factor);
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
int Objects::GetHitDiceNum(objHndl handle, bool getBase) {
	auto obj = objSystem->GetObject(handle);
	auto result = 0;
	if (getBase) {
		result = obj->GetInt32Array(obj_f_critter_level_idx).GetSize();
	}
	else {
		result = dispatch.Dispatch61GetLevel(handle, Stat::stat_level);
	}

	if (obj->IsNPC()) {
		result += obj->GetInt32(obj_f_npc_hitdice_idx, 0);
	}
	else { // HD for new PC races
		Dice racialHd = d20RaceSys.GetHitDice(critterSys.GetRace(handle, false));
		auto racialHdCount = racialHd.GetCount();
		result += racialHdCount;
	}
	
	return result;
}

int Objects::GetSize(objHndl handle, bool getBase) {
	return dispatch.DispatchGetSizeCategory(handle, getBase);
}

int Objects::GetSizeOffset(objHndl handle) {
	auto base = GetSize(handle, true);
	auto curr = GetSize(handle, false);
	return curr - base;
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
	return GetRotationTowardsLoc(from, GetLocationFull(to));
}

float Objects::GetRotationTowardsLoc(objHndl from, LocAndOffsets & locTo) {
	auto locFrom = GetLocationFull(from);

	auto rot = 5*M_PI/4 - AngleBetweenPoints(locFrom, locTo);
	if (rot < 0)
		rot = rot + 2 * M_PI;
	return (float) rot;
}

void Objects::FadeTo(objHndl handle, int targetOpacity, int tickTimeMs, int tickOpacityQuantum, int callbackMode) const
{
	auto obj = objSystem->GetObject(handle);
	auto cur = obj->GetInt32(obj_f_transparency);
	if (cur != targetOpacity){

		gameSystems->GetObjFade().SetValidationObj(handle);

		gameSystems->GetTimeEvent().Remove(TimeEventType::ObjFade, [](const TimeEvent & evt){
			if (evt.params[1].handle != gameSystems->GetObjFade().GetValidationObj())
				return false;
			gameSystems->GetObjFade().RemoveFromTable(evt.params[0].int32);
			return true;
		});

		auto newId = gameSystems->GetObjFade().AppendToTable(tickOpacityQuantum, cur, targetOpacity, tickTimeMs, callbackMode);
		GameTime evtTime(0, tickTimeMs);
		TimeEvent evt;
		evt.system = TimeEventType::ObjFade;
		evt.params[0].int32 = newId;
		evt.params[1].handle = handle;
		gameSystems->GetTimeEvent().Schedule(evt, tickTimeMs);
	}
	if (callbackMode == 3){
		temple::GetRef<int(__cdecl)(objHndl, int)>(0x1006D890)(handle, 1);
	}
	return; // TRUE
}

void Objects::SetTransparency(objHndl handle, int amt)
{
	temple::GetRef<void(__cdecl)(objHndl, int)>(0x10020060)(handle, amt);
}

void Objects::Move(objHndl handle, LocAndOffsets toLocation) {
	_Move(handle, toLocation);
}

#include "combat.h"
#include "turn_based.h"
#include <objlist.h>
#include <dungeon_master.h>

SecretDoorFlag Objects::GetSecretDoorFlags(objHndl handle) {
	return (SecretDoorFlag) _GetInternalFieldInt32(handle, obj_f_secretdoor_flags);
}

void Objects::Destroy(objHndl ObjHnd) {
	static set<objHndl> destroyed;
	logger->info("Destroying {}", ObjHnd);
	if (destroyed.find(ObjHnd) != destroyed.end()) {
		logger->error("Double destroying object {}", ObjHnd);
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

	auto type = GetType(ObjHnd);
	if (!IsCritterType(type))
	{
		if (IsEquipmentType(type))
		{
			auto parentObj = inventory.GetParent(ObjHnd);
			if (parentObj)
			{
				auto loc = GetLocation(parentObj);
				inventory.ItemRemove(ObjHnd);
				auto moveObj = temple::GetPointer<void(objHndl, locXY)>(0x100252D0);
				inventory.MoveItem(ObjHnd, loc);
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

	gameSystems->GetAnim().ClearForObject(ObjHnd);
	
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

bool Objects::IsPlayerControlled(objHndl handle){

	if (!handle)
		return false;

	if (dmSys.IsControllingNpcs()) {
		return true;
	}

	if (!party.IsInParty(handle)) {
		return false;
	}

	auto obj = objSystem->GetObject(handle);

	auto partyCount = party.GetLivingPartyMemberCount();

	if (obj->IsPC()){
		if (partyCount <= 1){ 		// ha! vanilla only checked this
			if (party.IsInParty(handle)) // vanilla didn't check this
				return true;
		}
	}

	if (party.ObjIsAIFollower(handle))
		return false;

	// check if charmed by someone
	auto leader = objHndl::null;
	if (d20Sys.d20Query(handle, DK_QUE_Critter_Is_Charmed)){
		leader = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Charmed);
		if (leader && !party.IsInParty(leader))
			return false;
	}

	// checked if afraid of someone & can see them
	if (d20Sys.d20Query(handle, DK_QUE_Critter_Is_Afraid)){
		objHndl fearer;
		fearer = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Afraid);
		if (fearer && locSys.DistanceToObj(handle, fearer) < 40.0 
			&& combatSys.HasLineOfAttack(fearer, handle)){
			return false;
		}
	}

	if (d20Sys.d20Query(handle, DK_QUE_Critter_Is_AIControlled)
		|| d20Sys.d20Query(handle, DK_QUE_Critter_Is_Confused)){
		return false;
	}

	

	return true;
	//return _IsPlayerControlled(handle);
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
		|| IsCritterType(type)
		|| IsEquipmentType(type))
		return false;
	
	return (GetFlags(handle) & OF_DYNAMIC) == 0;

}

bool Objects::IsUntargetable(objHndl handle)
{
	if (GetFlags(handle) & (OF_OFF | OF_CLICK_THROUGH | OF_DONTDRAW)) {
		return true;
	}

	if (IsUndetectedSecretDoor(handle)) {
		return true;
	}

	if (aiSys.IsRunningOff(handle)) {
		return true;
	}

	return false;
}

/* 0x10074800 */
int Objects::StatLevelGet(objHndl obj, Stat stat)
{
	return _StatLevelGet(obj, stat);
		}

int Objects::StatLevelGet(objHndl obj, Stat stat, int statArg) 
{  // WIP currently just handles stat_caster_level expansion
	return d20Stats.GetValue(obj, stat, statArg);
}

int Objects::StatLevelGetBase(objHndl obj, Stat stat)
{
	return _StatLevelGetBase(obj, stat);
}

/* 0x1004E810 */
int Objects::StatLevelGetBaseWithModifiers(objHndl handle, Stat stat, DispIoBonusList*evtObj){

	auto objBody = objSystem->GetObject(handle);
	auto dispatcher = objBody->GetDispatcher();
	if (dispatcher->IsValid()){
		return dispatch.DispatchForCritter(handle, evtObj, dispTypeStatBaseGet, (D20DispatcherKey)(stat + 1));
	}
	else{
		return objects.StatLevelGetBase(handle, stat);
	}

	return 0;
}

int Objects::StatLevelSetBase(objHndl obj, Stat stat, int value)
{
	return _StatLevelSetBase(obj, stat, value);
}

int32_t Objects::GetMoneyAmount(objHndl handle){

	if (!handle)
		return 0;

	auto obj = objSystem->GetObject(handle);

	
	switch(obj->type){
	case obj_t_money:
		return obj->GetInt32(obj_f_money_quantity) * inventory.GetCoinWorth( obj->GetInt32(obj_f_money_type));
	case obj_t_pc:
		return party.GetMoney();	
	}

	if (obj->IsNPC())
		return critterSys.MoneyAmount(handle);

	if (obj->IsContainer()){
		auto items = inventory.GetInventory(handle);
		auto totalWorth = 0;
		for (auto item: items){
			auto itemObj = objSystem->GetObject(item);
			if (itemObj->type == obj_t_money)
				totalWorth += GetMoneyAmount(item);
		}
		return totalWorth;
	}

	return 0;
}


Dispatcher * Objects::GetDispatcher(objHndl handle)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj)
		return nullptr;
	return obj->GetDispatcher();
}

void Objects::SetDispatcher(objHndl handle, uint32_t data32)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj)
		return;
	obj->SetDispatcher(reinterpret_cast<Dispatcher*>(data32));
}

int Objects::GetModFromStatLevel(int statLevel)
{
	return (statLevel - 10) >> 1;
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

BOOL Objects::IsPortalLocked(const objHndl& handle){
	return temple::GetRef<BOOL(__cdecl)(objHndl)>(0x1001FD70)(handle);
}

bool Objects::OpenLock(objHndl handle, bool isTimeEvent, bool openAdjacent)
{
	if (!handle)
		return FALSE;

	auto obj = objSystem->GetObject(handle);
	auto protoId = objSystem->GetProtoId(handle);
	if (protoId == 1000  // generic door
		|| (obj->type != obj_t_container && obj->type != obj_t_portal)) {
		return FALSE;
	}

	auto flagsField = obj_f_portal_flags;
	if (obj->type == obj_t_container)
		flagsField = obj_f_container_flags;

	auto flags = obj->GetInt32(flagsField);
	if (!config.disableDoorRelocking
		&& (flags & PortalFlag::OPF_LOCKED)) { // this is the same enum value for container
		TimeEvent evt;
		evt.system = TimeEventType::Lock;
		evt.params[0].handle = handle;
		gameSystems->GetTimeEvent().Schedule(evt, 3600000);
	}

	flags &= ~OPF_LOCKED;
	if (isTimeEvent && !config.disableDoorRelocking) {
		flags |= OPF_LOCKED;
	}
	obj->SetInt32(flagsField, flags);

	// added: open adjacent door (for double doors)
	if (openAdjacent && !isTimeEvent && obj->type == obj_t_portal) {
		auto loc = obj->GetLocationFull();
		
		// check ±5 tiles away in X/Y directions
		TileRect rects[2];
		rects[0].x1 = max(0, (int)loc.location.locx - 5 );
		rects[0].x2 = loc.location.locx + 5;
		rects[0].y1 = rects[0].y2 = loc.location.locy;

		rects[1].y1 = max(0, (int)loc.location.locy - 5);
		rects[1].y2 = loc.location.locy + 5;
		rects[1].x1 = rects[1].x2 = loc.location.locx;

		for (auto ri = 0; ri < 2; ++ri) {
			ObjList objList;
			objList.ListRect(rects[ri], ObjectListFilter::OLC_PORTAL);
			for (int i = 0; i < objList.size(); i++) {
				auto doorhandle = objList[i];
				if (handle == doorhandle)
					continue;
				auto distance = locSys.DistanceToObj(doorhandle, handle); // regards radius; double doors should have near 0 distance (or even negative distance) due to this
				if (distance > 1) // disregard far doors
					continue;
				OpenLock(doorhandle, false, false);
			}
		}
	}


	return IsPortalLocked(handle) != 0;
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

		replaceFunction<float(objHndl)>(0x10021C40, [](objHndl obj) {
				return objects.GetRadius(obj);
		});

		// obj_update_radius
		replaceFunction<void(objHndl, temple::AasHandle)>(0x10021500, [](objHndl objId, temple::AasHandle animId) {
			auto anim = gameSystems->GetAAS().BorrowByHandle(animId);
			if (anim) {
				objects.UpdateRadius(objId, *anim);
			}
		});

		// ObjIsUntargetable
		replaceFunction<BOOL(objHndl)>(0x1001fcb0, [](objHndl obj) {
			return objects.IsUntargetable(obj) ? TRUE : FALSE;
		});

		// anim_obj_set_aas_anim_id
		replaceFunction<int(objHndl, gfx::EncodedAnimId)>(0x10021d50, [](objHndl objId, gfx::EncodedAnimId animId) {
			objects.SetAnimId(objId, animId);
			return 0;
		});

		// obj_has_anim
		replaceFunction<int(objHndl, gfx::EncodedAnimId)>(0x10021d20, [](objHndl handle, gfx::EncodedAnimId animId) {
			return objects.HasAnimId(handle, animId) ? 1 : 0;
		});

		replaceFunction(0x1004E7F0, _abilityScoreLevelGet);
		replaceFunction(0x100257A0, _destroy);
		//orgMove = replaceFunction(0x10025950, Move);
		//orgMoveUpdateLoc = replaceFunction(0x100C1990, MoveUpdateLoc);

		// obj_get_closest_distance_from_mesh
		// The respective goal function that called this function was replaced and now calls AAS directly
		breakRegion(0x10022a30, 0x10022BB6);

		writeCall(0x10022AEF, HookedGetModelScale); 
		writeCall(0x100228A5, HookedGetModelScale); // RayCast
		writeCall(0x10023F1F, HookedGetModelScale); // Render related
		writeCall(0x10021E9F, HookedGetModelScale);
		
		// OpenLock
		static BOOL(__cdecl*orgRelockPortalSchedule)(objHndl, int) = replaceFunction<BOOL(__cdecl)(objHndl, int)>(0x1001FE40, [](objHndl handle, int isTimeEvent)->BOOL {
			auto openAdjacent = true; // fix for when opening double doors
			return objects.OpenLock(handle, isTimeEvent != 0, openAdjacent) ? TRUE : FALSE;
		});

		// Fixes issue with Troll PC characters dying to cloudkill despite level adj
		replaceFunction<int(__cdecl)(objHndl)>(0x100801D0, [](objHndl handle)->int {
			return objects.GetHitDiceNum(handle);
			});


		replaceFunction<BOOL(__cdecl)(objHndl)>(0x1002B390, [](objHndl handle) ->BOOL {
			return objects.IsPlayerControlled(handle) ? TRUE:FALSE;
			});
}
} objReplacements;

int(*ObjectReplacements::orgMove)(objHndl, LocAndOffsets);
#pragma endregion
