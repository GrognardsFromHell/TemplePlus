
#pragma once

#include "obj_structs.h"
#include "objarrays.h"
#include "spell_structs.h"
#include <stdint.h>

struct LocAndOffsets;
class OutputStream;
struct locXY;
struct Dispatcher;

// Stored in obj_f_script_idx array
#pragma pack(push, 1)
struct ObjectScript {
	int unk1;
	uint32_t counters;
	int scriptId;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TransientProps {
	uint32_t renderColor;
	uint32_t renderColors;
	uint32_t renderPalette;
	uint32_t renderScale;
	uint32_t renderAlpha;
	uint32_t renderX;
	uint32_t renderY;
	uint32_t renderWidth;
	uint32_t renderHeight;
	uint32_t palette;
	uint32_t color;
	uint32_t colors;
	uint32_t renderFlags;
	uint32_t tempId;
	uint32_t lightHandle;
	uint32_t overlayLightHandles;
	uint32_t internalFlags;
	uint32_t findNode;
	uint32_t animationHandle;
	uint32_t grappleState;
};

struct ObjectFieldDef;

enum class ObjectFieldType : uint32_t;

struct GameObjectBody {
	~GameObjectBody();
	ObjectType type;
	uint32_t field4;
	ObjectId id;
	ObjectId protoId;
	objHndl protoHandle;
	uint32_t field40;
	uint16_t hasDifs;
	uint16_t propCollectionItems;
	uint32_t* propCollBitmap;
	uint32_t* difBitmap;
	void** propCollection;
	TransientProps transientProps;
	uint32_t padding;

	bool IsProto() const;

#pragma region Type Tests
	bool IsItem() const {
		return type >= obj_t_weapon && type <= obj_t_generic || type == obj_t_bag;
	}
	bool IsContainer() const {
		return type == obj_t_container;
	}
	bool IsCritter() const {
		return type == obj_t_npc || type == obj_t_pc;
	}
	bool IsPC() const {
		return type == obj_t_pc;
	}
	bool IsNPC() const {
		return type == obj_t_npc;
	}
	bool IsStackable() const;
#pragma endregion

	int32_t GetInt32(obj_f field) const;
	float GetFloat(obj_f field) const;
	int64_t GetInt64(obj_f field) const;
	// This is a convenience version of GetObjectId
	objHndl GetObjHndl(obj_f field) const;
	// This gets the object handle and returns true, if it is valid (by validating it against the obj registry)
	// handleOut will always be set to the null handle if the handle is invalid.
	// If the handle is invalid, this function will also clear the storage location
	bool GetValidObjHndl(obj_f field, objHndl *handleOut);
	ObjectId GetObjectId(obj_f field) const;
	const char *GetString(obj_f field) const;
	void SetString(obj_f field, const char *text);

	GameInt32ReadOnlyArray GetInt32Array(obj_f field) const;
	GameInt64ReadOnlyArray GetInt64Array(obj_f field) const;
	GameObjectIdReadOnlyArray GetObjectIdArray(obj_f field) const;
	GameScriptReadOnlyArray GetScriptArray(obj_f field) const;
	GameSpellReadOnlyArray GetSpellArray(obj_f field) const;

	// Convenience array accessors
	int32_t GetInt32(obj_f field, size_t index) const;
	int32_t GetInt32Size(obj_f field) const;
	void SetInt32(obj_f field, size_t index, int32_t value);
	void AppendInt32(obj_f field, int32_t value);
	void RemoveInt32(obj_f field, size_t index);
	int64_t GetInt64(obj_f field, size_t index) const;
	int32_t GetInt64Size(obj_f field) const;
	void SetInt64(obj_f field, size_t index, int64_t value);
	void RemoveInt64(obj_f field, size_t index);
	ObjectId GetObjectId(obj_f field, size_t index) const;
	void SetObjectId(obj_f field, size_t index, const ObjectId &value);
	void RemoveObjectId(obj_f field, size_t index);
	objHndl GetObjHndl(obj_f field, size_t index) const;
	bool GetValidObjHndl(obj_f field, size_t index, objHndl *handleOut);
	void SetObjHndl(obj_f field, size_t index, objHndl value);
	void SetScript(obj_f field, size_t index, const ObjectScript &script);
	ObjectScript GetScript(obj_f field, size_t index) const;
	void RemoveScript(obj_f field, size_t index);
	void SetSpell(obj_f field, size_t index, const SpellStoreData &spell);
	SpellStoreData GetSpell(obj_f field, size_t index) const;
	void RemoveSpell(obj_f field, size_t index);
	void ClearArray(obj_f field);
	
	// Setters
	void SetInt32(obj_f field, int32_t value);
	void SetFloat(obj_f field, float value);
	void SetInt64(obj_f field, int64_t value);
	void SetObjHndl(obj_f field, objHndl value);
	void SetObjectId(obj_f field, const ObjectId &id);

	// Internal flags
	uint32_t GetInternalFlags() const;
	void SetInternalFlags(uint32_t internalFlags);

	void ResetDiffs();

	/**
	 * Removes a field from this object instance, which effectively
	 * resets the value of the field to the value from this object's
	 * prototype.
	 */
	void ResetField(obj_f field);
	
	/**
	 * Portals, Scenery, and Traps are "static objects" unless they have
	 * OF_DYNAMIC set to make them explicitly dynamic objects.
	 */
	bool IsStatic() const;

	/**
	 * Calls the given callback for each stored field in this object. For
	 * prototypess, the callback will be called for each field that the type
	 * could possibly have, even if the storage location is currently 0.
	 */
	bool ForEachField(std::function<bool(obj_f, void**)> callback);
	bool ForEachField(std::function<bool(obj_f, const void*)> callback) const;

	/**
	 * Returns true if the type of this object has the given field.
	 */
	bool SupportsField(obj_f field) const;

	/**
	 * If this object is currently storing persistable IDs, they are resolved to the handles of the
	 * corresponding objects.
	 */
	void UnfreezeIds();

	/**
	 * If this object is currently storing references to other objects as handles, those references will
	 * be converted to persistable object ids.
	 */
	void FreezeIds();

	// Removes bad inventory array entries
	void PruneNullInventoryItems();

	std::unique_ptr<GameObjectBody> Clone() const;

#pragma region Object Field Getters and Setters
	ObjectFlag GetFlags() const {
		return (ObjectFlag) GetInt32(obj_f_flags);
	}
	void SetFlags(ObjectFlag flags) {
		SetInt32(obj_f_flags, (int32_t)flags);
	}
	bool HasFlag(ObjectFlag flag) const {
		return (GetFlags() & flag) != 0;
	}
	void SetFlag(ObjectFlag flag, bool enabled) {
		if (enabled) {
			SetFlags((ObjectFlag)((int32_t)GetFlags() | flag));
		} else {
			SetFlags((ObjectFlag)((int32_t)GetFlags() & ~ (int32_t)flag));
		}
	}

	ItemFlag GetItemFlags() const {
		return (ItemFlag)GetInt32(obj_f_item_flags);
	}
	void SetItemFlags(ItemFlag flags)
	{
		SetInt32(obj_f_item_flags, (int32_t)flags);
	}
	void SetItemFlag(ItemFlag flag, bool enabled) {
		if (enabled) {
			SetItemFlags((ItemFlag)((int32_t)GetItemFlags() | flag));
		}
		else {
			SetItemFlags((ItemFlag)((int32_t)GetItemFlags() & ~(int32_t)flag));
		}
	}
	locXY GetLocation() const;
	LocAndOffsets GetLocationFull() const;
	void SetLocation(locXY location);

	Dispatcher* GetDispatcher() const {
		//if (type == obj_t_portal){
		//	//return nullptr; //should be nullptr anyway
		//	int dummy = 1;
		//}
		uint32_t dispatcher = GetInt32(obj_f_dispatcher);
		if (dispatcher == -1 || dispatcher == 0) {
			return nullptr;
		}
		return reinterpret_cast<Dispatcher*>(dispatcher);
	}

	void SetDispatcher(Dispatcher *dispatcher) {
		SetInt32(obj_f_dispatcher, reinterpret_cast<uint32_t>(dispatcher));
	}

#pragma endregion

#pragma region NPC Field Getters and Setters
	uint32_t GetNPCFlags() const {
		return GetInt32(obj_f_npc_flags);
	}
	void SetNPCFlags(uint32_t flags) {
		SetInt32(obj_f_npc_flags, flags);
	}
#pragma endregion

	// Utility function for containers and critters
	// Will iterate over the content of this object
	// If this object is not a container or critter, will do nothing.
	void ForEachChild(std::function<void(objHndl item)> callback) const;

#pragma region Transient Property Accessors
	uint32_t GetTemporaryId() const {
		return transientProps.tempId;
	}
#pragma endregion

#pragma region Persistence
	/**
	 * Writes this object to a file. Only supported for non-prototype objects.
	 * Prefixes the object's body with 0x77 (the object file version).
	 */
	bool Write(OutputStream &stream) const;

	void WriteDiffsToStream(OutputStream &stream) const;
	void LoadDiffsFromFile(objHndl handle, TioFile *file);
#pragma endregion
	
private:
	bool ValidateFieldForType(obj_f field) const;
	void** GetTransientStorage(obj_f field);
	void* const* GetTransientStorage(obj_f field) const;
	
	// Checks propBitmap1 for the given field
	bool HasDataForField(const ObjectFieldDef &field) const;

	// Determines the packed index in the prop coll for the given field
	size_t GetPropCollIdx(const ObjectFieldDef &field) const;

	// Gets a readable storage location, possibly in the object_s prototype
	// if this object doesnt have the requested field
	template<typename T>
	const T* GetStorageLocation(obj_f field) const;

	// Gets a storage location that can be written to, and if necessary, allocates
	// the storage location in the prop collection
	template<typename T>
	T* GetMutableStorageLocation(obj_f field);

	void MarkChanged(obj_f field);
		
	// Resolves the proto handle for this instance
	objHndl GetProtoHandle() const;

	// Resolves the proto object for this instance
	const GameObjectBody *GetProtoObj() const;

	// Frees storage that may have been allocated to store a property of the given type
	void FreeStorage(ObjectFieldType type, void* storage);

	GameInt32Array GetMutableInt32Array(obj_f field);
	GameInt64Array GetMutableInt64Array(obj_f field);
	GameObjectIdArray GetMutableObjectIdArray(obj_f field);
	GameScriptArray GetMutableScriptArray(obj_f field);
	GameSpellArray GetMutableSpellArray(obj_f field);

	/**
	 * Writes a field value to file.
	 */
	static void WriteFieldToStream(ObjectFieldType type, const void *value, OutputStream &stream);
};
#pragma pack(pop)

