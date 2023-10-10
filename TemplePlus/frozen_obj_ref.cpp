
#include "stdafx.h"
#include <infrastructure/vfs.h>
#include "frozen_obj_ref.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"

static_assert(temple::validate_size<FrozenObjRef, 40>::value);

// Originally @ 0x10020280
bool FrozenObjRef::Save(const objHndl & handle, const FrozenObjRef *ref, void * fh)
{
	if (!fh) {
		return false;
	}

	ObjectId objId = ObjectId::CreateNull();
	locXY loc = locXY::fromField(0);
	int mapNumber = 0;

	if (!ref) {
		if (handle) {
			objId = objSystem->GetIdByHandle(handle);
			mapNumber = gameSystems->GetMap().GetCurrentMapId();
			loc = objSystem->GetObject(handle)->GetLocation();
		}
		else {
			logger->debug("SaveTimeEventObjInfo(): Caught null handle when serializing time event!");
		}
	}
	else {
		loc = ref->location;
		objId = ref->guid;
		mapNumber = ref->mapNumber;
	}

	if (vfs->Write(&objId, sizeof(ObjectId), fh) != sizeof(ObjectId)
		|| vfs->Write(&loc, sizeof(locXY), fh) != sizeof(locXY)
		|| vfs->Write(&mapNumber, sizeof(int), fh) != sizeof(int))
		return false;

	return true;
}

// Originally @ 0x10020370
bool FrozenObjRef::Load(objHndl * handleOut, FrozenObjRef * ref, void * fh)
{
	Expects(handleOut);

	ObjectId objId;
	locXY location;
	int mapId;
	
	if (vfs->Read(&objId, sizeof(objId), fh) != sizeof(objId)
		|| vfs->Read(&location, sizeof(location), fh) != sizeof(location)
		|| vfs->Read(&mapId, sizeof(mapId), fh) != sizeof(mapId)) {
		return false;
	}

	if (objId.IsNull()) {
		*handleOut = objHndl::null;
		if (ref) {
			ref->guid = ObjectId::CreateNull();
			ref->location = locXY::fromField(0);
			ref->mapNumber = 0;
		}
	} else {
		// Again, as in unfreeze, location is completely ignored

		*handleOut = objSystem->GetHandleById(objId);
		if (!*handleOut) {
			logger->warn("FrozenObjRef::Load: Couldn't find object with guid {} @ {} on map {}", objId.ToString(), location, mapId);
		}

		if (ref) {
			ref->guid = objId;
			ref->location = location;
			ref->mapNumber = mapId;
		}
	}

	return true;

}

// Originally @ 0x10020540
FrozenObjRef FrozenObjRef::Freeze(objHndl handle)
{
	FrozenObjRef result;
	if (!handle) {
		result.guid.subtype = ObjectIdKind::Null;
		result.location = locXY::fromField(0);
		result.mapNumber = 0;
		return result;
	}

	auto obj = objSystem->GetObject(handle);
	if (obj->GetFlags() & OF_DESTROYED) {
		result.guid.body.guid.Data1 = 0xBEEFBEEF;
		result.guid.subtype = ObjectIdKind::Null;
		result.location = locXY::fromField(0);
		result.mapNumber = 0;
		return result;
	}

	auto objId = objSystem->GetPersistableId(handle);
	result.guid = objId;
	if (obj->IsStatic()) {
		result.location = obj->GetLocation();
		result.mapNumber = gameSystems->GetMap().GetCurrentMapId();
	}
	else {
		result.location = locXY::fromField(0);
		result.mapNumber = gameSystems->GetMap().GetCurrentMapId();
	}
	return result;
}

// Originally @ 0x10020610
bool FrozenObjRef::Unfreeze(const FrozenObjRef &ref, objHndl * handleOut)
{
	if (ref.guid.IsNull()) {
		*handleOut = objHndl::null;
		return true;
	}

	if (ref.location && ref.mapNumber != gameSystems->GetMap().GetCurrentMapId()) {
		*handleOut = objHndl::null;
		return false;
		// At this point, ToEE did query the map, but didn't do anything with it
	}

	*handleOut = objSystem->GetHandleById(ref.guid);
	return *handleOut != objHndl::null;
}
