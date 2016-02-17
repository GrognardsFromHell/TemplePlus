
#include "stdafx.h"
#include "objsystem.h"
#include "objfields.h"
#include "objregistry.h"

#include "util/fixes.h"
#include <util/streams.h>

static class ObjSystemHooks : public TempleFix {
public:
	
	const char * name() override
	{
		return "Object System Hooks";
	}

	void apply() override
	{

		// obj_is_proto
		replaceFunction<BOOL(objHndl)>(0x1009c950, [](objHndl obj) {
			return GetObj(obj)->IsProto() ? TRUE : FALSE;			
		});

		// obj_has_difs
		replaceFunction<BOOL(objHndl)>(0x1009c980, [](objHndl obj) {
			return (GetObj(obj)->hasDifs != 0) ? TRUE : FALSE;
		});

		// obj_clear_dif_info
		replaceFunction<void(objHndl)>(0x1009c9b0, [](objHndl obj) {
			GetObj(obj)->ResetDiffs();
		});

		//// obj_supports_field
		//replaceFunction<BOOL(objHndl, obj_f)>(0x1009ca00, [](objHndl obj, obj_f field) {
		//	return objectFields.DoesTypeSupportField(GetObj(obj)->type, field) ? TRUE : FALSE;
		//});

		// obj_get_id
		replaceFunction<ObjectId *(ObjectId *, objHndl)>(0x1009ca40, [](ObjectId *objectId, objHndl handle) {			
			*objectId = objSystem->GetPersistableId(handle);
			return objectId;
		});

		// obj_make_proto
		replaceFunction<void(ObjectId *, objHndl, int)>(0x1009cb50, [](ObjectId *idOut, objHndl handle, int protoId) {
			auto obj = GetObj(handle);
			obj->id = ObjectId::CreatePrototype(protoId);
			*idOut = obj->id;
			// TODO: I don't like this, because it doesnt remove the old obj->id from the index
			objSystem->mObjRegistry->AddToIndex(handle, obj->id);

		});

		static int iterationId = 0;
		static ObjRegistry::Container::iterator it;

		// Object_Tables_FirstNonProtoHandle
		replaceFunction<BOOL(objHndl *, int *)>(0x1009cc10, [](objHndl *handleOut, int *itStateOut) {
			*itStateOut = ++iterationId;

			if (!objSystem) {
				logger->error("Object system is being used after being destroyed");
				return FALSE;
			}
			
			it = objSystem->mObjRegistry->begin();

			while (it != objSystem->mObjRegistry->end()) {
				if (it->second->IsProto()) {
					++it;
					continue;
				}

				*handleOut = it->first;
				++it;
				return TRUE;
			}

			return FALSE;
		});
		// Object_Tables_NextNonProtoHandle
		replaceFunction<BOOL(objHndl *, int *)>(0x1009ccb0, [](objHndl *handleOut, int *itStateOut) {
			if (*itStateOut != iterationId) {
				throw TempleException("Only one iterator may be active at the same time.");
			}

			while (it != objSystem->mObjRegistry->end()) {
				if (it->second->IsProto()) {
					++it;
					continue;
				}

				*handleOut = it->first;
				++it;
				return TRUE;
			}

			return FALSE;
		});

		// obj_unload
		replaceFunction<void(objHndl)>(0x1009e0d0, [](objHndl handle) {
			objSystem->Remove(handle);
		});

		// obj_get_int32
		replaceFunction<int32_t(objHndl, obj_f)>(0x1009e1d0, [](objHndl handle, obj_f field) {
			// Special handling for some spellslinger methods that do not check for null-handles
			if (!handle) {
				return 0;
			}
			return GetObj(handle)->GetInt32(field);
		});

		// obj_get_float32
		replaceFunction<float(objHndl, obj_f)>(0x1009e260, [](objHndl handle, obj_f field) {
			return GetObj(handle)->GetFloat(field);
		});

		// obj_get_int64
		replaceFunction<int64_t(objHndl, obj_f)>(0x1009e2e0, [](objHndl handle, obj_f field) {
			return GetObj(handle)->GetInt64(field);
		});

		// obj_get_handle
		replaceFunction<objHndl(objHndl, obj_f)>(0x1009e360, [](objHndl handle, obj_f field) {
			return GetObj(handle)->GetObjHndl(field);
		});

		// obj_get_string
		replaceFunction<const char *(objHndl, obj_f)>(0x1009e430, [](objHndl handle, obj_f field) {
			return GetObj(handle)->GetString(field);
		});

		// obj_get_string_copy
		replaceFunction<void(objHndl, obj_f, char **)>(0x1009e4e0, [](objHndl handle, obj_f field, char **strOut) {
			auto str = GetObj(handle)->GetString(field);
			*strOut = _strdup(str);
		});

		// obj_array_get_int32
		replaceFunction<int32_t(objHndl, obj_f, size_t)>(0x1009e5c0, [](objHndl handle, obj_f field, size_t index) {
			return GetObj(handle)->GetInt32(field, index);
		});

		// obj_set_int32_or_float32
		replaceFunction<void(objHndl, obj_f, int32_t)>(0x100a0190, [](objHndl handle, obj_f field, int32_t value) {
			auto& fieldDef = objectFields.GetFieldDef(field);
			if (fieldDef.type == ObjectFieldType::Float32) {
				float fValue = *reinterpret_cast<float*>(&value);
				GetObj(handle)->SetFloat(field, fValue);
			} else if (fieldDef.type == ObjectFieldType::Int32) {
				GetObj(handle)->SetInt32(field, value);
			} else {
				throw TempleException("Trying to set field {} to an integer value, but it is of type {}",
					objectFields.GetFieldName(field), (int)fieldDef.type);
			}
		});

		// obj_set_int64
		replaceFunction<void(objHndl, obj_f, int64_t)>(0x100a0200, [](objHndl handle, obj_f field, int64_t value) {
			GetObj(handle)->SetInt64(field, value);
		});

		// obj_set_handle
		replaceFunction<void(objHndl, obj_f, objHndl)>(0x100a0280, [](objHndl handle, obj_f field, objHndl value) {
			GetObj(handle)->SetObjHndl(field, value);
		});

		// obj_supports_field
		replaceFunction<BOOL(objHndl, obj_f)>(0x1009ca00, [](objHndl obj, obj_f field) {
			return GetObj(obj)->SupportsField(field) ? TRUE : FALSE;
		});

		// obj_array_set_ptr
		replaceFunction<void(objHndl, obj_f, size_t, const void *)>(0x100a1540, [](objHndl obj, obj_f field, size_t index, const void *valuePtr) {
			auto type = objectFields.GetType(field);

			switch (type) {
			case ObjectFieldType::AbilityArray:
			case ObjectFieldType::Int32Array: {
				int32_t value = *reinterpret_cast<const int32_t*>(valuePtr);
				GetObj(obj)->SetInt32(field, index, value);
				break;
			}
			case ObjectFieldType::Int64Array: {
				int64_t value = *reinterpret_cast<const int64_t*>(valuePtr);
				GetObj(obj)->SetInt64(field, index, value);
				break;
			}
			case ObjectFieldType::ScriptArray: {
				const ObjectScript& value = *reinterpret_cast<const ObjectScript*>(valuePtr);
				GetObj(obj)->SetScript(field, index, value);
				break;
			}
			case ObjectFieldType::ObjArray: {
				const ObjectId& value = *reinterpret_cast<const ObjectId*>(valuePtr);
				GetObj(obj)->SetObjectId(field, index, value);
				break;
			}
			case ObjectFieldType::SpellArray: {
				const SpellStoreData& spellData = *reinterpret_cast<const SpellStoreData*>(valuePtr);
				GetObj(obj)->SetSpell(field, index, spellData);
				break; 
			}
			default:
				throw TempleException("Unsupported field type for set_array_ptr");
			}
		});

		// obj_array_get_ptr
		replaceFunction<void(objHndl, obj_f, size_t, void *)>(0x1009e770, [](objHndl obj, obj_f field, size_t index, void *dataOut) {
			auto type = objectFields.GetType(field);

			switch (type) {
			case ObjectFieldType::AbilityArray:
			case ObjectFieldType::Int32Array: {
				*reinterpret_cast<int32_t*>(dataOut) = GetObj(obj)->GetInt32(field, index);
				break;
			}
			case ObjectFieldType::Int64Array: {
				*reinterpret_cast<int64_t*>(dataOut) = GetObj(obj)->GetInt64(field, index);
				break;
			}
			case ObjectFieldType::ScriptArray: {
				*reinterpret_cast<ObjectScript*>(dataOut) = GetObj(obj)->GetScriptArray(field)[index];
				break;
			}
			case ObjectFieldType::ObjArray: {
				*reinterpret_cast<ObjectId*>(dataOut) = GetObj(obj)->GetObjectId(field, index);
				break;
			}
			case ObjectFieldType::SpellArray: {
				*reinterpret_cast<SpellStoreData*>(dataOut) = GetObj(obj)->GetSpellArray(field)[index];
				break;
			}
			default:
				throw TempleException("Unsupported field type for get_array_ptr");
			}
		});

		// obj_array_set_int32
		replaceFunction<void(objHndl, obj_f, size_t, int32_t, int32_t)>(0x100a1310, [](objHndl obj, obj_f field, size_t idx, int32_t value, int32_t value2) {
			// Fixes a bug in vanilla ToEE
			if (field == obj_f_critter_seen_maplist)
			{
				GetObj(obj)->SetInt64(field, idx, value);
			}
			else if(field == obj_f_npc_standpoints) {
				GetObj(obj)->SetInt64(field, idx, value | (static_cast<int64_t>(value2) << 32) );
			} else {
				GetObj(obj)->SetInt32(field, idx, value);
			}
		});

		// obj_set_string
		replaceFunction<void(objHndl, obj_f, const char *)>(0x100a0490, [](objHndl obj, obj_f field, const char *data) {
			GetObj(obj)->SetString(field, data);
		});

		// obj_array_actual_size
		replaceFunction<size_t(objHndl, obj_f)>(0x1009e7e0, [](objHndl obj, obj_f field) {
			auto type = objectFields.GetType(field);

			switch (type) {
			case ObjectFieldType::AbilityArray:
			case ObjectFieldType::Int32Array:
				return GetObj(obj)->GetInt32Array(field).GetSize();
			case ObjectFieldType::Int64Array:
				return GetObj(obj)->GetInt64Array(field).GetSize();
			case ObjectFieldType::ScriptArray: 
				return GetObj(obj)->GetScriptArray(field).GetSize();
			case ObjectFieldType::ObjArray: 
				return GetObj(obj)->GetObjectIdArray(field).GetSize();
			case ObjectFieldType::SpellArray: 
				return GetObj(obj)->GetSpellArray(field).GetSize();
			default:
				throw TempleException("Unsupported field type for set_array_ptr");
			}
		});

		// obj_sys_validate_sector
		replaceFunction<BOOL(BOOL)>(0x1009f550, [](BOOL requireHandleRefs) {
			return objSystem->ValidateSector(requireHandleRefs == TRUE) ? TRUE : FALSE;
		});

		// map_obj_unfreezeIds
		replaceFunction<void(objHndl)>(0x1009f9e0, [](objHndl handle) {
			objSystem->UnfreezeIds(handle);
		});

		// map_obj_freezeIds
		replaceFunction<void(objHndl)>(0x100a1080, [](objHndl handle) {
			objSystem->FreezeIds(handle);
		});

		// obj_array_get_handle
		replaceFunction<objHndl(objHndl, obj_f, size_t)>(0x1009e6d0, [](objHndl obj, obj_f field, size_t index) {
			return GetObj(obj)->GetObjHndl(field, index);
		});

		// map_obj_create
		replaceFunction<BOOL(objHndl, locXY, objHndl*)>(0x10028d20, [](objHndl protoHandle, locXY location, objHndl *handleOut) {
			*handleOut = objSystem->CreateObject(protoHandle, location);
			return TRUE;
		});

		// obj_array_set_handle
		replaceFunction<void(objHndl, obj_f, int, objHndl)>(0x100a14a0, [](objHndl obj, obj_f field, int index, objHndl value) {
			GetObj(obj)->SetObjHndl(field, index, value);
		});

		// obj_array_remove
		replaceFunction<void(objHndl, obj_f, int)>(0x100a15b0, [](objHndl obj, obj_f field, int index) {
			auto type = objectFields.GetType(field);

			switch (type) {
			case ObjectFieldType::AbilityArray:
			case ObjectFieldType::Int32Array:
				GetObj(obj)->RemoveInt32(field, index);
				break;
			case ObjectFieldType::Int64Array:
				GetObj(obj)->RemoveInt64(field, index);
				break;
			case ObjectFieldType::ScriptArray:
				GetObj(obj)->RemoveScript(field, index);
				break;
			case ObjectFieldType::ObjArray:
				GetObj(obj)->RemoveObjectId(field, index);
				break;
			case ObjectFieldType::SpellArray:
				GetObj(obj)->RemoveSpell(field, index);
				break;
			default:
				throw TempleException("Unsupported field type for obj_array_remove");
			}
		});

		// write_obj_to_file
		replaceFunction<BOOL(TioFile*, objHndl)>(0x1009fb00, [](TioFile *file, objHndl obj) {
			TioOutputStream stream(file);
			GetObj(obj)->Write(stream);
			return TRUE;
		});

		// obj_array_get_int64
		replaceFunction<int64_t(objHndl, obj_f, size_t)>(0x1009e640, [](objHndl obj, obj_f field, size_t index) {
			auto result = GetObj(obj)->GetInt64(field, index);
			// Fix for misuse of int-64 field in vanilla. 
			// Setter for 32-bit was used, so the upper 32-bit might be junk in old saves
			if (field == obj_f_critter_seen_maplist ) {
				result &= 0xFFFFFFFF;
			}
			return result;
		});

		// obj_get_handle_safe
		replaceFunction<BOOL(objHndl, obj_f, objHndl *)>(0x100a0320, [](objHndl obj, obj_f field, objHndl *handleOut) {
			return GetObj(obj)->GetValidObjHndl(field, handleOut) ? TRUE : FALSE;
		});

		// obj_array_get_handle_safe
		replaceFunction<BOOL(objHndl, obj_f, size_t, objHndl *)>(0x100a1380, [](objHndl obj, obj_f field, size_t index, objHndl *handleOut) {
			return GetObj(obj)->GetValidObjHndl(field, index, handleOut) ? TRUE : FALSE;
		});

		// obj_load
		replaceFunction<BOOL(TioFile*, objHndl*)>(0x100a11a0, [](TioFile *file, objHndl *handleOut) {
			*handleOut = 0;
			try {
				*handleOut = objSystem->LoadFromFile(file);
				return TRUE;
			} catch (TempleException &e) {
				logger->error("Unable to load object: {}", e.what());
				return FALSE;
			}
		});

		// obj_load_mem
		replaceFunction<BOOL(void*, objHndl*)>(0x100a1260, [](void *buffer, objHndl *handleOut) {
			*handleOut = 0;
			try {
				*handleOut = objSystem->LoadFromBuffer(buffer);
				return TRUE;
			}
			catch (TempleException &e) {
				logger->error("Unable to load object: {}", e.what());
				return FALSE;
			}
		});

		// obj_array_clear
		replaceFunction<void(objHndl, obj_f)>(0x1009e860, [](objHndl obj, obj_f field) {
			GetObj(obj)->ClearArray(field);
		});

		// obj_dif_read
		replaceFunction<BOOL(TioFile*, objHndl)>(0x1009fe20, [](TioFile* file, objHndl obj) {
			try {
				GetObj(obj)->LoadDiffsFromFile(obj, file);
				return TRUE;
			} catch (TempleException &e) {
				logger->error("Unable to load object diffs: {}", e.what());
				return FALSE;
			}
		});

		// obj_create_proto
		replaceFunction<void(ObjectType, objHndl*)>(0x100a1930, [](ObjectType type, objHndl *handleOut) {
			*handleOut = objSystem->CreateProto(type);
		});

		// obj_copy
		replaceFunction<BOOL(objHndl, locXY, objHndl*)>(0x10028d70, [](objHndl handle, locXY loc, objHndl *handleOut) {
			*handleOut = objSystem->Clone(handle, loc);
			return TRUE;
		});

		// obj_reset_field
		replaceFunction<void(objHndl, obj_f)>(0x1009e9c0, [](objHndl obj, obj_f field) {
			GetObj(obj)->ResetField(field);
		});

		// obj_write_diffs_to_file
		replaceFunction<BOOL(TioFile*, objHndl)>(0x1009fc10, [](TioFile *fh, objHndl handle) {
			TioOutputStream stream(fh);
			GetObj(handle)->WriteDiffsToStream(stream);
			return TRUE;
		});

		// obj_handle_write_to_mem
		replaceFunction<void(void **dataOut, size_t *sizeOut, objHndl)>(0x1009fb80, [](void **dataOut, size_t *sizeOut, objHndl obj) {
			MemoryOutputStream stream;
			GetObj(obj)->Write(stream);
			
			auto buffer = stream.GetBuffer();
			*dataOut = malloc(buffer.size());
			memcpy(*dataOut, buffer.data(), buffer.size());
			*sizeOut = buffer.size();
		});

	}

private:

	static GameObjectBody* GetObj(objHndl obj) {
		auto result = objSystem->GetObject(obj);
		Expects(!!result);
		return result;
	}

} hooks;
