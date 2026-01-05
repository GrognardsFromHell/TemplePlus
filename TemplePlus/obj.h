#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20.h"
#include "feat.h"
#include "description.h"
#include "faction.h"
#include "inventory.h"
#include "dice.h"
#include "gamesystems/objects/gameobject.h"

//forward declarations
struct LocationSys;
struct Pathfinding;
struct LegacySkillSystem;
struct FloatLineSystem;

namespace gfx {
	struct AnimatedModelParams;
	using AnimatedModelPtr = std::shared_ptr<class AnimatedModel>;
	class EncodedAnimId;
	}


struct Objects : temple::AddressTable {
	friend struct LegacyCritterSystem;

	// Retrieves the object flags for the given object handle
	ObjectFlag GetFlags(objHndl obj) {
		return (ObjectFlag)getInt32(obj, obj_f_flags);
	}
	void SetFlags(objHndl obj, ObjectFlag flags) {
		setInt32(obj, obj_f_flags, flags);
	}
	uint32_t getInt32(objHndl obj, obj_f field);
	uint64_t getInt64(objHndl obj, obj_f field);
	objHndl getObjHnd(objHndl obj, obj_f field);
	void SetFieldString(objHndl obj, obj_f field, char* value);
	const char* getString(objHndl obj, obj_f field);
	void SetFieldObjHnd(objHndl obj, obj_f field, objHndl value);
	void setInt32(objHndl obj, obj_f field, uint32_t dataIn);
	int32_t getArrayFieldInt32(objHndl obj, obj_f field, uint32_t index);
	int32_t getArrayFieldInt32Size(objHndl obj, obj_f field);
	int64_t getArrayFieldInt64(objHndl obj, obj_f field, uint32_t index);
	int32_t getArrayFieldInt64Size(objHndl obj, obj_f field);
	objHndl getArrayFieldObj(objHndl obj, obj_f field, uint32_t index);
	int GetAasHandle(objHndl handle);
	gfx::AnimatedModelPtr GetAnimHandle(objHndl obj);
	gfx::AnimatedModelParams GetAnimParams(objHndl obj);

	void ClearAnim(objHndl handle);
	void SetAnimId(objHndl obj, gfx::EncodedAnimId id);
	bool HasAnimId(objHndl obj, gfx::EncodedAnimId id);
	gfx::EncodedAnimId GetIdleAnim(objHndl obj);
	bool IsDoorOpen(objHndl obj);
	PortalFlag GetPortalFlags(objHndl obj) {
		return (PortalFlag)getInt32(obj, obj_f_portal_flags);
	}

	uint32_t abilityScoreLevelGet(objHndl, Stat, DispIO *dispIO = nullptr);
	locXY GetLocation(objHndl handle) {
		return locXY::fromField(_GetInternalFieldInt64(handle, obj_f_location));
	}
	LocAndOffsets GetLocationFull(objHndl handle) {
		LocAndOffsets result;
		result.location = locXY::fromField(_GetInternalFieldInt64(handle, obj_f_location));
		result.off_x = GetOffsetX(handle);
		result.off_y = GetOffsetY(handle);
		return result;
	}
	float GetOffsetX(objHndl handle) {
		return _GetInternalFieldFloat(handle, obj_f_offset_x);
	}
	float GetOffsetY(objHndl handle) {
		return _GetInternalFieldFloat(handle, obj_f_offset_y);
	}
	float GetOffsetZ(objHndl handle) {
		return _GetInternalFieldFloat(handle, obj_f_offset_z);
	}
	int GetOriginMapId(objHndl handle) {
		return _GetInternalFieldInt32(handle, obj_f_critter_teleport_map);
	}
	void SetOriginMapId(objHndl handle, int mapId) {
		_SetInternalFieldInt32(handle, obj_f_critter_teleport_map, mapId);
	}
	int GetNameId(objHndl handle) {
		return _GetInternalFieldInt32(handle, obj_f_name);
	}
	float GetRadius(objHndl handle, bool base = false);
	void SetRadius(objHndl handle, float radius) {
		_SetInternalFieldFloat(handle, obj_f_radius, radius);
	}
	float GetRenderHeight(objHndl handle) {
		return _GetInternalFieldFloat(handle, obj_f_3d_render_height);
	}
	void SetRenderHeight(objHndl handle, float renderHeight) {
		_SetInternalFieldFloat(handle, obj_f_3d_render_height, renderHeight);
	}
	float GetRotation(objHndl handle) {
		return _GetInternalFieldFloat(handle, obj_f_rotation);
	}
	float GetRotationPitch(objHndl handle) {
		return _GetInternalFieldFloat(handle, obj_f_rotation_pitch);
	}
	int GetScalePercent(objHndl handle);
	void SetRotation(objHndl handle, float rotation);

	ItemFlag GetItemFlags(objHndl handle) {
		return (ItemFlag)_GetInternalFieldInt32(handle, obj_f_item_flags);
	}
	OIF_WEAR GetItemWearFlags(objHndl handle) {
		return (OIF_WEAR)_GetInternalFieldInt32(handle, obj_f_item_wear_flags);
	}
	WeaponTypes GetWeaponType(objHndl handle) {
		return (WeaponTypes)_GetInternalFieldInt32(handle, obj_f_weapon_type);
	}
	int GetItemInventoryLocation(objHndl handle) {
		return _GetInternalFieldInt32(handle, obj_f_item_inv_location);
	}

	int GetScript(objHndl handle, int index);
	void SetScript(objHndl handle, int index, int scriptId);
	
	ObjectScript GetScriptAttachment(objHndl handle, int index);
	void SetScriptAttachment(objHndl handle, int index, const ObjectScript &script);

	Dice GetHitDice(objHndl handle); // This only makes sense for NPCs
	int GetHitDiceNum(objHndl handle, bool getBase = true); // getBase = true is vanilla behavior; otherwise this does a dispatch taking into account negative levels

	int GetSize(objHndl handle, bool getBase = false);

	// Gets the number of steps by which size-changing magic has modified the
	// object. Positive is increase, negative is decrease.
	int GetSizeOffset(objHndl handle);
	
	// Get NPC reaction towards another critter
	int GetReaction(objHndl of, objHndl towards) {
		return _GetReaction(of, towards);
	}

	// Adjust NPC reaction towards another critter
	void AdjustReaction(objHndl of, objHndl towards, int adjustment) {
		_AdjustReaction(of, towards, adjustment);
	}

	int GetDeity(objHndl critter) {
		return _StatLevelGet(critter, stat_deity);
	}

	/*
		Creates a new object at the given location.
		Returns the null handle if creation fails.
	*/
	objHndl Create(objHndl proto, locXY tile);

	/*
		Attempts to find a good free spot at the given location for an object with the
		given radius. If a spot was found, it returns true.
	*/
	bool FindFreeSpot(LocAndOffsets location, float radius, LocAndOffsets &freeSpotOut);

	/*
		Returns the handle for a prototype given its number from proto.tab
	*/
	objHndl GetProtoHandle(int protoNumber);

	/*
		Moves an object to a new position.
	*/
	void Move(objHndl handle, LocAndOffsets toLocation);

	/*
		Move this to AI? 
		Seems to force a critter to seek to move in order to avoid overlap with other objects.
	*/
	bool AiForceSpreadOut(objHndl handle);

	/*
		Same as above, but allows specifying another location than the current obj location.
		The location also seems to receive the final location the obj will be moved to by the
		function.		
	*/
	bool AiForceSpreadOut(objHndl handle, LocAndOffsets &location);
	
	locXY TargetRandomTileNear(objHndl handle, int distance);

	/*
		Calculates the rotation for obj from when it is facing object "to" directly.
		The angle is defined differently than the normal x,y polar coordinate - 
	    it increases ** clock wise **, and 0 is when the facing is towards the top of the screen.
	*/
	float GetRotationTowards(objHndl from, objHndl to);

	/*
	  As above, but toward an arbitrary location rather than an object.
	*/
	float GetRotationTowardsLoc(objHndl from, LocAndOffsets & locTo);
	
	/*
		Fades an object to a certain opacity, in time step tickTimeMs and opacity quantum tickQuantum
		callbackMode:
		0 - nothing special
		1 - unsets AiFlag::RunningOff and destroys at the end
		2 - unsets AiFlag::RunningOff and marks OF_OFF at the end and 
		3 - (should be as 1, but isn't??), will also poop items
	*/
	void FadeTo(objHndl obj, int targetOpacity, int tickTimeMs, int tickQuantum, int callbackMode = 0) const;
	void SetTransparency(objHndl handle, int amt);
	void SetFlag(objHndl obj, ObjectFlag flag) {
		_SetFlag(obj, flag);
	}
	void ClearFlag(objHndl obj, ObjectFlag flag) {
		_ClearFlag(obj, flag);
	}

	/*
		Toggles a portal open/closed state.
	*/
	void PortalToggleOpen(objHndl handle) {
		_PortalToggleOpen(handle);
	}

	void ContainerToggleOpen(objHndl handle) {
		_ContainerToggleOpen(handle);
	}

	bool IsUndetectedSecretDoor(objHndl handle) {
		auto flags = GetSecretDoorFlags(handle);
		return (flags & OSDF_SECRET_DOOR) && !(flags & OSDF_SECRET_DOOR_FOUND);
	}

	SecretDoorFlag GetSecretDoorFlags(objHndl handle);

	bool HasSpellEffects(objHndl obj) {
		return _HasSpellEffects(obj);
	}

	bool IsRenderHeightSet(objHndl handle) {
		return (GetFlags(handle) & OF_HEIGHT_SET) != 0;
	}

	bool IsRadiusSet(objHndl handle) {
		return (GetFlags(handle) & OF_RADIUS_SET) != 0;
	}

	void Destroy(objHndl obj);


#pragma region Common
	ObjectId GetId(objHndl handle);
	ObjectType GetType(objHndl obj);
	int32_t GetHPCur(objHndl obj);
	bool IsPortal(objHndl handle){
		return IsPortalType(GetType(handle));
	};
	bool IsPortalType(ObjectType type) const{
		return type == obj_t_portal;
	}
	bool IsCritter(objHndl obj) {
		return IsCritterType(GetType(obj));
	}
	bool IsCritterType(ObjectType type) const {
		return type == obj_t_npc || type == obj_t_pc;
	}
	bool IsContainer(objHndl objHnd) {
		return IsContainerType(GetType(objHnd));
	}
	bool IsContainerType(ObjectType type) const {
		return type == obj_t_container || type == obj_t_bag;
	}
	// TODO: Consider renaming to IsItem()
	bool IsEquipment(objHndl obj) {
		return IsEquipmentType(GetType(obj));
	}
	bool IsEquipmentType(ObjectType type) const {
		return type >= obj_t_weapon && type <= obj_t_generic || type == obj_t_bag;
	}
	bool IsStaticType(ObjectType type) const {
		return type != obj_t_projectile && type != obj_t_container && !IsCritterType(type) && !IsEquipmentType(type);
	}
	bool IsNPC(objHndl obj) {
		return GetType(obj) == obj_t_npc;
	}
	bool IsPlayerControlled(objHndl obj);
	std::string GetDisplayName(objHndl obj, objHndl observer);
	bool IsStatic(objHndl handle);

	/**
	 * Is the object not targetable by mouse?
	 * Was @ 1001FCB0
	 */
	bool IsUntargetable(objHndl handle);

	int StatLevelGet(objHndl obj, Stat stat);
	int StatLevelGet(objHndl obj, Stat stat, int statArg);  // WIP currently just handles stat_caster_level expansion
	int StatLevelGetBase(objHndl obj, Stat stat); // can return floating point numbers too (e.g. movement speed)
	int StatLevelGetBaseWithModifiers(objHndl obj, Stat stat, DispIoBonusList*evtObj = nullptr); // this returns the base stat including racial modifiers and such
	int StatLevelSetBase(objHndl obj, Stat stat, int value);
	int32_t GetMoneyAmount(objHndl handle); // get total money amount, in Copper Pieces; for PCs, returns party money, for containers, gets total money inside, and for coins, gets the quantity and multiplies by worth
#pragma endregion

#pragma region Dispatcher Stuff

	Dispatcher* GetDispatcher(objHndl obj);
	void SetDispatcher(objHndl obj, uint32_t data32);

	int GetModFromStatLevel(int statLevel); // returns modifier from stat level e.g. Dex 15 -> +2
	bool IsPortalOpen(objHndl obj);
	BOOL IsPortalLocked(const objHndl& handle);
	bool OpenLock(objHndl handle, bool isTimeEvent, bool openAdjacent = false);
	
	int GetTempId(objHndl handle);
	int GetAlpha(objHndl handle);


	static int IsCritterProne(objHndl handle);
#pragma endregion

#pragma region Subsystems
	DispatcherSystem dispatch;

	//LegacyD20System d20;

	LegacySkillSystem * skill;

	LegacyFeatSystem feats;

	LegacyDescriptionSystem description;

	FactionSystem factions;

	InventorySystem inventory;

	LocationSys * loc;

	Pathfinding * pathfinding;

	FloatLineSystem  * floats;
#pragma endregion

	void UpdateRenderHeight(objHndl obj, gfx::AnimatedModel &anim);
	void UpdateRadius(objHndl obj, gfx::AnimatedModel &anim);

	Objects();
#pragma region Privates
private:
	ObjectId *(__cdecl *_GetId)(ObjectId *pIdOut, objHndl handle);
	int(__cdecl *_GetReaction)(objHndl of, objHndl towards);
	void(__cdecl *_AdjustReaction)(objHndl of, objHndl towards, int change);
	void(__cdecl *_GetDisplayName)(objHndl obj, objHndl observer, char *pNameOut);
	float(__cdecl *_GetRadius)(objHndl ObjHnd);

	int(__cdecl *_GetInternalFieldInt32)(objHndl ObjHnd, int nfield);
	int(__cdecl *_GetInternalFieldInt32Array)(objHndl ObjHnd, int nfield, int index);
	float(__cdecl *_GetInternalFieldFloat)(objHndl ObjHnd, int nfield);
	int64_t(__cdecl *_GetInternalFieldInt64)(objHndl ObjHnd, int nfield);
	int32_t(__cdecl *_StatLevelGet)(objHndl ObjHnd, Stat);
	int(__cdecl *_StatLevelGetBase)(objHndl ObjHnd, Stat);
	int(__cdecl *_StatLevelSetBase)(objHndl ObjHnd, Stat, int);
	int(__cdecl *_GetSize)(objHndl handle);
	void(__cdecl *_SetInternalFieldInt32)(objHndl objHnd, obj_f field, uint32_t data32);
	void(__cdecl *_SetInternalFieldFloat)(objHndl objHnd, obj_f field, float data);
	bool(__cdecl * _IsPlayerControlled)(objHndl objHnd);
	uint32_t(__cdecl *_IsObjDeadNullDestroyed)(objHndl);
	void (__cdecl *_Move)(objHndl handle, LocAndOffsets toLocation);
	bool(__cdecl * _FindFreeSpot)(LocAndOffsets loc, float radius, LocAndOffsets &freeSpot);
	bool (__cdecl *_Create)(objHndl proto, locXY tile, objHndl *pHandleOut);
	bool (__cdecl *_AiForceSpreadOut)(objHndl handle, LocAndOffsets *location);
	char ** _DLLFieldNames;
	void (__cdecl *_TargetRandomTileNear)(objHndl handle, int distance, locXY *pLocOut);

	void(__cdecl *_FadeTo)(objHndl obj, int targetOpacity, int fadeTime, int unk1, int unk2);

	void (__cdecl *_SetFlag)(objHndl obj, ObjectFlag flag);
	void (__cdecl *_ClearFlag)(objHndl obj, ObjectFlag flag);
	void (__cdecl *_PortalToggleOpen)(objHndl handle);
	void (__cdecl *_ContainerToggleOpen)(objHndl handle);

	bool (__cdecl *_HasSpellEffects)(objHndl obj);

	void (__cdecl *_Destroy)(objHndl obj);

	int(__cdecl * _ObjectIdPrint)(char * printOut, ObjectId objId);

#pragma endregion
} ;

extern Objects objects;

// const auto TestSizeOfDispatcher = sizeof(Dispatcher); // 0x138 as it should be
