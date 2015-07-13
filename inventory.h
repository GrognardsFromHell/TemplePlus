#pragma once
#include "common.h"
#include "dispatcher.h"

struct InventorySystem : AddressTable
{
	
	objHndl(__cdecl *GetSubstituteInventory)  (objHndl);
	objHndl(__cdecl *GetItemAtInvIdx)(objHndl, uint32_t nIdx); // returns the item at obj_f_critter_inventory subIdx nIdx  (or obj_f_container_inventory for containers); Note the difference to ItemWornAt! (this is a more low level function)
	objHndl(__cdecl *ItemWornAt)(objHndl, uint32_t nItemSlot);
	objHndl(__cdecl *FindMatchingStackableItem)(objHndl objHndReceiver, objHndl objHndItem); // TODO: rewrite so it doesn't stack items with different descriptions and/or caster levels, so potions/scrolls of different caster levels don't stack
	
	
	void (__cdecl *sub_100FF500)(Dispatcher *dispatcher, objHndl objHndItem, uint32_t itemInvLocation);
	uint32_t(__cdecl *IsItemEffectingConditions)(objHndl objHndItem, uint32_t itemInvLocation);
	
	/*
	Container can both be a critter or an item.
	nameId must be the MES line number for the item name.
	*/
	objHndl FindItemByName(objHndl container, int nameId);

	/*
	Container can both be a critter or an item.
	*/
	objHndl FindItemByProtoHandle(objHndl container, objHndl protoHandle, bool skipWorn = false);

	objHndl FindItemByProtoId(objHndl container, int protoId, bool skipWorn = false);

	int SetItemParent(objHndl item, objHndl parent, int flags);
	int IsNormalCrossbow(objHndl weapon);
	int IsThrowingWeapon(objHndl weapon);
	ArmorType GetArmorType(int armorFlags);
	int ItemDrop(objHndl item);
	int GetParent(objHndl item);
	/*
		0 - light weapon; 1 - can wield one handed; 2 - must wield two handed; 3 (???)
	*/
	int (__cdecl *GetWieldType)(objHndl wielder, objHndl item);
	static obj_f GetInventoryListField(objHndl objHnd);

	/*
		Identifies all items held or contained within the given parent.
	*/
	void (__cdecl *IdentifyAll)(objHndl parent);

	/*
		Tries to wield the best items, unclear what the optional item argument does.
	*/
	void (__cdecl *WieldBestAll)(objHndl critter, objHndl item);

	/*
		Clears the inventory of the given object. Keeps items that have the PERSISTENT flag set if
		the second argument is TRUE.
	*/
	void (__cdecl *Clear)(objHndl parent, BOOL keepPersistent);

	int ItemRemove(objHndl item);
	int ItemGetAdvanced(objHndl item, objHndl parent, int slotIdx, int flags);

	InventorySystem()
	{
		rebase(GetSubstituteInventory, 0x1007F5B0);
		rebase(GetItemAtInvIdx, 0x100651B0);
		rebase(ItemWornAt,      0x10065010);
		rebase(GetWieldType,    0x10066580);
		rebase(FindMatchingStackableItem, 0x10067DF0);

		rebase(sub_100FF500, 0x100FF500);
		rebase(IsItemEffectingConditions, 0x100FEFA0);

		rebase(_FindItemByName, 0x100643F0);
		rebase(_FindItemByProto, 0x100644B0);
		rebase(_SetItemParent, 0x1006B6C0);

		rebase(IdentifyAll, 0x10064C70);
		rebase(WieldBestAll, 0x1006D100);
		rebase(Clear,		0x10069E00);
		rebase(_ItemRemove, 0x10069F60);
		rebase(_ItemDrop, 0x1006AA60);
	}

private:
	int(__cdecl *_SetItemParent)(objHndl item, objHndl parent, int flags);
	objHndl(__cdecl *_FindItemByName)(objHndl container, int nameId);
	objHndl(__cdecl *_FindItemByProto)(objHndl container, objHndl proto, bool skipWorn);
	int(__cdecl*_ItemRemove)(objHndl item);
	int(__cdecl*_ItemDrop)(objHndl item);
};

extern InventorySystem inventory;