#include "stdafx.h"
#include "common.h"
#include "inventory.h"
#include "obj.h"
#include "critter.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamesystems.h"
#include "util/fixes.h"
#include "condition.h"
#include <python/python_integration_obj.h>
#include "float_line.h"
#include "party.h"
#include "rng.h"


InventorySystem inventory;

struct InventorySystemAddresses : temple::AddressTable
{
	int (__cdecl*ItemGetAdvanced)(objHndl item, objHndl parent, int slotIdx, int flags);
	int(__cdecl*GetParent)(objHndl, objHndl*);
	int(__cdecl*ItemInsertGetLocation)(objHndl item, objHndl receiver, int* idxOut, objHndl bag, char flags);
	void(__cdecl*InsertAtLocation)(objHndl item, objHndl receiver, int itemInsertLocation);
	ItemErrorCode(__cdecl*TransferWithFlags)(objHndl item, objHndl receiver, int invenIdx, char flags, objHndl bag);
	InventorySystemAddresses()
	{
		rebase(GetParent, 0x10063E80);
		rebase(ItemInsertGetLocation,	0x10069000);
		rebase(InsertAtLocation,		0x100694B0);
		rebase(ItemGetAdvanced,			0x1006A810);
		rebase(TransferWithFlags,0x1006B040);

		
	}
} addresses;

static class InventoryHooks : public TempleFix {
public: 

	static int (__cdecl*orgItemInsertGetLocation)(objHndl, objHndl, int*, objHndl, int);

	void apply() override 	{

		replaceFunction<int(__cdecl)(objHndl, objHndl, int)>(0x1006B6C0, [](objHndl item, objHndl receiver, int flags){
			return inventory.SetItemParent(item, receiver, flags);
		});

		orgItemInsertGetLocation = replaceFunction<int(__cdecl)(objHndl, objHndl, int*, objHndl, int)>(0x10069000, [](objHndl item, objHndl receiver, int* itemInsertLoc, objHndl bag, int flags){
			return inventory.ItemInsertGetLocation(item, receiver, itemInsertLoc, bag, flags);
		});

		replaceFunction<objHndl(__cdecl)(objHndl, uint32_t)>(0x100651B0, [](objHndl handle, uint32_t idx){
			return inventory.GetItemAtInvIdx(handle, idx);
		});

		// PoopItems
		replaceFunction<void (objHndl, int)>(0x1006DA00, [](objHndl obj, int unflagNoTransfer)	{
			auto objType = objects.GetType(obj);
			auto invenField = inventory.GetInventoryListField(obj);
			obj_f invenNumField = obj_f_critter_inventory_num;
			
			if (objType == obj_t_container){
				auto conFlags = objects.getInt32(obj, obj_f_container_flags);
				if (conFlags & OCOF_INVEN_SPAWN_ONCE){
					inventory.SpawnInvenSourceItems(obj);
				}
				invenNumField = obj_f_container_inventory_num;
			} 
			else{
				if (objType <= obj_t_generic || objType > obj_t_npc)
					return;
			}

			auto invenNum = objects.getInt32(obj, invenNumField);
			auto invenNumActualSize= objSystem->GetObject(obj)->GetObjectIdArray(invenField).GetSize();
			if (invenNum != invenNumActualSize)	{
				logger->debug("Inventory array size for {} ({}) does not equal associated num field on PoopItems. Arraysize: {}, numfield: {}", description.getDisplayName(obj), objSystem->GetObject(obj)->id.ToString(), invenNumActualSize, invenNum);
				
			}

			auto objLoc = objects.GetLocation(obj);
			auto moveObj = temple::GetPointer<void(objHndl, locXY)>(0x100252D0);
			for (int i = invenNumActualSize - 1; i >= 0 ;i-- ){
				auto item = objects.getArrayFieldObj(obj, invenField, i);
				if (!item)
					continue;
				auto spFlags = objects.getInt32(item, obj_f_spell_flags);
				auto itemFlags = objects.getInt32(item, obj_f_item_flags);
				inventory.ForceRemove(item, obj);
				if ( (spFlags & SpellFlags::SF_4000) || itemFlags & (OIF_NO_DROP | OIF_NO_DISPLAY) || (itemFlags & OIF_NO_LOOT)){
					
					moveObj(item, objLoc);
					objects.Destroy(item);
				} else if ( unflagNoTransfer)
				{
					objects.setInt32(item, obj_f_item_flags, itemFlags & ~OIF_NO_TRANSFER);
					moveObj(item, objLoc);
				} else
				{
					moveObj(item, objLoc);
				}
			}
		});


		// FindMatchingStackableItem
		replaceFunction<objHndl(objHndl, objHndl)>(0x10067DF0, [](objHndl receiver, objHndl item) {
			return inventory.FindMatchingStackableItem(receiver, item);
		});

		// DoNpcLooting
		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x1006C170, [](objHndl opener, objHndl container){

			return inventory.DoNpcLooting(opener, container)?TRUE:FALSE;
			
		});
	};

	
} hooks;
int(__cdecl*InventoryHooks::orgItemInsertGetLocation)(objHndl, objHndl, int*, objHndl, int);


bool InventorySystem::IsInvIdxWorn(int invIdx){
	return invIdx >= INVENTORY_WORN_IDX_START && invIdx <= INVENTORY_WORN_IDX_END;
}

objHndl InventorySystem::GetItemAtInvIdx(objHndl handle, uint32_t nIdx){
	auto invenField = obj_f_begin;  
	auto invenNumField = obj_f_begin;  inventory.GetInventoryNumField(handle);
	if (objects.IsCritter(handle)){
		if (d20Sys.d20Query(handle, DK_QUE_Polymorphed))
			return objHndl::null;
		invenField = inventory.GetInventoryListField(handle);
		invenNumField = inventory.GetInventoryNumField(handle);
	}
	else{
		invenField = inventory.GetInventoryListField(handle);
		invenNumField = inventory.GetInventoryNumField(handle);
	}
	auto obj = objSystem->GetObject(handle);
	auto numItems = obj->GetInt32(invenNumField);
			
	for (auto i=0; i < numItems; i++){
		auto item = obj->GetObjHndl(invenField, i);
		if (!item) {
			logger->debug("obj {} name {} contains null object", handle, obj->GetInt32(obj_f_name));
		}
		else if (!objects.IsEquipment(item)){
			auto itemName = objects.getInt32(item, obj_f_name);
			logger->debug("obj {} name {} contains object {} name {}", handle, obj->GetInt32(obj_f_name), item, itemName);
		}
		if (inventory.GetInventoryLocation(item) == nIdx)
			return item;
	}
	return objHndl::null;
}

objHndl InventorySystem::FindMatchingStackableItem(objHndl receiver, objHndl item){
	if (!item)
		return objHndl::null;
	if (!receiver)
		return objHndl::null;

	auto itemObj = gameSystems->GetObj().GetObject(item);
	if (!inventory.IsIdentified(item))
		return objHndl::null;

	// if not identified - does not stack
	auto itemProto = gameSystems->GetObj().GetProtoId(item);
	if (itemProto == 12000) // generic item proto
		return objHndl::null;

	// cycle thru inventory
	auto recObj = gameSystems->GetObj().GetObject(receiver);
	auto invenField = inventory.GetInventoryListField(receiver);

	auto invenNum = recObj->GetInt32(inventory.GetInventoryNumField(receiver));
	for (auto i = 0; i < invenNum; i++){
		auto invenItem = recObj->GetObjHndl(invenField, i);
		if (!invenItem) // bad item???
			continue;

		// ensure not same item handle
		if (item == invenItem)
			continue;

		// ensure same proto ID
		auto invenItemProto = gameSystems->GetObj().GetProtoId(invenItem);
		if (invenItemProto != itemProto)
			continue;

		// ensure is identified
		if (!inventory.IsIdentified(invenItem))
			continue;

		auto invenItemObj = gameSystems->GetObj().GetObject(invenItem);

		// if item worn - ensure is ammo
		auto invenItemLoc = invenItemObj->GetInt32(obj_f_item_inv_location);
		if (inventory.IsInvIdxWorn(invenItemLoc)){
			if (invenItemObj->type != obj_t_ammo)
				continue;
		}

		if (itemObj->type == obj_t_scroll || itemObj->type == obj_t_food){ 
			// ensure potions/scrolls of different levels / schools do not stack
			auto itemSpell = itemObj->GetSpell(obj_f_item_spell_idx, 0);
			auto invenItemSpell = invenItemObj->GetSpell(obj_f_item_spell_idx, 0);
			if (itemSpell.spellLevel != invenItemSpell.spellLevel
				|| (itemObj->type == obj_t_scroll 
					&& spellSys.IsArcaneSpellClass(itemSpell.classCode) 
					!= spellSys.IsArcaneSpellClass(invenItemSpell.classCode) ))
				continue;
		}

		return invenItem;
	}
	
	return objHndl::null;
}

void InventorySystem::WieldBest(objHndl handle, int invSlot, objHndl target){

	if (invSlot == INVENTORY_WORN_IDX_START + EquipSlot::WeaponSecondary){
		return;
	}
	auto existingItem = GetItemAtInvIdx(handle, invSlot);

	temple::GetRef<void(__cdecl)(objHndl, int, objHndl)>(0x1006CCC0)(handle, invSlot, target);
}

int InventorySystem::GetItemWieldCondArg(objHndl item, uint32_t condId, int argOffset)
{
	// loops through the item wielder conditions to find condId
	
	auto argCount = 0;
		// for every non-matching item, increment the argCount
		// this finds the correct start position for the arguments 
		// of the sought-after condition
		// then return the value at offset of subIdx

	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto condArray = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	if (condArray.GetSize() <= 0)
		return 0;
	for (auto i = 0u; i < condArray.GetSize();i++)
	{
		auto wielderCondId = condArray[i];
		auto wielderCond = conds.GetById(wielderCondId);
		
		if (wielderCond && condId == wielderCondId)	{
			return itemObj->GetInt32Array(obj_f_item_pad_wielder_argument_array)[argOffset + argCount];
		} 
		
		if (wielderCond)
		{
			argCount += wielderCond->numArgs;
		}
	}

	// no matches found, return 0
	return 0;

}

void InventorySystem::RemoveWielderCond(objHndl item, uint32_t condId){
	if (!condId || !item)
		return;
	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto wielderConds = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	auto wielderArgs = itemObj->GetInt32Array(obj_f_item_pad_wielder_argument_array);
	auto argIdx = 0;
	for (auto i = 0u; i < wielderConds.GetSize();i++)
	{
		auto wCondId = wielderConds[i];
		auto wCond = conds.GetById(wCondId);
		if (!wCond) {
			logger->error("InventorySystem::RemoveWielderCond: Invalid condition! Item {}" , description.getDisplayName(item));
			return;
		}
		if (wCondId == condId){
			for (auto j = i + 1; j < wielderConds.GetSize(); j++ )
				itemObj->SetInt32(obj_f_item_pad_wielder_condition_array, j - 1,
					itemObj->GetInt32(obj_f_item_pad_wielder_condition_array, j));
			itemObj->RemoveInt32(obj_f_item_pad_wielder_condition_array, wielderConds.GetSize()-1);
			
			// remove corresponding args
			for (auto j = argIdx + wCond->numArgs; j < wielderArgs.GetSize(); j++){
				itemObj->SetInt32(obj_f_item_pad_wielder_argument_array, j - wCond->numArgs,
					itemObj->GetInt32(obj_f_item_pad_wielder_argument_array, j));
			}
			auto orgLen = wielderArgs.GetSize();
			for (auto j = orgLen - wCond->numArgs; j < orgLen; j++ )
				itemObj->RemoveInt32(obj_f_item_pad_wielder_argument_array, j);
			return;
		}
		argIdx += wCond->numArgs;
	}

}

objHndl InventorySystem::ItemWornAt(objHndl handle, EquipSlot nItemSlot) const
{
	if (!handle) {
		return objHndl::null;
	}
	return _ItemWornAt(handle, nItemSlot);
}

objHndl InventorySystem::ItemWornAt(objHndl handle, int slot) const
{
	if (!handle) {
		return objHndl::null;
	}
	return _ItemWornAt(handle, static_cast<EquipSlot>(slot));
}

objHndl InventorySystem::FindItemByName(objHndl container, int nameId) {
	return _FindItemByName(container, nameId);
}

objHndl InventorySystem::FindItemByProtoHandle(objHndl container, objHndl protoHandle, bool skipWorn) {
	return _FindItemByProto(container, protoHandle, skipWorn);
}

objHndl InventorySystem::FindItemByProtoId(objHndl container, int protoId, bool skipWorn) {
	auto protoHandle = objects.GetProtoHandle(protoId);
	return _FindItemByProto(container, protoHandle, skipWorn);
}

int InventorySystem::SetItemParent(objHndl item, objHndl parent, int flags)  {
	return SetItemParent(item, parent, (ItemInsertFlags)flags);
}

int InventorySystem::IsNormalCrossbow(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		auto weapType = objects.GetWeaponType(weapon);
		if (weapType == wt_heavy_crossbow || weapType == wt_light_crossbow)
			return TRUE; // TODO: should this include repeating crossbow? I think the context is reloading action in some cases
		// || weapType == wt_hand_crossbow
	}
	return FALSE;
}

int InventorySystem::IsThrowingWeapon(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		WeaponAmmoType ammoType = (WeaponAmmoType)objects.getInt32(weapon, obj_f_weapon_ammo_type);
		if (ammoType > wat_dagger && ammoType <= wat_bottle) // thrown weapons   TODO: should this include daggers??
		{
			return 1;
		}
	}
	return 0;
}

bool InventorySystem::UsesWandAnim(const objHndl item){
	if (!item)
		return false;

	auto itemObj = gameSystems->GetObj().GetObject(item);
	if (!itemObj->IsItem())
		return false;

	auto itemFlags = (ItemFlag)itemObj->GetInt32(obj_f_item_flags);
	return (itemFlags & ItemFlag::OIF_USES_WAND_ANIM) != 0;
}

bool InventorySystem::IsTripWeapon(objHndl weapon){
	if (!weapon)
		return false;

	auto weaponObj = gameSystems->GetObj().GetObject(weapon);
	if (weaponObj->type != obj_t_weapon)
		return false;

	auto weapType = (WeaponTypes)weaponObj->GetInt32(obj_f_weapon_type);
	switch (weapType){
	case wt_dire_flail:
	case wt_heavy_flail:
	case wt_light_flail:
	case wt_gnome_hooked_hammer:
	case wt_guisarme:
	case wt_halberd:
	case wt_kama:
	case wt_scythe:
	case wt_sickle:
	case wt_whip:
	case wt_spike_chain:
		return true;
	default: 
		return false;
	}

}

ArmorType InventorySystem::GetArmorType(int armorFlags)
{
	if (armorFlags & ARMOR_TYPE_NONE)
		return ARMOR_TYPE_NONE;
	return (ArmorType) (armorFlags & (ARMOR_TYPE_LIGHT | ARMOR_TYPE_MEDIUM | ARMOR_TYPE_HEAVY) );
}

BOOL InventorySystem::GetQuantityField(const objHndl item, obj_f* qtyField){
	return temple::GetRef<BOOL(objHndl, obj_f*)>(0x100641B0)(item, qtyField);
}

int InventorySystem::GetQuantity(objHndl item)
{
	auto getQty = temple::GetRef<int(__cdecl)(objHndl)>(0x100642B0);
	return getQty(item);
}

int InventorySystem::ItemDrop(objHndl item)
{
	return _ItemDrop(item);
}

int InventorySystem::ItemDrop(objHndl critter, EquipSlot slot) {
	auto item = inventory.GetItemAtInvIdx(critter, InvIdxForSlot(slot));
	if (!item)
		return FALSE;

	auto invenLoc = GetInventoryLocation(item);
	if (!IsInvIdxWorn(invenLoc))
		return TRUE;

	ItemDrop(item);
	return TRUE;
}

int InventorySystem::SetItemParent(objHndl item, objHndl receiver, ItemInsertFlags flags){

	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto recObj = gameSystems->GetObj().GetObject(receiver);
	
	if (objects.IsContainer(receiver)){
		return ItemGetAdvanced(item, receiver, -1, ItemInsertFlags::IIF_Use_Max_Idx_200);
	}
	
	if (!objects.IsCritter(receiver)){
		return 0;
	}
	
	if ( gameSystems->GetObj().GetProtoId(item) == 6239){ // Darley's Neckalce special casing
		return ItemGetAdvanced(item, receiver, 201, flags); // EquipSlot::Necklace + 200
	}
	
	return ItemGetAdvanced(item, receiver, -1, flags);
	
}

objHndl InventorySystem::GetParent(objHndl item)
{
	objHndl parent = objHndl::null;
	if (!addresses.GetParent(item, &parent)) {
		return objHndl::null;
	}
	return parent;
}

bool InventorySystem::IsRangedWeapon(objHndl weapon){
	if (!weapon)
		return false;
	if (objects.GetType(weapon) != obj_t_weapon)
		return false;
	if (objects.getInt32(weapon, obj_f_weapon_flags) & OWF_RANGED_WEAPON)
		return true;
	return false;
}

bool InventorySystem::IsVisibleInventoryFull(objHndl handle){

	auto obj = objSystem->GetObject(handle);
	auto invenNumField = obj_f_critter_inventory_num;
	auto invenField = obj_f_critter_inventory_list_idx;
	
	if (obj->type == obj_t_container){
		invenNumField = obj_f_container_inventory_num;
		invenField = obj_f_container_inventory_list_idx;
	}

	auto numItems = obj->GetInt32(invenNumField);
	if (numItems <= 0 || numItems < CRITTER_INVENTORY_SLOT_COUNT) return false;
	
	for (int i = 0; i < CRITTER_INVENTORY_SLOT_COUNT; i++){
		if ( !inventory.GetItemAtInvIdx(handle, i) )
			return false;
	}
	return true;
}

int InventorySystem::GetInventory(objHndl objHandle, objHndl ** inventoryArray){

	*inventoryArray = nullptr;
	auto obj = objSystem->GetObject(objHandle);
	auto invenNumField = obj_f_critter_inventory_num;
	auto invenField = obj_f_critter_inventory_list_idx;

	if (obj->type == obj_t_container)
	{
		invenNumField = obj_f_container_inventory_num;
		invenField = obj_f_container_inventory_list_idx;
	} 
	auto numItems = obj->GetInt32(invenNumField);
	if (numItems <= 0) return 0;
	*inventoryArray = static_cast<objHndl*>(malloc(sizeof(objHndl)*numItems));
	for (int i = 0; i < numItems; i++)
	{
		(*inventoryArray)[i] = obj->GetObjHndl(invenField, i);
	}
	return numItems;
}

std::vector<objHndl> InventorySystem::GetInventory(objHndl handle){
	
	std::vector<objHndl> result;

	auto obj = objSystem->GetObject(handle);
	auto invenNumField = obj_f_critter_inventory_num;
	auto invenField = obj_f_critter_inventory_list_idx;

	if (obj->type == obj_t_container)
	{
		invenNumField = obj_f_container_inventory_num;
		invenField = obj_f_container_inventory_list_idx;
	}
	auto numItems = obj->GetInt32(invenNumField);
	if (numItems <= 0) 
		return result;

	for (int i = 0; i < numItems; i++){
		result.push_back(obj->GetObjHndl(invenField, i));
	}

	return result;
}

int InventorySystem::GetInventoryLocation(objHndl item){
	if (!item){
		logger->error("GetInventoryLocation: called on null item!");
		return 0;
	}

	if(!objects.IsEquipment(item)){
		logger->error("Item: item_parent: ERROR: Called on non-item!\n");
	}	
	return objects.getInt32(item, obj_f_item_inv_location);
}

ItemFlag InventorySystem::GetItemFlags(objHndl item)
{
	return static_cast<ItemFlag>(objects.getInt32(item, obj_f_item_flags));
}

int InventorySystem::IsItemNonTransferable(objHndl item, objHndl receiver)
{
	objHndl parent = GetParent(item);
	ItemFlag itemFlags = inventory.GetItemFlags(item);
	if ((itemFlags & OIF_NO_DROP)
		&& (!receiver || !parent || objSystem->GetProtoId(item) == 6239 || receiver != parent))
		return IEC_Item_Cannot_Be_Dropped;
	if ( (itemFlags & OIF_NO_TRANSFER )
		&& receiver != parent && parent && !critterSys.IsDeadNullDestroyed(parent))
		return IEC_NPC_Will_Not_Drop;
	if (!(itemFlags & OIF_NO_TRANSFER_SPECIAL) || receiver == parent)
	{
		return 0;
	}
	return IEC_Item_Cannot_Be_Dropped;
}

int InventorySystem::ItemInsertGetLocation(objHndl item, objHndl receiver, int* itemInsertLocation, objHndl bag, char flags){

	auto hasLocationOutput = itemInsertLocation != nullptr;
	auto parentObj = objSystem->GetObject(receiver);
	auto invIdx = INVENTORY_IDX_UNDEFINED;

	if (d20Sys.d20Query(receiver, DK_QUE_Polymorphed))
		return IEC_Cannot_Use_While_Polymorphed;


	auto isUseWieldSlots = (flags & IIF_Use_Wield_Slots) != 0;

	auto itemSthg_10067F90 = temple::GetRef<ItemErrorCode(__cdecl)(objHndl, int, objHndl)>(0x10067F90);
	auto itemCheckSlotAndWieldFlags = temple::GetRef<ItemErrorCode(__cdecl)(objHndl, objHndl, int)>(0x10067680);

	if (parentObj->IsCritter() && hasLocationOutput){
		
		if (flags & IIF_Allow_Swap 	|| (!IsInvIdxWorn(*itemInsertLocation) && !isUseWieldSlots)	)
		{
			if (isUseWieldSlots){
				for (auto equipSlot=0; equipSlot < INVENTORY_WORN_IDX_COUNT; equipSlot++){

					auto itemWornAtSlot = ItemWornAt(receiver, equipSlot);
					auto itemFlagCheck = itemCheckSlotAndWieldFlags(item, receiver, InvIdxForSlot(equipSlot)) == IEC_OK;
					auto slotIsOccupied = 
						*itemInsertLocation == INVENTORY_IDX_UNDEFINED 
					    && isUseWieldSlots
						&& itemWornAtSlot != objHndl::null
						&& itemFlagCheck;

					if (slotIsOccupied){
						if (ItemWornAt(receiver, equipSlot) != item){
							*itemInsertLocation = InvIdxForSlot(equipSlot);
							return IEC_Wield_Slot_Occupied;
						}
					}
					else if (itemWornAtSlot == objHndl::null
						     && itemFlagCheck
						     && itemWornAtSlot != item){
						*itemInsertLocation = InvIdxForSlot(equipSlot);
						return IEC_OK;
					}
				}
			}
		}
		else{

		//	auto res = InventoryHooks::orgItemInsertGetLocation(item, receiver, itemInsertLocation, bag, flags);//addresses.ItemInsertGetLocation(item, receiver, itemInsertLocation, bag, flags);
		//	return res;

			auto shouldTrySlots = true;
			auto result = IEC_Wrong_Type_For_Slot;
			if (*itemInsertLocation != INVENTORY_IDX_UNDEFINED) {
				result = itemSthg_10067F90(item, *itemInsertLocation, receiver);
				if (result == IEC_OK){
					shouldTrySlots = false;
					invIdx = *itemInsertLocation;
				}
				else{
					if (result != IEC_Wrong_Type_For_Slot && result != IEC_Wield_Slot_Occupied)
						return result;
				}
			}

			if (shouldTrySlots){
				auto found = false;
				for (auto i = 0; i < INVENTORY_WORN_IDX_COUNT; i++){
					auto equipSlot = (EquipSlot)i;
					if (!ItemWornAt(receiver, equipSlot)){
						result = itemSthg_10067F90(item, i + INVENTORY_WORN_IDX_START, receiver);
						if (result == IEC_OK){
							found = true;
							invIdx = i + INVENTORY_WORN_IDX_START;
							break;
						}
					}
				}
				if (!found)
					return result;
			}

		} 
		// TODO
		//return addresses.ItemInsertGetLocation(item, receiver, itemInsertLocation, bag, flags);
	}

	auto itemObj = objSystem->GetObject(item);
	

	// handling for stackable items, and money for PCs
	if (itemObj->IsStackable() && FindMatchingStackableItem(receiver, item) // this is vanilla; I suppose it doesn't matter, since it'll stack it anyway in the calling function (but I wonder if it isn't better to return the stackable item's index?)
		|| itemObj->type == obj_t_money && parentObj->IsPC()){
		if (hasLocationOutput)
			*itemInsertLocation = 0; 
		return IEC_OK;
	}

	// if already found it in the above section
	if (invIdx != INVENTORY_IDX_UNDEFINED){
		if (hasLocationOutput)
			*itemInsertLocation = invIdx;
		return IEC_OK;
	}
	

	auto tmp = INVENTORY_IDX_UNDEFINED;
	if (!hasLocationOutput) {
		itemInsertLocation = &tmp;
	}
	auto maxSlot = 120;

	// already provided with designated location
	if (*itemInsertLocation != INVENTORY_IDX_UNDEFINED && IsInvIdxWorn(*itemInsertLocation)){
		invIdx = *itemInsertLocation;
	}
	// Containers
	else if (!parentObj->IsCritter()){
		if (!parentObj->IsContainer())
			return IEC_No_Room_For_Item;

		if (*itemInsertLocation != INVENTORY_IDX_UNDEFINED && !GetItemAtInvIdx(receiver, *itemInsertLocation)){
			invIdx = *itemInsertLocation;
			if (hasLocationOutput)
				*itemInsertLocation = invIdx;
			return IEC_OK;
		}
		maxSlot = 120;
		if (flags & IIF_Use_Max_Idx_200) // fix - vanilla lacked this line
			maxSlot = INVENTORY_WORN_IDX_START;
		invIdx = FindEmptyInvIdx(item, receiver, 0, maxSlot);
	}
	// Critters
	else if (*itemInsertLocation != INVENTORY_IDX_UNDEFINED	&& !GetItemAtInvIdx(receiver, *itemInsertLocation)){
		invIdx = *itemInsertLocation;
	}
	else {

		if (flags & IIF_Use_Max_Idx_200){
			maxSlot = INVENTORY_WORN_IDX_START;
			invIdx = FindEmptyInvIdx(item, receiver, 0, maxSlot);
		}
		else if ( (flags & IIF_Use_Bags) && BagFindLast(receiver) != objHndl::null){
			auto receiverBag = BagFindLast(receiver);
			if (receiverBag){
				auto bagMaxIdx = BagGetContentMaxIdx(receiver, receiverBag);
				invIdx = FindEmptyInvIdx(item, receiver, 0, bagMaxIdx); // todo BUG!
			}
		}
		else if(flags & IIF_Allow_Swap){// bug?
			if (*itemInsertLocation != INVENTORY_IDX_UNDEFINED)
				return IEC_Wield_Slot_Occupied;
			return IEC_No_Room_For_Item;
		}
		else if (bag){
			auto bagMaxIdx = BagGetContentMaxIdx(receiver, bag);
			auto bagBaseIdx = BagGetContentStartIdx(receiver, bag);
			invIdx = FindEmptyInvIdx(item, receiver, bagBaseIdx, bagMaxIdx);
		}
		else{
			maxSlot = CRITTER_INVENTORY_SLOT_COUNT;
			invIdx = FindEmptyInvIdx(item, receiver, 0, maxSlot);
		}
	}
		
	if (invIdx != INVENTORY_IDX_UNDEFINED) {
		if (hasLocationOutput)
			*itemInsertLocation = invIdx;
		return IEC_OK;
	}
	return IEC_No_Room_For_Item;

	//return addresses.ItemInsertGetLocation(item, receiver, itemInsertLocation, bag, flags);
}

void InventorySystem::InsertAtLocation(objHndl item, objHndl receiver, int itemInsertLocation)
{
	addresses.InsertAtLocation(item, receiver, itemInsertLocation);
}

int InventorySystem::ItemUnwield(objHndl item)
{
	auto invenLoc = GetInventoryLocation(item);
	if (!IsInvIdxWorn(invenLoc ))
		return TRUE;

	objHndl parent = GetParent(item);
	int itemInsertLocation = 0;
	if (IsItemNonTransferable(item, parent)
		|| inventory.ItemInsertGetLocation(item, parent, &itemInsertLocation, objHndl::null, 0))
		return FALSE;
	ItemRemove(item);
	inventory.InsertAtLocation(item, parent, itemInsertLocation);
	return TRUE;
}

int InventorySystem::ItemUnwieldByIdx(objHndl obj, int i)
{
	auto item = inventory.GetItemAtInvIdx(obj, i);
	if (!item)
		return 1;
	return ItemUnwield(item);
}

int InventorySystem::ItemUnwield(objHndl critter, EquipSlot slot){
	return ItemUnwieldByIdx( critter, InvIdxForSlot(slot));
}

int InventorySystem::Wield(objHndl critter, objHndl item, EquipSlot slot){

	if (!item)
		return FALSE;

	auto canWield = false;

	auto existingItem = inventory.ItemWornAt(critter, slot);
	if (existingItem){
		if (inventory.ItemUnwield(existingItem))
			canWield = true;
	}
	else
		canWield = true;

	if (canWield)
		ItemPlaceInIdx(item, inventory.InvIdxForSlot(slot));

	return TRUE;
}

ItemErrorCode InventorySystem::TransferWithFlags(objHndl item, objHndl receiver, int invenIdx, char flags, objHndl bag) // see ItemInsertFlags
{
	return addresses.TransferWithFlags(item, receiver, invenIdx, flags, bag);
}

void InventorySystem::ItemPlaceInIdx(objHndl item, int idx)
{
	auto parent = GetParent(item);
	TransferWithFlags(item, parent, idx, 4, objHndl::null);
}

int InventorySystem::GetAppraisedWorth(objHndl item, objHndl appraiser, objHndl vendor, SkillEnum skillEnum)
{
	auto getAppraisedWorth = temple::GetRef<int(__cdecl)(objHndl, objHndl, objHndl, SkillEnum)>(0x10069970);
	return getAppraisedWorth(item, appraiser, vendor, skillEnum);
}

void InventorySystem::MoneyToCoins(int money, int* plat, int* gold, int* silver, int* copper)
{
	int remMoney = money;
	if (plat)
	{
		*plat = remMoney / 1000;
		remMoney = remMoney % 1000;
	}
	if (gold)
	{
		*gold = remMoney / 100;
		remMoney = remMoney % 100;
	}
	if (silver)
	{
		*silver = remMoney / 10;
		remMoney = remMoney % 10;
	}
	if (copper)
	{
		*copper = remMoney;
	}
}

int32_t InventorySystem::GetCoinWorth(int32_t coinType){

	static int32_t coinWorths[] = { 1,10,100,1000 };
	if (coinType >= 0 && coinType <= 3){
		return coinWorths[coinType];
	}
	
	return 0u;
}

int InventorySystem::GetWieldType(objHndl wielder, objHndl item, bool regardEnlargement) const
{
	if (!regardEnlargement)
		return _GetWieldType(wielder, item);

	if (!item)
		return 4;

	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto itemType = itemObj->type;

	if (itemType == obj_t_armor){
		auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
		auto armorType = inventory.GetArmorType(armorFlags);
		if (armorType == ArmorType::ARMOR_TYPE_SHIELD || armorType == ArmorType::ARMOR_TYPE_NONE)
			return  ( itemObj->GetInt32(obj_f_item_wear_flags) & OIF_WEAR::OIF_WEAR_BUCKLER ) != OIF_WEAR::OIF_WEAR_BUCKLER;
	}

	
	auto wielderSize = dispatch.DispatchGetSizeCategory(wielder);
	auto wielderSizeBase = regardEnlargement ? gameSystems->GetObj().GetObject(wielder)->GetInt32(obj_f_size) : wielderSize;
	auto itemSize = itemObj->GetInt32(obj_f_size);

	auto wieldType = 3;

	// if regardEnlargement is true, wielderSizeBase is the same as wielderSize i.e. it is implicit that the itemSize is enlarged along with the wielder
	if (itemSize < wielderSizeBase){
		wieldType = 0;
	} 
	else if (itemSize == wielderSizeBase)
	{
		wieldType = 1;
	} 
	else if (itemSize == wielderSizeBase + 1)
	{
		wieldType = 2;
	} 
	else if (itemSize == wielderSize + 1 )
	{
		wieldType = 2;
	}

	// check if it requires 2 handed by the item wear flags
	if (wieldType != 3)
	{
		if (itemObj->GetInt32(obj_f_item_wear_flags) & OIF_WEAR::OIF_WEAR_2HAND_REQUIRED)
			wieldType = 2;
	}

	if (itemType != obj_t_weapon)
		return wieldType;

	auto weaponType = static_cast<WeaponTypes>(itemObj->GetInt32(obj_f_weapon_type));


	// special casing for Bastard Swords and Dwarven Waraxes
	if (weaponType == wt_bastard_sword)
	{
		if (!feats.HasFeatCountByClass(wielder, FEAT_EXOTIC_WEAPON_PROFICIENCY_BASTARD_SWORD)) // if has no EWF Bastard sword - must dual wield
			wieldType = (wielderSizeBase <= 4) + 2;
		return wieldType;
	} 
	
	if (weaponType != wt_dwarven_waraxe)
	{
		return wieldType;
	}

	if (feats.HasFeatCountByClass(wielder, FEAT_EXOTIC_WEAPON_PROFICIENCY_DWARVEN_WARAXE) || objects.StatLevelGet(wielder, stat_race) == race_dwarf)
	{
		return wieldType;
	} 
	
	if (wielderSize == 5)
	{
		return 2;
	} 

	return 2 * (wielderSize <= 5) + 1;	
}

obj_f InventorySystem::GetInventoryListField(objHndl objHnd)
{
	if (objects.IsContainer(objHnd)) return obj_f_container_inventory_list_idx;

	//if (objects.IsCritter(objHnd)) 	
	return obj_f_critter_inventory_list_idx;
	
	//return (obj_f)0;
}

obj_f InventorySystem::GetInventoryNumField(objHndl objHnd)
{
	if (objects.IsContainer(objHnd))
		return obj_f_container_inventory_num;
	return obj_f_critter_inventory_num;
}

void InventorySystem::WieldBestAll(objHndl critter, objHndl tgt){
	
	for (auto invIdx = INVENTORY_WORN_IDX_START; invIdx <  INVENTORY_WORN_IDX_END; invIdx++){
		WieldBest(critter, invIdx, tgt);
	}
}

void InventorySystem::ForceRemove(objHndl item, objHndl parent)
{
	_ForceRemove(item, parent);
}

bool InventorySystem::IsProficientWithArmor(objHndl obj, objHndl armor) const
{
	auto isProfWithArmor = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1007C410);
	return isProfWithArmor(obj, armor) != 0;
}

void InventorySystem::GetItemMesLine(MesLine* line)
{
	auto itemMes = temple::GetRef<MesHandle>(0x10AA8464);
	mesFuncs.GetLine_Safe(itemMes, line);
}

const char* InventorySystem::GetItemErrorString(ItemErrorCode itemErrorCode)
{
	if (itemErrorCode <= 0)
		return nullptr;
	MesLine line;
	line.key = 100 + itemErrorCode;
	GetItemMesLine(&line);
	return line.value;
}

bool InventorySystem::IsMagicItem(objHndl itemHandle)
{
	return (gameSystems->GetObj().GetObject(itemHandle)->GetItemFlags() & OIF_IS_MAGICAL) != 0;
}

bool InventorySystem::IsIdentified(objHndl itemHandle)
{
	if (!itemHandle)
		return false;
	auto obj = gameSystems->GetObj().GetObject(itemHandle);

	if (!obj->IsItem())
		return false;

	if (obj->GetItemFlags() & ItemFlag::OIF_IS_MAGICAL)
		return (obj->GetItemFlags() & ItemFlag::OIF_IDENTIFIED);

	return true;
}

void InventorySystem::QuantitySet(const objHndl& item, int qtyNew){
	obj_f qtyField;
	if (GetQuantityField(item, &qtyField)){
		gameSystems->GetObj().GetObject(item)->SetInt32(qtyField, qtyNew);
	}
}

objHndl InventorySystem::SplitObjFromStack(objHndl item, locXY & loc){
	return temple::GetRef<objHndl(__cdecl)(objHndl, locXY&)>(0x10066B00)(item, loc);
}

int InventorySystem::ItemWeight(objHndl item){
	auto obj = objSystem->GetObject(item);
	if (obj->type == obj_t_money)
		return 0;

	auto baseWeight = obj->GetInt32(obj_f_item_weight);
	obj_f qtyField;
	if (inventory.GetQuantityField(item, &qtyField)){
		return baseWeight * obj->GetInt32(qtyField) / 4;
	}
	return baseWeight;
}

void InventorySystem::ItemSpellChargeConsume(const objHndl& item, int chargesUsedUp){
	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto spellCharges = itemObj->GetInt32(obj_f_item_spell_charges_idx);
	if (spellCharges == -1)
		return;

	// stacked items (scrolls, potions)
	
	auto itemQty = inventory.GetQuantity(item);
	if (itemQty  > 1){
		inventory.QuantitySet(item, itemQty - 1);
		return;
	}
	else if (itemQty == 1) {
		objects.Destroy(item);
		return;
	}

	// items with charges
	itemObj->SetInt32(obj_f_item_spell_charges_idx, spellCharges - chargesUsedUp);

	auto itemFlags = inventory.GetItemFlags(item);
	if (!(itemFlags & OIF_EXPIRES_AFTER_USE))
		return;

	if (spellCharges - chargesUsedUp <= 0)
		objects.Destroy(item);

	return;
}

bool InventorySystem::IsBuckler(objHndl shield)
{
	if (!shield)
		return false;

	return (gameSystems->GetObj().GetObject(shield)->GetInt32(obj_f_item_wear_flags) & OIF_WEAR::OIF_WEAR_BUCKLER) ? true: false;
}

void InventorySystem::ItemRemove(objHndl item)
{
	auto parent = GetParent(item);
	if (!parent)
	{
		logger->warn("Warning: item_remove called on item that doesn't think it has a parent_obj.");
		return;
	}
	ForceRemove(item, parent);
	auto objType = objects.GetType(parent);
	if (objType == obj_t_pc || objType == obj_t_npc)
	{
		d20Sys.d20SendSignal(parent, DK_SIG_Inventory_Update, item);
	}
	// return _ItemRemove(item);
}

BOOL InventorySystem::ItemGetAdvanced(objHndl item, objHndl parent, int invIdx, int flags){

	auto itemInsertLocation = -1;

	if (!item)
		return FALSE;

	auto itemObj = objSystem->GetObject(item);
	if (!itemObj->IsItem())	{
		logger->error("ItemGetAdvanced call on non-item!");
		return FALSE;
	}
	
	if (!parent)
		return FALSE;
	auto parentObj = objSystem->GetObject(parent);

	// if is already inventory item
	if (itemObj->GetFlags() & ObjectFlag::OF_INVENTORY){
		return temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, int)>(0x1006A3A0)(item, parent, invIdx);
	}

	// run the object's san_get script
	if (!pythonObjIntegration.ExecuteObjectScript(parent, item, ObjScriptEvent::Get))
		return FALSE;

	auto itemAtSlot = objHndl::null;
	auto invIdxIsWornSlot = false;
	if (IsInvIdxWorn(invIdx)){
		invIdxIsWornSlot = true;
		itemAtSlot = GetItemAtInvIdx(parent, invIdx);
	}

	auto insertionErrorCode = (ItemErrorCode)ItemInsertGetLocation(item, parent, &itemInsertLocation, objHndl::null, flags);
	if (insertionErrorCode)
	{
		auto dummy = 1;
	}

	// handle insertion error
	if (insertionErrorCode && (insertionErrorCode != IEC_No_Room_For_Item || itemAtSlot || !invIdxIsWornSlot)){
		

		if (parentObj->type == obj_t_pc)
			return FALSE;

		if (parentObj->type == obj_t_npc){
			floatSys.floatMesLine(parent, 1, FloatLineColor::White, GetItemErrorString(insertionErrorCode));
			return insertionErrorCode;
		}

		return insertionErrorCode;
	}

	// on success

	temple::GetRef<int(__cdecl)(objHndl, objHndl)>(0x1001F770)(item, parent); // MakeItemParented  (removes it from the sector object list and sets its obj_f_item_parent to parent, and sets the OF_INVENTORY flag for the item)
	if (invIdx == -1)
		invIdx = itemInsertLocation;
	else if (invIdxIsWornSlot){
		if (itemAtSlot){
			invIdx = itemInsertLocation;
		}
	}
	else
	{
		int indices[960] = {0,}; // index corresponds to invIdx, content corresponds to whether it's occupied and where in the critter's inventory field (is position + 1); 0 if unoccupied
		temple::GetRef<int(__cdecl)(objHndl, int*)>(0x100674A0)(parent, indices); // maps indices to obj inventory field indices
		if (! temple::GetRef<int(__cdecl)(objHndl, objHndl, int, int*)>(0x10068E80)(item, parent, invIdx, indices))
		{
			invIdx = itemInsertLocation;
		}
	}

	InsertAtLocation(item, parent, invIdx);
	if (parentObj->IsPC()){
		auto testInvIdx = itemObj->GetInt32(obj_f_item_inv_location);
		auto d = 1;
	}

	temple::GetRef<int(__cdecl)(objHndl)>(0x10067640)(item); // ItemDecayExpireEvents

	return TRUE;

	//return addresses.ItemGetAdvanced(item, parent, invIdx, flags);
}

bool InventorySystem::ItemCanBePickpocketed(objHndl item){
	if (GetParent(item) && GetInventoryLocation(item) >= INVENTORY_WORN_IDX_START || ItemWeight(item) > 1)
		return false;

	if (objSystem->GetObject(item)->GetItemFlags() & OIF_NO_PICKPOCKET)
		return false;

	return true;
}

bool InventorySystem::NpcCanLoot(objHndl handle){
	return (!critterSys.IsDeadNullDestroyed(handle)
		&& !critterSys.IsDeadOrUnconscious(handle)
		&& !((objects.getInt32(handle, obj_f_npc_pad_i_3) & 0xF) == NLT_Nothing)
		&& !d20Sys.d20QueryWithData(handle, DK_QUE_Critter_Has_Condition, conds.GetByName("Animal Companion Animal"), 0)
		&& !critterSys.IsUndead(handle)
		&& !d20Sys.d20QueryWithData(handle, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Summoned"), 0)
		);
}

bool InventorySystem::NpcWillLoot(objHndl item, NpcLootingType lootType){

	if (lootType == NLT_Nothing)
		return false;

	auto itemObj = objSystem->GetObject(item);
	if (itemObj->GetItemFlags() & (OIF_NO_LOOT | OIF_NO_NPC_PICKUP | OIF_NO_DISPLAY))
		return false;
	if (itemObj->type == obj_t_key)
		return false;
	if (itemObj->type == obj_t_money)
		return lootType != NLT_ArcaneScrollsOnly;
	// not money or keys
	if (lootType == NLT_HalfShareMoneyOnly)
		return false;
	if (lootType == NLT_ArcaneScrollsOnly){
		if (itemObj->type != obj_t_scroll)
			return false;
		auto spInfo = itemObj->GetSpell(obj_f_item_spell_idx, 0);
		return spellSys.IsArcaneSpellClass(spInfo.classCode);
	}

	return true;
}

bool InventorySystem::ItemTransferTo(objHndl item, objHndl receiver, int invIdx){
	auto transferTo = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, int)>(0x1006A3A0);
	return transferTo(item, receiver, invIdx) != FALSE;
}

bool InventorySystem::DoNpcLooting(objHndl opener, objHndl container){

	auto partySize = party.GroupNPCFollowersLen() + party.GroupPCsLen(); // in vanilla this was GroupListLen, which is wrong because it includes AI followers such as animal companions
	auto npcSize = party.GroupNPCFollowersLen();

	// if no NPCs, return
	if (!npcSize)
		return false;

	struct LooterInfo {
		objHndl handle;
		int lootingType; // raw data
		NpcLootingType shareType;
		/*
		determined by shareType and party size;
		0 if no shares
		*/
		int shareDivider;
		int itemWorthTotal; // in cp
		int lastItemWorth;
		int moneyGiven[4] = {0,}; // cp, sp, gp, pp
		LooterInfo(objHndl npc) :handle(npc) {
			lootingType = objects.getInt32(npc, obj_f_npc_pad_i_3);
			shareType = (NpcLootingType)(lootingType & 0xF);
			itemWorthTotal
 = 100 * ((lootingType >> 8) & 0xFFFF00);
			lastItemWorth
 = 1600 * (lootingType & 0xFFF0);
			auto partySize = party.GroupNPCFollowersLen() + party.GroupPCsLen(); // in vanilla this was GroupListLen, which is wrong because it includes AI followers such as animal companions
			switch (shareType) {
			case NpcLootingType::NLT_Normal:
				shareDivider = partySize;
				break;
			case NpcLootingType::NLT_HalfShareMoneyOnly:
				shareDivider = 2 * partySize;
				break;
			case NLT_ArcaneScrollsOnly:
			case NLT_Nothing:
				shareDivider = 0;
				break;
			case NLT_ThirdOfAll:
				shareDivider = (partySize <= 3) ? partySize : 3;
				break;
			case NLT_FifthOfAll:
				shareDivider = (partySize <= 5) ? partySize : 5;
				break;
			default:
				break;
			}
		};
	};

	std::vector<LooterInfo> looterInfo;

	// fill a list of NPCs that can loot
	for (auto i = 0; i < npcSize; i++) {
		auto npc = party.GroupNPCFollowersGetMemberN(i);
		auto npcCanLoot = inventory.NpcCanLoot(npc);
		if (!npcCanLoot)
			continue;
		looterInfo.push_back(LooterInfo(npc));
	}

	// if none can loot, return
	if (!looterInfo.size())
		return false;

	// get the container's inventory
	auto items = inventory.GetInventory(container);
	if (!items.size())
		return false;

	// count coin totals first
	int moneyArray[4] = { 0, }; // cp, sp, gp, pp
	for (auto item : items){
		if (objects.GetType(item) != obj_t_money)
			continue;
		auto moneyType = objects.getInt32(item, obj_f_money_type);
		if (moneyType >= 4 || moneyType < 0)
			continue;
		moneyArray[moneyType] = objects.getInt32(item, obj_f_money_quantity);
	}

	// calcualte each NPC's share of coins
	for (auto i=0; i < 4; i++){
		for (auto &li:looterInfo){
			if (li.shareDivider){
				li.moneyGiven[i] = moneyArray[i] / li.shareDivider;
			}
		}
	}

	// randomize the item order
	for (auto i=0; i < items.size(); i++){
		auto randIdx = rngSys.GetInt(0, items.size()-1);
		auto randItem = items[randIdx];
		auto orgItem = items[i];
		items[i] = randItem;
		items[randIdx] = orgItem;
	}

	auto result = false; // has any item been looted?


	for (auto item: items){
		auto itemIsTaken = false;
		auto itemObj = objSystem->GetObject(item);
		auto itemWorth = itemObj->GetInt32(obj_f_item_worth);
		if (!itemWorth) // don't let NPCs take worthless items
			continue;

		if (itemObj->type == obj_t_money)
			itemWorth = objects.GetMoneyAmount(item);
		
		for (auto &li: looterInfo){
			li.itemWorthTotal += itemWorth;
		}

		for (auto &li: looterInfo){

			if (itemIsTaken || !inventory.NpcWillLoot(item, li.shareType))
				continue;

			if (itemObj->type ==obj_t_money){
				auto moneyType = itemObj->GetInt32(obj_f_money_type);
				auto moneyQty = itemObj->GetInt32(obj_f_money_quantity);
				auto moneyGiven = moneyQty;
				if (li.moneyGiven[moneyType] <= moneyGiven)
					moneyGiven = li.moneyGiven[moneyType];
				li.moneyGiven[moneyType] -= moneyGiven;
				if (moneyGiven > 0){
					auto transferMoney = temple::GetRef<void(__cdecl)(int, objHndl, objHndl, int, objHndl)>(0x1006B970);
					transferMoney(moneyType, container, li.handle, moneyGiven, item);
					auto itemDescr = description.getDisplayName(item, opener);
					floatSys.floatMesLine(li.handle, 1, FloatLineColor::LightBlue, itemDescr);
					logger->info("{} looted {}", description.getDisplayName(li.handle, opener), itemDescr);
					if (moneyGiven == moneyQty){
						itemIsTaken = true;
					}
					result = true;
				}
			}
			else{
				auto loc = objSystem->GetObject(container)->GetLocation();
				auto splitItem = inventory.SplitObjFromStack(item, loc);
				auto itemWasSplit = splitItem != item; // checks if the item was actually split

				auto itemTransferSuccess = false;
				if (inventory.ItemTransferTo(splitItem, li.handle )){
					// guard against encumbering NPC as a result of the item transfer
					if (!d20Sys.d20Query(li.handle, DK_QUE_Critter_Is_Encumbered_Heavy)
						&& !d20Sys.d20Query(li.handle, DK_QUE_Critter_Is_Encumbered_Overburdened)){

						if (objects.GetFlags(splitItem) & OF_DESTROYED){ // I think this happens after the ItemTransferTo?
							splitItem = inventory.FindMatchingStackableItem(li.handle, splitItem);
						}

						auto itemDescr = description.getDisplayName(splitItem, opener);
						floatSys.floatMesLine(li.handle, 1, FloatLineColor::LightBlue, itemDescr);
						logger->info("{} looted {}", description.getDisplayName(li.handle, opener), itemDescr);
						itemIsTaken = true;
						result = true;
						li.lastItemWorth = itemWorth;
						itemTransferSuccess = true;
					}
					else{ // try to restore the item I think?
						if (objects.GetFlags(splitItem) & OF_DESTROYED) {

							splitItem = inventory.FindMatchingStackableItem(li.handle, splitItem);
							splitItem = inventory.SplitObjFromStack(splitItem, loc);
							if (!itemWasSplit)
								item = splitItem;
						}
					}
				}

				if (!itemTransferSuccess){
					inventory.ItemTransferTo(splitItem, container);
				}

			}

			if (li.itemWorthTotal >= li.lastItemWorth * li.shareDivider){
				li.lastItemWorth = 0;
				li.itemWorthTotal = 0;
			}
		}

	}


	// update the looting status field
	for (auto &it : looterInfo){

		auto lastItemWorth = it.lastItemWorth / 100 >> 8;
		auto totalItemWorth = it.itemWorthTotal / 100 >> 8;

		int newVal = 0x10 * ( (totalItemWorth)<<12 | lastItemWorth & 0xFFF ) | it.shareType;
		objects.setInt32(it.handle, obj_f_npc_pad_i_3, newVal);
	}

	return result;
}

const std::string & InventorySystem::GetAttachBone(objHndl handle)
{
	auto wearFlags = objects.GetItemWearFlags(handle);
	auto slot = objects.GetItemInventoryLocation(handle);
	static std::string sNoBone;
	static std::string sBoneLeftForearm = "Bip01 L Forearm";
	static std::array<std::string, 16> sBoneNames = {
		"HEAD_REF", // helm
		"CHEST_REF", // necklace
		"",			 // gloves
		"HANDR_REF", //main hand
		"HANDL_REF", // offhand
		"CHEST_REF", // armor
		"HANDR_REF", // left ring
		"HANDL_REF", // right ring
		"", // arrows
		"", // cloak
		"", // shield
		"", // robe
		"", // bracers
		"", // bardic instrument
		"" // misc (thieves' tools, belt etc)
	};

	if (slot < INVENTORY_WORN_IDX_START) {
		return sNoBone; // Apparently not equipped
	}

	auto type = objects.GetType(handle);
	if (wearFlags & OIF_WEAR_2HAND_REQUIRED && type == obj_t_weapon) {
		auto weaponType = objects.GetWeaponType(handle);
		if (weaponType == wt_shortbow || weaponType == wt_longbow || weaponType == wt_spike_chain)
			return sBoneNames[4]; // HANDL_REF
	}
	else if (type == obj_t_armor) {
		return sBoneLeftForearm;
	}

	return sBoneNames[slot - INVENTORY_WORN_IDX_START];
}

int InventorySystem::GetSoundIdForItemEvent(objHndl item, objHndl wielder, objHndl tgt, int eventType)
{
	return temple::GetRef<int(__cdecl)(objHndl, objHndl, objHndl, int)>(0x1006E0B0)(item, wielder, tgt, eventType);
}

int InventorySystem::InvIdxForSlot(EquipSlot slot){
	return slot + INVENTORY_WORN_IDX_START;
}

int InventorySystem::InvIdxForSlot(int slot){
	return InvIdxForSlot(static_cast<EquipSlot>(slot));
}

int InventorySystem::FindEmptyInvIdx(objHndl item, objHndl parent, int idxMin, int idxMax){

	for (auto i=idxMin; i < idxMax; i++){
		if (GetItemAtInvIdx(parent, i) == objHndl::null)
			return i;
	}

	return INVENTORY_IDX_UNDEFINED;
}

objHndl InventorySystem::BagFindLast(objHndl parent){

	for (auto i = INVENTORY_BAG_IDX_END; i >= INVENTORY_BAG_IDX_START; i--){
		auto res = GetItemAtInvIdx(parent, i);
		if (res)
			return res;
	}

	return objHndl::null;
}

int InventorySystem::BagFindInvenIdx(objHndl parent, objHndl receiverBag){
	for (auto i=INVENTORY_BAG_IDX_START; i <= INVENTORY_BAG_IDX_END; i++){
		if (GetItemAtInvIdx(parent, i) == receiverBag)
			return i;
	}
	return INVENTORY_IDX_UNDEFINED;
}

int InventorySystem::BagGetContentStartIdx(objHndl parent, objHndl receiverBag){

	auto bagIdx = BagFindInvenIdx(parent, receiverBag);
	return 24 * (bagIdx - INVENTORY_BAG_IDX_START);
}

int InventorySystem::BagGetContentMaxIdx(objHndl parent, objHndl receiverBag){

	auto bagContentStartIdx = BagGetContentStartIdx(parent, receiverBag);


	auto bagSize = objSystem->GetObject(receiverBag)->GetInt32(obj_f_bag_size);

	auto bagRows = 0;
	switch (bagSize){
	case 1:
		bagRows = 4;
		break;
	case 2:
		bagRows = 2;
		break;
	default:
		bagRows = 0;
		break;
	}
	
	if (bagSize == 1){
		return bagContentStartIdx + 6 * bagRows;
	}
	else if (bagSize == 2){
		return bagContentStartIdx + 4 * bagRows;
	}
	
	return bagContentStartIdx;
}
