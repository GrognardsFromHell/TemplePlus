
#pragma once

#include <unordered_map>
#include <memory>

#include "../../obj_structs.h"

#include <EASTL/hash_map.h>

#include "gameobject.h"

namespace eastl {
	template <>
	struct hash<ObjectId> {
		size_t operator()(ObjectId const& id) const;
	};
	
	template<>
	struct hash<objHndl> : std::hash<objHndl> {};
}

class ObjRegistry {
public:
	ObjRegistry();

	using Container = eastl::hash_map<objHndl, std::unique_ptr<GameObjectBody>>;
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
	eastl::hash_map<ObjectId, objHndl> mObjectIndex;
	uint32_t mNextId = 1;

	objHndl lastObj = objHndl::null;
	GameObjectBody* lastObjBody = nullptr;
};
