#include "stdafx.h"

#include "util/fixes.h"

#include <obj.h>
#include <maps.h>
struct ObjectId;

static class ObjectIdHooks : public TempleFix {
public:

	static void GeneratePermanentId(ObjectId* idOut);
	static void GeneratePositionalId(ObjectId* idOut, objHndl handle);
	static ObjectId* GeneratePrototypeId(ObjectId* idOut, int prototypeId);

	static BOOL ObjectIdIsValid(ObjectId id);
	static BOOL ObjectIdLessThan(ObjectId a, ObjectId b);
	static BOOL ObjectIdEquals(ObjectId a, ObjectId b);
	static void ObjectIdToString(char*, ObjectId id);

	void apply() override {
		replaceFunction(0x100c21b0, GeneratePermanentId);
		replaceFunction(0x100c21e0, GeneratePositionalId);
		replaceFunction(0x100c2250, ObjectIdIsValid);
		replaceFunction(0x100c2270, ObjectIdLessThan);
		replaceFunction(0x100c2390, ObjectIdEquals);
		replaceFunction(0x100c2430, GeneratePrototypeId);
		replaceFunction(0x100c2460, ObjectIdToString);
	}

} hooks;

void ObjectIdHooks::GeneratePermanentId(ObjectId* idOut) {
	*idOut = ObjectId::CreatePermanent();
}

void ObjectIdHooks::GeneratePositionalId(ObjectId* idOut, objHndl handle) {
	if (objects.IsStatic(handle)) {
		auto loc = objects.GetLocation(handle);
		*idOut = ObjectId::CreatePositional(
			maps.GetCurrentMapId(),
			loc.locx,
			loc.locy,
			objects.GetTempId(handle)
		);
	} else {
		logger->error("Tried to get a positional id for a permanent object.");
		idOut->subtype = ObjectIdKind::Null;
	}
}

ObjectId* ObjectIdHooks::GeneratePrototypeId(ObjectId* idOut, int prototypeId) {
	*idOut = ObjectId::CreatePrototype((uint16_t) prototypeId);
	return idOut;
}

BOOL ObjectIdHooks::ObjectIdIsValid(ObjectId id) {
	auto ok = id.subtype == ObjectIdKind::Null
		|| id.subtype == ObjectIdKind::Permanent
		|| id.subtype == ObjectIdKind::Prototype
		|| id.subtype == ObjectIdKind::Positional;
	return ok ? TRUE : FALSE;
}

bool operator<(const GUID& lhs, const GUID& rhs) {

	if (lhs.Data1 < rhs.Data1) {
		return true;
	}
	if (lhs.Data1 > rhs.Data1) {
		return false;
	}

	if (lhs.Data2 < rhs.Data2) {
		return true;
	}
	if (lhs.Data2 > rhs.Data2) {
		return false;
	}

	if (lhs.Data3 < rhs.Data3) {
		return true;
	}
	if (lhs.Data3 > rhs.Data3) {
		return false;
	}

	for (auto i = 0; i < 8; ++i) {
		if (lhs.Data4[i] < rhs.Data4[i]) {
			return true;
		}

		if (lhs.Data4[i] > rhs.Data4[i]) {
			return false;
		}
	}

	return false; // They are actually equal

}

BOOL ObjectIdHooks::ObjectIdLessThan(ObjectId a, ObjectId b) {
	if ((int) a.subtype < (int)b.subtype) {
		return TRUE;
	}

	switch (a.subtype) {
	case ObjectIdKind::Prototype:
		if (a.body.protoId < b.body.protoId) {
			return TRUE;
		}
		return FALSE;
	case ObjectIdKind::Permanent:
		if (a.body.guid < b.body.guid) {
			return TRUE;
		} else {
			return FALSE;
		}
	case ObjectIdKind::Positional:
		if (a.body.pos.x < b.body.pos.x)
			return TRUE;
		if (a.body.pos.x > b.body.pos.x)
			return FALSE;
		if (a.body.pos.y < b.body.pos.y)
			return TRUE;
		if (a.body.pos.y > b.body.pos.y)
			return FALSE;
		if (a.body.pos.tempId < b.body.pos.tempId)
			return TRUE;
		if (a.body.pos.tempId > b.body.pos.tempId)
			return FALSE;
		if (a.body.pos.mapId < b.body.pos.mapId)
			return TRUE;
		return FALSE;
	default:
		return FALSE;
	}
}

BOOL ObjectIdHooks::ObjectIdEquals(ObjectId a, ObjectId b) {

	if (a.subtype != b.subtype) {
		return FALSE;
	}

	switch (a.subtype) {
	case ObjectIdKind::Null:
		return TRUE;
	case ObjectIdKind::Prototype:
		if (a.body.protoId == b.body.protoId) {
			return TRUE;
		} else {
			return FALSE;
		}
	case ObjectIdKind::Permanent:
		if (a.body.guid == b.body.guid) {
			return TRUE;
		} else {
			return FALSE;
		}
	case ObjectIdKind::Positional:
		if (a.body.pos.x == b.body.pos.x
			&& a.body.pos.y == b.body.pos.y
			&& a.body.pos.tempId == b.body.pos.tempId
			&& a.body.pos.mapId == b.body.pos.tempId) {
			return TRUE;
		} else {
			return FALSE;
		}
	default:
		return FALSE;
	}
}

void ObjectIdHooks::ObjectIdToString(char* nameOut, ObjectId id) {
	strcpy(nameOut, id.ToString().c_str());
}

bool ObjectId::operator==(const ObjectId& other) const {

	if (subtype != other.subtype) {
		return false;
	}

	switch (subtype) {
	case ObjectIdKind::Null:
		return true;
	case ObjectIdKind::Prototype:
		if (body.protoId == other.body.protoId) {
			return true;
		} else {
			return false;
		}
	case ObjectIdKind::Permanent:
		if (body.guid == other.body.guid) {
			return true;
		} else {
			return false;
		}
	case ObjectIdKind::Positional:
		if (body.pos.x == other.body.pos.x
			&& body.pos.y == other.body.pos.y
			&& body.pos.tempId == other.body.pos.tempId
			&& body.pos.mapId == other.body.pos.tempId) {
			return true;
		} else {
			return false;
		}
	default:
		return false;
	}

}

std::string ObjectId::ToString() const {
	switch (subtype) {
	case ObjectIdKind::Null:
		return "NULL";
	case ObjectIdKind::Prototype:
		return fmt::format("A_{:08X}", GetPrototypeId());
	case ObjectIdKind::Permanent:
		return fmt::format("G_{:08X}_{:04X}_{:04X}_{:02X}{:02X}_{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}",
		                   body.guid.Data1,
		                   body.guid.Data2,
		                   body.guid.Data3,
		                   body.guid.Data4[0],
		                   body.guid.Data4[1],
		                   body.guid.Data4[2],
		                   body.guid.Data4[3],
		                   body.guid.Data4[4],
		                   body.guid.Data4[5],
		                   body.guid.Data4[6],
		                   body.guid.Data4[7]
		);
	case ObjectIdKind::Positional:
		return fmt::format("P_{:08X}_{:08X}_{:08X}_{:08X}",
		                   body.pos.x,
		                   body.pos.y,
		                   body.pos.tempId,
		                   body.pos.mapId
		);
	case ObjectIdKind::Handle:
		return fmt::format("Handle_{:X}",
		                   body.handle
		);
	case ObjectIdKind::Blocked:
		return "Blocked";
	default:
		return "UNKNOWN";
	}
}

ObjectId ObjectId::CreatePermanent()
{
	GUID guid;
	if (!SUCCEEDED(CoCreateGuid(&guid))) {
		throw TempleException("Unable to generate a permanent GUID");
	}

	ObjectId result;
	result.subtype = ObjectIdKind::Permanent;
	result.body.guid = guid;
	return result;
}

ObjectId ObjectId::CreatePositional(int mapId, int tileX, int tileY, int tempId)
{
	ObjectId result;
	result.subtype = ObjectIdKind::Positional;
	result.body.pos.x = tileX;
	result.body.pos.y = tileY;
	result.body.pos.tempId = tempId;
	result.body.pos.mapId = maps.GetCurrentMapId();
	return result;
}

ObjectId ObjectId::CreatePrototype(uint16_t prototypeId)
{
	ObjectId result;
	result.subtype = ObjectIdKind::Prototype;
	result.body.protoId = prototypeId;
	return result;
}

ObjectId ObjectId::CreateHandle(objHndl handle)
{
	ObjectId result;
	result.subtype = ObjectIdKind::Handle;
	result.body.handle = handle;
	return result;
}
