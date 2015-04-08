#pragma once

//#include "temple_functions.h"
#include "dispatcher.h"
#include "common.h"
#include "d20.h"
#include "feat.h"


struct Objects : AddressTable {
	Objects();

	// Retrieves the object flags for the given object handle
	uint32_t GetFlags(objHndl obj) {
		return _GetInternalFieldInt32(obj, obj_f_flags);
	}


#pragma region Common Attributes
	ObjectType GetType(objHndl obj) {
		return static_cast<ObjectType>(_GetInternalFieldInt32(obj, obj_f_type));
	}

	uint32_t GetRace(objHndl obj) {
		return _StatLevelGet(obj, stat_race);
	}

	bool IsCritter(objHndl obj) {
		auto type = GetType(obj);
		return type == obj_t_npc || type == obj_t_pc;
	}

	enum_monster_category GetCategory(objHndl objHnd)
	{
		if (objHnd != 0) {
			if (IsCritter(objHnd)) {
				auto monCat = _GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
				return (enum_monster_category)(monCat & 0xFFFFFFFF) ;
			}
		}
		return mc_type_monstrous_humanoid; // default - so they have at least a weapons proficiency
	};

	bool IsCategoryType(objHndl objHnd, enum_monster_category categoryType) {
		if (objHnd != 0) {
			if (IsCritter(objHnd)) {
				auto monCat = _GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
				return (monCat & 0xFFFFFFFF) == categoryType;
			}
		}
		return 0;
	}

	bool IsCategorySubtype(objHndl objHnd, enum_monster_category categoryType) {
		if (objHnd != 0) {
			if (IsCritter(objHnd)) {
				auto monCat = _GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
				return ((monCat >> 32) & 0xFFFFFFFF) == categoryType;
			}
		}
		return 0;
	}

	bool IsUndead(objHndl objHnd) {
		return IsCategoryType(objHnd, mc_type_undead);
	}

	bool IsOoze(objHndl objHnd) {
		return IsCategoryType(objHnd, mc_type_ooze);
	}

	bool IsSubtypeFire(objHndl objHnd) {
		return IsCategorySubtype(objHnd, mc_subtye_fire);
	}



	uint32_t StatLevelGet(objHndl obj, Stat stat)
	{
		return _StatLevelGet(obj, stat);
	};

	uint32_t GetInt32(objHndl obj, obj_f fieldIdx)
	{
		return _GetInternalFieldInt32(obj, fieldIdx);
	};

#pragma endregion

#pragma region Dispatcher Stuff

	Dispatcher* GetDispatcher(objHndl obj) {
		return (Dispatcher *)(_GetInternalFieldInt32(obj, obj_f_dispatcher));
	}

	void SetDispatcher(objHndl obj, uint32_t data32) {
		_SetInternalFieldInt32(obj, obj_f_dispatcher, data32);
		return;
	}

	DispatcherSystem dispatch;

	D20System d20;

	FeatSystem feats;

#pragma endregion

private:
	int(__cdecl *_GetInternalFieldInt32)(objHndl ObjHnd, int nFieldIdx);
	int64_t(__cdecl *_GetInternalFieldInt64)(objHndl ObjHnd, int nFieldIdx);
	int32_t(__cdecl *_StatLevelGet)(objHndl ObjHnd, Stat);
	void(__cdecl *_SetInternalFieldInt32)(objHndl objHnd, obj_f fieldIdx, uint32_t data32);
} ;

extern Objects objects;

// const auto TestSizeOfDispatcher = sizeof(Dispatcher); // 0x138 as it should be

