#pragma once
#include "common.h"
#include "dispatcher.h"
#include <map>


enum EquipSlot : uint32_t;

enum ItemErrorCode: uint32_t
{
	IEC_OK = 0,
	IEC_Cannot_Transfer = 1,
	IEC_Item_Too_Heavy = 2,
	IEC_No_Room_For_Item = 3,
	IEC_Cannot_Use_While_Polymorphed = 4,
	IEC_Cannot_Pickup_Magical_Items = 5,
	IEC_Cannot_Pickup_Techno_Items = 6,
	IEC_Item_Cannot_Be_Dropped = 7,
	IEC_NPC_Will_Not_Drop =8,
	IEC_Wrong_Type_For_Slot =9,
	IEC_No_Free_Hand =10,
	IEC_Crippled_Arm_Prevents_Wielding =11,
	IEC_Item_Too_Large =12,
	IEC_Has_No_Art =13,
	IEC_Opposite_Gender =14,
	IEC_Cannot_Wield_Magical =15,
	IEC_Cannot_Wield_Techno =16,
	IEC_Wield_Slot_Occupied =17,
	IEC_Prohibited_Due_To_Class =18
};


struct InventorySystem : temple::AddressTable
{
	
	objHndl(__cdecl *GetSubstituteInventory)  (objHndl);
	objHndl(__cdecl *GetItemAtInvIdx)(objHndl, uint32_t nIdx); // returns the item at obj_f_critter_inventory subIdx nIdx  (or obj_f_container_inventory for containers); Note the difference to ItemWornAt! (this is a more low level function)
	objHndl(__cdecl *FindMatchingStackableItem)(objHndl objHndReceiver, objHndl objHndItem); // TODO: rewrite so it doesn't stack items with different descriptions and/or caster levels, so potions/scrolls of different caster levels don't stack
	
	
	void (__cdecl *sub_100FF500)(Dispatcher *dispatcher, objHndl objHndItem, uint32_t itemInvLocation);
	uint32_t(__cdecl *IsItemEffectingConditions)(objHndl objHndItem, uint32_t itemInvLocation);
	
	/*
	 finds the argument argOffset for the specified condition in obj_f_item_pad_wielder_condition_array
	*/
	static int GetItemWieldCondArg(objHndl item, uint32_t condId, int argOffset); 
	/*
		removes a condition along with its accompanying args in obj_f_item_pad_wielder_condition_array and obj_f_item_pad_wielder_argument_array
	*/
	static void RemoveWielderCond(objHndl item, uint32_t condId);

	objHndl ItemWornAt(objHndl, EquipSlot nItemSlot) const;
	objHndl ItemWornAt(objHndl, int nItemSlot) const;
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
	bool UsesWandAnim(const objHndl item);
	static bool IsTripWeapon(objHndl weapon);
	ArmorType GetArmorType(int armorFlags);
	
	BOOL GetQuantityField(const objHndl item, obj_f * qtyField); // gets the relevant quantity field for the item ; returns 0 if irrelevant
	int GetQuantity(objHndl item); // note: returns 0 for items with no quantity fields!
	void QuantitySet(const objHndl& item, int qtyNew);

	objHndl GetParent(objHndl item);
	bool IsRangedWeapon(objHndl weapon);
	int GetInventory(objHndl obj, objHndl** inventoryArray);
	int GetInventoryLocation(objHndl item);
	ItemFlag GetItemFlags(objHndl item);
	bool IsItemNonTransferable(objHndl item, objHndl receiver);
	int ItemInsertGetLocation(objHndl item, objHndl receiver, int* itemInsertLocation, objHndl bag, char flags);
	void InsertAtLocation(objHndl item, objHndl receiver, int itemInsertLocation);
	int ItemUnwield(objHndl item);
	int ItemUnwieldByIdx(objHndl obj, int i);
	
	ItemErrorCode TransferWithFlags(objHndl item, objHndl receiver, int invenInt, char flags, objHndl bag);
	void ItemPlaceInIdx(objHndl item, int idx);
	int ItemDrop(objHndl item);
	int ItemGet(objHndl item, objHndl receiver, int flags);
	void ForceRemove(objHndl item, objHndl parent);
	bool IsProficientWithArmor(objHndl obj, objHndl armor) const;
	void GetItemMesLine(MesLine* line);
	const char* GetItemErrorString(ItemErrorCode itemErrorCode);
	
	static bool IsMagicItem(objHndl itemHandle);
	static bool IsIdentified(objHndl itemHandle);
	void ItemSpellChargeConsume(const objHndl& item, int chargesUsedUp = 1);
	static bool IsBuckler(objHndl shield);
	void(__cdecl*_ForceRemove)(objHndl, objHndl);
	void ItemRemove(objHndl item); // pretty much same as ForceRemove, but also send a d20 signal for inventory update, and checks for parent first
	int ItemGetAdvanced(objHndl item, objHndl parent, int slotIdx, int flags);

	/*
		gets the item's sell price (in Copper Pieces) when dealing with a vendor
	*/
	int GetAppraisedWorth(objHndl item, objHndl appraiser, objHndl vendor, SkillEnum skillEnum);
	void MoneyToCoins(int appraisedWorth, int* plat, int* gold, int* silver, int* copper);
	
	/*
		0 - light weapon; 1 - can wield one handed; 2 - must wield two handed; 3 (???)
		if regardEnlargement is true:
		   assumes that the weapon is enlarged along with the character 
		   (so it will actually use the base critter size to determine wield type)
	*/
	int GetWieldType(objHndl wielder, objHndl item, bool regardEnlargement = false) const;
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


	// When equipped, which bone of the parent obj does this item attach to?
	const std::string &GetAttachBone(objHndl item);

	// spawn the items for this object according to invensource.mes
	int (__cdecl*SpawnInvenSourceItems)(objHndl obj);

	InventorySystem()
	{
		rebase(GetSubstituteInventory, 0x1007F5B0);
		rebase(GetItemAtInvIdx, 0x100651B0);
		rebase(_ItemWornAt,      0x10065010);
		rebase(_GetWieldType,    0x10066580);
		rebase(FindMatchingStackableItem, 0x10067DF0);

		rebase(sub_100FF500, 0x100FF500);
		rebase(IsItemEffectingConditions, 0x100FEFA0);

		rebase(_FindItemByName, 0x100643F0);
		rebase(_FindItemByProto, 0x100644B0);
		rebase(_SetItemParent, 0x1006B6C0);

		rebase(IdentifyAll, 0x10064C70);
		rebase(WieldBestAll, 0x1006D100);
		
		rebase(_ForceRemove, 0x10069AE0);
		rebase(Clear,		 0x10069E00);
		rebase(_ItemRemove,  0x10069F60);
		
		rebase(_ItemDrop,	 0x1006AA60);

		rebase(SpawnInvenSourceItems, 0x1006D3D0);
	}

private:
	int(__cdecl *_SetItemParent)(objHndl item, objHndl parent, int flags);
	objHndl(__cdecl *_FindItemByName)(objHndl container, int nameId);
	objHndl(__cdecl *_FindItemByProto)(objHndl container, objHndl proto, bool skipWorn);
	int(__cdecl*_ItemRemove)(objHndl item);
	int(__cdecl*_ItemDrop)(objHndl item);
	objHndl(__cdecl *_ItemWornAt)(objHndl, EquipSlot nItemSlot);
	int(__cdecl *_GetWieldType)(objHndl wielder, objHndl item);
};

extern InventorySystem inventory;