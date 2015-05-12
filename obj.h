#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20.h"
#include "feat.h"
#include "description.h"
#include "faction.h"
#include "feat.h"
#include "inventory.h"
#include "dice.h"


//forward declarations
struct LocationSys;
struct Pathfinding;
struct SkillSystem;
struct FloatLineSystem;

// Stored in obj_f_script_idx array
#pragma pack(push, 1)
struct ObjectScript {
	int unk1;
	uint32_t counters;
	int scriptId;
};
#pragma pack(pop)
struct FieldDataMax { uint32_t data[8]; }; // for wrapping "objSetField" calls that get input by value; this is the largest data size that I know of

struct Objects : AddressTable {
	friend struct CritterSystem;

	// Verifies if the handle is valid
	bool VerifyHandle(objHndl handle) {
		return _VerifyHandle(handle);
	}

	// Retrieves the object flags for the given object handle
	uint32_t GetFlags(objHndl obj) {
		return getInt32(obj, obj_f_flags);
	}
	uint32_t getInt32(objHndl obj, obj_f fieldIdx);
	uint64_t getInt64(objHndl obj, obj_f fieldIdx);
	objHndl getObjHnd(objHndl obj, obj_f fieldIdx);
	void setInt32(objHndl obj, obj_f fieldIdx, uint32_t dataIn);
	void setArrayFieldByValue(objHndl obj, obj_f fieldIdx, uint32_t subIdx, FieldDataMax data);
	int32_t getArrayFieldInt32(objHndl obj, obj_f fieldIdx, uint32_t subIdx);
	void getArrayField(objHndl obj, obj_f fieldIdx, uint32_t subIdx, void * dataOut);
	uint32_t getArrayFieldNumItems(objHndl obj, obj_f fieldIdx);

	uint32_t abilityScoreLevelGet(objHndl, Stat, DispIO *);
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
	int GetOriginMapId(objHndl handle) {
		return _GetInternalFieldInt32(handle, obj_f_critter_teleport_map);
	}
	void SetOriginMapId(objHndl handle, int mapId) {
		_SetInternalFieldInt32(handle, obj_f_critter_teleport_map, mapId);
	}
	int GetNameId(objHndl handle) {
		return _GetInternalFieldInt32(handle, obj_f_name);
	}
	float GetRadius(objHndl handle) {
		return _GetRadius(handle);
	}
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
	void SetRotation(objHndl handle, float rotation);

	int GetScript(objHndl handle, int index);
	void SetScript(objHndl handle, int index, int scriptId);
	
	ObjectScript GetScriptAttachment(objHndl handle, int index);
	void SetScriptAttachment(objHndl handle, int index, const ObjectScript &script);

	Dice GetHitDice(objHndl handle); // This only makes sense for NPCs
	int GetHitDiceNum(objHndl handle);
	int GetSize(objHndl handle);
	
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
	*/
	float GetRotationTowards(objHndl from, objHndl to);

	/*
		Fades an object to a certain opacity.
	*/
	void FadeTo(objHndl obj, int targetOpacity, int fadeTimeInMs, int unk1, int unk2);
	
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

	bool SecretdoorDetect(objHndl door, objHndl viewer) {
		return _SecretdoorDetect(door, viewer);
	}

	bool HasSpellEffects(objHndl obj) {
		return _HasSpellEffects(obj);
	}

	void Destroy(objHndl obj) {
		_Destroy(obj);
	}

#pragma region Common
	ObjectId GetId(objHndl handle);
	objHndl GetHandle(const ObjectId &id);
	ObjectType GetType(objHndl obj);
	uint32_t IsDeadNullDestroyed(objHndl obj);
	uint32_t IsUnconscious(objHndl obj);
	int32_t GetHPCur(objHndl obj);
	bool IsCritter(objHndl obj);
	bool IsNPC(objHndl obj);
	bool IsPlayerControlled(objHndl obj);
	uint32_t ObjGetProtoNum(objHndl obj);
	string GetDisplayName(objHndl obj, objHndl observer);

	uint32_t StatLevelGet(objHndl obj, Stat stat);
	int StatLevelGetBase(objHndl obj, Stat stat);
	int StatLevelSetBase(objHndl obj, Stat stat, int value);
#pragma endregion

#pragma region Dispatcher Stuff

	Dispatcher* GetDispatcher(objHndl obj) {
		return (Dispatcher *)getInt32(obj, obj_f_dispatcher);
	}

	void SetDispatcher(objHndl obj, uint32_t data32) {
		setInt32(obj, obj_f_dispatcher, data32);
		return;
	}

	
#pragma endregion

#pragma region Subsystems
	DispatcherSystem dispatch;

	D20System d20;

	SkillSystem * skill;

	FeatSystem feats;

	DescriptionSystem description;

	FactionSystem factions;

	InventorySystem inventory;

	LocationSys * loc;

	Pathfinding * pathfinding;

	FloatLineSystem  * floats;
#pragma endregion

#pragma region Memory Internals
	uint32_t DoesTypeSupportField(uint32_t objType, _fieldIdx objField);

	void PropFetcher(GameObjectBody* objBody, obj_f fieldIdx, void * dataOut);

	void InsertDataIntoInternalStack(GameObjectBody * objBody, obj_f fieldIdx, void * dataIn);

	objHndl lookupInHandlesList(ObjectId objId);// 100C3050

	int ObjectIdPrint(char * printOut, ObjectId objId);
#pragma endregion 

	Objects();
#pragma region Privates
private:
	bool (__cdecl *_VerifyHandle)(objHndl handle);
	ObjectId *(__cdecl *_GetId)(ObjectId *pIdOut, objHndl handle);
	objHndl (__cdecl *_GetHandle)(ObjectId id);
	int(__cdecl *_GetReaction)(objHndl of, objHndl towards);
	void(__cdecl *_AdjustReaction)(objHndl of, objHndl towards, int change);
	void(__cdecl *_GetDisplayName)(objHndl obj, objHndl observer, char *pNameOut);
	float(__cdecl *_GetRadius)(objHndl ObjHnd);
	void setArrayFieldLowLevel(GameObjectBody * objBody, void * sourceData, obj_f fieldIdx, uint32_t subIdx);
	void fieldNonexistantDebug(objHndl obj, GameObjectBody* objBody, obj_f fieldIdx, uint32_t objType, char* accessType);
	void getArrayFieldInternal(GameObjectBody * objBody, void * outAddr, obj_f fieldIdx, uint32_t subIdx); // _nFieldIdx@<eax>, _nFieldSubIdx@<ecx>

	int(__cdecl *_GetInternalFieldInt32)(objHndl ObjHnd, int nFieldIdx);
	int(__cdecl *_GetInternalFieldInt32Array)(objHndl ObjHnd, int nFieldIdx, int index);
	float(__cdecl *_GetInternalFieldFloat)(objHndl ObjHnd, int nFieldIdx);
	int64_t(__cdecl *_GetInternalFieldInt64)(objHndl ObjHnd, int nFieldIdx);
	int32_t(__cdecl *_StatLevelGet)(objHndl ObjHnd, Stat);
	int(__cdecl *_StatLevelGetBase)(objHndl ObjHnd, Stat);
	int(__cdecl *_StatLevelSetBase)(objHndl ObjHnd, Stat, int);
	int(__cdecl *_GetSize)(objHndl handle);
	void(__cdecl *_SetInternalFieldInt32)(objHndl objHnd, obj_f fieldIdx, uint32_t data32);
	uint32_t(__cdecl *_getArrayFieldNumItems)(objHndl obj, obj_f fieldIdx);// 1009E7E0
	void(__cdecl * _setArrayFieldLowLevel)(obj_f fieldIdx, uint32_t subIdx); // GameObjectBody *@<ecx>, sourceData *@<eax>
	void(__cdecl *_SetInternalFieldFloat)(objHndl objHnd, obj_f fieldIdx, float data);
	bool(__cdecl * _IsPlayerControlled)(objHndl objHnd);
	uint32_t(__cdecl *_ObjGetProtoNum)(objHndl);
	uint32_t(__cdecl *_IsObjDeadNullDestroyed)(objHndl);
	GameObjectBody * (__cdecl *_GetMemoryAddress)(objHndl ObjHnd);
	bool(__cdecl *_DoesObjectFieldExist)();
	void(__cdecl * _ObjectPropFetcher)();
	void (__cdecl *_Move)(objHndl handle, LocAndOffsets toLocation);
	bool(__cdecl * _FindFreeSpot)(LocAndOffsets loc, float radius, LocAndOffsets &freeSpot);
	bool (__cdecl *_Create)(objHndl proto, locXY tile, objHndl *pHandleOut);
	bool (__cdecl *_AiForceSpreadOut)(objHndl handle, LocAndOffsets *location);
	void (__cdecl *_getArrayFieldInternal)(GameObjectBody * objBody, void * out); // _nFieldIdx@<eax>, _nFieldSubIdx@<ecx>
	objHndl(__cdecl*_lookupInHandlesList)(ObjectId objId);
	char ** _DLLFieldNames;
	void(__cdecl * _InsetDataIntoInternalStack)();//(int nFieldIdx, void *, ToEEObjBody *@<eax>);
	void (__cdecl *_TargetRandomTileNear)(objHndl handle, int distance, locXY *pLocOut);

	void(__cdecl *_FadeTo)(objHndl obj, int targetOpacity, int fadeTime, int unk1, int unk2);

	void (__cdecl *_SetFlag)(objHndl obj, ObjectFlag flag);
	void (__cdecl *_ClearFlag)(objHndl obj, ObjectFlag flag);
	void (__cdecl *_PortalToggleOpen)(objHndl handle);
	void (__cdecl *_ContainerToggleOpen)(objHndl handle);

	bool (__cdecl *_SecretdoorDetect)(objHndl door, objHndl viewer);
	bool (__cdecl *_HasSpellEffects)(objHndl obj);

	void (__cdecl *_Destroy)(objHndl obj);

	int(__cdecl * _ObjectIdPrint)(char * printOut, ObjectId objId);
#pragma endregion
} ;

extern Objects objects;

// const auto TestSizeOfDispatcher = sizeof(Dispatcher); // 0x138 as it should be

uint32_t _obj_get_int(objHndl obj, obj_f fieldIdx);
void _obj_set_int(objHndl obj, obj_f fieldIdx, uint32_t dataIn);
uint32_t _abilityScoreLevelGet(objHndl obj, Stat abScore, DispIO * dispIO);
void _setArrayFieldByValue(objHndl obj, obj_f fieldIdx, uint32_t subIdx, FieldDataMax data);
int32_t _getArrayFieldInt32(objHndl obj, obj_f fieldIdx, uint32_t subIdx);
uint64_t __cdecl _getInt64(objHndl obj, obj_f fieldIdx);
objHndl __cdecl _getObjHnd(objHndl obj, obj_f fieldIdx);