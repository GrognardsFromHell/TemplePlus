#pragma once

#include <gsl/gsl.h>
#include <iosfwd>

#include "temple_enums.h"

// Define objHndl as a struct that contains just the handle value
#pragma pack(push, 1)
struct objHndl {
	uint64_t handle;

	explicit operator bool() const {
		return !!handle;
	}

	objHndl& operator=(uint64_t handle) {
		this->handle = handle;
		return *this;
	}

	uint32_t GetHandleLower() const {
		return (uint32_t)(handle & 0xffffffff);
	}

	uint32_t GetHandleUpper() const {
		return (uint32_t)((handle >> 32) & 0xffffffff);
	}

	static objHndl FromUpperAndLower(uint32_t upper, uint32_t lower) {
		return{
			(((uint64_t) upper) << 32)
			| (((uint64_t)lower) & 0xFFFFFFFF)
		};
	}

	static const objHndl null;

};
#pragma pack(pop)

/**
 * Compares two object handles for equality. They are equal if their
 * handle value is equal.
 */
inline bool operator ==(const objHndl &a, const objHndl &b) {
	return a.handle == b.handle;
}
inline bool operator !=(const objHndl &a, const objHndl &b) {
	return a.handle != b.handle;
}
inline bool operator <(const objHndl &a, const objHndl &b) {
	return a.handle < b.handle;
}
inline bool operator >(const objHndl &a, const objHndl &b) {
	return a.handle > b.handle;
}
std::ostream &operator <<(std::ostream &out, const objHndl &handle);

namespace std {
	template <> struct hash<objHndl> {
		size_t operator()(const objHndl &x) const {
			return std::hash<uint64_t>()(x.handle);
		}
	};
}

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