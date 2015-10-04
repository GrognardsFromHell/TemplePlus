#pragma once

#pragma pack(push, 1)
/*
struct ObjectId {
uint16_t subtype;
uint16_t something;
uint32_t field4;
GUID guid;
};*/




typedef uint64_t objHndl;
typedef uint32_t _fieldIdx;
typedef uint32_t _fieldSubIdx;
typedef uint32_t _mapNum;
typedef uint32_t _key;


struct ObjectId {
	uint16_t subtype;
	uint16_t something;
	uint32_t field4; // always zero from what I've seen
	GUID guid;
};

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