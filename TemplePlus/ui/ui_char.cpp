#include "stdafx.h"
#include "common.h"
#include "config/config.h"
#include "d20.h"
#include "util/fixes.h"
#include "ui/ui.h"
#include "tig/tig_msg.h"
#include <infrastructure/keyboard.h>
#include <party.h>
#include <gamesystems/objects/objsystem.h>

#define NUM_SPELLBOOK_SLOTS 18 // 18 in vanilla


struct UiCharSpellPacket
{
	WidgetType1 * classSpellbookWnd;
	WidgetType1 * classMemorizeWnd;
	WidgetType1 * spellbookSpellWnds[NUM_SPELLBOOK_SLOTS];
	WidgetType1 * memorizeSpellWnds[NUM_SPELLBOOK_SLOTS];
	WidgetType2 * metamagicButtons[NUM_SPELLBOOK_SLOTS];
	WidgetType3 * spellbookScrollbar;
	WidgetType3 * memorizeScrollbar;
	SpellStoreData spellsKnown[SPELL_ENUM_MAX];
	uint32_t spellsKnownNum;
	SpellStoreData spellsMemorized[SPELL_ENUM_MAX];
	uint32_t spellsMemorizedNum;
};
static_assert(sizeof(UiCharSpellPacket) == 0xC970, "UiCharSpellPacket should have size 51568 "); //  51568

struct UiCharSpellsNavPacket
{
	uint32_t classCode;
	WidgetType2* button;
	int numSpellsForLvl[10]; // starting from level 0 (cantrips), and including bonus from ability modifier and school specialization
};

static_assert(sizeof(UiCharSpellsNavPacket) == 0x30, "UiCharSpellPacket should have size 48"); //  48

struct UiCharAddresses : temple::AddressTable
{
	UiCharSpellsNavPacket *uiCharSpellsNavPackets;
	UiCharSpellPacket * uiCharSpellPackets; // size
	int * uiCharSpellsNavClassTabIdx;
	WidgetType1** uiCharSpellsMainWnd;
	WidgetType1** uiCharSpellsNavClassTabWnd;
	WidgetType1** uiCharSpellsSpellsPerDayWnd;
	WidgetType2** uiCharSpellsPerDayLevelBtns; // array of 6 buttons for levels 0-5
	UiCharAddresses()
	{
		rebase(uiCharSpellsMainWnd, 0x10C81BC0);
		rebase(uiCharSpellsNavClassTabWnd, 0x10C81BC4);
		rebase(uiCharSpellsSpellsPerDayWnd, 0x10C81BC8);
		rebase(uiCharSpellsPerDayLevelBtns, 0x10C81BCC);
		rebase(uiCharSpellsNavPackets, 0x10C81BE4);
		rebase(uiCharSpellPackets, 0x10C81E24);
		rebase(uiCharSpellsNavClassTabIdx, 0x10D18F68);
	}
} addresses;

class CharUiSystem : TempleFix
{
public: 
	const char* name() override { return "CharUi - disabling stat text draw calls";} 

	static BOOL MemorizeSpellMsg(int widId, TigMsg* tigMsg)
	{
		if (tigMsg->type == TigMsgType::WIDGET)
		{
			int dumy = 1;

			if (tigMsg->arg2 == 0)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 1)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 2)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 3)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 4)
			{
				dumy = 1;
			}
		}
		return orgMemorizeSpellMsg(widId, tigMsg);
	};
	static BOOL (*orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);

	static BOOL SpellbookSpellsMsg(int widId, TigMsg* tigMsg)
	{
		if (tigMsg->type == TigMsgType::WIDGET)
		{
			int dumy = 1;
			if (tigMsg->arg2 == 0)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 1)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 2)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 3)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 4)
			{
				dumy = 1;
			}
		}
		else if (tigMsg->type == TigMsgType::MOUSE)
		{
			int dummy = 1;
			if (tigMsg->arg4 & MSF_RMB_RELEASED)
			{
				auto shit = &addresses.uiCharSpellPackets[0];
				dummy = 1;
			}
			else if (tigMsg->arg4 & MSF_UNK_4000)
			{
				dummy = 1;
			}
		}
		return orgSpellbookSpellsMsg(widId, tigMsg);
	};
	static BOOL(*orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);

	static void SpellsShow(objHndl obj)
	{
		orgSpellsShow(obj);
		auto shit = &addresses.uiCharSpellPackets[0];
		int dummy = 1;
	};
	static void(*orgSpellsShow)(objHndl obj);


	static objHndl GetCritterLooted(); // this may be a substitute inventory object when buying from NPCs with shops
	static objHndl GetVendor();

	static int GetInventoryState(); // 1 - looting, 2 - bartering, 4 - using magic on inventory item
	static int InventorySlotMsg(int widId, TigMsg* msg);
	static BOOL (*orgInventorySlotMsg)(int widId, TigMsg* msg);

	void apply() override 
	{
		if (config.usingCo8) // disabling stat text draw calls
		{
			writeHex(0x1011DD4D, "90 90 90 90 90");
		}
		int charSheetAttackCodeForAttackBonusDisplay = 1 + ATTACK_CODE_OFFHAND;
		write(0x101C45F3 + 7, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
		write(0x101C8C7B + 4, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));

		charSheetAttackCodeForAttackBonusDisplay = 1 + ATTACK_CODE_OFFHAND + 1; //the offhand
		write(0x101C491B + 7, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
		write(0x101C8D74 + 4, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
		
		//orgSpellbookSpellsMsg = replaceFunction(0x101B8F10, SpellbookSpellsMsg);
		//orgMemorizeSpellMsg= replaceFunction(   0x101B9360, MemorizeSpellMsg);
		orgSpellsShow = replaceFunction(0x101B5D80, SpellsShow);

		static bool (__cdecl* orgUiCharLootingLootWndMsg)(int , TigMsg* ) = replaceFunction<bool(__cdecl)(int , TigMsg* ) >(0x101406D0, [](int widId, TigMsg* msg)
		{
			if (msg->type == TigMsgType::MOUSE)
			{
				int dummy = 1;
			}

			if (msg->type == TigMsgType::WIDGET)
			{
				int dummy = 1;
			}
			return orgUiCharLootingLootWndMsg(widId, msg);
		});

		orgInventorySlotMsg = replaceFunction<int(__cdecl)(int, TigMsg*) >(0x10157DC0, InventorySlotMsg);
		


	}
} charUiSys;

objHndl CharUiSystem::GetCritterLooted()
{
	return temple::GetRef<objHndl>(0x10BE6EC0);
}

objHndl CharUiSystem::GetVendor()
{
	return temple::GetRef<objHndl>(0x10BE6EC8);
}

int CharUiSystem::GetInventoryState()
{
	return temple::GetRef<int>(0x10BE994C);
}

int CharUiSystem::InventorySlotMsg(int widId, TigMsg* msg)
{
	if (ui.CharLootingIsActive())
	{
		if (msg->type == TigMsgType::MOUSE)
		{
			if (msg->arg4 & MSF_LMB_CLICK)
			{
				if ( infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) )
				{
					objHndl critterLooted = GetCritterLooted(); // may be substitute inventory object (i.e. a container)

					auto uiCharInvItemGetFromWidId = temple::GetRef<objHndl(__cdecl)(int)>(0x10157060);
					auto item = uiCharInvItemGetFromWidId(widId);
					objHndl vendor = GetVendor();

					logger->debug("Quickselling item {} to {} ({})", description.getDisplayName(item), description.getDisplayName(vendor),description.getDisplayName(critterLooted));
					
					objHndl appraiser = party.PartyMemberWithHighestSkill(SkillEnum::skill_appraise);
					
					
					int appraisedWorth = inventory.GetAppraisedWorth(item, appraiser, vendor, SkillEnum::skill_appraise);
					int plat =0, gold=0, silver=0, copper=0;
					if (appraisedWorth && GetInventoryState() == 2){
						
						int qty = max(1, inventory.GetQuantity(item));
						appraisedWorth *= qty;
						inventory.MoneyToCoins(appraisedWorth, &plat, &gold, &silver, &copper);
					}
					
					ItemErrorCode itemTransferError = inventory.TransferWithFlags(item, critterLooted, -1, 1+2+4+8+16, 0i64);
					if (itemTransferError == IEC_OK){
						objSystem->GetObject(item)->SetItemFlag(OIF_IDENTIFIED, 1);
						party.MoneyAdj(plat, gold, silver, copper);
					} 
					else if (itemTransferError == IEC_No_Room_For_Item){
						int itemIdx = 100;
						for (itemIdx = 100; itemIdx< 255; itemIdx++){
							if (!inventory.GetItemAtInvIdx(critterLooted, itemIdx))
								break;
						}
						if (itemIdx < 255)
							itemTransferError = inventory.TransferWithFlags(item, critterLooted, itemIdx, 1 + 2 + 4 + 8 , 0i64);
					}
					return 1;
				}
				
			}
		}
	}


	if (msg->type == TigMsgType::WIDGET)
	{
		int dummy = 1;
	}
	return orgInventorySlotMsg(widId, msg);
}

BOOL(*CharUiSystem::orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);
BOOL(*CharUiSystem::orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);
void(*CharUiSystem::orgSpellsShow)(objHndl obj);
BOOL(*CharUiSystem::orgInventorySlotMsg)(int widId, TigMsg* msg);