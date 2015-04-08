#include "common.h"
#include "dispatcher.h"

struct InventorySystem : AddressTable
{
	void (__cdecl *sub_100FF500)(Dispatcher *dispatcher, objHndl objHndItem, uint32_t itemInvLocation);
	uint32_t(__cdecl *IsItemEffectingConditions)(objHndl objHndItem, uint32_t itemInvLocation);
	InventorySystem()
	{
		rebase(sub_100FF500, 0x100FF500);
		rebase(IsItemEffectingConditions, 0x100FEFA0);
	}
};

extern InventorySystem inventory;