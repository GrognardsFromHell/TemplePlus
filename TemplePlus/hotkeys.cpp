#include "stdafx.h"
#include "radialmenu.h"
#include "tig/tig_mes.h"
#include "hotkeys.h"
#include "tio\tio.h"
#include "gamesystems/legacy.h"
#include "util/fixes.h"
#include "obj.h"
#include "critter.h"
#include "action_sequence.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "infrastructure/keyboard.h"

HotkeySystem hotkeys;

struct HotkeyAddresses : temple::AddressTable
{
	int * assignableKeys; // size 39 int array, holding DirectX Input Key numbers
	RadialMenuEntry * hotkeyTable; // 39 entries; index corresponds to key given in assignableKeys
	MesHandle * hotkeyMes;
	char * hotkeyPopupBodyText; // confirmation dialog text; 512 chars
	char * hotkeyTexts; // 128 chars each, 39 entries, describing the hotkey assignment

	RadialMenuEntry** radMenuEntryToBind;
	int * keyIdxToBind;

	int(__cdecl* HotkeyTableSearch)(RadialMenuEntry* radMenuEntry);
	const char * (__cdecl* GetDikLetter)(int dik);
	const char * (__cdecl* GetHotkeyLetter)(RadialMenuEntry radMenuEntry); // fetches the letter assigned to this radial menu entry from hotkey.mes
	int(__cdecl* IsReversedHotkey)(int dik);
	void(__cdecl* HotkeyAssignCallback)(char * cancelFlag); // assigns a hotkey (in the hotkeyTable) according to some global vars


	HotkeyAddresses()
	{
		rebase(assignableKeys, 0x102E8B78);
		rebase(hotkeyTable, 0x10BD0248);
		rebase(hotkeyMes, 0x10BD0D40);
		rebase(hotkeyPopupBodyText, 0x10BD0D48);
		rebase(hotkeyTexts, 0x10BD0F48);

		rebase(HotkeyTableSearch, 0x100F3DF0);
		rebase(GetDikLetter, 0x100F3E30);
		rebase(GetHotkeyLetter, 0x100F3E80);
		rebase(IsReversedHotkey, 0x100F3ED0);
		rebase(HotkeyAssignCallback, 0x100F4030);

		rebase(keyIdxToBind, 0x115B1ED0);
		rebase(radMenuEntryToBind, 0x115B1ED4);
	}
} hkAddresses;


class HotkeyReplacements : TempleFix
{
public: 
	static BOOL HotkeyCompare(RadialMenuEntry& first, RadialMenuEntry & second);
	static BOOL HotkeyActivate(objHndl obj);
	void apply() override 
	{
		replaceFunction(0x100F3B80, HotkeyInit);
		replaceFunction(0x100F3BC0, HotkeyExit);
		replaceFunction(0x100F3BD0, SaveHotkeys);
		replaceFunction(0x100F3C80, LoadHotkeys);
		replaceFunction(0x100F4030, HotkeyAssignCallback);
		replaceFunction(0x100F0380, HotkeyCompare);
		replaceFunction(0x100F0B80, HotkeyActivate);

		replaceFunction<int(InGameKeyEvent &, int , int )>(0x10143450, [](InGameKeyEvent &msg, int modifier, int keyEvt){
			return uiSystems->GetManager().CharacterSelect(msg, modifier, keyEvt)? TRUE: FALSE;
		});
	}
} hotkeyReplacements;

BOOL HotkeyReplacements::HotkeyCompare(RadialMenuEntry& first, RadialMenuEntry& second)
{
	auto actionType = first.d20ActionType;

	if (actionType != second.d20ActionType) {
		return FALSE;
	}

	if (actionType == D20A_ACTIVATE_DEVICE_FREE
		|| actionType == D20A_ACTIVATE_DEVICE_STANDARD
		|| actionType == D20A_ACTIVATE_DEVICE_SPELL)
		return first.textHash == second.textHash;

	if (actionType == D20A_USE_ITEM){
		if (first.d20SpellData.spellEnumOrg != second.d20SpellData.spellEnumOrg)
			return FALSE;

		if (first.d20SpellData.metaMagicData != second.d20SpellData.metaMagicData)
			return FALSE;
		//return first.textHash == second.textHash;
		return first.d20SpellData.spellSlotLevel == second.d20SpellData.spellSlotLevel;

	}

	if (first.d20ActionData1 != second.d20ActionData1)
		return FALSE;

	if (first.d20SpellData.spellEnumOrg != second.d20SpellData.spellEnumOrg)
		return FALSE;

	if (first.d20SpellData.metaMagicData != second.d20SpellData.metaMagicData)
		return FALSE;

	if (first.d20ActionType == D20A_NONE &&first.textHash != second.textHash)
		return FALSE;

	if (first.dispKey != second.dispKey)
		return FALSE;

	return TRUE;
}

BOOL HotkeyReplacements::HotkeyActivate(objHndl obj)
{
	auto radMenuForHK = temple::GetRef<RadialMenu*>(0x115B2050);

	if (!radMenuForHK)
		return FALSE;

	auto radMenuNodeCount = temple::GetRef<int>(0x118676C0);
	if (radMenuNodeCount > radMenuForHK->nodeCount) {
		return FALSE;
	}

	auto& activeRadialMenu = temple::GetRef<const RadialMenu*>(0x115B2048);
	activeRadialMenu = radialMenus.GetForObj(obj);

	auto& radEntry = radMenuForHK->nodes[radMenuNodeCount -1].entry;

	auto& activeRadialMenuNode = temple::GetRef<int>(0x115B204C);
	activeRadialMenuNode = radMenuNodeCount - 1;

	if (radEntry.d20ActionType == D20A_CAST_SPELL)
		actSeqSys.ActSeqSpellReset();
	else if (radEntry.d20ActionType == D20A_USE_ITEM && radEntry.d20SpellData.spellEnumOrg != 0){
		actSeqSys.ActSeqSpellReset();
	}

	auto nodeType = radEntry.type;
	auto result = FALSE;
	if (nodeType == RadialMenuEntryType::Action)
	{
		if (radEntry.callback){
			result = radEntry.callback(obj, &radEntry);
		}
	}
	else if (nodeType == RadialMenuEntryType::Slider)// will toggle between min/max values
	{
		temple::GetRef<void(__cdecl)(objHndl, RadialMenuEntry&)>(0x100F05C0)(obj, radEntry); // toggle value to min/max
		temple::GetRef<void(__cdecl)(objHndl, RadialMenuEntry&)>(0x100F05F0)(obj, radEntry); // activate / deactivate float line
		result = FALSE;
	}
	else if (nodeType == RadialMenuEntryType::Toggle)
	{
		if (radEntry.callback) {
			result = radEntry.callback(obj, &radEntry);
		}
		temple::GetRef<void(__cdecl)(objHndl, RadialMenuEntry&)>(0x100F05F0)(obj, radEntry); // activate / deactivate float line
	}


	activeRadialMenu = nullptr;
	activeRadialMenuNode = -1;
	return result;

}

int HotkeySystem::SaveHotkeys(TioFile* file)
{
	for (int i = 0; i < NUM_ASSIGNABLE_HOTKEYS; i++)
	{
		RadialMenuEntry radMenu = hkAddresses.hotkeyTable[i];
		if (radMenu.d20ActionType != -2)
		{
			char hkText[128] = { 0, };
			memcpy(hkText, &hkAddresses.hotkeyTexts[128 * i], 128);
			if (!tio_fwrite(&i, sizeof(i), 1, file)
				|| tio_fwrite(&radMenu, sizeof(RadialMenuEntry), 1, file) != 1
				|| tio_fwrite(hkText, 1, 128, file) != 128)
				return 0;
		}
	}
	int terminator = -1;
	return tio_fwrite(&terminator, sizeof(terminator), 1, file) == 1;
}

int HotkeySystem::SaveHotkeys(FILE* file)
{
	for (int i = 0; i < NUM_ASSIGNABLE_HOTKEYS; i++)
	{
		RadialMenuEntry radMenu = hkAddresses.hotkeyTable[i];
		if (radMenu.d20ActionType != -2)
		{
			char hkText[128] = { 0, };
			memcpy(hkText, &hkAddresses.hotkeyTexts[128 * i], 128);
			if (!fwrite(&i, sizeof(i), 1, file)
				|| fwrite(&radMenu, sizeof(RadialMenuEntry), 1, file) != 1
				|| fwrite(&hkAddresses.hotkeyTexts[128 * i], 1, 128, file) != 128)
				return 0;
		}
	}
	int terminator = -1;
	return fwrite(&terminator, sizeof(terminator), 1, file) == 1;
}

int HotkeySystem::LoadHotkeys(GameSystemSaveFile* gsFile)
{
	return LoadHotkeys(gsFile->file);
}

int HotkeySystem::LoadHotkeys(TioFile* file)
{
	int buffer;
	while (tio_fread(&buffer, sizeof(buffer), 1, file) == 1)
	{
		if (buffer == -1) { //terminator char
			return 1;
		}

		RadialMenuEntry radMenu;
		if (tio_fread(&radMenu, sizeof(RadialMenuEntry), 1, file) != 1)
			break;
		hkAddresses.hotkeyTable[buffer] = radMenu;

		char hkText[128];
		if (tio_fread(hkText, 1, 128, file) != 128)
			break;
		memcpy(&hkAddresses.hotkeyTexts[128 * buffer], hkText, 128);
	}
	return 0;
}

void HotkeySystem::HotkeyInit()
{
	for (int i = 0; i < NUM_ASSIGNABLE_HOTKEYS; i++)
	{
		hkAddresses.hotkeyTable[i].d20ActionType = D20A_UNASSIGNED;
	}
	mesFuncs.Open("mes\\hotkeys.mes", hkAddresses.hotkeyMes);	
}

void HotkeySystem::HotkeyExit()
{
	mesFuncs.Close(*hkAddresses.hotkeyMes);
	//auto file = tio_fopen("hotkeys.sco", "wb");

	//tio_fclose(file);
}

void HotkeySystem::HotkeyAssignCallback(int cancelFlag)
{
	if (!cancelFlag)
	{
		auto hotkeyTable = hkAddresses.hotkeyTable;
		auto hkIdx = *hkAddresses.keyIdxToBind;
		auto radMenuEntryToBind = *hkAddresses.radMenuEntryToBind;

		int radMenuIdx = hkAddresses.HotkeyTableSearch(radMenuEntryToBind);
		if (radMenuIdx != -1)
			hotkeyTable[radMenuIdx].d20ActionType = D20A_UNASSIGNED;

		hotkeyTable[hkIdx] = *radMenuEntryToBind;
		auto hkText = &hkAddresses.hotkeyTexts[128 * hkIdx];
		strncpy(hkText, radMenuEntryToBind->text, 127);
		hotkeyTable[hkIdx].text = hkText;

		auto file = tio_fopen("hotkeys.sco", "wb");
		SaveHotkeys(file);
		tio_fclose(file);

	}
}

BOOL HotkeySystem::IsReservedHotkey(uint32_t dinputKey)
{
	auto isReservedKey = temple::GetRef<BOOL(__cdecl)(uint32_t)>(0x100F3ED0);
	return isReservedKey(dinputKey);
}

int HotkeySystem::HotkeyReservedPopup(uint32_t dinputKey)
{
	auto hotkeyReservedPopup = temple::GetRef<BOOL(__cdecl)(uint32_t)>(0x100F3F20);
	return hotkeyReservedPopup(dinputKey);
}

BOOL HotkeySystem::IsNormalNonreservedHotkey(uint32_t dinputKey)
{
	auto isNormalNonreservedHotkey = temple::GetRef<BOOL(__cdecl)(uint32_t)>(0x100F3D20);
	return isNormalNonreservedHotkey(dinputKey);
}

bool HotkeySystem::IsKeyPressed(int virtualKey)
{
	return infrastructure::gKeyboard.IsKeyPressed(virtualKey);
}

void __cdecl HotkeyInit()
{
	hotkeys.HotkeyInit();
}

void __cdecl HotkeyExit()
{
	hotkeys.HotkeyExit();
}

int __cdecl SaveHotkeys(TioFile* file)
{
	return hotkeys.SaveHotkeys(file);
}


int __cdecl LoadHotkeys(GameSystemSaveFile* file)
{
	return hotkeys.LoadHotkeys(file);
}

void __cdecl HotkeyAssignCallback(int cancelFlag)
{
	hotkeys.HotkeyAssignCallback(cancelFlag);
}