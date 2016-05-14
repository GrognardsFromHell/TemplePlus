
#include "stdafx.h"
#include "objregistry.h"

#include "../../obj.h"

const size_t objBodySize = 168; // Passed in to Object_Tables_Init
static_assert(temple::validate_size<GameObjectBody, objBodySize>::value, "Object structure has incorrect size.");

static std::hash<int> sIntHasher;
static std::hash<uint64_t> sHandleHasher;

hash<ObjectId>::result_type hash<ObjectId>::operator()(argument_type const& id) const {
	result_type result;
	switch (id.subtype) {
	default:
	case ObjectIdKind::Null:
		return 0;
	case ObjectIdKind::Prototype:
		return sIntHasher(id.GetPrototypeId());
	case ObjectIdKind::Permanent:
		result = sHandleHasher(*(uint64_t*)&id.body.guid.Data1);
		result ^= sHandleHasher(*(uint64_t*)&id.body.guid.Data4[0]);
		return result;
	case ObjectIdKind::Positional:
		result = sHandleHasher(*(uint64_t*)&id.body.pos.x);
		result ^= sHandleHasher(*(uint64_t*)&id.body.pos.tempId);
		return result;
	case ObjectIdKind::Handle:
		return std::hash<objHndl>()(id.GetHandle());
	case ObjectIdKind::Blocked:
		return 0;
	}
}

ObjRegistry::ObjRegistry() {
	mObjects.reserve(8192);
}

ObjectId ObjRegistry::GetIdByHandle(objHndl handle) {

	auto obj = Get(handle);

	if (obj == nullptr) {
		ObjectId nullId;
		nullId.subtype = ObjectIdKind::Null;
		return nullId;
	}

	return obj->id;

}

objHndl ObjRegistry::GetHandleById(ObjectId id) {

	auto it = mObjectIndex.find(id);

	if (it == mObjectIndex.end()) {
		return objHndl::null;
	}

	return it->second;

}

void ObjRegistry::AddToIndex(objHndl handle, ObjectId objectId) {
	mObjectIndex[objectId] = handle;
}

void ObjRegistry::RemoveDynamicObjectsFromIndex() {
	auto idIt = mObjectIndex.begin();
	while (idIt != mObjectIndex.end()) {
		if (!idIt->first.IsPrototype()) {
			idIt = mObjectIndex.erase(idIt);
		}
		else {
			++idIt;
		}
	}
}

void ObjRegistry::Clear() {

	lastObj = 0;
	lastObjBody = nullptr;

	// We will concurrently modify the object handle list when we remove them,
	// so we make a copy here first
	std::vector<objHndl> toRemove;
	toRemove.reserve(mObjects.size());
	for (auto& it : mObjects) {
		toRemove.push_back(it.first);
	}

	logger->info("Letting {} leftover objects leak.", toRemove.size());

	mObjectIndex.clear();

}

ObjRegistry::It ObjRegistry::Remove(objHndl handle) {

	if (lastObj == handle) {
		lastObj = 0;
		lastObjBody = nullptr;
	}

	auto it = mObjects.find(handle);
	if (it != mObjects.end()) {
		// Destroy index entries as well
		mObjectIndex.erase(it->second->id);

		return mObjects.erase(it);
	}
	return it;
}

bool ObjRegistry::Contains(objHndl handle) {
	return mObjects.find(handle) != end();
}

objHndl ObjRegistry::Add(std::unique_ptr<GameObjectBody>&& ptr) {

	auto obj = ptr.get();
	
	objHndl id{ mNextId++ | ((uint64_t)obj) << 32 };

	auto it = mObjects.find(id);
	assert(mObjects.find(id) == mObjects.end());

	mObjects.emplace(std::make_pair(id, std::move(ptr)));

	// Cache for later use
	lastObj = id;
	lastObjBody = obj;

	return id;

}

GameObjectBody* ObjRegistry::Get(objHndl handle) {

	if (!handle) {
		return nullptr;
	}

	return (GameObjectBody*)(handle.handle >> 32);

	// This would be the traditional way and it does detect when handles
	// are no longer valid
	if (lastObj == handle) {
		return lastObjBody;
	}

	auto it = mObjects.find(handle);
	if (it == mObjects.end()) {
		return nullptr;
	}

	lastObj = it->first;
	lastObjBody = it->second.get();
	return it->second.get();

}

