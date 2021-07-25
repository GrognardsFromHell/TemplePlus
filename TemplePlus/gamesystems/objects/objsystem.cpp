
#include "stdafx.h"
#include "objsystem.h"
#include "objregistry.h"
#include "../gamesystems.h"
#include "../mapsystem.h"
#include "../legacymapsystems.h"
#include "../legacysystems.h"
#include "../map/sector.h"
#include "../../objlist.h"
#include "../../critter.h"
#include "objfields.h"
#include "objfind.h"
#include "arrayidxbitmaps.h"

ObjSystem* objSystem = nullptr;

//*****************************************************************************
//* Obj
//*****************************************************************************

ObjSystem::ObjSystem(const GameSystemConf &config) {

	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1009dff0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Obj");
	}

	mObjRegistry = std::make_unique<ObjRegistry>();
	mObjFind = std::make_unique<ObjFindSupport>();

	Expects(!objSystem);
	objSystem = this;

}

ObjSystem::~ObjSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1009c8c0);
	shutdown();

	Expects(objSystem == this);
	objSystem = nullptr;
}

const std::string &ObjSystem::GetName() const {
	static std::string name("Obj");
	return name;
}

void ObjSystem::CompactIndex()
{
	mObjRegistry->RemoveDynamicObjectsFromIndex();
}

objHndl ObjSystem::GetHandleById(ObjectId id)
{

	// Is it already a handle?
	if (id.IsHandle()) {
		return id.GetHandle();
	}
	
	auto handle = mObjRegistry->GetHandleById(id);

	if (handle) {
		return handle;
	}

	// Check for positional IDs in the map
	if (!id.IsPositional())
		return objHndl::null;

	auto pos = id.body.pos;

	if (gameSystems->GetMap().GetCurrentMapId() != pos.mapId) {
		return objHndl::null;
	}

	ObjList list;
	locXY loc;
	loc.locx = pos.x;
	loc.locy = pos.y;
	list.ListTile(loc, OLC_IMMOBILE);
	
	for (auto i = 0; i < list.size(); ++i) {
		auto candidate = list.get(i);
		auto tempId = objects.GetTempId(candidate);
		if (tempId == pos.tempId) {
			mObjRegistry->AddToIndex(candidate, id);
			/*if (!mObjRegistry->GetHandleById(id)) {
				logger->debug("WTF; cannot find just added handle");
				return objHndl::null;
			}*/
			return candidate;
		}
	}

	return objHndl::null;

}

ObjectId ObjSystem::GetIdByHandle(objHndl handle)
{
	return mObjRegistry->GetIdByHandle(handle);
}

void ObjSystem::Remove(objHndl handle)
{
	auto obj = mObjRegistry->Get(handle);

	// Remove associated obj find nodes
	if (!obj->IsProto()) {
		mObjFind->Remove(handle, obj);
	}
	
	mObjRegistry->Remove(handle);
}

void ObjSystem::FreezeIds(objHndl handle)
{
	auto obj = GetObject(handle);
	obj->FreezeIds();
}

void ObjSystem::UnfreezeIds(objHndl handle)
{
	auto obj = GetObject(handle);
	obj->UnfreezeIds();
}

void ObjSystem::PruneNullInventoryIds(objHndl handle)
{
	auto obj = GetObject(handle);
	obj->PruneNullInventoryItems();
}

bool ObjSystem::IsValidHandle(objHndl handle)
{
	if (!handle) {
		return true;
	}
	
	return mObjRegistry->Contains(handle);
}

int ObjSystem::GetProtoId(objHndl obj)
{
	static auto obj_get_proto_id = temple::GetPointer<int(objHndl)>(0x10039320);
	return obj_get_proto_id(obj);
}

GameObjectBody * ObjSystem::GetObject(objHndl handle)
{
	return mObjRegistry->Get(handle);
}

ObjectId ObjSystem::GetPersistableId(objHndl handle)
{
	if (!handle) {
		return ObjectId::CreateNull();
	}

	if (!objSystem->IsValidHandle(handle)) {
		return ObjectId::CreateNull();
	}

	auto obj = GetObject(handle);
	
	// This may happen for sector objs, but when are those not static anyway?
	if (obj->id.IsNull()) {

		// Generate a positional ID for static objects
		if (obj->IsStatic()) {
			auto loc = obj->GetLocation();
			obj->id = ObjectId::CreatePositional(
				gameSystems->GetMap().GetCurrentMapId(),
				loc.locx,
				loc.locy,
				obj->GetTemporaryId()
			);
		} else {
			obj->id = ObjectId::CreatePermanent();
		}

		obj->hasDifs = true;

		// Make the new id known to the registry
		mObjRegistry->AddToIndex(handle, obj->id);
	}

	return obj->id;

}

bool ObjSystem::ValidateSector(bool requireHandleRefs)
{

	// Check all objects
	for (auto &entry : *mObjRegistry) {
		auto obj = entry.second.get();

		// Primary keys for objects must be persistable ids
		if (!obj->id.IsPersistable()) {
			logger->error("Found non persistable object id {}", obj->id.ToString());
			return false;
		}

		if (obj->IsProto()) {
			continue;
		}

		switch (obj->type)
		{
		case obj_t_container:			
			ValidateInventory(obj, obj_f_container_inventory_list_idx, obj_f_container_inventory_num, requireHandleRefs);
			break;
		case obj_t_pc:
		case obj_t_npc:
			ValidateInventory(obj, obj_f_critter_inventory_list_idx, obj_f_critter_inventory_num, requireHandleRefs);
			break;			
		case obj_t_portal:
		case obj_t_scenery:
		case obj_t_projectile:
		case obj_t_weapon:
		case obj_t_ammo:
		case obj_t_armor:
		case obj_t_money:
		case obj_t_food:
		case obj_t_scroll:
		case obj_t_key:
		case obj_t_written:
		case obj_t_generic:
		case obj_t_trap:
		case obj_t_bag:
			break;
		default:
			logger->error("{} has unknown object type {}.", obj->id.ToString(), obj->type);
			return false;
		}
	}

	return true;
}

objHndl ObjSystem::GetProtoHandle(uint16_t protoId)
{
	ObjectId objId = ObjectId::CreatePrototype(protoId);
	return GetHandleById(objId);
}

objHndl ObjSystem::CreateObject(objHndl protoHandle, locXY location)
{
	auto protoObj = GetObject(protoHandle);
	Expects(protoObj && protoObj->IsProto());
		
	auto newHandle = mObjRegistry->Add(std::make_unique<GameObjectBody>());
	auto obj = mObjRegistry->Get(newHandle);

	obj->protoId = protoObj->id;
	obj->type = protoObj->type;
	obj->field40 = 0; // TODO Is this even used?
	obj->hasDifs = 0;
	obj->propCollectionItems = 0;
	obj->propCollection = nullptr;
	
	// We allocate one array for both bitmaps
	auto bitmapLen = objectFields.GetBitmapBlockCount(obj->type);
	obj->propCollBitmap = new uint32_t[bitmapLen * 2];
	obj->difBitmap = &obj->propCollBitmap[bitmapLen];
	for (size_t i = 0; i < bitmapLen; ++i) {
		obj->propCollBitmap[i] = 0;
		obj->difBitmap[i] = 0;
	}
	
	memset(&obj->transientProps, 0, sizeof(obj->transientProps));

	obj->id = ObjectId::CreatePermanent();
	AddToIndex(obj->id, newHandle);
	
	obj->SetLocation(location);

	if (obj->IsNPC()) {
		obj->SetInt64(obj_f_critter_teleport_dest, location);

		StandPoint standpoint;
		standpoint.mapId = gameSystems->GetMap().GetCurrentMapId();
		standpoint.location.location = location;
		standpoint.location.off_x = 0;
		standpoint.location.off_y = 0;
		standpoint.jumpPointId = -1;

		critterSys.SetStandPoint(newHandle, StandPointType::Day, standpoint);
		critterSys.SetStandPoint(newHandle, StandPointType::Night, standpoint);

		auto flags = obj->GetNPCFlags();
		flags |= ONF_WAYPOINTS_DAY;
		obj->SetNPCFlags(flags);
	}

	FindNodeAllocate(newHandle);

	InitDynamic(obj, newHandle, location);

	return newHandle;
}

objHndl ObjSystem::LoadFromFile(TioFile* file) {
	uint32_t header;
	if (tio_fread(&header, sizeof(header), 1, file) != 1) {
		throw TempleException("Couldn't read the object header.");
	}

	if (header != 0x77) {
		throw TempleException("Expected object header 0x77, but got 0x{:x}", header);
	}

	ObjectId protoId;
	if (tio_fread(&protoId, sizeof(protoId), 1, file) != 1) {
		throw TempleException("Couldn't read the prototype id.");
	}

	if (!protoId.IsPrototype()) {
		throw TempleException("Expected a prototype id, but got type {} instead.", (int)protoId.subtype);
	}

	ObjectId objId;
	if (tio_fread(&objId, sizeof(objId), 1, file) != 1) {
		throw TempleException("Couldn't read the object id.");
	}
	
	// Null IDs are allowed for sector objects
	if (!objId.IsPermanent() && !objId.IsNull()) {
		throw TempleException("Expected an object id of type Permanent, but got type {} instead.", (int)objId.subtype);
	}

	uint32_t typeCode;
	if (tio_fread(&typeCode, sizeof(typeCode), 1, file) != 1) {
		throw TempleException("Unable to read object type");
	}


	auto obj = std::make_unique<GameObjectBody>();
	obj->protoId = protoId;
	obj->id = objId;
	obj->type = (ObjectType)typeCode;
	
	// Initialize and load bitmaps
	auto propLen = objectFields.GetBitmapBlockCount(obj->type);
	obj->propCollBitmap = new uint32_t[propLen * 2];
	obj->hasDifs = 0;
	obj->difBitmap = &obj->propCollBitmap[propLen];
	memset(&obj->difBitmap[0], 0, propLen * sizeof(uint32_t));
		
	uint16_t propCount;
	if (tio_fread(&propCount, sizeof(propCount), 1, file) != 1) {
		throw TempleException("Couldn't read property count.");
	}

	if (tio_fread(&obj->propCollBitmap[0], sizeof(uint32_t) * propLen, 1, file) != 1) {
		throw TempleException("Couldn't read the property bitmap");
	}

	obj->propCollectionItems = propCount;
	obj->propCollection = new void*[propCount];
	
	obj->ForEachField([&](obj_f field, void** storageLocation)  {
		*storageLocation = nullptr;
		ReadFieldValue(field, storageLocation, file);
		return true;
	});

	memset(&obj->transientProps, 0, sizeof(obj->transientProps));
	obj->SetInternalFlags(1); // Storing persistent IDs at the moment
	
	// Add it to the registry
	auto id = obj->id;
	auto handle = mObjRegistry->Add(std::move(obj));
	if (!id.IsNull()) {
		mObjRegistry->AddToIndex(handle, id);
		logger->trace("Loaded object {} to handle {}", id.ToString(), handle);
	}

	FindNodeAllocate(handle);

	return handle;

}

class ObjBuffer {
public:
	ObjBuffer(void *ptr) : ptr((uint8_t*)ptr) {}

	char* ReadString() {
		uint32_t len = Read<uint32_t>();
		auto res = (char*)malloc(len + 1);
		memcpy(res, ptr, len + 1);
		res[len] = 0;
		ptr += len + 1;
		return res;
	}

	template<typename T>
	T Read() {
		T result;
		memcpy(&result, ptr, sizeof(T));
		ptr += sizeof(T);
		return result;
	}

	ArrayHeader *ReadArray() {
		ArrayHeader header;
		memcpy(&header, ptr, sizeof(header));
		ptr += sizeof(header);

		auto arr = (ArrayHeader*)malloc(sizeof(ArrayHeader) + header.elSize * header.count);
		arr->elSize = header.elSize;
		arr->count = header.count;

		memcpy(arr->GetData(), ptr, header.elSize * header.count);
		ptr += header.elSize * header.count;

		arr->idxBitmapId = arrayIdxBitmaps.DeserializeFromMemory(&ptr);

		return arr;
	}

private:
	uint8_t* ptr;
};

objHndl ObjSystem::LoadFromBuffer(void* bufferPtr) {

	ObjBuffer buffer(bufferPtr);

	auto header = buffer.Read<uint32_t>();
	if (header != 0x77) {
		throw TempleException("Expected object header 0x77, but got 0x{:x}", header);
	}

	auto protoId = buffer.Read<ObjectId>();
	if (!protoId.IsPrototype()) {
		throw TempleException("Expected a prototype id, but got type {} instead.", (int)protoId.subtype);
	}

	auto objId = buffer.Read<ObjectId>();
	// Null IDs are allowed for sector objects
	if (!objId.IsPermanent() && !objId.IsNull()) {
		throw TempleException("Expected an object id of type Permanent, but got type {} instead.", (int)objId.subtype);
	}

	auto typeCode = buffer.Read<uint32_t>();
	auto obj = std::make_unique<GameObjectBody>();
	obj->protoId = protoId;
	obj->id = objId;
	obj->type = (ObjectType)typeCode;

	// Initialize and load bitmaps
	auto propLen = objectFields.GetBitmapBlockCount(obj->type);
	obj->propCollBitmap = new uint32_t[propLen * 2];
	obj->hasDifs = 0;
	obj->difBitmap = &obj->propCollBitmap[propLen];
	memset(&obj->difBitmap[0], 0, propLen * sizeof(uint32_t));

	uint16_t propCount = buffer.Read<uint16_t>();

	for (size_t i = 0; i < propLen; ++i) {
		obj->propCollBitmap[i] = buffer.Read<uint32_t>();
	}

	obj->propCollectionItems = propCount;
	obj->propCollection = new void*[propCount];

	obj->ForEachField([&](obj_f field, void** storageLocation) {
		*storageLocation = nullptr;
		ReadFieldValue(field, storageLocation, buffer);
		return true;
	});

	memset(&obj->transientProps, 0, sizeof(obj->transientProps));
	obj->SetInternalFlags(1); // Storing persistent IDs at the moment

							  // Add it to the registry
	auto id = obj->id;
	auto handle = mObjRegistry->Add(std::move(obj));
	if (!id.IsNull()) {
		mObjRegistry->AddToIndex(handle, id);
	}

	FindNodeAllocate(handle);

	return handle;

}

void ObjSystem::ForEachObj(std::function<void(objHndl, GameObjectBody&)> callback) const {

	for (auto &entry : *mObjRegistry) {
		if (entry.second->IsProto()) {
			continue; // Only instances
		}
		callback(entry.first, *entry.second);
	}

}

objHndl ObjSystem::FindObjectByIdStr(string id_str)
{
	for (auto& entry : *mObjRegistry) {
		if (entry.second->IsProto()) {
			continue; // Only instances
		}
		if (entry.second->id.ToString()._Equal(id_str)) {
			return entry.first;
		}
	}
	return objHndl::null;
}

objHndl ObjSystem::CreateProto(ObjectType type) {

	auto objPtr = std::make_unique<GameObjectBody>();
	auto obj = objPtr.get();
	obj->type = type;
	obj->id = ObjectId::CreatePermanent();

	auto handle = mObjRegistry->Add(std::move(objPtr));
	mObjRegistry->AddToIndex(handle, obj->id);

	obj->protoId.subtype = ObjectIdKind::Blocked;
	obj->protoHandle = 0;

	auto bitmapLen = objectFields.GetBitmapBlockCount(type);
	obj->difBitmap = new uint32_t[bitmapLen];
	for (size_t i = 0; i < bitmapLen; ++i) {
		obj->difBitmap[i] = 0;
	}

	memset(&obj->transientProps, 0xFFu, sizeof(obj->transientProps));

	auto count = objectFields.GetSupportedFieldCount(type);
	obj->propCollectionItems = (uint16_t) count;
	obj->propCollection = new void*[count];
	for (size_t i = 0; i < count; ++i) {
		obj->propCollection[i] = nullptr;
	}
	
	static auto obj_proto_set_defaults = temple::GetPointer<void(objHndl)>(0x100a1620);
	obj_proto_set_defaults(handle);

	return handle;
}

objHndl ObjSystem::Clone(objHndl handle, locXY location) {

	auto src = GetObject(handle);
	auto destPtr = src->Clone();
	auto dest = destPtr.get();
	auto result = mObjRegistry->Add(std::move(destPtr));
	mObjRegistry->AddToIndex(result, dest->id);

	// Clone the inventory as well	
	size_t childIdx = 0;
	src->ForEachChild([&](objHndl child) {
		auto clonedChildPtr = GetObject(child)->Clone();
		auto clonedChild = clonedChildPtr.get();
		auto childHandle = mObjRegistry->Add(std::move(clonedChildPtr));
		mObjRegistry->AddToIndex(childHandle, clonedChild->id);
		
		auto invField = inventory.GetInventoryListField(result);
		dest->SetObjHndl(invField, childIdx++, childHandle);
		clonedChild->SetObjHndl(obj_f_item_parent, result);
	});

	FindNodeAllocate(result);

	dest->SetDispatcher(nullptr);
	InitDynamic(dest, result, location);
	
	LocAndOffsets extendedLoc;
	extendedLoc.location = location;
	extendedLoc.off_x = 0;
	extendedLoc.off_y = 0;
	objects.Move(result, extendedLoc);

	if (dest->IsNPC()) {
		StandPoint standpoint;
		standpoint.location.location = location;
		standpoint.location.off_x = 0;
		standpoint.location.off_y = 0;
		standpoint.mapId = gameSystems->GetMap().GetCurrentMapId();
		standpoint.jumpPointId = -1;

		critterSys.SetStandPoint(result, StandPointType::Day, standpoint);
		critterSys.SetStandPoint(result, StandPointType::Night, standpoint);
	}

	return result;
}

bool ObjSystem::ValidateInventory(const GameObjectBody * container, obj_f idxField, obj_f countField, bool requireHandles)
{
	auto actualCount = container->GetObjectIdArray(idxField).GetSize();
		
	if (actualCount != container->GetInt32(countField)) {
		logger->error("Count stored in {} doesn't match actual item count of {}.",
			objectFields.GetFieldName(countField), objectFields.GetFieldName(idxField));
		return false;
	}
	auto containerHandle = GetHandleById(container->id);
	for (size_t i = 0; i < actualCount; ++i) {
		auto itemId = container->GetObjectId(idxField, i);
		
		auto positional = fmt::format("Entry {} in {}@{} of {} ({})",
			itemId.ToString(), objectFields.GetFieldName(idxField), i, container->id.ToString(), containerHandle);

		if (itemId.IsNull()) {
			logger->error("{} is null", positional);
			return false;
		} else if (!itemId.IsHandle()) {
			if (requireHandles) {
				logger->error("{} is not a handle, but handles are required.", positional);
				return false;
			}
			
			if (!itemId.IsPersistable()) {
				logger->error("{} is not a valid persistable id.", positional);
				return false;
			}
		}

		auto itemObj = GetObject(GetHandleById(itemId));

		if (!itemObj) {
			logger->error("{} does not resolve to a loaded object.", positional);
			return false;
		}

		if (itemObj == container) {
			logger->error("{} is contained inside of itself.", positional);
			return false;
		}

		// Only items are allowed in containers
		if (!itemObj->IsItem()) {
			logger->error("{} is not an item.");
			return false;
		}
	}

	return true;
}

void ObjSystem::AddToIndex(ObjectId id, objHndl handle)
{
	mObjRegistry->AddToIndex(handle, id);
}

void ObjSystem::InitDynamic(GameObjectBody * obj, objHndl handle, locXY location)
{
	// Mark the object and all its children as dynamic
	obj->SetFlag(OF_DYNAMIC, true);
	obj->ForEachChild([&](objHndl item) {
		auto itemObj = GetObject(item);
		itemObj->SetFlag(OF_DYNAMIC, true);
	});

	// Add the new object to the sector system if needed
	SectorLoc sectorLoc(location);
	if (gameSystems->GetMapSector().IsSectorLoaded(sectorLoc)) {
		LockedMapSector sector(sectorLoc);
		sector.AddObject(handle);
	}
	gameSystems->GetMapSector().RemoveSectorLight(handle);

	/// Init NPC state
	if (obj->IsNPC()) {
		gameSystems->GetAI().AddAiTimer(handle);
	}
 	if (obj->IsCritter()) {
		d20Sys.d20Status->D20StatusInit(handle);
	}

	// Apply random sizing of the 3d model if requested
	auto flags = obj->GetFlags();
	if (flags & OF_RANDOM_SIZE) {
		auto scale = obj->GetInt32(obj_f_model_scale);
		scale -= RandomIntRange(0, 20);
		obj->SetInt32(obj_f_model_scale, scale);
	}

	static auto possibly_spawn_inven_source = temple::GetPointer<void(objHndl)>(0x1006dcf0);
	possibly_spawn_inven_source(handle);

	static auto sub_10025050 = temple::GetPointer<int(objHndl, int)>(0x10025050);
	sub_10025050(handle, 2);

	LocAndOffsets fromLoc;
	fromLoc.location.locx = 0;
	fromLoc.location.locy = 0;
	fromLoc.off_x = 0;
	fromLoc.off_y = 0;

	LocAndOffsets toLoc = fromLoc;
	toLoc.location = location;

	static auto objevent_notify_moved = temple::GetPointer<void(objHndl, LocAndOffsets, LocAndOffsets)>(0x10045290);
	objevent_notify_moved(handle, fromLoc, toLoc);

}

void ObjSystem::FindNodeAllocate(objHndl handle)
{
	static auto obj_find_node_allocate = temple::GetPointer<void(objHndl)>(0x100c1130);
	obj_find_node_allocate(handle);
}

void ObjSystem::FindNodeMove(objHndl handle) {
	static auto obj_find_move = temple::GetPointer<void(objHndl)>(0x100c1280);
	obj_find_move(handle);
}

void ObjSystem::ReadFieldValue(obj_f field, void** storageLoc, TioFile *file) {

	auto type = objectFields.GetType(field);

	uint8_t dataPresent;
	int32_t int32Value;
	float floatValue;
	int64_t int64Value;
	ObjectId objIdValue;
	uint32_t strLen;

	auto pos = tio_ftell(file);

	switch (type) {
	case ObjectFieldType::Int32: 
		if (tio_fread(&int32Value, sizeof(int32Value), 1, file) == 1) {
			*reinterpret_cast<decltype(int32Value)*>(storageLoc) = int32Value;
			return;
		}
		break;
	case ObjectFieldType::Float32: 
		if (tio_fread(&floatValue, sizeof(floatValue), 1, file) == 1) {
			*reinterpret_cast<decltype(floatValue)*>(storageLoc) = floatValue;
			return;
		}
		break;
	case ObjectFieldType::Int64:
		if (tio_fread(&dataPresent, sizeof(dataPresent), 1, file) != 1) {
			break;
		}
		if (!dataPresent) {
			return;
		}
		if (tio_fread(&int64Value, sizeof(int64Value), 1, file) == 1) {
			*storageLoc = new decltype(int64Value)(int64Value);
			return;
		}
		break;
	case ObjectFieldType::Obj: 
		if (tio_fread(&dataPresent, sizeof(dataPresent), 1, file) != 1) {
			break;
		}
		if (!dataPresent) {
			return;
		}
		if (tio_fread(&objIdValue, sizeof(objIdValue), 1, file) == 1) {
			if (!objIdValue.IsPersistable()) {
				throw TempleException("Read an invalid object id {} for field {}", 
					objIdValue, objectFields.GetFieldName(field));
			}
			*storageLoc = new decltype(objIdValue)(objIdValue);
			return;
		}
		break;
	case ObjectFieldType::String:
		if (tio_fread(&dataPresent, sizeof(dataPresent), 1, file) != 1) {
			break;
		}
		if (!dataPresent) {
			return;
		}
		if (tio_fread(&strLen, sizeof(strLen), 1, file) != 1) {
			break;
		}
		*storageLoc = malloc(strLen + 1);
		if (tio_fread(*storageLoc, strLen + 1, 1, file) == 1) {
			reinterpret_cast<char*>(*storageLoc)[strLen] = '\0';
			return;
		}
		free(*storageLoc);
		break;
	case ObjectFieldType::AbilityArray:
	case ObjectFieldType::UnkArray:
	case ObjectFieldType::Int32Array:
	case ObjectFieldType::Int64Array:
	case ObjectFieldType::ScriptArray:
	case ObjectFieldType::Unk2Array:
	case ObjectFieldType::ObjArray:
	case ObjectFieldType::SpellArray: {
		if (tio_fread(&dataPresent, sizeof(dataPresent), 1, file) != 1) {
			break;
		}
		if (!dataPresent) {
			return;
		}

		ArrayHeader header;
		if (tio_fread(&header, sizeof(header), 1, file) != 1) {
			break;
		}

		auto arr = (ArrayHeader*) malloc(sizeof(ArrayHeader) + header.elSize * header.count);
		arr->elSize = header.elSize;
		arr->count = header.count;

		if (tio_fread(arr->GetData(), header.elSize, header.count, file) != header.count) {
			free(arr);
			break;
		}

		*storageLoc = arr;
		arr->idxBitmapId = arrayIdxBitmaps.DeserializeFromFile(file);
		return;
	}
	default: 
		throw TempleException("Cannot deserialize field type {}", (int)type);
	}

	throw TempleException("Couldn't read field {} @ {}.", objectFields.GetFieldName(field), pos);

}

void ObjSystem::ReadFieldValue(obj_f field, void** storageLoc, ObjBuffer& buffer) {

	auto type = objectFields.GetType(field);

	switch (type) {
	case ObjectFieldType::Int32:
		*reinterpret_cast<int32_t*>(storageLoc) = buffer.Read<int32_t>();
		break;
	case ObjectFieldType::Float32:
		*reinterpret_cast<float*>(storageLoc) = buffer.Read<float>();
		break;
	case ObjectFieldType::Int64:
		if (buffer.Read<int8_t>() == 1) {
			*storageLoc = new int64_t(buffer.Read<int64_t>());
		}
		break;
	case ObjectFieldType::Obj:
		if (buffer.Read<int8_t>() == 1) {
			auto objId = buffer.Read<ObjectId>();
			*storageLoc = new int64_t(buffer.Read<int64_t>());

			if (!objId.IsPersistable()) {
				throw TempleException("Read an invalid object id {} for field {}",
					objId, objectFields.GetFieldName(field));
			}
		}
		break;
	case ObjectFieldType::String:
		if (buffer.Read<int8_t>() == 1) {
			*storageLoc = buffer.ReadString();
		}
		break;
	case ObjectFieldType::AbilityArray:
	case ObjectFieldType::UnkArray:
	case ObjectFieldType::Int32Array:
	case ObjectFieldType::Int64Array:
	case ObjectFieldType::ScriptArray:
	case ObjectFieldType::Unk2Array:
	case ObjectFieldType::ObjArray:
	case ObjectFieldType::SpellArray: {
		if (buffer.Read<int8_t>() == 1) {
			*storageLoc = buffer.ReadArray();
		}
		break;
	}
	default:
		throw TempleException("Cannot deserialize field type {}", (int)type);
	}

}
