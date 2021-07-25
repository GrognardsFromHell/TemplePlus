
#include "stdafx.h"
#include "gameobject.h"
#include "objfields.h"
#include "objsystem.h"
#include "objregistry.h"
#include "arrayidxbitmaps.h"
#include "tio/tio.h"
#include "util/streams.h"
#include <config/config.h>

GameObjectBody::~GameObjectBody()
{
	ForEachField([=](obj_f field, void** storage) {
		auto &fieldDef = objectFields.GetFieldDef(field);
		FreeStorage(fieldDef.type, *storage);
		return true;
	});

	// Delete transient props
	if (!IsProto()) {
		FreeStorage(ObjectFieldType::AbilityArray, reinterpret_cast<void*>(transientProps.renderAlpha));
		FreeStorage(ObjectFieldType::Int32Array, reinterpret_cast<void*>(transientProps.overlayLightHandles));
	}

	// This is the "old" way of doing it
	if (IsProto()) {
		free(difBitmap);
	} else {
		free(propCollBitmap);
	}
	delete[] propCollection;
}

bool GameObjectBody::IsProto() const
{
	return protoId.IsBlocked();
}

bool GameObjectBody::IsStackable() const{
	auto flags = 0;
	switch(type){
	
		case obj_t_weapon:
			return GetInt32(obj_f_weapon_type) == wt_shuriken;
		case obj_t_food:
			flags = GetItemFlags();
			if ( (flags & OIF_IS_MAGICAL) && (flags & OIF_EXPIRES_AFTER_USE) )
				return true;
			return false;
		case obj_t_generic:
			return GetInt32(obj_f_category) == 5; // jewelry / gems
		case obj_t_armor:
			return GetInt32(obj_f_category) == 17; // necklaces
		case obj_t_ammo:
		case obj_t_money:
		case obj_t_scroll:
			return true;
		default:
			return false;
	}

}

int32_t GameObjectBody::GetInt32(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Int32);

	if (field == obj_f_type) {
		return type;
	}
	if (field == obj_f_model_scale)
	{
		auto dummy = 1;
	}

	// For 32-bit integers, instead of a pointer to the value, 
	// the pointer itself is the 32-bit value
	auto storageLoc = GetStorageLocation<int32_t>(field);
	if (!storageLoc) {
		return int32_t();
	} else {
		return *storageLoc;
	}

}

int32_t GameObjectBody::GetInt32Size(obj_f field) const
{
	return GetInt32Array(field).GetSize();
}

float GameObjectBody::GetFloat(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Float32);

	if (field == obj_f_speed_run)
	{
		auto dummy = 1;
	}

	// For 32-bit floats, instead of a pointer to the value, 
	// the pointer itself is the 32-bit float
	auto storageLoc = GetStorageLocation<float>(field);
	if (!storageLoc) {
		return float();
	} else {
		return *storageLoc;
	}
}

int64_t GameObjectBody::GetInt64(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Int64);
	auto storageLoc = GetStorageLocation<int64_t*>(field);
	if (!storageLoc || !*storageLoc) {
		return int64_t();
	} else {
		return **storageLoc;
	}
}

int32_t GameObjectBody::GetInt64Size(obj_f field) const
{
	return GetInt64Array(field).GetSize();
}

objHndl GameObjectBody::GetObjHndl(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Obj);

	// Special case for prototype handle
	if (field == obj_f_prototype_handle) {
		return GetProtoHandle();
	}

	// Keep in mind that handles are stored in the form of ObjectIds
	auto storageLoc = GetStorageLocation<ObjectId*>(field);
	if (!storageLoc || !(*storageLoc) || !(*storageLoc)->IsHandle()) {
		return objHndl();
	} else {
		return (*storageLoc)->GetHandle();
	}
}

bool GameObjectBody::GetValidObjHndl(obj_f field, objHndl * handleOut)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Obj);

	// Special case for prototype handle
	if (field == obj_f_prototype_handle) {
		auto handle = GetProtoHandle();
		if (!objSystem->IsValidHandle(handle)) {
			*handleOut = objHndl();
			return false;
		} else {
			*handleOut = handle;
			return true;
		}
	}

	// Keep in mind that handles are stored in the form of ObjectIds
	auto storageLoc = GetStorageLocation<ObjectId*>(field);

	// Null handles are considered valid by this function
	if (!*storageLoc || (*storageLoc)->IsNull()) {
		*handleOut = objHndl();
		return true;
	}

	// If it's not a handle id, that is probably an error
	if (!(*storageLoc)->IsHandle()) {
		*handleOut = objHndl();
		return false;
	}
	
	// Use the handle from the id and validate it before returning it
	auto handle = (*storageLoc)->GetHandle();
	if (!objSystem->IsValidHandle(handle)) {
		// Since we know the handle is invalid, we'll fix the issue in the object
		ResetField(field);

		*handleOut = objHndl();
		return false;
	} else {
		*handleOut = handle;
		return true;
	}
}

ObjectId GameObjectBody::GetObjectId(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Obj);

	// Keep in mind that handles are stored in the form of ObjectIds
	auto storageLoc = GetStorageLocation<ObjectId*>(field);

	// Null handles are considered valid by this function
	if (!storageLoc || !*storageLoc) {
		ObjectId result;
		result.subtype = ObjectIdKind::Null;
		return result;
	}
	
	return **storageLoc;
}

const char * GameObjectBody::GetString(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::String);

	// Keep in mind that handles are stored in the form of ObjectIds
	auto storageLoc = GetStorageLocation<const char*>(field);

	return *storageLoc;
}

void GameObjectBody::SetString(obj_f field, const char * text)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::String);

	auto storageLoc = GetMutableStorageLocation<char*>(field);
	if (*storageLoc) {
		free(*storageLoc);
	}
	*storageLoc = _strdup(text);
}

GameInt32ReadOnlyArray GameObjectBody::GetInt32Array(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Int32Array
		|| objectFields.GetType(field) == ObjectFieldType::AbilityArray);

	auto storageLoc = GetStorageLocation<ArrayHeader*>(field);
	return GameInt32ReadOnlyArray(const_cast<ArrayHeader**>(storageLoc));
}

GameInt64ReadOnlyArray GameObjectBody::GetInt64Array(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Int64Array);

	auto storageLoc = GetStorageLocation<ArrayHeader*>(field);
	return GameInt64ReadOnlyArray(const_cast<ArrayHeader**>(storageLoc));
}

GameObjectIdReadOnlyArray GameObjectBody::GetObjectIdArray(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::ObjArray);

	auto storageLoc = GetStorageLocation<ArrayHeader*>(field);
	return GameObjectIdReadOnlyArray(const_cast<ArrayHeader**>(storageLoc));
}

GameScriptReadOnlyArray GameObjectBody::GetScriptArray(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::ScriptArray);

	auto storageLoc = GetStorageLocation<ArrayHeader*>(field);
	return GameScriptReadOnlyArray(const_cast<ArrayHeader**>(storageLoc));
}

GameSpellReadOnlyArray GameObjectBody::GetSpellArray(obj_f field) const
{
	Expects(objectFields.GetType(field) == ObjectFieldType::SpellArray);

	auto storageLoc = GetStorageLocation<ArrayHeader*>(field);
	return GameSpellReadOnlyArray(const_cast<ArrayHeader**>(storageLoc));
}

GameInt32Array GameObjectBody::GetMutableInt32Array(obj_f field)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Int32Array
		|| objectFields.GetType(field) == ObjectFieldType::AbilityArray);

	auto storageLoc = GetMutableStorageLocation<ArrayHeader*>(field);
	return GameInt32Array(storageLoc);
}

GameInt64Array GameObjectBody::GetMutableInt64Array(obj_f field)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::Int64Array);

	auto storageLoc = GetMutableStorageLocation<ArrayHeader*>(field);
	return GameInt64Array(storageLoc);
}

GameObjectIdArray GameObjectBody::GetMutableObjectIdArray(obj_f field)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::ObjArray);

	auto storageLoc = GetMutableStorageLocation<ArrayHeader*>(field);
	return GameObjectIdArray(storageLoc);
}

GameScriptArray GameObjectBody::GetMutableScriptArray(obj_f field)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::ScriptArray);

	auto storageLoc = GetMutableStorageLocation<ArrayHeader*>(field);
	return GameScriptArray(storageLoc);
}

GameSpellArray GameObjectBody::GetMutableSpellArray(obj_f field)
{
	Expects(objectFields.GetType(field) == ObjectFieldType::SpellArray);

	auto storageLoc = GetMutableStorageLocation<ArrayHeader*>(field);
	return GameSpellArray(storageLoc);
}

void GameObjectBody::WriteFieldToStream(ObjectFieldType type, const void* value, OutputStream &stream)
{
	switch (type) {
	case ObjectFieldType::Int32:
		stream.WriteUInt32((uint32_t) value);
		break;
	case ObjectFieldType::Float32:
		stream.WriteFloat(*(float*)&value);
		break;
	case ObjectFieldType::Int64: 
		if (value) {
			stream.WriteUInt8(1);
			stream.WriteInt64(*(uint64_t*)value);
		} else {
			stream.WriteUInt8(0);
		}
		break;
	case ObjectFieldType::String:
		if (!value) {
			stream.WriteUInt8(0);
		} else {
			stream.WriteUInt8(1);

			auto str = reinterpret_cast<const char*>(value);
			auto len = strlen(str);
			stream.WriteUInt32(len);
			
			// ToEE writes the strlen, but includes the 0 byte anyway
			stream.WriteBytes((const uint8_t*)str, len + 1);
		}
		break;
	case ObjectFieldType::Obj: 
		if (value) {
			stream.WriteUInt8(1);
			Expects(reinterpret_cast<const ObjectId*>(value)->IsPersistable());
			stream.WriteObjectId(*(const ObjectId*)value);
		} else {
			stream.WriteUInt8(0);
		}
		break;
	case ObjectFieldType::AbilityArray:
	case ObjectFieldType::UnkArray:
	case ObjectFieldType::Int32Array:
	case ObjectFieldType::Int64Array:
	case ObjectFieldType::ScriptArray:
	case ObjectFieldType::Unk2Array:
	case ObjectFieldType::ObjArray:
	case ObjectFieldType::SpellArray:
		if (!value) {
			stream.WriteUInt8(0);
		} else {
			stream.WriteUInt8(1);

			auto header = reinterpret_cast<const ArrayHeader*>(value);
			stream.WriteBytes((const uint8_t*) value, sizeof(ArrayHeader) + header->elSize * header->count);

			// Save the array index bitmap as well
			arrayIdxBitmaps.SerializeToStream(header->idxBitmapId, stream);
		}
		break;
	default:
		throw TempleException("Cannot write unknown field type to file.");
	}
}

int32_t GameObjectBody::GetInt32(obj_f field, size_t index) const
{
	return GetInt32Array(field)[index];
}

void GameObjectBody::SetInt32(obj_f field, size_t index, int32_t value)
{
	GetMutableInt32Array(field).Set(index, value);
}

void GameObjectBody::AppendInt32(obj_f field, int32_t value){
	GetMutableInt32Array(field).Append(value);
}

void GameObjectBody::RemoveInt32(obj_f field, size_t index)
{
	GetMutableInt32Array(field).Remove(index);
}

int64_t GameObjectBody::GetInt64(obj_f field, size_t index) const
{
	return GetInt64Array(field)[index];
}

void GameObjectBody::SetInt64(obj_f field, size_t index, int64_t value)
{
	GetMutableInt64Array(field).Set(index, value);
}

void GameObjectBody::RemoveInt64(obj_f field, size_t index)
{
	GetMutableInt64Array(field).Remove(index);
}

ObjectId GameObjectBody::GetObjectId(obj_f field, size_t index) const
{
	return GetObjectIdArray(field)[index];
}

void GameObjectBody::SetObjectId(obj_f field, size_t index, const ObjectId & value)
{
	GetMutableObjectIdArray(field).Set(index, value);
}

void GameObjectBody::RemoveObjectId(obj_f field, size_t index)
{
	GetMutableObjectIdArray(field).Remove(index);
}

objHndl GameObjectBody::GetObjHndl(obj_f field, size_t index) const
{
	auto objId = GetObjectId(field, index);
	if (objId.IsHandle()) {
		return objId.GetHandle();
	}
	return objHndl::null;
}

bool GameObjectBody::GetValidObjHndl(obj_f field, size_t index, objHndl* handleOut) {
	Expects(objectFields.GetType(field) == ObjectFieldType::ObjArray);

	// Get the stored object id
	auto objId = GetObjectId(field, index);

	// Null handles are considered valid by this function
	if (objId.IsNull()) {
		*handleOut = objHndl();
		return true;
	}

	// If it's not a handle id, that is probably an error
	if (!objId.IsHandle()) {
		*handleOut = objHndl();
		return false;
	}

	// Use the handle from the id and validate it before returning it
	auto handle = objId.GetHandle();
	if (!objSystem->IsValidHandle(handle)) {
		// Since we know the handle is invalid, we'll fix the issue in the object
		RemoveObjectId(field, index);
		*handleOut = objHndl();
		return false;
	} else {
		*handleOut = handle;
		return true;
	}
}

void GameObjectBody::SetScript(obj_f field, size_t index, const ObjectScript& script)
{
	GetMutableScriptArray(field).Set(index, script);
}

ObjectScript GameObjectBody::GetScript(obj_f field, size_t index) const
{
	return GetScriptArray(field)[index];
}

void GameObjectBody::RemoveScript(obj_f field, size_t index)
{
	GetMutableScriptArray(field).Remove(index);
}

void GameObjectBody::SetSpell(obj_f field, size_t index, const SpellStoreData& spell)
{
	GetMutableSpellArray(field).Set(index, spell);
}

SpellStoreData GameObjectBody::GetSpell(obj_f field, size_t index) const
{
	return GetSpellArray(field)[index];
}

void GameObjectBody::RemoveSpell(obj_f field, size_t index)
{
	GetMutableSpellArray(field).Remove(index);
}

void GameObjectBody::ClearArray(obj_f field)
{
	Expects(!IsProto());

	switch (objectFields.GetType(field)) {
		case ObjectFieldType::Int32Array:
		case ObjectFieldType::AbilityArray:
			GetMutableInt32Array(field).Clear();
			break;
		case ObjectFieldType::Int64Array:
			GetMutableInt64Array(field).Clear();
			break;
		case ObjectFieldType::ScriptArray:
			GetMutableScriptArray(field).Clear();
			break;
		case ObjectFieldType::ObjArray:
			GetMutableObjectIdArray(field).Clear();
			break;
		case ObjectFieldType::SpellArray:
			GetMutableSpellArray(field).Clear();
			break;
		default: 
			throw TempleException("Cannot clear a non-array field: {}", 
				objectFields.GetFieldName(field));
	}
		
}

void GameObjectBody::SetInt32(obj_f field, int32_t value)
{
	Expects(objectFields.GetFieldDef(field).type == ObjectFieldType::Int32);
	auto storageLoc = GetMutableStorageLocation<int32_t>(field);
	if (!storageLoc) {
		logger->error("Trying to set field {} on object {}, whose type doesn't support it.", 
			GetObjectFieldName(field), id.ToString());
		return;
	}
	*storageLoc = value;
}

void GameObjectBody::SetFloat(obj_f field, float value)
{
	Expects(objectFields.GetFieldDef(field).type == ObjectFieldType::Float32);
	auto storageLoc = GetMutableStorageLocation<float>(field);
	if (!storageLoc) {
		logger->error("Trying to set field {} on object {}, whose type doesn't support it.",
			GetObjectFieldName(field), id.ToString());
		return;
	}
	*storageLoc = value;
}

void GameObjectBody::SetInt64(obj_f field, int64_t value)
{
	Expects(objectFields.GetFieldDef(field).type == ObjectFieldType::Int64);
	auto storageLoc = GetMutableStorageLocation<int64_t*>(field);
	if (!storageLoc) {
		logger->error("Trying to set field {} on object {}, whose type doesn't support it.",
			GetObjectFieldName(field), id.ToString());
		return;
	}
	if (*storageLoc) {
		**storageLoc = value;
	} else {
		*storageLoc = new int64_t(value);
	}
	if (field == obj_f_location) {
		objSystem->FindNodeMove(objSystem->GetHandleById(id));
	}
}

void GameObjectBody::SetObjHndl(obj_f field, objHndl value)
{
	if (!value) {
		SetObjectId(field, ObjectId::CreateNull());
	} else {
		SetObjectId(field, ObjectId::CreateHandle(value));
	}
}


void GameObjectBody::SetObjHndl(obj_f field, size_t index, objHndl value)
{
	if (!value) {
		SetObjectId(field, index, ObjectId::CreateNull());
	}
	else {
		SetObjectId(field, index, ObjectId::CreateHandle(value));
	}
}

void GameObjectBody::SetObjectId(obj_f field, const ObjectId &value)
{
	// TODO: Additional validation regarding internal flags that checks whether
	// this id is valid for the id storage state of this object (persistent ids vs. handles)
	Expects(objectFields.GetFieldDef(field).type == ObjectFieldType::Obj);
	auto storageLoc = GetMutableStorageLocation<ObjectId*>(field);
	if (!storageLoc) {
		logger->error("Trying to set field {} on object {}, whose type doesn't support it.",
			GetObjectFieldName(field), id.ToString());
		return;
	}
	if (*storageLoc) {
		**storageLoc = value;
	} else {
		*storageLoc = new ObjectId(value);
	}
}

uint32_t GameObjectBody::GetInternalFlags() const
{
	Expects(!IsProto());
	return transientProps.internalFlags;
}

void GameObjectBody::SetInternalFlags(uint32_t internalFlags)
{
	Expects(!IsProto());
	transientProps.internalFlags = internalFlags;
}

void GameObjectBody::ResetDiffs()
{
	// Reset diff state to 0
	if (hasDifs) {
		for (size_t i = 0; i < objectFields.GetBitmapBlockCount(type); ++i) {
			difBitmap[i] = 0;
		}
	}
	hasDifs = 0;
}

void GameObjectBody::ResetField(obj_f field)
{
	// This method has no effect on prototypes
	if (IsProto()) {
		return;
	}

	auto &fieldDef = objectFields.GetFieldDef(field);
	if (HasDataForField(fieldDef)) {
		auto idx = GetPropCollIdx(fieldDef);
		// Copy the property behind the one we remove
		--propCollectionItems;
		for (size_t i = idx; i < propCollectionItems; ++i) {
			propCollection[i] = propCollection[i + 1];
		}
		propCollection = (void**) realloc(propCollection, sizeof(void*) * propCollectionItems);

		propCollBitmap[fieldDef.bitmapBlockIdx] &= ~fieldDef.bitmapMask;
		difBitmap[fieldDef.bitmapBlockIdx] &= ~fieldDef.bitmapMask;
	}	
}

bool GameObjectBody::IsStatic() const
{
	if (type == obj_t_portal || type == obj_t_scenery || type == obj_t_trap) {
		return !HasFlag(OF_DYNAMIC);
	}
	return false;
}

bool GameObjectBody::ForEachField(std::function<bool(obj_f, void**)> callback)
{
	if (IsProto()) {
		return objectFields.IterateTypeFields(type, [=](obj_f field) {
			auto& fieldDef = objectFields.GetFieldDef(field);
			auto storageLoc = &propCollection[fieldDef.protoPropIdx];
			return callback(field, storageLoc);
		});
	}

	return objectFields.IterateTypeFields(type, [=](obj_f field) {
		auto& fieldDef = objectFields.GetFieldDef(field);

		// Does this object have the prop?
		if (HasDataForField(fieldDef)) {
			auto idx = GetPropCollIdx(fieldDef);
			auto storageLoc = &propCollection[idx];
			return callback(field, storageLoc);
		}

		return true;
	});
}

bool GameObjectBody::ForEachField(std::function<bool(obj_f, const void*)> callback) const
{
	if (IsProto()) {
		return objectFields.IterateTypeFields(type, [=](obj_f field) {
			auto& fieldDef = objectFields.GetFieldDef(field);
			auto value = propCollection[fieldDef.protoPropIdx];
			return callback(field, value);
		});
	}

	return objectFields.IterateTypeFields(type, [=](obj_f field) {
		auto& fieldDef = objectFields.GetFieldDef(field);

		// Does this object have the prop?
		if (HasDataForField(fieldDef)) {
			auto idx = GetPropCollIdx(fieldDef);
			auto value = propCollection[idx];
			return callback(field, value);
		}

		return true;
	});
}

bool GameObjectBody::SupportsField(obj_f field) const
{
	return objectFields.DoesTypeSupportField(type, field);
}

void GameObjectBody::UnfreezeIds()
{
	auto internalFlags = GetInternalFlags();

	if (!(internalFlags & 1)) {
		logger->info("Object references in object {} are already unfrozen.", id.ToString());
		return;
	}

	if (IsCritter()) {
		logger->trace("  Unfreezing {}", objSystem->GetHandleById(this->id) );
	}
	

	ForEachField([=](obj_f field, void **storageLocation) {
		if (!*storageLocation) {
			return true;
		}

		auto fieldType = objectFields.GetType(field);
		if (fieldType == ObjectFieldType::Obj) {
			auto objectIdPtr = reinterpret_cast<ObjectId**>(storageLocation);
			auto handle = objSystem->GetHandleById(**objectIdPtr);
			if (handle) {
				**objectIdPtr = ObjectId::CreateHandle(handle);
			} else {
				**objectIdPtr = ObjectId::CreateNull();
			}
		} else if (fieldType == ObjectFieldType::ObjArray) {
			auto objectIdArray = GameObjectIdArray(reinterpret_cast<ArrayHeader**>(storageLocation));

			if (field == obj_f_critter_inventory_list_idx) {
				objectIdArray.ForEachIndex([&](size_t idx) {
					auto& objId = objectIdArray[idx];
					auto handle = objSystem->GetHandleById(objId);
					if (!handle) {
						logger->error("Null inventory item! idx = {}, GUID = {}", idx, objId.ToString());
					}
				});
			};

			objectIdArray.ForEachIndex([&](size_t idx) {
				auto &objId = objectIdArray[idx];
				auto handle = objSystem->GetHandleById(objId);
				if (handle) {
					objectIdArray.Set(idx, ObjectId::CreateHandle(handle));
				}
				else {
					objectIdArray.Set(idx, ObjectId::CreateNull());
				}
			});
		}
		return true;
	});

	// Mark as not frozen
	internalFlags &= ~1;
	SetInternalFlags(internalFlags);
}

void GameObjectBody::FreezeIds()
{
	auto internalFlags = GetInternalFlags();

	if (internalFlags & 1) {
		logger->info("Object references in object {} are already frozen.", id.ToString());
		return;
	}

	ForEachField([=](obj_f field, void **storageLocation) {
		if (!*storageLocation) {
			return true;
		}

		auto fieldType = objectFields.GetType(field);
		if (fieldType == ObjectFieldType::Obj) {
			auto objectIdPtr = reinterpret_cast<ObjectId**>(storageLocation);
			if ((*objectIdPtr)->IsHandle()) {
				auto obj = objSystem->GetObject(objSystem->GetHandleById(**objectIdPtr));
				**objectIdPtr = obj->id;
			}
		} else if (fieldType == ObjectFieldType::ObjArray) {
			auto objectIdArray = GameObjectIdArray(reinterpret_cast<ArrayHeader**>(storageLocation));
			objectIdArray.ForEachIndex([&](size_t idx) {
				auto &objId = objectIdArray[idx];
				if (objId.IsHandle()) {
					auto obj = objSystem->GetObject(objSystem->GetHandleById(objId));
					objectIdArray.Set(idx, obj->id);
				}
			});
		}
		return true;
	});

	// Mark as frozen
	internalFlags |= 1;
	SetInternalFlags(internalFlags);
}

void GameObjectBody::PruneNullInventoryItems()
{
	auto handle = objSystem->GetHandleById(this->id);
	if (!handle) return;

	auto flags = GetInternalFlags();
	if (flags & 1) { // obj refs are still frozen
		logger->error("PruneNullInventoryItems: cannot do for frozen refs. Obj {} {}", handle, id.ToString());
		return;
	}


	auto invenField = inventory.GetInventoryListField(handle);
	auto invenCountField = inventory.GetInventoryNumField(handle);

	auto invenCountFromField = GetInt32(invenCountField);
	auto invenCount = GetObjectIdArray(invenField).GetSize();
	if (invenCount != invenCountFromField) {
		logger->error("Inventory array count does not equal associated num field.  Array: {}, Field: {}", invenCount, invenCountFromField);
	}

	for (auto i = 0; i < invenCount; ++i) {
		auto item = GetObjHndl(invenField, i);
		if (!item) {
			logger->error("PruneNullInventoryItems: null inventory item (idx {}) on {}!", i, handle);

			auto lastItem = GetObjHndl(invenField, invenCount - 1);
			SetObjHndl(invenField, i--, lastItem);

			RemoveObjectId(invenField, invenCount - 1);
			invenCountFromField--;
			invenCount--;
			SetInt32(invenCountField, invenCountFromField);
			continue;
		}
	}
}

std::unique_ptr<GameObjectBody> GameObjectBody::Clone() const {
	
	if (!id.IsPermanent() && !id.IsPositional()) {
		throw TempleException("Cannot copy an object with an id of type {}", (int)id.subtype);
	}

	auto obj = std::make_unique<GameObjectBody>();
	obj->id = ObjectId::CreatePermanent();

	obj->protoId = protoId;
	obj->protoHandle = protoHandle;
	obj->type = type;

	obj->propCollectionItems = propCollectionItems;
	obj->hasDifs = 0;
	obj->propCollection = new void*[obj->propCollectionItems];

	auto bitmapLen = objectFields.GetBitmapBlockCount(obj->type);
	obj->propCollBitmap = new uint32_t[bitmapLen * 2];
	obj->difBitmap = &obj->propCollBitmap[bitmapLen];
	memcpy(&obj->propCollBitmap[0], &propCollBitmap[0], bitmapLen * sizeof(uint32_t));

	memset(&obj->transientProps, 0, sizeof(obj->transientProps));

	// Copy field values
	obj->ForEachField([&](obj_f field, void** storage) {
		auto src = GetStorageLocation<void*>(field);

		switch (objectFields.GetType(field)) {
			// Storage by value
		case ObjectFieldType::Float32:
		case ObjectFieldType::Int32:
			*storage = *src;
			break;
		case ObjectFieldType::Int64: 
			if (*src) {
				*storage = new int64_t(**GetStorageLocation<int64_t*>(field));
			}
			break;
		case ObjectFieldType::String: 
			if (*src) {
				*storage = _strdup(*GetStorageLocation<char*>(field));
			}
			break;
		case ObjectFieldType::Obj: 
			if (*src) {
				*storage = new ObjectId(**GetStorageLocation<ObjectId*>(field));
			}
			break;
		case ObjectFieldType::AbilityArray:
		case ObjectFieldType::UnkArray:
		case ObjectFieldType::Int32Array:
		case ObjectFieldType::Int64Array:
		case ObjectFieldType::ScriptArray:
		case ObjectFieldType::Unk2Array:
		case ObjectFieldType::ObjArray:
		case ObjectFieldType::SpellArray:
			if (*src) {
				auto arr = (ArrayHeader*) *src;
				auto dest = (ArrayHeader*) malloc(sizeof(ArrayHeader) + arr->elSize * arr->count);
				memcpy(dest, arr, sizeof(ArrayHeader) + arr->elSize * arr->count);
				dest->idxBitmapId = arrayIdxBitmaps.Clone(dest->idxBitmapId);
				*storage = dest;
			}
			break;
		default: 
			throw TempleException("Unable to copy field {}", objectFields.GetFieldName(field));
		}

		return true;
	});

	return std::move(obj);
}

locXY GameObjectBody::GetLocation() const
{
	return locXY::fromField(GetInt64(obj_f_location));
}

LocAndOffsets GameObjectBody::GetLocationFull() const
{
	return  LocAndOffsets::create(locXY::fromField(GetInt64(obj_f_location)), GetFloat(obj_f_offset_x), GetFloat(obj_f_offset_y));
}

void GameObjectBody::SetLocation(locXY location)
{
	SetInt64(obj_f_location, location);
}




void GameObjectBody::ForEachChild(std::function<void(objHndl item)> callback) const
{
	obj_f countField, indexField;

	if (IsContainer()) {
		indexField = obj_f_container_inventory_list_idx;
		countField = obj_f_container_inventory_num;
	} else if (IsCritter()) {
		indexField = obj_f_critter_inventory_list_idx;
		countField = obj_f_critter_inventory_num;
	} else { 
		return;
	}

	auto count = GetInt32(countField);
	for (int i = 0; i < count; ++i) {
		auto item = GetObjHndl(indexField, i);
		callback(item);
	}
}

bool GameObjectBody::Write(OutputStream &stream) const
{
	Expects(!IsProto());

	stream.WriteUInt32(0x77);
	stream.WriteObjectId(protoId);
	stream.WriteObjectId(id);
	stream.WriteUInt32((uint32_t)type);

	// Write the number of properties we are going to write
	stream.WriteUInt16(propCollectionItems);

	// Write the is-property-set bitmap blocks
	auto bitmapLen = objectFields.GetBitmapBlockCount(type);
	stream.WriteBytes((uint8_t*) &propCollBitmap[0], sizeof(uint32_t) * bitmapLen);

	return ForEachField([&](obj_f field, const void *value) {
		 WriteFieldToStream(objectFields.GetType(field), value, stream);
		 return true;
	});
}

void GameObjectBody::WriteDiffsToStream(OutputStream &stream) const {

	Expects(hasDifs == 1);

	stream.WriteUInt32(0x77); // Version
	stream.WriteUInt32(0x12344321); // Magic header

	// NOTE: this seems redundant since for obj diffs the id comes first anyway
	stream.WriteObjectId(id);
	
	auto bitmapLen = objectFields.GetBitmapBlockCount(type);

	// Validate dif bitmap
	objectFields.IterateTypeFields(type, [&](obj_f field)
	{
		auto& fieldDef = objectFields.GetFieldDef(field);
		bool hasDifs = (difBitmap[fieldDef.bitmapBlockIdx] & fieldDef.bitmapMask) != 0;
		bool hasData = HasDataForField(fieldDef);
		if (hasDifs && !hasData) {
			logger->error("Obj has difs for {}, but no data!", objectFields.GetFieldName(field));
		}
		return true;
	});
	
	stream.WriteBytes((uint8_t*) &difBitmap[0], sizeof(uint32_t) * bitmapLen);

	// Write each field that is different
	ForEachField([&](obj_f field, const void *storage) {
		auto& fieldDef = objectFields.GetFieldDef(field);
		if (difBitmap[fieldDef.bitmapBlockIdx] & fieldDef.bitmapMask) {
			WriteFieldToStream(fieldDef.type, storage, stream);
		}
		return true;
	});

	stream.WriteUInt32(0x23455432); // Magic footer

}

void GameObjectBody::LoadDiffsFromFile(objHndl handle, TioFile* file) {

	if (!(transientProps.internalFlags & 1)) {
		throw TempleException("Cannot load difs for an object that is not storing persistable ids.");
	}

	uint32_t version;
	if (tio_fread(&version, sizeof(uint32_t), 1, file) != 1
		|| version != 0x77) {
		throw TempleException("Cannot read object file version.");
	}

	uint32_t magicNumber;
	if (tio_fread(&magicNumber, sizeof(uint32_t), 1, file) != 1
		|| magicNumber != 0x12344321) {
		throw TempleException("Cannot read diff header.");
	}

	ObjectId id;
	if (tio_fread(&id, sizeof(ObjectId), 1, file) != 1) {
		throw TempleException("Cannot read object id.");
	}

	if (!this->id.IsNull()) {
		if (this->id != id) {
			throw TempleException("ID {} of diff record differs from object id {}", id, this->id);
		}
	} else {
		this->id = id;
		if (!id.IsNull()) {
			objSystem->AddToIndex(id, handle);
			//logger->debug("Loaded object id {} (via diffs)", id.ToString());
		}
	}

	auto bitmapLen = objectFields.GetBitmapBlockCount(type);
	if (tio_fread(&difBitmap[0], bitmapLen * sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to read diff property bitmap");
	}
	hasDifs = 1;
		
	// TODO: Make more efficient
	objectFields.IterateTypeFields(type, [&](obj_f field) {
		// Is it marked for diffs?
		auto& fieldDef = objectFields.GetFieldDef(field);
		if (!(difBitmap[fieldDef.bitmapBlockIdx] & fieldDef.bitmapMask)) {
			return true;
		}

		// Read the object field value
		auto storageLoc = GetMutableStorageLocation<void*>(field);
		FreeStorage(fieldDef.type, *storageLoc);
		*storageLoc = nullptr;
		objSystem->ReadFieldValue(field, storageLoc, file);
		return true;
	});

	if (tio_fread(&magicNumber, sizeof(uint32_t), 1, file) != 1
		|| magicNumber != 0x23455432) {
		throw TempleException("Cannot read diff footer.");
	}

	objSystem->FindNodeMove(handle);

}

bool GameObjectBody::ValidateFieldForType(obj_f field) const
{
	if (!objectFields.DoesTypeSupportField(type, field)) {
		logger->error("Accessing unsupported field {} ({}) in type {}",
			objectFields.GetFieldName(field), field, GetObjectTypeName(type));
		return false;
	}
	return true;
}

void ** GameObjectBody::GetTransientStorage(obj_f field)
{
	Expects(objectFields.IsTransient(field));
	int index = field - obj_f_transient_begin - 1;
	auto storagePtr = reinterpret_cast<void**>(&transientProps);
	return &storagePtr[index];
}

void * const * GameObjectBody::GetTransientStorage(obj_f field) const
{
	Expects(objectFields.IsTransient(field));
	int index = field - obj_f_transient_begin - 1;
	auto storagePtr = reinterpret_cast<void* const *>(&transientProps);
	return &storagePtr[index];
}

bool GameObjectBody::HasDataForField(const ObjectFieldDef &field) const
{
	Expects(!IsProto());
	return (propCollBitmap[field.bitmapBlockIdx] & field.bitmapMask) != 0;
}

size_t GameObjectBody::GetPropCollIdx(const ObjectFieldDef & field) const
{
	Expects(!IsProto());

	size_t count = 0;
	for (int i = 0; i < field.bitmapBlockIdx; ++i) {
		count += arrayIdxBitmaps.PopCnt(propCollBitmap[i]);
	}
	count += arrayIdxBitmaps.PopCntConstrained(
		propCollBitmap[field.bitmapBlockIdx],
		field.bitmapBitIdx
	);

	return count;
}

void GameObjectBody::MarkChanged(obj_f field)
{
	if (IsProto()) {
		return; // Dont mark prototype objects as changed
	}

	if (objectFields.IsTransient(field)) {
		return; // Dont mark transient fields as changed
	}

	hasDifs = true;
	auto &fieldDef = objectFields.GetFieldDef(field);
	difBitmap[fieldDef.bitmapBlockIdx] |= fieldDef.bitmapMask;
}

objHndl GameObjectBody::GetProtoHandle() const
{
	if (!protoHandle) {
		const_cast<GameObjectBody*>(this)->protoHandle
			= objSystem->GetHandleById(protoId);
	}
	return protoHandle;
}

const GameObjectBody * GameObjectBody::GetProtoObj() const
{
	auto protoHandle = GetProtoHandle();
	if (!protoHandle) {
		logger->error("Proto not found {}", protoId.body.protoId);
		return nullptr;
	}
	return objSystem->mObjRegistry->Get(protoHandle);
}

void GameObjectBody::FreeStorage(ObjectFieldType type, void * storage)
{
	if (!storage) {
		return;
	}

	switch (type) {
	case ObjectFieldType::Int64:
		delete reinterpret_cast<int64_t*>(storage);
		break;
	case ObjectFieldType::Obj:
		delete reinterpret_cast<objHndl*>(storage);
		break;
	case ObjectFieldType::String:
		delete reinterpret_cast<char*>(storage);
		break;
	case ObjectFieldType::AbilityArray:
	case ObjectFieldType::UnkArray:
	case ObjectFieldType::Int32Array:
	case ObjectFieldType::Int64Array:
	case ObjectFieldType::ScriptArray:
	case ObjectFieldType::Unk2Array:
	case ObjectFieldType::ObjArray:
	case ObjectFieldType::SpellArray:
		arrayIdxBitmaps.Free(((ArrayHeader*)storage)->idxBitmapId);
		free(storage);
		break;
	default:
		break; // No allocated memory
	}
}

template<typename T>
const T * GameObjectBody::GetStorageLocation(obj_f field) const
{
	if (!ValidateFieldForType(field)) {
		logger->error("Trying to get field {} on object {}, whose type doesn't support it.",
			GetObjectFieldName(field), id.ToString());
		return nullptr;
	}

	auto& fieldDef = objectFields.GetFieldDef(field);

	const void* storageLocation;
	if (IsProto()) {
		storageLocation = &propCollection[fieldDef.protoPropIdx];
	} else if (objectFields.IsTransient(field)) {
		storageLocation = GetTransientStorage(field);
	} else if (HasDataForField(fieldDef)) {
		auto propCollIdx = GetPropCollIdx(fieldDef);
		storageLocation = &propCollection[propCollIdx];
	} else {
		// Fall back to the storage in the parent prototype
		storageLocation = &GetProtoObj()->propCollection[fieldDef.protoPropIdx];
	}

	return reinterpret_cast<const T*>(storageLocation);
}

template<typename T>
T* GameObjectBody::GetMutableStorageLocation(obj_f field)
{
	static_assert(sizeof(T) == sizeof(intptr_t), "Storage locations must have pointer size");

	if (!ValidateFieldForType(field)) {
		return nullptr;
	}

	auto& fieldDef = objectFields.GetFieldDef(field);
	
	void* storageLocation;
	if (IsProto()) {
		storageLocation = &propCollection[fieldDef.protoPropIdx];
	} else if (objectFields.IsTransient(field)) {
		storageLocation = GetTransientStorage(field);
	} else if (HasDataForField(fieldDef)) {
		auto propCollIdx = GetPropCollIdx(fieldDef);
		storageLocation = &propCollection[propCollIdx];
	} else {
		// Allocate the storage location
		propCollBitmap[fieldDef.bitmapBlockIdx] |= fieldDef.bitmapMask;

		// TODO: This should just be a vector or similar so we can use the STL's insert function
		propCollection = (void**)realloc(propCollection, (propCollectionItems + 1) * sizeof(void*));
		size_t desiredIdx = GetPropCollIdx(fieldDef);
		propCollectionItems++;
				
		for (size_t i = propCollectionItems - 1; i > desiredIdx; --i) {
			propCollection[i] = propCollection[i - 1];
		}

		propCollection[desiredIdx] = nullptr;
		storageLocation = &propCollection[desiredIdx];

		// TODO For certain fields we need to copy the value from the prototype here
		if (objectFields.IsArrayType(fieldDef.type)) {
			auto proto = GetProtoObj();
			auto arr = reinterpret_cast<ArrayHeader*>(proto->propCollection[fieldDef.protoPropIdx]);
			if (arr) {
				auto arrSize = sizeof(ArrayHeader) + arr->count * arr->elSize;
				auto arrCopy = (ArrayHeader*) malloc(arrSize);
				memcpy(arrCopy, arr, arrSize);
				arrCopy->idxBitmapId = arrayIdxBitmaps.Clone(arr->idxBitmapId);
				propCollection[desiredIdx] = arrCopy;
			}
		}
	}

	MarkChanged(field);

	return reinterpret_cast<T*>(storageLocation);
}
