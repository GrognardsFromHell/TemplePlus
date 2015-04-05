#pragma once

#include "temple_functions.h"
#include "temple_enums.h"

struct SubDispDef;
struct CondStruct;
struct DispatcherCallbackArgs;
struct Dispatcher;

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

struct DispIO {
	enum_dispIO_type dispIOType;
};

struct CondNode : TempleAlloc {
	CondStruct* condStruct;
	CondNode* nextCondNode;
	uint32_t flags;
	uint32_t args[6];

	explicit CondNode(CondStruct *cond);
};

struct SubDispNode : TempleAlloc {
	SubDispDef* subDispDef;
	CondNode* condNode;
	SubDispNode* next;
};

struct SubDispDef {
	enum_disp_type dispType;
	uint32_t dispKey;
	void (__cdecl *dispCallback)(SubDispNode* subDispNode, objHndl objHnd, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO);
	uint32_t data1;
	uint32_t data2;
};

struct CondStruct {
	char* condName;
	int numArgs;
	SubDispDef subDispDefs;
};

struct DispatcherCallbackArgs {
	SubDispNode* subDispNode;
	objHndl objHndCaller;
	enum_disp_type dispType;
	uint32_t dispKey;
	DispIO* dispIO;
};

struct DispIO14h : DispIO {
	CondStruct* condStruct;
	uint32_t outputFlag;
	uint32_t arg1;
	uint32_t arg2;

	DispIO14h() {
		dispIOType = dispIOType0;
		condStruct = nullptr;
		outputFlag = 0;
		arg1 = 0;
		arg2 = 0;
	}
};

struct DispIO20h : DispIO {
	uint32_t interrupt;
	uint32_t field_8;
	uint32_t field_C;
	uint32_t val1;
	uint32_t val2;
	uint32_t okToAdd;
	CondNode* condNode;

	DispIO20h() {
		dispIOType = dispIOType0;
		condNode = nullptr;
		val1 = 0;
		val2 = 0;
		interrupt = 0;
	}
};

struct Dispatcher :TempleAlloc {
	objHndl objHnd;
	CondNode* attributeConds;
	CondNode* itemConds;
	CondNode* otherConds;
	SubDispNode* subDispNodes[ dispTypeCount ];
};

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

private:
	int(__cdecl *_GetInternalFieldInt32)(objHndl ObjHnd, int nFieldIdx);
	int64_t(__cdecl *_GetInternalFieldInt64)(objHndl ObjHnd, int nFieldIdx);
	int(__cdecl *_StatLevelGet)(objHndl ObjHnd, Stat);

} objects;

// const auto TestSizeOfDispatcher = sizeof(Dispatcher); // 0x138 as it should be

Dispatcher* DispatcherInit(objHndl objHnd);
void DispIO_Size32_Type21_Init(DispIO20h* dispIO);
uint32_t Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t Dispatch63(objHndl objHnd, DispIO* dispIO);
uint32_t ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
