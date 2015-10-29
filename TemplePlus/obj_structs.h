#pragma once

#include <gsl/gsl.h>

#pragma pack(push, 1)

typedef uint64_t objHndl;
typedef uint32_t _fieldIdx;
typedef uint32_t _fieldSubIdx;
typedef uint32_t _mapNum;
typedef uint32_t _key;

#pragma pack(push, 8)

enum class ObjectIdKind : uint16_t {
	Null = 0,
	Prototype = 1,
	Permanent = 2,
	Positional = 3,
	Handle = 0xFFFE,
	Blocked = 0xFFFF
};

union ObjectIdBody {
	GUID guid;
	uint32_t protoId;
	objHndl handle;
	struct {
		int x;
		int y;
		int tempId;
		int mapId;
	} pos;
};

struct ObjectId {
	ObjectIdKind subtype = ObjectIdKind::Null;
	int unk = 0;
	ObjectIdBody body;

	bool IsNull() const {
		return subtype == ObjectIdKind::Null;
	}

	bool IsPermanent() const {
		return subtype == ObjectIdKind::Permanent;
	}

	bool IsPrototype() const {
		return subtype == ObjectIdKind::Prototype;
	}
	
	bool IsPositional() const {
		return subtype == ObjectIdKind::Positional;
	}

	bool IsHandle() const {
		return subtype == ObjectIdKind::Handle;
	}

	int GetPrototypeId() const {
		Expects(IsPrototype());
		return body.protoId;
	}

	objHndl GetHandle() const {
		Expects(IsHandle());
		return body.handle;
	}

	operator bool() const {
		return !IsNull();
	}

	bool operator ==(const ObjectId &other) const;

	std::string ToString() const;

};
#pragma pack(pop)

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

#pragma pack(pop)