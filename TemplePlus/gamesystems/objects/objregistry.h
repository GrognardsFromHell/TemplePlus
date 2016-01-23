
#pragma once

#include <unordered_map>
#include <memory>

#include "../../obj_structs.h"

#include "gameobject.h"

namespace std {
	template <>
	struct hash<ObjectId> {
		typedef ObjectId argument_type;
		typedef std::size_t result_type;

		result_type operator()(argument_type const& id) const;
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
	uint32_t mNextId = 1;

	objHndl lastObj = 0;
	GameObjectBody* lastObjBody = nullptr;
};
