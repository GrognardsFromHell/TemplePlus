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
#include "gamesystems/legacysystems.h"
#include <infrastructure/meshes.h>
#include "ai.h"
#include "animgoals/anim.h"
#include "gamesystems/objects/objevent.h"
#include "ui/ui_logbook.h"
#include <config\config.h>
#include <mod_support.h>

InventorySystem inventory;

struct InventorySystemAddresses : temple::AddressTable
{
	int (__cdecl*ItemGetAdvanced)(objHndl item, objHndl parent, int slotIdx, int flags);
	int(__cdecl*GetParent)(objHndl, objHndl*);
	int(__cdecl*ItemInsertGetLocation)(objHndl item, objHndl receiver, int* idxOut, objHndl bag, char flags);
	void(__cdecl*InsertAtLocation)(objHndl item, objHndl receiver, int itemInsertLocation);
	void(__cdecl* UiOpenContainer_0)(objHndl triggerer, objHndl container);
	ItemErrorCode(__cdecl*TransferWithFlags)(objHndl item, objHndl receiver, int invenIdx, int flags, objHndl bag);
	InventorySystemAddresses()
	{
		rebase(GetParent, 0x10063E80);
		rebase(ItemInsertGetLocation,	0x10069000);
		rebase(InsertAtLocation,		0x100694B0);
		rebase(ItemGetAdvanced,			0x1006A810);
		rebase(TransferWithFlags,0x1006B040);
		rebase(UiOpenContainer_0, 0x1014DEB0);
		
	}
} addresses;

static class InventoryHooks : public TempleFix {
public: 

	static int (__cdecl*orgItemInsertGetLocation)(objHndl, objHndl, int*, objHndl, int);
	static int(__cdecl*orgIsProficientWithArmor)(objHndl, objHndl);
	static bool(__cdecl*orgIsIncompatibleWithDruid)(objHndl, objHndl);

	void apply() override 	{

		// ItemRemove
		replaceFunction<void(__cdecl)(objHndl)>(0x10069F60, [](objHndl item) {
			inventory.ItemRemove(item);
		});

		orgIsProficientWithArmor = replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x1007C410, [](objHndl obj, objHndl armor) {
			return inventory.IsProficientWithArmor(obj, armor) ? 1 : 0;
		});

		// ItemForceRemove
		static void(__cdecl*orgItemForceRemove)(objHndl, objHndl)= replaceFunction<void(__cdecl)(objHndl, objHndl)>(0x10069AE0, [](objHndl item, objHndl parent) {
			/*auto parent_ = inventory.GetParent(item);
			if (!parent_ || !parent){
				logger->warn("item_force_remove called on item that doesn't think it has a parent.");
			}
			orgItemForceRemove(item, parent);*/
			inventory.ForceRemove(item, parent);
		});

		// TransferWithFlags
		replaceFunction<ItemErrorCode(__cdecl)(objHndl, objHndl, int, int, objHndl)>(0x1006B040, [](objHndl item, objHndl receiver, int invIdx, int flags, objHndl bag){
			return inventory.TransferWithFlags(item, receiver, invIdx, flags, bag);
		});

		replaceFunction<ItemErrorCode(__cdecl)(objHndl, objHndl, objHndl, int*, int, int)>(0x1006A000, [](objHndl owner, objHndl receiver, objHndl item, int* a4, int invSlot, int flags) {
			return inventory.ItemTransferFromTo(owner, receiver, item, a4, invSlot, flags);
		});

		replaceFunction<int(__cdecl)(objHndl, objHndl, int)>(0x1006B6C0, [](objHndl item, objHndl receiver, int flags){
			return inventory.SetItemParent(item, receiver, flags);
		});

		orgItemInsertGetLocation = replaceFunction<int(__cdecl)(objHndl, objHndl, int*, objHndl, int)>(0x10069000, [](objHndl item, objHndl receiver, int* itemInsertLoc, objHndl bag, int flags){
			return inventory.ItemInsertGetLocation(item, receiver, itemInsertLoc, bag, flags);
		});

		replaceFunction<void(__cdecl)(objHndl, objHndl, int)>(0x100694B0, [](objHndl item, objHndl receiver, int itemInsertLocation){
			inventory.InsertAtLocation(item, receiver, itemInsertLocation);
		});

		replaceFunction<objHndl(__cdecl)(objHndl, uint32_t)>(0x100651B0, [](objHndl handle, uint32_t idx){
			return inventory.GetItemAtInvIdx(handle, idx);
		});

		orgIsIncompatibleWithDruid = replaceFunction<bool(objHndl item, objHndl critter)>(0x10066430, [](objHndl item, objHndl critter) {
			return inventory.IsIncompatibleWithDruid(item, critter);
		});

		// ClearInventory
		replaceFunction<void(__cdecl)(objHndl, BOOL)>(0x10069E00, [](objHndl handle, BOOL preservePersistentItems){
			if (!handle)
				return;
			auto obj = objSystem->GetObject(handle);
			auto invenNumFld = inventory.GetInventoryNumField(handle);
			auto invenField = inventory.GetInventoryListField(handle);
			
			auto N = obj->GetInt32(invenNumFld);
			auto persistentN = 0;
			auto invenNumActualSize = obj->GetObjectIdArray(invenField).GetSize();
			if (N != invenNumActualSize) {
				logger->debug("Inventory array size for {} does not equal associated num field on ClearInventory. Arraysize: {}, numfield: {}", handle, invenNumActualSize, N);
			}
			auto loc = obj->GetLocation();

			while ( obj->GetInt32(invenNumFld) > persistentN){
				auto item = obj->GetObjHndl(invenField, persistentN);
				if (preservePersistentItems && (objects.GetItemFlags(item) & OIF_PERSISTENT) ){
					persistentN++;
					continue;
				}
				inventory.ForceRemove(item, handle);
				inventory.MoveItem(item, loc);
				objects.Destroy(item);
			}

			if (obj->IsCritter()){
				auto setMoneyZero = temple::GetRef<void(__cdecl)(objHndl)>(0x1007FCC0); 
				setMoneyZero(handle);
			}
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
				logger->debug("Inventory array size for {} ({}) does not equal associated num field on PoopItems. Arraysize: {}, numfield: {}", obj, objSystem->GetObject(obj)->id.ToString(), invenNumActualSize, invenNum);
				
			}

			auto objLoc = objects.GetLocation(obj);
			for (int i = invenNumActualSize - 1; i >= 0 ;i-- ){
				auto item = objects.getArrayFieldObj(obj, invenField, i);
				if (!item)
					continue;
				auto spFlags = objects.getInt32(item, obj_f_spell_flags);
				auto itemFlags = objects.getInt32(item, obj_f_item_flags);
				inventory.ForceRemove(item, obj);
				if ( (spFlags & SpellFlags::SF_4000) || itemFlags & (OIF_NO_DROP | OIF_NO_DISPLAY) || (itemFlags & OIF_NO_LOOT)){
					inventory.MoveItem(item, objLoc);
					objects.Destroy(item);
				} else if ( unflagNoTransfer)
				{
					objects.setInt32(item, obj_f_item_flags, itemFlags & ~OIF_NO_TRANSFER);
					inventory.MoveItem(item, objLoc);
				} else
				{
					inventory.MoveItem(item, objLoc);
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

		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x10066730, [](objHndl item, objHndl wielder){
			return (int)inventory.GetWeaponAnimId(item, wielder);
		});

		replaceFunction<int(__cdecl)(objHndl, objHndl, objHndl, SkillEnum)>(0x10069970, [](objHndl item, objHndl appraiser, objHndl vendor, SkillEnum skillEnum) {
			return inventory.GetAppraisedWorth(item, appraiser, vendor, skillEnum);
		});

		replaceFunction<int(__cdecl)(objHndl, objHndl, objHndl, int)>(0x100698E0, [](objHndl item, objHndl parent, objHndl appraiser, int skillIdx) {
			return inventory.GetAppraisedTransactionSum(item, parent, appraiser, (SkillEnum)skillIdx);
		});

		// GetWieldType
		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x10066580, [](objHndl wielder, objHndl item) {
			return inventory.GetWieldType(wielder, item, false);
		});
	};

	
} hooks;
int(__cdecl*InventoryHooks::orgItemInsertGetLocation)(objHndl, objHndl, int*, objHndl, int);
int(__cdecl*InventoryHooks::orgIsProficientWithArmor)(objHndl, objHndl);
bool(__cdecl* InventoryHooks::orgIsIncompatibleWithDruid)(objHndl, objHndl);



void InventorySystem::MoveItem(const objHndl& item, const locXY& loc){
	auto itemObj = objSystem->GetObject(item);
	if (!itemObj){
		logger->error("MoveItem called on invalid item handle!");
		return;
	}
	auto flags = itemObj->GetFlags();
	if (flags & OF_DESTROYED){
		return;
	}
	itemObj->SetFlag(OF_INVENTORY, false);
	itemObj->SetLocation(loc);
	itemObj->SetFloat(obj_f_offset_x, 0.f);
	itemObj->SetFloat(obj_f_offset_y, 0.f);

	SectorLoc secLoc(loc);
	auto &mapSectorSys = gameSystems->GetMapSector();
	if (mapSectorSys.IsSectorLoaded(secLoc)){
		LockedMapSector sector(secLoc);
		sector.AddObject(item);
	}

	itemObj->SetInt32(obj_f_render_flags, 0);
	mapSectorSys.RemoveSectorLight(item);

	static auto sub_10025050 = temple::GetPointer<int(objHndl, int)>(0x10025050);
	sub_10025050(item, 2);

	objEvents.ListItemNew(item, LocAndOffsets::null, LocAndOffsets::create(loc,0.f,0.f));

	//return temple::GetRef<void(__cdecl)(objHndl, locXY)>(0x100252D0)(item, loc);
}

int InventorySystem::PcWeaponComboGetValue(objHndl handle, int idx)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj)
		return 0;
	return obj->GetInt32(obj_f_pc_weaponslots_idx, idx);
}

void InventorySystem::PcWeaponComboSetValue(objHndl handle, int idx, int value)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj)
		return;
	return obj->SetInt32(obj_f_pc_weaponslots_idx, idx, value);
}

bool InventorySystem::IsInvIdxWorn(int invIdx){
	return invIdx >= INVENTORY_WORN_IDX_START && invIdx <= INVENTORY_WORN_IDX_END;
}

bool InventorySystem::IsIncompatibleWithDruid(objHndl item, objHndl critter)
{
	bool result = InventoryHooks::orgIsIncompatibleWithDruid(item, critter);  //Just call the old version now

	if (result) {
		auto res = dispatch.DispatchIgnoreDruidOathCheck(critter, item);
		if (res) return false;
	}

	return result;
}

bool InventorySystem::ItemAccessibleDuringPolymorph(objHndl item)
{
	if (!config.wildShapeUsableItems)
		return false;

	auto itemObj = objSystem->GetObject(item);
	if (!itemObj) return false;
	
	auto objType = itemObj->type;
	if (objType == obj_t_weapon)
		return false;
	auto wearFlags = itemObj->GetInt32(obj_f_item_wear_flags);
	if (objType == obj_t_armor) {
		if (wearFlags & OIF_WEAR_ARMOR) {
			auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
			auto armorType = GetArmorType(armorFlags);
			if (armorType != ARMOR_TYPE_SHIELD)
				return false;
		}
		if ( (wearFlags & (OIF_WEAR_NECKLACE | OIF_WEAR_RING | OIF_WEAR_CLOAK | OIF_WEAR_HELMET | OIF_WEAR_BRACERS) ) ) {
			return true;
		}
	}
	if (objType == obj_t_food) {
		return true;
	}
	if (objType == obj_t_generic) {
		if (wearFlags & OIF_USES_WAND_ANIM)
			return false;
		return true;
	}

	return false;
}

objHndl InventorySystem::GetItemAtInvIdx(objHndl handle, uint32_t nIdx){
	auto invenField = obj_f_begin;  
	auto invenNumField = obj_f_begin;  inventory.GetInventoryNumField(handle);
	auto regardPolymorph = false; // New! House Rules option to allow polymorphed to benefit from items
	if (objects.IsCritter(handle)){
		if (d20Sys.d20Query(handle, DK_QUE_Polymorphed)) {
			if (!config.wildShapeUsableItems) {
				return objHndl::null;
			}
			regardPolymorph = true;
		}
			
		invenField = inventory.GetInventoryListField(handle);
		invenNumField = inventory.GetInventoryNumField(handle);
	}
	else{
		invenField = inventory.GetInventoryListField(handle);
		invenNumField = inventory.GetInventoryNumField(handle);
	}
	auto obj = objSystem->GetObject(handle);
	auto numItems = obj->GetInt32(invenNumField);
			
	auto result = objHndl::null;
	for (auto i=0; i < numItems; i++){
		auto item = obj->GetObjHndl(invenField, i);
		if (!item) {
			logger->debug("GetItemAtInvIdx: obj {} name {} contains null object", handle, obj->GetInt32(obj_f_name));
		}
		else if (!objects.IsEquipment(item)){
			auto itemName = objects.getInt32(item, obj_f_name);
			logger->debug("GetItemAtInvIdx: obj {} name {} contains non-equipment object {} name {}", handle, obj->GetInt32(obj_f_name), item, itemName);
		}
		if (inventory.GetInventoryLocation(item) == nIdx) {
			result = item;
			break;
		}
	}

	if (regardPolymorph && result != objHndl::null) {
		if (!inventory.ItemAccessibleDuringPolymorph(result))
			return objHndl::null;
		
	}

	return result;
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
				|| itemSpell.spellEnum != invenItemSpell.spellEnum // Temple+: added this for automated scribed scrolls (who all use the same proto)
				|| (itemObj->type == obj_t_scroll 
					&& spellSys.IsArcaneSpellClass(itemSpell.classCode) 
					!= spellSys.IsArcaneSpellClass(invenItemSpell.classCode) ))
				continue;
		}

		return invenItem;
	}
	
	return objHndl::null;
}

/* 0x10066B00 */
objHndl InventorySystem::SplitObjectFromStack(objHndl handle, locXY& tgtLoc)
{
	return temple::GetRef<objHndl(__cdecl)(objHndl, locXY&)>(0x10066B00)(handle, tgtLoc);
}

void InventorySystem::WieldBest(objHndl handle, int invSlot, objHndl target){

	if (invSlot == INVENTORY_WORN_IDX_START + EquipSlot::WeaponSecondary){
		return;
	}
	auto existingItem = GetItemAtInvIdx(handle, invSlot);

	temple::GetRef<void(__cdecl)(objHndl, int, objHndl)>(0x1006CCC0)(handle, invSlot, target);
}

/* Originally 0x100FEFA0 */
bool InventorySystem::IsItemEffectingConditions(objHndl objHndItem, uint32_t itemInvLocation)
{
	auto itemObj = objSystem->GetObject(objHndItem);
	if (!itemObj) return FALSE;
	auto itemWearFlags = itemObj->GetInt32(obj_f_item_wear_flags);

	
	if (itemWearFlags || itemObj->type == obj_t_weapon) {
		return IsInvIdxWorn(itemInvLocation);
	}
	if (itemObj->type == obj_t_armor) {
		auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
		if (GetArmorType(armorFlags) == ArmorType::ARMOR_TYPE_SHIELD)
			return IsInvIdxWorn(itemInvLocation);
		// Apparently it relies on worn armors having wear flags? a bit weird...
	}
	
	return true;
}

bool InventorySystem::ItemHasCondition(objHndl item, uint32_t condId) const
{
	auto itemObj = gameSystems->GetObj().GetObject(item);
	if (!itemObj)
		return false;
	
	auto condArray = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	
	for (auto i = 0u; i < condArray.GetSize(); ++i) {
		if (condArray[i] == condId)
			return true;
	}

	return false;
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

/* 0x10105110 
 Added spellId optional condition
*/
void InventorySystem::RemoveWielderCond(objHndl item, uint32_t condId, int spellId){
	if (!condId || !item)
		return;
	auto wCond = conds.GetById(condId);
	if (!wCond) {
		logger->error("InventorySystem::RemoveWielderCond: Invalid condition! Item {}", item);
		return;
	}
	if (spellId >= 0 && wCond->numArgs < 5) {
		logger->error("RemoveWielderCond: encountered item condition removal with spell ID key, but condition has less than 5 args!");
		return;
	}

	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto wielderConds = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	auto wielderArgs = itemObj->GetInt32Array(obj_f_item_pad_wielder_argument_array);
	auto argIdx = 0;
	for (auto i = 0u; i < wielderConds.GetSize(); i++)
	{
		auto wCondId = wielderConds[i];
		
		auto isMatch = wCondId == condId;
		
		// T+: added check for spellId
		if (isMatch && spellId >= 0) {
			auto condSpellId = itemObj->GetInt32(obj_f_item_pad_wielder_argument_array, argIdx + 4);
			if (condSpellId != spellId) {
				isMatch = false;
			}
		}

		if (isMatch){
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
		argIdx += conds.GetById(wCondId)->numArgs;
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

bool InventorySystem::IsLoadableWeapon(objHndl weapon, bool strict)
{
	if (objects.GetType(weapon) != obj_t_weapon) return false;

	switch (objects.GetWeaponType(weapon))
	{
	case wt_light_crossbow:
	case wt_heavy_crossbow:
		return true;
	case wt_sling:
		return strict || config.stricterRulesEnforcement;
	// TODO: should this include repeating crossbow? hand crossbow?
	default:
		return false;
	}
}

int InventorySystem::IsThrowingWeapon(objHndl weapon)
{
	if (objects.GetType(weapon) != obj_t_weapon)
	{
		return FALSE;
	}
	WeaponAmmoType ammoType = (WeaponAmmoType)objects.getInt32(weapon, obj_f_weapon_ammo_type);
	if (ammoType > wat_dagger && ammoType <= wat_bottle) // thrown weapons   TODO: should this include daggers??
	{
		return 1;
	}
	if (ammoType == wat_dagger)
	{
		WeaponFlags weaponFlags = (WeaponFlags)objects.getInt32(weapon, obj_f_weapon_flags);
		if (weaponFlags & OWF_DEFAULT_THROWS)
		{
			return 1;
		}
	}
	
	return FALSE;
}

bool InventorySystem::IsGrenade(objHndl weapon)
{
	if (objects.GetType(weapon) != obj_t_weapon) {
		return false;
	}
	WeaponAmmoType ammoType = (WeaponAmmoType)objects.getInt32(weapon, obj_f_weapon_ammo_type);
	if (ammoType >= wat_ball_of_fire && ammoType <= wat_unk18){
		return true;
	}
	return false;
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

ArmorType InventorySystem::GetArmorType(objHndl armor)
{
	if (!armor) return ARMOR_TYPE_NONE;

	return GetArmorType(objects.getInt32(armor, obj_f_armor_flags));
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

objHndl InventorySystem::GetParent(objHndl item) {
	objHndl parent = objHndl::null;
	if (!item || !objects.IsEquipment(item)){
		logger->debug("GetParent: Called on non-Item!");
		return parent;
	}
	auto obj = objSystem->GetObject(item);
	auto flags = obj->GetFlags();
	if (!(flags & OF_INVENTORY))
		return parent;
	return obj->GetObjHndl(obj_f_item_parent);
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
	if (itemInsertLocation == -1){
		logger->error("InsertAtLocation: Attempt to insert an item at location -1!");
	}

	auto receiverObj = objSystem->GetObject(receiver);
	if (!receiverObj){
		logger->error("InsertAtLocation: Null handle receiver object! Returning.");
		return;
	}
	auto itemObj = objSystem->GetObject(item);
	if (!item){
		logger->error("InsertAtLocation: Null handle item object! Returning.");
		return;
	}
		

	auto itemType = objects.GetType(item);
	obj_f qtyField = obj_f_begin;
	auto isQuantity = inventory.GetQuantityField(item, &qtyField);

	auto receiverType = objects.GetType(receiver);
	auto invenNumField = inventory.GetInventoryNumField(receiver);
	auto invenField    = inventory.GetInventoryListField(receiver);
	
	auto mergeStackables = false;

	if (itemType == obj_t_money){
		
		if (receiverObj->IsPC()){
			party.GiveMoneyFromItem(item);
			auto loc = receiverObj->GetLocation();
			inventory.MoveItem(item, loc);
			pythonObjIntegration.ExecuteObjectScript(receiver, item, ObjScriptEvent::InsertItem);
			objects.Destroy(item);
			return;
		}
		
		if (receiverObj->IsNPC()){
			auto stackable = FindMatchingStackableItem(receiver, item);
			if (stackable){
				auto itemQty = itemObj->GetInt32(qtyField);
				auto stackQty = objects.getInt32(stackable, qtyField);
				objects.setInt32(stackable, qtyField, itemQty + stackQty);
				auto loc = receiverObj->GetLocation();
				inventory.MoveItem(item, loc);
				pythonObjIntegration.ExecuteObjectScript(receiver, item, ObjScriptEvent::InsertItem);
				objects.Destroy(item);
				return;
			}
		}
		mergeStackables = false;
	}
	else if (itemType == obj_t_key){
		if (receiverObj->IsPC()){
			auto t = gameSystems->GetTimeEvent().GetTime();
			auto keyId = itemObj->GetInt32(obj_f_key_key_id);
			uiLogbook.MarkKey(keyId, t);
			auto loc = receiverObj->GetLocation();
			inventory.MoveItem(item, loc);
			pythonObjIntegration.ExecuteObjectScript(receiver, item, ObjScriptEvent::InsertItem);
			objects.Destroy(item);
			return;
		}
	}
	else if (itemType == obj_t_ammo){
		mergeStackables = true;
	}
	else if (!IsInvIdxWorn(itemInsertLocation)){
		mergeStackables = true;
	}

	if (mergeStackables && isQuantity){
		auto stackable = FindMatchingStackableItem(receiver, item);
		if (stackable) {
			auto itemQty = itemObj->GetInt32(qtyField);
			auto stackQty = objects.getInt32(stackable, qtyField);
			objects.setInt32(stackable, qtyField, itemQty + stackQty);
			auto loc = receiverObj->GetLocation();
			inventory.MoveItem(item, loc);
			gameSystems->GetAnim().NotifySpeedRecalc(receiver);
			pythonObjIntegration.ExecuteObjectScript(receiver, item, ObjScriptEvent::InsertItem);
			objects.Destroy(item);
			return;
		}
	}

	// Set internal fields
	auto invenCount = receiverObj->GetInt32(invenNumField);
	receiverObj->SetInt32(invenNumField, invenCount + 1);
	receiverObj->SetObjHndl(invenField, invenCount, item);
	itemObj->SetInt32(obj_f_item_inv_location, itemInsertLocation);
	itemObj->SetObjHndl(obj_f_item_parent, receiver);

	// Do updates and notifications
	if (inventory.IsInvIdxWorn(itemInsertLocation)){
		static auto inheritLightFlags = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x10066260);
		inheritLightFlags(item, receiver);
		pythonObjIntegration.ExecuteObjectScript(receiver, item, ObjScriptEvent::WieldOn);
		critterSys.UpdateModelEquipment(receiver);
	}

	if (receiverObj->IsNPC()){
		receiverObj->SetInt64(obj_f_npc_ai_flags64, receiverObj->GetInt64(obj_f_npc_ai_flags64) | AiFlag::CheckWield);
	}
	if (party.IsInParty(receiver) && itemInsertLocation >= INVENTORY_IDX_HOTBAR_START && itemInsertLocation <= INVENTORY_IDX_HOTBAR_END){
		temple::GetRef<void(__cdecl)(objHndl, int)>(0x1009A510)(item, itemInsertLocation); // arcanum leftover, does nothing in ToEE
	}
	if (receiverObj->IsCritter()){
		d20StatusSys.initItemConditions(receiver);
		d20Sys.d20SendSignal(receiver, DK_SIG_Inventory_Update, item);
	}
	if (receiver){
		gameSystems->GetAnim().NotifySpeedRecalc(receiver);
		// nullsub_1009A3D0
	}
	pythonObjIntegration.ExecuteObjectScript(receiver, item, ObjScriptEvent::InsertItem);

	//addresses.InsertAtLocation(item, receiver, itemInsertLocation);
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

ItemErrorCode InventorySystem::TransferWithFlags(objHndl item, objHndl receiver, int invIdx, int flags, objHndl bag) // see ItemInsertFlags
{
	auto flagsDebug = (ItemInsertFlags)flags;
	auto parent = GetParent(item);
	if (!parent){
		return IEC_Cannot_Transfer;
	}
	auto receiverObj = objSystem->GetObject(receiver);
	auto itemObj = objSystem->GetObject(item), parentObj = objSystem->GetObject(parent);
	if (!receiverObj || !itemObj || !parentObj){
		return IEC_Cannot_Transfer;
	}

	auto itemInvLocation = itemObj->GetInt32(obj_f_item_inv_location);
	auto itemInsertLocation = INVENTORY_IDX_UNDEFINED;
	if (invIdx != INVENTORY_IDX_UNDEFINED)
		itemInsertLocation = invIdx;

	if (!receiverObj->IsCritter() && invIdx != INVENTORY_IDX_UNDEFINED && IsInvIdxWorn(invIdx)){
		return IEC_No_Room_For_Item;
	}

	auto isNonTransferable = IsItemNonTransferable(item, receiver);
	if (isNonTransferable){
		if (isNonTransferable > 0){
			logger->warn("TransferWithFlags: item {} cannot be removed from {}'s inventory, reason = {}.", item, parent, isNonTransferable);
		}
		return (ItemErrorCode)isNonTransferable;
	}

	auto result = (ItemErrorCode)ItemInsertGetLocation(item, receiver, &itemInsertLocation, bag, flags);
	int unk = 0;
	if (result == IEC_Wrong_Type_For_Slot){
		/* Handle Wrong Type For Slot error
		 try to transfer to matching equip slot
		*/

		if (invIdx == INVENTORY_IDX_UNDEFINED || !(flags & IIF_Use_Wield_Slots)){
			logger->warn("TransferWithFlags: item ( {} ) cannot be inserted into ( {} )'s inventory, reason = ( {} )", item, parent, GetItemErrorString(IEC_Wrong_Type_For_Slot));
			return IEC_Wrong_Type_For_Slot;
		}

		result = TransferToEquippedSlot(parent, receiver, item, &unk, invIdx, itemInsertLocation, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
		
	}

	if (result != IEC_OK){
		/* Handle Wield Slot Occupied error
		  try to swap with existing item.
		 */
		if (result != IEC_Wield_Slot_Occupied || !(flags & IIF_Allow_Swap)){
			logger->warn("TransferWithFlags: item ( {} ) cannot be inserted into ( {} )'s inventory, reason = ( {} )", item, parent, GetItemErrorString(result));
			return result;
		}
		// try to swap Secondary Weapon with Shield slot (this looks a bit weird but eh)
		if (itemInsertLocation == INVENTORY_WORN_IDX_START + EquipSlot::WeaponSecondary && !d20Sys.d20Query(parent, DK_QUE_Polymorphed)
			&& (parentObj->IsCritter() || parentObj->IsContainer() )){
			auto invenField = GetInventoryListField(parent);
			auto invenNumField = GetInventoryNumField(parent);
			auto invenCount = parentObj->GetInt32(invenNumField);
			for (auto i = 0; i < invenCount; i++){
				auto itemShield = parentObj->GetObjHndl(invenField, i);
				if (!itemShield) continue;
				if (objects.GetItemInventoryLocation(itemShield) == INVENTORY_WORN_IDX_START + EquipSlot::Shield){
					logger->warn("TransferWithFlags: item ( {} ) cannot be inserted into ( {} )'s inventory, reason = ( {} )", item, parent, GetItemErrorString(IEC_Wield_Slot_Occupied));
					return IEC_Wield_Slot_Occupied;
				}
			}
		}

		auto existingItem = GetItemAtInvIdx(receiver, itemInsertLocation);
		if (!existingItem){
			logger->warn("TransferWithFlags: item ( {} ) cannot be inserted into ( {} )'s inventory, reason = ( {} )", item, parent, GetItemErrorString(IEC_Wield_Slot_Occupied));
			return IEC_Wield_Slot_Occupied;
		}
		result = ItemTransferSwap(parent, receiver, item, existingItem, &unk, itemInsertLocation, itemInsertLocation, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
	}


	// IEC_OK
	if (itemInsertLocation == invIdx || itemInsertLocation != INVENTORY_IDX_UNDEFINED){
		if ( (flags & IIF_Allow_Swap) && GetItemAtInvIdx(parent, invIdx)){
			itemInsertLocation = invIdx;
		}
		result = ItemTransferFromTo(parent, receiver, item, &unk, itemInsertLocation, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
	}

	// Unspecified Inventory Slot
	if (invIdx == INVENTORY_IDX_UNDEFINED){
		if (flags & IIF_Use_Wield_Slots){
			result = TransferToEquippedSlot(parent, receiver, item, &unk, INVENTORY_IDX_UNDEFINED, INVENTORY_IDX_UNDEFINED, flags);
			TransferPcInvLocation(item, itemInvLocation);
			return result;
		}
		if ( !(flags & IIF_4)){
			logger->warn("TransferWithFlags: item ( {} ) cannot be inserted into ( {} )'s inventory, reason = ( {} )", item, parent, GetItemErrorString(IEC_OK));
			return IEC_Cannot_Transfer;
		}
		result = ItemTransferFromTo(parent, receiver, item, &unk, INVENTORY_IDX_UNDEFINED, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
	} 
	
	// Specified Slot
	// Check if slot is clear
	auto existingItem = GetItemAtInvIdx(receiver, invIdx);
	if (!existingItem){
		result = ItemTransferFromTo(parent, receiver, item, &unk, invIdx, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
	}
	// If not, check swappage
	if (flags & IIF_Allow_Swap){
		result = ItemTransferSwap(parent, receiver, item, existingItem, &unk, invIdx, INVENTORY_IDX_UNDEFINED, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
	}
	// If not, try unspecified slot 
	if (!(flags & IIF_Use_Wield_Slots)){
		if (!(flags &IIF_4)){
			return IEC_No_Room_For_Item;
		}
		result = ItemTransferFromTo(parent, receiver, item, &unk, INVENTORY_IDX_UNDEFINED, flags);
		TransferPcInvLocation(item, itemInvLocation);
		return result;
	}

	// Go over wield slots
	for (auto equipSlot = 0; equipSlot < EquipSlot::Count; ++equipSlot){
		if (ItemWornAt(receiver, equipSlot)) continue;
			
		if (IEC_OK == CheckTransferToWieldSlot(item, equipSlot + INVENTORY_WORN_IDX_START, receiver)){
			result = ItemTransferFromTo(parent, receiver, item, &unk, equipSlot + INVENTORY_WORN_IDX_START, flags);
			TransferPcInvLocation(item, itemInvLocation);
			return result;
		}
	}
	return IEC_Cannot_Transfer;
	
	// return addresses.TransferWithFlags(item, receiver, invIdx, flags, bag);
}

void InventorySystem::TransferPcInvLocation(objHndl item, int itemInvLocation){
	auto parent = GetParent(item);
	if (!parent){
		return;
	}
	auto parentObj = objSystem->GetObject(parent);
	if (!parentObj)
		return;
	if (parentObj->IsPC()){
		auto oldInvLocation = objSystem->GetObject(item)->GetInt32(obj_f_item_inv_location);
		PcInvLocationSet(parent, itemInvLocation, oldInvLocation);
	}
}

void InventorySystem::PcInvLocationSet(objHndl parent, int itemInvLocation, int itemInvLocationNew){
	auto parentObj = objSystem->GetObject(parent);
	for (auto i = 1; i < 21; ++i){
		if (parentObj->GetInt32(obj_f_pc_weaponslots_idx, i) == itemInvLocation){
			parentObj->SetInt32(obj_f_pc_weaponslots_idx, i, itemInvLocationNew);
		}
	}
}

ItemErrorCode InventorySystem::CheckSlotAndWieldFlags(objHndl item, objHndl receiver, int invIdx){
	return temple::GetRef<ItemErrorCode(__cdecl)(objHndl, objHndl, int)>(0x10067680)(item, receiver, invIdx);
}

ItemErrorCode InventorySystem::TransferToEquippedSlot(objHndl parent, objHndl receiver, objHndl item, int* unk,
	int invIdx, int itemInsertLocation, int flags){
	auto receiverObj = objSystem->GetObject(receiver);
	if (!receiverObj || !receiverObj->IsCritter()){
		return IEC_Cannot_Transfer;
	}

	for (auto equipSlot = 0; equipSlot <= EquipSlot::Count; equipSlot++){
		if (CheckSlotAndWieldFlags(item, receiver, equipSlot + INVENTORY_WORN_IDX_START))
			continue;
		auto existingItem = ItemWornAt(receiver, equipSlot);
		if (!existingItem){
			auto result = ItemTransferFromTo(parent, receiver, item, unk, equipSlot + INVENTORY_WORN_IDX_START, flags);
			return result;
		}
		auto invLocation = objSystem->GetObject(existingItem)->GetInt32(obj_f_item_inv_location);
		if (flags & IIF_Allow_Swap){
			auto result = ItemTransferSwap(parent, receiver, item, existingItem, unk, invLocation, itemInsertLocation, flags);
			return result;
		}
	}

	return IEC_Cannot_Transfer;
}

/* 0x1006BB50 */
void InventorySystem::ItemPlaceInIdx(objHndl item, int idx)
{
	auto parent = GetParent(item);
	TransferWithFlags(item, parent, idx, 4, objHndl::null);
}

int InventorySystem::IsTradeGoods(objHndl item)
{
	auto type = objects.GetType(item);
	auto cat = objects.getInt32(item, obj_f_category);
	if (type == obj_t_generic && cat == 5) // jewelry / gems
		return 1;
	if (type == obj_t_armor && cat == 17) // stackable jewelry
		return 1;
	return 0;
}

int InventorySystem::GetAppraisedWorth(objHndl item, objHndl appraiser, objHndl vendor, SkillEnum skillEnum)
{
	auto skillLevelGet_1007D430 = temple::GetRef<double(__cdecl)(objHndl, int)>(0x1007D430);
	auto skillLevel = skillLevelGet_1007D430(appraiser, skillEnum);
	if (skillLevel > 19) skillLevel = 19;

	double price = (double)GetSellWorth(item, appraiser, vendor, skillEnum);
	if (!price)
		return 0;

	double result = ((double)skillLevel * 0.029999999 + 0.40000001) * price;

	if (modSupport.IsZMOD()) {
		if (IsTradeGoods(item)) {
			return objects.getInt32(item, obj_f_item_worth);
		}
		auto getInvenSourceType = temple::GetRef<int(__cdecl)(objHndl)>(0x10064040);
		auto invenSourceType = getInvenSourceType(vendor);

		// if invenSourceType is not specified, then do not lower price
		if (!invenSourceType) {
			return (int)result;
		}
	} 
	auto getIsInVendorBuyList = temple::GetRef<int(__cdecl)(objHndl, objHndl)>(0x10066CD0);
	auto isInVendorBuyList = getIsInVendorBuyList(item, vendor);
	if (!isInVendorBuyList)
		result = result * 0.5;
	return (int)result;
}

int InventorySystem::GetAppraisedTransactionSum(objHndl item, objHndl parent, objHndl appraiser, SkillEnum skillEnum)
{
	auto skillLevelGet_1007D430 = temple::GetRef<double(__cdecl)(objHndl, int)>(0x1007D430);
	auto skillLevel = skillLevelGet_1007D430(appraiser, skillEnum);
	if (skillLevel > 19) skillLevel = 19;

	double price = (double)GetSellWorth(item, parent, appraiser, skillEnum);
	if (!price)
		return 0;

	if (modSupport.IsZMOD()){
		return (int)price;
	}
	double result = ((1.6 - (double)skillLevel * 0.029999999)) * price;
	return (int)result;
}

int InventorySystem::GetSellWorth(objHndl item, objHndl appraiser, objHndl vendor, SkillEnum skillEnum)
{
	auto getSellWorth = temple::GetRef<int(__cdecl)(objHndl, objHndl, objHndl, int)>(0x10067D20);
	return getSellWorth(item, appraiser, vendor, skillEnum);
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

// 0: light weapon
// 1: 1-handed weapon
// 2: 2-handed weapon
// 3: bastard sword/dwarf waraxe too big to wield
// 4: null weapon
int InventorySystem::GetWieldType(objHndl wielder, objHndl item, bool regardEnlargement) const
{
	if (!item)
		return 4;

	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto itemType = itemObj->type;

	if (itemType == obj_t_armor){
		auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
		auto armorType = inventory.GetArmorType(armorFlags);
		if (armorType == ArmorType::ARMOR_TYPE_SHIELD) {
			// if we can shield bash, fall through to use size based 
			if (!d20Sys.d20Query(wielder, DK_QUE_Can_Shield_Bash)) {
				if (IsBuckler(item)) return 0;
				return 3;
			}
		} else if (armorType == ArmorType::ARMOR_TYPE_NONE) {
			// uncertain what this case is, but it was in the original
			return !IsBuckler(item);
		}
	}


	auto wielderSize = critterSys.GetSize(wielder);
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
	else if (itemSize == wielderSize + 1)
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

	auto hasExotic = feats.HasFeatCountByClass(wielder, FEAT_EXOTIC_WEAPON_PROFICIENCY_DWARVEN_WARAXE);
	auto isDwarf = objects.StatLevelGet(wielder, stat_race) == race_dwarf;
	// TODO: make a specific martial feat that no one will ever take?
	auto hasMartial = feats.HasFeatCountByClass(wielder, FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL);
	// only require martial proficiency if strict enforcement is on
	hasMartial |= !config.stricterRulesEnforcement;

	if (hasExotic || (isDwarf && hasMartial))
	{
		return wieldType;
	} 
	
	if (wielderSize == 5)
	{
		return 2;
	} 

	return 2 * (wielderSize <= 5) + 1;	
}

bool InventorySystem::IsFinesse(objHndl wielder, objHndl weapon) const
{
	// natural weapons are light
	if (!weapon) return true;

	auto wieldType = GetWieldType(wielder, weapon, true);
	auto weapType =
		static_cast<WeaponTypes>(objects.getInt32(weapon, obj_f_weapon_type));

	if (wieldType == 0) return true; // light weapon

	// base sizes, to ignore enlarge/reduce
	auto weapSize = objects.getInt32(weapon, obj_f_size);
	auto wieldSize = objects.getInt32(wielder, obj_f_size);

	switch (weapType)
	{
	case wt_rapier:
	case wt_spike_chain:
		// rapiers and spiked chains of appropriate size can be finessed
		return weapSize <= wieldSize;
	default:
		return false;
	}
}

bool InventorySystem::IsWieldedTwoHanded(objHndl weapon, objHndl wielder, bool special){
	if (!weapon) return false;

	auto weapObj = gameSystems->GetObj().GetObject(weapon);
	auto weapType = (WeaponTypes)weapObj->GetInt32(obj_f_weapon_type);

	// special case - rapiers are always wielded one handed
	if (weapType == wt_rapier){
		return false;
	}

	auto offhandWeapon = ItemWornAt(wielder, EquipSlot::WeaponSecondary);
	auto shield = ItemWornAt(wielder, EquipSlot::Shield);
  // are you holding the weapon with your buckler hand?
	bool shieldAllowsTwoHandedWield = (shield != objHndl::null) && IsBuckler(shield);
	if (shieldAllowsTwoHandedWield){
		if (d20Sys.d20Query(wielder, DK_QUE_Is_Preferring_One_Handed_Wield))
			shieldAllowsTwoHandedWield = false;
	}
	bool hasInterferingOffhand = false;
	if (offhandWeapon != objHndl::null){
		hasInterferingOffhand = true;
	}
	if (shield != objHndl::null){
		hasInterferingOffhand |= !shieldAllowsTwoHandedWield;
	}
	auto wieldType = GetWieldType(wielder, weapon, true);
	// the wield type if the weapon is not enlarged along with the critter
	auto wieldTypeMod = GetWieldType(wielder, weapon, false);

	bool isTwohandedWieldable = !hasInterferingOffhand && !special;

	switch (wieldType)
	{
	case 0: // light weapon
		switch (wieldTypeMod)
		{
		case 0:
			isTwohandedWieldable = false;
			break;
		case 1: // benefitting from enlargement of weapon
		case 2:
		default:
			break;
		}
	case 1: // single handed wield if weapon is unaffected
		switch (wieldTypeMod)
		{
		case 0: // only in reduce person; going to assume the "beneficial" case that the reduction was made voluntarily and hence you let the weapon stay larger
		case 1:
		case 2:
		default:
			break;
		}
	case 2: // two handed wield if weapon is unaffected
		switch (wieldTypeMod)
		{
		case 0:
		case 1: // only in reduce person
			break;
		case 2:
			if (hasInterferingOffhand) // shouldn't really be possible to hold two Two Handed Weapons... maybe if player is cheating
			{
				logger->warn("Illegally wielding weapon along withoffhand!");
			}
		default:
			break;
		}
	case 3:
		break;
	case 4:
	default:
		break;
	}

	return isTwohandedWieldable;
}



gfx::WeaponAnimType InventorySystem::GetWeaponAnimId(objHndl item, objHndl wielder, bool special){
	auto weap = objSystem->GetObject(item);
	auto wieldType = GetWieldType(wielder, item, false);
	WeaponTypes wtype = (WeaponTypes)weap->GetInt32(obj_f_weapon_type);

	if (wieldType == 4) return gfx::WeaponAnimType::Unarmed;
	if (IsWieldedTwoHanded(item, wielder, special)) {
		switch (wtype)
		{
		case WeaponTypes::wt_short_sword:
			// cutlass is a slashing shortsword
			if (weap->GetInt32(obj_f_weapon_attacktype) == (int32_t)DamageType::Slashing)
				return gfx::WeaponAnimType::Greatsword;
			else
				return gfx::WeaponAnimType::Spear;
		// piercing weapons
		case WeaponTypes::wt_dagger:
		case WeaponTypes::wt_rapier:
			return gfx::WeaponAnimType::Spear;
		case WeaponTypes::wt_orc_double_axe:
		case WeaponTypes::wt_dwarven_urgrosh:
			if (d20Sys.d20Query(wielder, DK_QUE_Is_Two_Weapon_Fighting) > 1)
				return gfx::WeaponAnimType::Staff;
			else
				return gfx::WeaponAnimType::Greataxe;
		case WeaponTypes::wt_quarterstaff:
		case WeaponTypes::wt_gnome_hooked_hammer:
			if (d20Sys.d20Query(wielder, DK_QUE_Is_Two_Weapon_Fighting) > 1)
				return gfx::WeaponAnimType::Staff;
			else
				return gfx::WeaponAnimType::Greathammer;
		default:
			// original two handed anim array
			return (gfx::WeaponAnimType)temple::GetRef<int[74]>(0x102BE668)[wtype];
		}
	} else {
		switch (wtype)
		{
		// piercing weapons
		case WeaponTypes::wt_short_sword: // was sword
			// cutlass is a slashing shortsword
			if (weap->GetInt32(obj_f_weapon_attacktype) == (int32_t)DamageType::Slashing)
				return gfx::WeaponAnimType::Sword;
		case WeaponTypes::wt_rapier: // was sword
			return gfx::WeaponAnimType::Dagger;
		// slashing weapons
		case WeaponTypes::wt_kukri: // was dagger
			return gfx::WeaponAnimType::Sword;
		default:
			// original single handed anim array
			return (gfx::WeaponAnimType)temple::GetRef<int[74]>(0x102BE540)[wtype];
		}
	}
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
	
	critterSys.SuspendModelUpdate(true); // added to improve perf
	for (auto invIdx = INVENTORY_WORN_IDX_START; invIdx <  INVENTORY_WORN_IDX_END; invIdx++){
		WieldBest(critter, invIdx, tgt);
	}
	critterSys.SuspendModelUpdate(false);

	critterSys.UpdateModelEquipment(critter);
}

void InventorySystem::ForceRemove(objHndl item, objHndl parent){
	if (!parent) {
		logger->warn("ForceRemove called on null parent!");
		return;
	}

	auto _parent = GetParent(item);
	auto isparty = false;
	if (!_parent){
		logger->warn("ForceRemove called on item that doesn't think it has a parent.");
		// return;
	}
	else{
		if (parent != _parent){
			logger->warn("ForceRemove called on item with different parent");
		}
		isparty = party.IsInParty(parent);
	}

	auto parentObj = objSystem->GetObject(parent);
	auto invenField = obj_f_container_inventory_list_idx;
	auto invenNumField = obj_f_container_inventory_num;
	if (!parentObj->IsContainer()){
		invenField = obj_f_critter_inventory_list_idx;
		invenNumField = obj_f_critter_inventory_num;
	}
	auto N = parentObj->GetInt32(invenNumField);

	auto idx = -1;
	for (auto i=0; i < N; i++){
		if (item == parentObj->GetObjHndl(invenField, i)){
			idx = i;
			break;
		}
	}
	if (idx < 0){
		logger->error("ForceRemove: Couldn't match object in parent!");
		return;
	}
	
	if (parentObj->IsCritter() && !(parentObj->GetFlags() & OF_DESTROYED)){
		dispatch.Dispatch68ItemRemove(parent);
	}

	// Delete item from inventory list
	auto itemObj = objSystem->GetObject(item);
	auto invIdxOrg = itemObj->GetInt32(obj_f_item_inv_location);
	
	while (idx < N-1){
		auto tmp = parentObj->GetObjHndl(invenField, idx + 1);
		parentObj->SetObjHndl(invenField, idx, tmp);
		idx++;
	}
	parentObj->RemoveObjectId(invenField, N-1);
	parentObj->SetInt32(invenNumField, N - 1);
	itemObj->SetInt32(obj_f_item_inv_location, -1);
	
	if (inventory.IsInvIdxWorn(invIdxOrg)){
		// unset OWF_WEAPON_LOADED
		// `true` forces in case you've loaded a save with a loaded sling without
		// strict rules enabled; clears anyway.
		if (inventory.IsLoadableWeapon(item, true)) {
			itemObj->SetInt32(obj_f_weapon_flags, itemObj->GetInt32(obj_f_weapon_flags) & ~OWF_WEAPON_LOADED);
		}
		static auto inheritLightFlags = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x10066260);
		inheritLightFlags(item, parent);
		pythonObjIntegration.ExecuteObjectScript(parent, item, ObjScriptEvent::WieldOff);
		critterSys.UpdateModelEquipment(parent);
	}
	else{
		if (invIdxOrg >= INVENTORY_IDX_HOTBAR_START && invIdxOrg <= INVENTORY_IDX_HOTBAR_END){
			static auto hotbarRelatedArcanumLeftover = temple::GetRef<void(__cdecl)()>(0x1009A500);
			hotbarRelatedArcanumLeftover();
		}
	}

	if (parentObj->IsContainer()){
		if (!gameSystems->GetItem().editorMode && gameSystems->GetItem().junkpileActive){
			static auto junkpileOnRemoveItem = temple::GetRef<void(__cdecl)(objHndl)>(0x10069A20);
			junkpileOnRemoveItem(parent);
		}
	}
	else if (parentObj->IsNPC()){
		auto aiFlags = (AiFlag) (parentObj->GetInt64(obj_f_npc_ai_flags64) | AiFlag::CheckWield);
		parentObj->SetInt64(obj_f_npc_ai_flags64, aiFlags);
	}
	if (parent){
		gameSystems->GetAnim().NotifySpeedRecalc(parent);
		// nullsub_1009A3D0 - removed
	}
	pythonObjIntegration.ExecuteObjectScript(parent, item, ObjScriptEvent::RemoveItem);
	if (parentObj->IsCritter() && ! (parentObj->GetFlags() & OF_DESTROYED) ){
		d20StatusSys.initItemConditions(parent);
		critterSys.BuildRadialMenu(parent);
	}

	//_ForceRemove(item, parent);
}

bool InventorySystem::IsProficientWithArmor(objHndl obj, objHndl armor) const
{

	if (obj) {
		int res = d20Sys.D20QueryPython(obj, "Has Light Shield Proficency");
		if (res != 0) {
			auto itemObj = gameSystems->GetObj().GetObject(armor);
			auto itemType = itemObj->type;

			if (itemType == obj_t_armor) {
				auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
				auto armorType = inventory.GetArmorType(armorFlags);
				if (armorType == ArmorType::ARMOR_TYPE_SHIELD) {
					auto spellFailure = itemObj->GetInt32(obj_f_armor_arcane_spell_failure);
					if (spellFailure <= 5) {  //Only a light shield or buckler will have a spell failure this low
						return 1;
					}
				}
			}
		}
	}
	return InventoryHooks::orgIsProficientWithArmor(obj, armor) != 0;
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

bool InventorySystem::IsDoubleWeapon(objHndl weapon)
{
	if (!weapon) return false;

	switch (objects.GetWeaponType(weapon))
	{
	case wt_quarterstaff:
	case wt_gnome_hooked_hammer:
	case wt_orc_double_axe:
	case wt_dire_flail:
	case wt_two_bladed_sword:
	case wt_dwarven_urgrosh:
		return true;

	default:
		return false;
	}
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

ItemErrorCode InventorySystem::CheckTransferToWieldSlot(objHndl item, int invSlot, objHndl receiver){
	return temple::GetRef<ItemErrorCode(__cdecl)(objHndl, int, objHndl)>(0x10067F90)(item, invSlot, receiver);
}

ItemErrorCode InventorySystem::ItemTransferFromTo(objHndl owner, objHndl receiver, objHndl item, int * a4, int invSlot, int flags)
{
	auto result = IEC_OK;
	auto ownerObj = objSystem->GetObject(owner), receiverObj = objSystem->GetObject(receiver), itemObj = objSystem->GetObject(item);
	if (!receiverObj || !itemObj){
		return result;
	}
	if (ownerObj && ownerObj->IsCritter() && IsInvIdxWorn(invSlot)){
		result = CheckTransferToWieldSlot(item, invSlot, receiver);

		//Fix for Atari bug #81
		if (invSlot == InvIdxForSlot(EquipSlot::Shield)) {  //Shield slot
			if (result == IEC_OK || flags & IIF_Allow_Swap) {
				bool incompatible = IsIncompatibleWithDruid(item, receiver);
				if (incompatible) {
					return IEC_Incompatible_With_Druid;
				}
			}
		}

		// Workaround for armor that causes crashes on dwarves
		if (invSlot == InvIdxForSlot(EquipSlot::Armor)){
			if (result == IEC_OK || flags & IIF_Allow_Swap) {
				if (critterSys.GetRace(owner) == Race::race_dwarf) {
					auto armorMeshId = itemObj->GetInt32(obj_f_base_mesh);
					if (armorMeshId == 12172) {
						return IEC_Has_No_Art;
					}
				}
			}
		}

		if (result != IEC_OK){
			if (!(flags & IIF_Allow_Swap)){
				return result;
			}
			auto existingItem = GetItemAtInvIdx(owner, invSlot);
			if (!existingItem) {
				return result;
			}
			result = ItemTransferSwap(owner, receiver, item, existingItem, a4, invSlot, invSlot, flags);
			return result;
		}
	}

	if (owner == receiver){
		ItemRemove(item);
		InsertAtLocation(item, receiver, invSlot);
		return IEC_OK;
	}
	// owner != receiver
	ItemRemove(item);
	itemObj->SetItemFlag(OIF_NO_TRANSFER, false);
	InsertAtLocation(item, receiver, invSlot);
	return IEC_OK;
}

ItemErrorCode InventorySystem::ItemTransferSwap(objHndl owner, objHndl receiver, objHndl item, objHndl itemPrevious,
	int* a4, int equippedItemSlot, int destItemSlotMaybe, int flags)
{	
	// Atari bug #90 fix
	if (equippedItemSlot == InvIdxForSlot(EquipSlot::Shield)) {
		const bool incompatible = IsIncompatibleWithDruid(item, receiver);
		if (incompatible) {
			return IEC_Incompatible_With_Druid;
		}
	}

	auto itemObj = objSystem->GetObject(item);
	auto receiverObj = objSystem->GetObject(receiver);
	if (!itemObj || !receiverObj) {
		return IEC_Cannot_Transfer;
	}

	// Workaround for specific armor model that causes crashes on dwarves
	if (equippedItemSlot == InvIdxForSlot(EquipSlot::Armor) && receiverObj->IsCritter()) {
		if (critterSys.GetRace(receiver) == Race::race_dwarf) {

			auto armorMeshId = itemObj->GetInt32(obj_f_base_mesh);
			if (armorMeshId == 12172) {
				return IEC_Has_No_Art;
			}
		}
	}
	
	static auto itemTransferSwap = temple::GetRef<ItemErrorCode(__cdecl)(objHndl, objHndl, objHndl, objHndl, int*, int, int, int)>(0x1006AB50);
	auto result = itemTransferSwap(owner, receiver, item, itemPrevious, a4, equippedItemSlot, destItemSlotMaybe, flags);
	return result;
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
			itemWorthTotal = 100 * ((lootingType >> 8) & 0xFFFF00);
			lastItemWorth = 1600 * (lootingType & 0xFFF0);
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
	for (auto i = 0u; i < npcSize; i++) {
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
	for (size_t i=0; i < items.size(); i++){
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

void InventorySystem::UiOpenContainer(objHndl triggerer, objHndl container)
{
	addresses.UiOpenContainer_0(triggerer, container);
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
