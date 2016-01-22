#pragma once

#include <gsl/gsl.h>

#include "temple_enums.h"


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
	
	bool IsBlocked() const {
		return subtype == ObjectIdKind::Blocked;
	}

	// Can this object id be persisted and later restored to a handle?
	bool IsPersistable() const {
		return IsNull() || IsPermanent() || IsPrototype() || IsPositional();
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

	// Randomly generates a GUID and returns an object id that contains it
	static ObjectId CreatePermanent();

	// Creates a positional object id
	static ObjectId CreatePositional(int mapId, int tileX, int tileY, int tempId);

	// Creates a prototype object id
	static ObjectId CreatePrototype(uint16_t prototypeId);

	// Creates a null object id
	static ObjectId CreateNull() {
		ObjectId result;
		result.subtype = ObjectIdKind::Null;
		return result;
	}

	static ObjectId CreateHandle(objHndl handle);

};
#pragma pack(pop)
