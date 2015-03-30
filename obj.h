
#pragma once

#include "temple_functions.h"
#include "temple_enums.h"

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

/*
	Flags supported by all object types.
	Query with objects.GetFlags
*/
enum ObjectFlag : uint32_t {
	OF_DESTROYED = 1,
	OF_OFF,
	OF_FLAT,
	OF_TEXT,
	OF_SEE_THROUGH,
	OF_SHOOT_THROUGH,
	OF_TRANSLUCENT,
	OF_SHRUNK,
	OF_DONTDRAW,
	OF_INVISIBLE,
	OF_NO_BLOCK,
	OF_CLICK_THROUGH,
	OF_INVENTORY,
	OF_DYNAMIC,
	OF_PROVIDES_COVER,
	OF_RANDOM_SIZE,
	OF_NOHEIGHT,
	OF_WADING,
	OF_UNUSED_40000,
	OF_STONED,
	OF_DONTLIGHT,
	OF_TEXT_FLOATER,
	OF_INVULNERABLE,
	OF_EXTINCT,
	OF_TRAP_PC,
	OF_TRAP_SPOTTED,
	OF_DISALLOW_WADING,
	OF_UNUSED_08000000,
	OF_HEIGHT_SET,
	OF_ANIMATED_DEAD,
	OF_TELEPORTED,
	OF_RADIUS_SET
};

enum ItemFlag : uint32_t {
	OIF_IDENTIFIED = 0x1,
	OIF_WONT_SELL = 0x2,
	OIF_IS_MAGICAL = 0x4,
	OIF_NO_PICKPOCKET = 0x8,
	OIF_NO_DISPLAY = 0x10,
	OIF_NO_DROP = 0x20,
	OIF_NEEDS_SPELL = 0x40,
	OIF_CAN_USE_BOX = 0x80,
	OIF_NEEDS_TARGET = 0x100,
	OIF_LIGHT_SMALL = 0x200,
	OIF_LIGHT_MEDIUM = 0x400,
	OIF_LIGHT_LARGE = 0x800,
	OIF_LIGHT_XLARGE = 0x1000,
	OIF_PERSISTENT = 0x2000,
	OIF_MT_TRIGGERED = 0x4000,
	OIF_STOLEN = 0x8000,
	OIF_USE_IS_THROW = 0x10000,
	OIF_NO_DECAY = 0x20000,
	OIF_UBER = 0x40000,
	OIF_NO_NPC_PICKUP = 0x80000,
	OIF_NO_RANGED_USE = 0x100000,
	OIF_VALID_AI_ACTION = 0x200000,
	OIF_DRAW_WHEN_PARENTED = 0x400000,
	OIF_EXPIRES_AFTER_USE = 0x800000,
	OIF_NO_LOOT = 0x1000000,
	OIF_USES_WAND_ANIM = 0x2000000,
	OIF_NO_TRANSFER = 0x4000000
};

enum PortalFlag : uint32_t {
	OPF_LOCKED = 0x1,
	OPF_JAMMED = 0x2,
	OPF_MAGICALLY_HELD = 0x4,
	OPF_NEVER_LOCKED = 0x8,
	OPF_ALWAYS_LOCKED = 0x10,
	OPF_LOCKED_DAY = 0x20,
	OPF_LOCKED_NIGHT = 0x40,
	OPF_BUSTED = 0x80,
	OPF_NOT_STICKY = 0x100,
	OPF_OPEN = 0x200
};

enum SecretDoorFlag : uint32_t {
	OSDF_DC_0 = 0x1,
	OSDF_DC_1 = 0x2,
	OSDF_DC_2 = 0x4,
	OSDF_DC_3 = 0x8,
	OSDF_DC_4 = 0x10,
	OSDF_DC_5 = 0x20,
	OSDF_DC_6 = 0x40,
	OSDF_RANK_0 = 0x80,
	OSDF_RANK_1 = 0x100,
	OSDF_RANK_2 = 0x200,
	OSDF_RANK_3 = 0x300,
	OSDF_RANK_4 = 0x400,
	OSDF_RANK_5 = 0x800,
	OSDF_RANK_6 = 0x1000,
	OSDF_SECRET_DOOR = 0x2000,
	OSDF_SECRET_DOOR_FOUND = 0x4000
};

extern struct Objects : AddressTable {
	Objects();

	// Retrieves the object flags for the given object handle
	uint32_t GetFlags(objHndl obj) {
		return _GetInternalFieldInt32(obj, 22);
	}

	ObjectType GetType(objHndl obj) {
		return static_cast<ObjectType>(_GetInternalFieldInt32(obj, obj_f_type));
	}

	bool IsCritter(objHndl obj) {
		auto type = GetType(obj);
		return type == obj_t_npc || type == obj_t_pc;
	}

private:
	int (__cdecl *_GetInternalFieldInt32)(objHndl ObjHnd, int nFieldIdx);
} objects;
