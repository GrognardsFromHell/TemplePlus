#pragma once
#include "common.h"
#include "dispatcher.h"

struct InventorySystem : AddressTable
{
	
	objHndl(__cdecl *GetSubstituteInventory)  (objHndl);
	objHndl(__cdecl *GetItemAtInventoryLocation)(objHndl, uint32_t nIdx);
	objHndl(__cdecl *ItemWornAt)(objHndl, uint32_t nItemSlot);
	objHndl(__cdecl *FindMatchingStackableItem)(objHndl objHndReceiver, objHndl objHndItem); // TODO: rewrite so it doesn't stack items with different descriptions and/or caster levels, so potions/scrolls of different caster levels don't stack
	
	
	void (__cdecl *sub_100FF500)(Dispatcher *dispatcher, objHndl objHndItem, uint32_t itemInvLocation);
	uint32_t(__cdecl *IsItemEffectingConditions)(objHndl objHndItem, uint32_t itemInvLocation);



	InventorySystem()
	{
		rebase(GetSubstituteInventory, 0x1007F5B0);
		rebase(GetItemAtInventoryLocation, 0x100651B0);
		rebase(ItemWornAt, 0x10065010);
		rebase(FindMatchingStackableItem, 0x10067DF0);

		rebase(sub_100FF500, 0x100FF500);
		rebase(IsItemEffectingConditions, 0x100FEFA0);
		
		
	}
};

extern InventorySystem inventory;