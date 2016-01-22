
#pragma once

#include <unordered_map>
#include <memory>

#include "../../obj.h"
#include "../../obj_structs.h"

#include "gameobject.h"

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
