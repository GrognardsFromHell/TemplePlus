
#pragma once

#include "../gamesystem.h"
#include "../../obj.h"
#include "../../obj_structs.h"
#include "../../spell_structs.h"

#include "gameobject.h"

#ifdef GetObject
#undef GetObject
#endif

// This variant is used depending on the field type
#pragma pack(push, 1)
union ObjFieldValue {
	int32_t int32;
	int64_t int64;
	const char* string;
	float float32;
	ObjectScript script;
	SpellStoreData spell;
	ObjectId objectId;
};
#pragma pack(pop)

enum class ObjectFieldType : uint32_t;

class ObjBuffer;

class ObjSystem : public GameSystem {
friend class ObjRegistryHooks;
friend struct GameObjectBody;
friend class ObjSystemHooks;
public:
	static constexpr auto Name = "Obj";
	ObjSystem(const GameSystemConf &config);
	~ObjSystem();
	const std::string &GetName() const override;

	void CompactIndex();

	// Get the handle for an object by its object id
	objHndl GetHandleById(ObjectId id);
	
	// Get the object id for an object identified by its handle
	ObjectId GetIdByHandle(objHndl handle);

	// Frees the memory associated with the game object and removes it from the object table
	void Remove(objHndl handle);

	// Converts object fields and object array fields from handles to IDs
	void FreezeIds(objHndl handle);

	// Converts object fields and object array fields from IDs to handles
	void UnfreezeIds(objHndl handle);

	// Prunes null inventory items
	void PruneNullInventoryIds(objHndl handle);

	// Checks if the given handle points to an active object. Null handles
	// are considered valid
	bool IsValidHandle(objHndl handle);
	
	/*
		gets the proto ID number for the object
	*/
	int GetProtoId(objHndl obj);

	GameObjectBody* GetObject(objHndl handle);

	// Resolve an id for persisting a reference to the given object
	ObjectId GetPersistableId(objHndl handle);
	
	bool ValidateSector(bool requireHandles);

	/**
	 * Returns the handle to a prototype with the given prototype id or the null handle.
	 */
	objHndl GetProtoHandle(uint16_t protoId);

	/**
	 * Creates a new object with the given prototype at the given location.
	 */
	objHndl CreateObject(objHndl protoHandle, locXY location);

	/**
	 * Loads an object from the given file.
	 */
	objHndl LoadFromFile(TioFile *file);

	/**
	* Loads an object from the given memory buffer.
	*/
	objHndl LoadFromBuffer(void* buffer);

	/**
	 * Calls a given callback for each non prototype object.
	 */
	void ForEachObj(std::function<void(objHndl, GameObjectBody&)> callback) const;

	/**
	 * Finds object based on id.toString() == id_str.
	 */
	objHndl FindObjectByIdStr(string id_str);

	/**
	 * Create a new empty prototype object.
	 */
	objHndl CreateProto(ObjectType type);

	/**
	 * Clone an existing object and give it the requested location.
	 */
	objHndl Clone(objHndl handle, locXY location);
	
private:
	std::unique_ptr<class ObjRegistry> mObjRegistry;
	std::unique_ptr<class ObjFindSupport> mObjFind;

	bool ValidateInventory(const GameObjectBody *container, obj_f idxField, obj_f countField, bool requireHandles);

	void AddToIndex(ObjectId id, objHndl handle);

	void InitDynamic(GameObjectBody *obj, objHndl handle, locXY location);

	void FindNodeAllocate(objHndl handle);
	void FindNodeMove(objHndl handle);

	void ReadFieldValue(obj_f field, void** storageLoc, TioFile *file);

	void ReadFieldValue(obj_f field, void** storageLoc, ObjBuffer &buffer);

};

extern ObjSystem* objSystem;
