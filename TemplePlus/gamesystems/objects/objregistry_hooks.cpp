#include "stdafx.h"

#include <temple/dll.h>

#include "util/fixes.h"

#include "obj_structs.h"
#include <obj.h>
#include <objlist.h>
#include <maps.h>

const size_t objBodySize = 168; // Passed in to Object_Tables_Init
static_assert(temple::validate_size<GameObjectBody, objBodySize>::value, "Object structure has incorrect size.");

static struct ObjAdresses : temple::AddressTable {

	void (*ObjFindRemove)(objHndl handle);

	ObjAdresses() {
		rebase(ObjFindRemove, 0x1009E0D0);
	}
} addresses;

namespace std {
	template <>
	struct hash<ObjectId> {
		typedef ObjectId argument_type;
		typedef std::size_t result_type;

		std::hash<int> mIntHasher;
		std::hash<uint64_t> mHandleHasher;

		result_type operator()(argument_type const& id) const {
			result_type result;
			switch (id.subtype) {
			default:
			case ObjectIdKind::Null:
				return 0;
			case ObjectIdKind::Prototype:
				return mIntHasher(id.GetPrototypeId());
			case ObjectIdKind::Permanent:
				result = mHandleHasher(*(uint64_t*)&id.body.guid.Data1);
				result ^= mHandleHasher(*(uint64_t*)&id.body.guid.Data4[0]);
				return result;
			case ObjectIdKind::Positional:
				result = mHandleHasher(*(uint64_t*)&id.body.pos.x);
				result ^= mHandleHasher(*(uint64_t*)&id.body.pos.tempId);
				return result;
			case ObjectIdKind::Handle:
				return mHandleHasher(id.GetHandle());
			case ObjectIdKind::Blocked:
				return 0;
			}
		}
	};
}

class ObjRegistry {
public:
	ObjRegistry();

	using Container = std::unordered_map<objHndl, std::unique_ptr<GameObjectBody>>;
	using It = Container::iterator;

	ObjectId GetIdByHandle(objHndl handle);
	objHndl GetHandleById(ObjectId id);
	void AddToIndex(objHndl handle, ObjectId objectId);

	// Remove any object from the index that is not a prototype
	void RemoveDynamicObjectsFromIndex();

	void Clear();
	It Remove(objHndl handle);
	bool Contains(objHndl handle);

	objHndl Add(std::unique_ptr<GameObjectBody>&& ptr);

	GameObjectBody* Get(objHndl handle);

	It begin() {
		return mObjects.begin();
	}

	It end() {
		return mObjects.end();
	}

private:
	Container mObjects;
	std::unordered_map<ObjectId, objHndl> mObjectIndex;
	objHndl mNextId = 1;

	objHndl lastObj = 0;
	GameObjectBody* lastObjBody = nullptr;
};

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
		} else {
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
		addresses.ObjFindRemove(handle);
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

std::unique_ptr<ObjRegistry> objRegistry;

static class ObjRegistryHooks : public TempleFix {
public:
	const char* name() override {
		return "Object Registry Hooks";
	}

	static void Object_Tables_Init(int nToEEObject_BodySize, BOOL editor);
	static ObjectId* Object_Tables_GetIdByHandle(ObjectId* idOut, objHndl handle);
	static void Object_Tables_CompactIdx();
	static void Object_Tables_Destroy();
	static GameObjectBody* Object_Tables_GetItem(objHndl handle);
	static void Object_Tables_AddToIndex(ObjectId ObjectId, objHndl handle);

	static BOOL Object_Tables_FirstHandle(objHndl* handleOut, int* iteratorOut);
	static BOOL Object_Tables_NextHandle(objHndl* handleOut, int* iteratorInOut);

	static BOOL Object_Tables_IsValidHandle(objHndl handle);
	static GameObjectBody* Object_Tables_Add(objHndl* handleOut);
	static void Object_Tables_Remove(objHndl handle);
	static objHndl Object_Tables_perm_lookup(ObjectId id);

	static ObjRegistry::It it;

	void apply() override {
		replaceFunction(0x100c2580, Object_Tables_Init);
		replaceFunction(0x100c2660, Object_Tables_GetIdByHandle);
		replaceFunction(0x100c26c0, Object_Tables_CompactIdx);
		replaceFunction(0x100c2960, Object_Tables_Destroy);
		replaceFunction(0x100c2a70, Object_Tables_GetItem);
		replaceFunction(0x100c2ac0, Object_Tables_AddToIndex);
		replaceFunction(0x100c2ba0, Object_Tables_FirstHandle);
		replaceFunction(0x100c2c50, Object_Tables_NextHandle);
		replaceFunction(0x100c2d00, Object_Tables_IsValidHandle);
		replaceFunction(0x100c2e90, Object_Tables_Add);
		replaceFunction(0x100c3030, Object_Tables_Remove);
		replaceFunction(0x100C3050, Object_Tables_perm_lookup);

		// The following internal functions of the module are not replaced:
		// replaceFunction(0x100c2790, Object_Tables_AddBucket);
		// replaceFunction(0x100c2810, _Object_Tables_AddToFreeList);
		// replaceFunction(0x100c2870, _Object_Tables_FindInIdx);
		// replaceFunction(0x100c2db0, _Object_Tables_GetFreeIdx);
	}
} hooks;

ObjRegistry::It ObjRegistryHooks::it;

void ObjRegistryHooks::Object_Tables_Init(int nToEEObject_BodySize, BOOL editor) {
	Expects(nToEEObject_BodySize == sizeof(GameObjectBody));
	Expects(editor == FALSE);

	objRegistry = std::make_unique<ObjRegistry>();
}

ObjectId* ObjRegistryHooks::Object_Tables_GetIdByHandle(ObjectId* idOut, objHndl handle) {

	*idOut = objRegistry->GetIdByHandle(handle);
	return idOut;

}

void ObjRegistryHooks::Object_Tables_CompactIdx() {
	objRegistry->RemoveDynamicObjectsFromIndex();
}

/*BOOL ObjRegistryHooks::_Object_Tables_AddBucket() {

}

BOOL ObjRegistryHooks::_Object_Tables_FindInIdx(ObjectId id, int* idxOut) {
}*/

void ObjRegistryHooks::Object_Tables_Destroy() {
	objRegistry->Clear();
}

GameObjectBody* ObjRegistryHooks::Object_Tables_GetItem(objHndl handle) {
	return objRegistry->Get(handle);
}

void ObjRegistryHooks::Object_Tables_AddToIndex(ObjectId ObjectId, objHndl handle) {
	objRegistry->AddToIndex(handle, ObjectId);
}

/**
	NOTE: ToEE will remove items from the obj registy while using this iterator function.
	For that reason, the global iterator object will always point to the *next* object
	instead of the current object. unordered_map guarantees that iterators for items 
	will be unaffected by item removal unless they point to the removed item.
*/
BOOL ObjRegistryHooks::Object_Tables_FirstHandle(objHndl* handleOut, int* iteratorOut) {

	it = objRegistry->begin();
	if (it == objRegistry->end()) {
		*handleOut = 0;
		return FALSE;
	}

	*handleOut = it->first;
	++it;
	return TRUE;

}

BOOL ObjRegistryHooks::Object_Tables_NextHandle(objHndl* handleOut, int* iteratorIn) {

	if (it == objRegistry->end()) {
		*handleOut = 0;
		return FALSE;
	}

	*handleOut = it->first;
	++it;
	return TRUE;

}

BOOL ObjRegistryHooks::Object_Tables_IsValidHandle(objHndl handle) {
	if (!handle) {
		return TRUE;
	}
	if (!objRegistry->Contains(handle)) {
		logger->debug("Invalid handle {} checked for validity.", handle);
		return FALSE;
	}
	return TRUE;
}

GameObjectBody* ObjRegistryHooks::Object_Tables_Add(objHndl* handleOut) {

	auto obj = std::make_unique<GameObjectBody>();
	auto result = obj.get();

	*handleOut = objRegistry->Add(std::move(obj));
	return result;

}

void ObjRegistryHooks::Object_Tables_Remove(objHndl handle) {
	objRegistry->Remove(handle);
}

objHndl ObjRegistryHooks::Object_Tables_perm_lookup(ObjectId id) {

	auto handle = objRegistry->GetHandleById(id);

	if (handle) {
		return handle;
	}

	// Check for positional IDs in the map
	if (!id.IsPositional())
		return 0;

	auto pos = id.body.pos;

	if (maps.GetCurrentMapId() != pos.mapId) {
		return 0;
	}

	ObjList list;
	locXY loc;
	loc.locx = pos.x;
	loc.locy = pos.y;
	list.ListTile(loc, 0x2000A);

	for (auto i = 0; i < list.size(); ++i) {
		auto candidate = list.get(i);
		auto tempId = objects.GetTempId(candidate);
		if (tempId == pos.tempId) {
			objRegistry->AddToIndex(candidate, id);
			return candidate;
		}
	}

	return 0;

}
