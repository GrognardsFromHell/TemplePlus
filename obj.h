#pragma once

#include "temple_functions.h"
#include "temple_enums.h"
#include "dispatcher.h"



#pragma pack(push, 1)
/*
struct ObjectId {
	uint16_t subtype;
	uint16_t something;
	uint32_t field4;
	GUID guid;
};*/

struct TransientProps {
	uint32_t renderColor;
	uint32_t renderColors;
	uint32_t renderPalette;
	uint32_t renderScale;
	uint32_t renderAlpha;
	uint32_t renderX;
	uint32_t renderY;
	uint32_t renderWidth;
	uint32_t renderHeight;
	uint32_t palette;
	uint32_t color;
	uint32_t colors;
	uint32_t renderFlags;
	uint32_t tempId;
	uint32_t lightHandle;
	uint32_t overlayLightHandles;
	uint32_t internalFlags;
	uint32_t findNode;
	uint32_t animationHandle;
	uint32_t grappleState;
};

struct GameObjectBody {
	uint32_t type;
	uint32_t field4;
	ObjectId id;
	ObjectId protoId;
	uint64_t protoHandle;
	uint32_t field40;
	uint16_t propCollectionHas;
	uint16_t propCollectionItems;
	uint32_t propBitmap1;
	uint32_t propBitmap2;
	uint32_t propCollection;
	TransientProps transientProps;
	uint32_t padding;
};

struct GameObject {
	uint32_t header;
	GameObjectBody body;
};
#pragma pack(pop)



extern struct Objects : AddressTable {
	Objects();

	// Retrieves the object flags for the given object handle
	uint32_t GetFlags(objHndl obj) {
		return _GetInternalFieldInt32(obj, obj_f_flags);
	}

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

	Dispatcher* GetDispatcher(objHndl obj) {
		return (Dispatcher *)(_GetInternalFieldInt32(obj, obj_f_dispatcher));
	}

	uint32_t StatLevelGet(objHndl obj, Stat stat)
	{
		return _StatLevelGet(obj, stat);
	};

	uint32_t GetInt32(objHndl obj, obj_f fieldIdx)
	{
		return _GetInternalFieldInt32(obj, fieldIdx);
	};

	void SetDispatcher(objHndl obj, uint32_t data32) {
		_SetInternalFieldInt32(obj, obj_f_dispatcher, data32);
		return;
	}

private:
	int(__cdecl *_GetInternalFieldInt32)(objHndl ObjHnd, int nFieldIdx);
	int64_t(__cdecl *_GetInternalFieldInt64)(objHndl ObjHnd, int nFieldIdx);
	int32_t(__cdecl *_StatLevelGet)(objHndl ObjHnd, Stat);
	void(__cdecl *_SetInternalFieldInt32)(objHndl objHnd, obj_f fieldIdx, uint32_t data32);
} objects;

// const auto TestSizeOfDispatcher = sizeof(Dispatcher); // 0x138 as it should be

