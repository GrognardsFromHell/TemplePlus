#pragma once
#include "common.h"
#include "dispatcher.h"
#include <map>


enum EquipSlot : uint32_t;

#define CRITTER_INVENTORY_SLOT_COUNT 24 // amount of inventory slots visible
#define INVENTORY_WORN_IDX_START 200 // the first inventory index for worn items
#define INVENTORY_WORN_IDX_END 216 // the last index for worn items (non-inclusive, actually)
#define INVENTORY_WORN_IDX_COUNT 16
#define INVENTORY_BAG_IDX_START 217
#define INVENTORY_BAG_IDX_END 221

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


enum ItemInsertFlags : uint8_t {
	IIF_None = 0,
	IIF_Allow_Swap = 0x1,
	IIF_Use_Wield_Slots = 0x2, // will let the item transfer try to insert in the wielded item slots (note: will not replace if there is already an item equipped!)
	IIF_4 = 0x4,
	IIF_Use_Max_Idx_200 = 0x8, // will use up to inventory index 200 (invisible slots)
	IIF_10 = 0x10,
	IIF_Use_Bags = 0x20, // use inventory indices of bags (not really supported in ToEE)
	IIF_40 = 0x40,
	IIF_80 = 0x80
};

struct InventorySystem : temple::AddressTable
{
	
	static bool IsInvIdxWorn(int invIdx); // does the inventory index refer to a designated "worn item" slot?

	objHndl(__cdecl *GetSubstituteInventory)  (objHndl);
	objHndl(__cdecl *GetItemAtInvIdx)(objHndl, uint32_t nIdx); // returns the item at obj_f_critter_inventory subIdx nIdx  (or obj_f_container_inventory for containers); Note the difference to ItemWornAt! (this is a more low level function)
	objHndl FindMatchingStackableItem(objHndl objHndReceiver, objHndl objHndItem);
	void WieldBest(objHndl handle, int invSlot, objHndl target);


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

	
	int IsNormalCrossbow(objHndl weapon);
	int IsThrowingWeapon(objHndl weapon);
	bool UsesWandAnim(const objHndl item);
	static bool IsTripWeapon(objHndl weapon);
	ArmorType GetArmorType(int armorFlags);
	
	BOOL GetQuantityField(const objHndl item, obj_f * qtyField); // gets the relevant quantity field for the item ; returns 0 if irrelevant
	int GetQuantity(objHndl item); // note: returns 0 for items with no quantity fields!
	void QuantitySet(const objHndl& item, int qtyNew);

	int ItemWeight(objHndl item); // returns weight of item (or item stack if applicable)

	
	bool IsRangedWeapon(objHndl weapon);
	bool IsVisibleInventoryFull(objHndl obj);
	int GetInventory(objHndl obj, objHndl** inventoryArray);
	std::vector<objHndl> GetInventory(objHndl obj);
	int GetInventoryLocation(objHndl item);
	ItemFlag GetItemFlags(objHndl item);
	int IsItemNonTransferable(objHndl item, objHndl receiver);
	
	
	objHndl GetParent(objHndl item);
	int SetItemParent(objHndl item, objHndl parent, int flags);
	int SetItemParent(objHndl item, objHndl receiver, ItemInsertFlags flags);
	ItemErrorCode TransferWithFlags(objHndl item, objHndl receiver, int invenInt, char flags, objHndl bag); // see ItemInsertFlags
	void ItemPlaceInIdx(objHndl item, int idx);
	int ItemInsertGetLocation(objHndl item, objHndl receiver, int* itemInsertLocation, objHndl bag, char flags);
	void InsertAtLocation(objHndl item, objHndl receiver, int itemInsertLocation);
	int Wield(objHndl handle, objHndl item, EquipSlot slot = EquipSlot::Invalid);

	void ForceRemove(objHndl item, objHndl parent);
	int ItemDrop(objHndl item);
	int ItemDrop(objHndl critter, EquipSlot slot);
	int ItemUnwield(objHndl item);
	int ItemUnwieldByIdx(objHndl obj, int i);
	int ItemUnwield(objHndl critter, EquipSlot slot);

	bool IsProficientWithArmor(objHndl obj, objHndl armor) const;
	void GetItemMesLine(MesLine* line);
	const char* GetItemErrorString(ItemErrorCode itemErrorCode);
	
	static bool IsMagicItem(objHndl itemHandle);
	static bool IsIdentified(objHndl itemHandle);
	void ItemSpellChargeConsume(const objHndl& item, int chargesUsedUp = 1);
	static bool IsBuckler(objHndl shield);
	void(__cdecl*_ForceRemove)(objHndl, objHndl);
	void ItemRemove(objHndl item); // pretty much same as ForceRemove, but also send a d20 signal for inventory update, and checks for parent first
	BOOL ItemGetAdvanced(objHndl item, objHndl parent, int slotIdx, int flags);

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
	static obj_f GetInventoryNumField(objHndl objHnd);
	/*
		Identifies all items held or contained within the given parent.
	*/
	void (__cdecl *IdentifyAll)(objHndl parent);

	/*
		Tries to wield the best items, unclear what the optional item argument does.
	*/
	void (__cdecl *WieldBestAll)(objHndl critter, objHndl tgt);

	/*
		Clears the inventory of the given object. Keeps items that have the PERSISTENT flag set if
		the second argument is TRUE.
	*/
	void (__cdecl *Clear)(objHndl parent, BOOL keepPersistent);


	bool ItemCanBePickpocketed(objHndl item); // checks if the item is lightweight, unequipped, and not marked OIF_NO_PICKPOCKET
	
	// When equipped, which bone of the parent obj does this item attach to?
	const std::string &GetAttachBone(objHndl item);

	// spawn the items for this object according to invensource.mes
	int (__cdecl*SpawnInvenSourceItems)(objHndl obj);

	int GetSoundIdForItemEvent(objHndl item, objHndl wielder, objHndl tgt, int eventType);

	InventorySystem()
	{
		rebase(GetSubstituteInventory, 0x1007F5B0);
		rebase(GetItemAtInvIdx, 0x100651B0);
		rebase(_ItemWornAt,      0x10065010);
		rebase(_GetWieldType,    0x10066580);
		//rebase(FindMatchingStackableItem, 0x10067DF0);

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

	int InvIdxForSlot(EquipSlot slot); // converts EquipSlot to inventory index
	int InvIdxForSlot(int slot); // converts EquipSlot to inventory index

	int FindEmptyInvIdx(objHndl item, objHndl parent, int idxMin, int idxMax); // finds empty slot between idxMin and idxMax (not including idxMax, but including idxMin)
	objHndl BagFindLast(objHndl parent); // gets the highest index bag
	int BagFindInvenIdx(objHndl parent, objHndl receiverBag); // finds the index of receiverBag inside parent's inventory
	int BagGetContentStartIdx(objHndl receiver, objHndl receiverBag);
	int BagGetContentMaxIdx(objHndl receiver, objHndl receiverBag);
};

extern InventorySystem inventory;
