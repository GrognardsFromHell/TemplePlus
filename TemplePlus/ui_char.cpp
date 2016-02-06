#include "stdafx.h"
#include "common.h"
#include "config/config.h"
#include "d20.h"
#include "util/fixes.h"
#include "ui/ui.h"
#include "tig/tig_msg.h"

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
			if (tigMsg->arg4 &0x40)
			{
				auto shit = &addresses.uiCharSpellPackets[0];
				dummy = 1;
			}
			else if (tigMsg->arg4 & 0x4000)
			{
				dummy = 1;
			}
		}
		return orgSpellbookSpellsMsg(widId, tigMsg);
	};
	static BOOL(*orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);


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

	}
} charUiSys;

BOOL(*CharUiSystem::orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);
BOOL(*CharUiSystem::orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);