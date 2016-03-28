#include "stdafx.h"
#include "common.h"
#include "inventory.h"
#include "obj.h"
#include "critter.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamesystems.h"
#include "util/fixes.h"

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
	const char* name() override { 
		return "Inventory Hooks";
	} 
	
	void apply() override 	{
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
	};
} hooks;

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
	return _SetItemParent(item, parent, flags);
}

int InventorySystem::IsNormalCrossbow(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		auto weapType = objects.GetWeaponType(weapon);
		if (weapType == wt_heavy_crossbow || weapType == wt_light_crossbow)
			return 1; // TODO: should this include repeating crossbow? I think the context is reloading action in some cases
		// || weapType == wt_hand_crossbow
	}
	return 0;
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

ArmorType InventorySystem::GetArmorType(int armorFlags)
{
	if (armorFlags & ARMOR_TYPE_NONE)
		return ARMOR_TYPE_NONE;
	return (ArmorType) (armorFlags & (ARMOR_TYPE_LIGHT | ARMOR_TYPE_MEDIUM | ARMOR_TYPE_HEAVY) );
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

int InventorySystem::ItemGet(objHndl item, objHndl receiver, int flags)
{
	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto recObj = gameSystems->GetObj().GetObject(receiver);
	
	if (objects.IsContainer(receiver)){
		return ItemGetAdvanced(item, receiver, -1, 0x8);
	}
	
	if (!objects.IsCritter(receiver)){
		return 0;
	}
	
	if ( gameSystems->GetObj().GetProtoId(item) == 6239){ // Darley's Neckalce
		return ItemGetAdvanced(item, receiver, 201, flags);
	}

	return ItemGetAdvanced(item, receiver, -1, flags);
	
}

objHndl InventorySystem::GetParent(objHndl item)
{
	objHndl parent = 0i64;
	if (!addresses.GetParent(item, &parent))
		return 0;
	return parent;
}

bool InventorySystem::IsRangedWeapon(objHndl weapon)
{
	if (objects.GetType(weapon) != obj_t_weapon)
		return false;
	if (objects.getInt32(weapon, obj_f_weapon_flags) & OWF_RANGED_WEAPON)
		return true;
	return false;
}

int InventorySystem::GetInventory(objHndl objHandle, objHndl ** inventoryArray)
{
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

int InventorySystem::GetInventoryLocation(objHndl item)

{
	ObjectType objType;
	if(item && objects.IsEquipment(item))
		return objects.getInt32(item, obj_f_item_inv_location);
	
	logger->error("Item: item_parent: ERROR: Called on non-item!\n");
	if (item)
		return objects.getInt32(item, obj_f_item_inv_location);
	return 0;
	
}

ItemFlag InventorySystem::GetItemFlags(objHndl item)
{
	return static_cast<ItemFlag>(objects.getInt32(item, obj_f_item_flags));
}

bool InventorySystem::IsItemNonTransferable(objHndl item, objHndl receiver)
{
	objHndl parent = GetParent(item);
	ItemFlag itemFlags = inventory.GetItemFlags(item);
	if ((itemFlags & OIF_NO_DROP)
		&& (!receiver || !parent || objSystem->GetProtoId(item) == 6239 || receiver != parent))
		return 7;
	if ( (itemFlags & OIF_NO_TRANSFER )
		&& receiver != parent && parent && !critterSys.IsDeadNullDestroyed(parent))
		return 8;
	if (!(itemFlags & OIF_NO_TRANSFER_SPECIAL) || receiver == parent)
	{
		return 0;
	}
	return 7;
}

int InventorySystem::ItemInsertGetLocation(objHndl item, objHndl receiver, int* itemInsertLocation, objHndl bag, char flags)
{
	return addresses.ItemInsertGetLocation(item, receiver, itemInsertLocation, bag, flags);
}

void InventorySystem::InsertAtLocation(objHndl item, objHndl receiver, int itemInsertLocation)
{
	addresses.InsertAtLocation(item, receiver, itemInsertLocation);
}

int InventorySystem::ItemUnwield(objHndl item)
{
	auto invenLoc = GetInventoryLocation(item);
	if (invenLoc < 200 || invenLoc > 216)
		return 1;
	objHndl parent = GetParent(item);
	int itemInsertLocation = 0;
	if (IsItemNonTransferable(item, parent)
		|| inventory.ItemInsertGetLocation(item, parent, &itemInsertLocation, 0, 0))
		return 0;
	ItemRemove(item);
	inventory.InsertAtLocation(item, parent, itemInsertLocation);
	return 1;
}

int InventorySystem::ItemUnwieldByIdx(objHndl obj, int i)
{
	auto item = inventory.GetItemAtInvIdx(obj, i);
	if (!item)
		return 1;
	return ItemUnwield(item);
}

ItemErrorCode InventorySystem::TransferWithFlags(objHndl item, objHndl receiver, int invenIdx, char flags, objHndl bag)
{
	return addresses.TransferWithFlags(item, receiver, invenIdx, flags, bag);
}

void InventorySystem::ItemPlaceInIdx(objHndl item, int idx)
{
	auto parent = GetParent(item);
	TransferWithFlags(item, parent, idx, 4, 0i64);
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

obj_f InventorySystem::GetInventoryListField(objHndl objHnd)
{
	if (objects.IsCritter(objHnd)) 	return obj_f_critter_inventory_list_idx;
	if (objects.IsContainer(objHnd)) return obj_f_container_inventory_list_idx;
	return (obj_f)0;
}

void InventorySystem::ForceRemove(objHndl item, objHndl parent)
{
	_ForceRemove(item, parent);
}

bool InventorySystem::IsProficientWithArmor(objHndl obj, objHndl armor) const
{
	auto isProfWithArmor = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1007C410);
	return isProfWithArmor(obj, armor);
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

int InventorySystem::ItemGetAdvanced(objHndl item, objHndl parent, int slotIdx, int flags)
{
	return addresses.ItemGetAdvanced(item, parent, slotIdx, flags);
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

	if (slot < 200) {
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

	return sBoneNames[slot - 200];
}
