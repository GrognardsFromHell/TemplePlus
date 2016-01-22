
#include "stdafx.h"
#include "objregistry.h"

const size_t objBodySize = 168; // Passed in to Object_Tables_Init
static_assert(temple::validate_size<GameObjectBody, objBodySize>::value, "Object structure has incorrect size.");

static struct ObjAdresses : temple::AddressTable {

	void(*ObjUnload)(objHndl handle);

	ObjAdresses() {
		rebase(ObjUnload, 0x1009E0D0);
	}
} addresses;

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
		return 0;
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

	logger->info("Destroying {} leftover objects.", toRemove.size());

	for (auto handle : toRemove) {
		addresses.ObjUnload(handle);
	}

	mObjectIndex.clear();

}

ObjRegistry::It ObjRegistry::Remove(objHndl handle) {

	if (lastObj == handle) {
		lastObj = 0;
		lastObjBody = nullptr;
	}

	auto it = mObjects.find(handle);
	if (it != mObjects.end()) {
		return mObjects.erase(it);
	}
	return it;
}

bool ObjRegistry::Contains(objHndl handle) {
	return mObjects.find(handle) != end();
}

objHndl ObjRegistry::Add(std::unique_ptr<GameObjectBody>&& ptr) {

	auto id = mNextId++;
	auto obj = ptr.get();

	mObjects.emplace(std::make_pair(id, std::move(ptr)));

	// Cache for later use
	lastObj = id;
	lastObjBody = obj;

	return id;

}

GameObjectBody* ObjRegistry::Get(objHndl handle) {

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

