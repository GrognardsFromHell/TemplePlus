#include "stdafx.h"
#include "radialmenu.h"
#include "tig/tig_mes.h"
#include "hotkeys.h"
#include "tio\tio.h"
#include "gamesystems/legacy.h"
#include "util/fixes.h"

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
} addresses;


class HotkeyReplacements : TempleFix
{
public: 
	const char* name() override { return "Hotkey Function Replacements";} 
	void apply() override 
	{
		replaceFunction(0x100F3B80, HotkeyInit);
		replaceFunction(0x100F3BC0, HotkeyExit);
		replaceFunction(0x100F3BD0, SaveHotkeys);
		replaceFunction(0x100F3C80, LoadHotkeys);
		replaceFunction(0x100F4030, HotkeyAssignCallback);
	}
} hotkeyReplacements;


int HotkeySystem::SaveHotkeys(TioFile* file)
{
	for (int i = 0; i < NUM_ASSIGNABLE_HOTKEYS; i++)
	{
		if (addresses.hotkeyTable[i].d20ActionType != -2)
		{
			if (!tio_fwrite(&i, sizeof(i), 1, file)
				|| tio_fwrite(&addresses.hotkeyTable[i], sizeof(RadialMenuEntry), 1, file) != 1
				|| tio_fwrite(&addresses.hotkeyTexts[128 * i], 1, 128, file) != 128)
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
		if (addresses.hotkeyTable[i].d20ActionType != -2)
		{
			if (!fwrite(&i, sizeof(i), 1, file)
				|| fwrite(&addresses.hotkeyTable[i], sizeof(RadialMenuEntry), 1, file) != 1
				|| fwrite(&addresses.hotkeyTexts[128 * i], 1, 128, file) != 128)
				return 0;
		}
	}
	int terminator = -1;
	return fwrite(&terminator, sizeof(terminator), 1, file) == 1;
}

int HotkeySystem::LoadHotkeys(GameSystemSaveFile* gsFile)
{
	int buffer;
	while (tio_fread(&buffer, sizeof(buffer), 1, gsFile->file) == 1)
	{
		if (buffer == -1) return 1; //terminator char
		if (tio_fread(&addresses.hotkeyTable[buffer], sizeof(RadialMenuEntry), 1, gsFile->file) != 1 )
			break;
		if (tio_fread(&addresses.hotkeyTexts[buffer], 1, 128, gsFile->file) != 128 )
			break;
	}
	return 0;
}

void HotkeySystem::HotkeyInit()
{
	for (int i = 0; i < NUM_ASSIGNABLE_HOTKEYS; i++)
	{
		addresses.hotkeyTable[i].d20ActionType = D20A_UNASSIGNED;
	}
	mesFuncs.Open("mes\\hotkeys.mes", addresses.hotkeyMes);
	auto file = fopen("hotkeys.sco", "rb");
	int buffer;
	if (file)
	{
		while (fread(&buffer, sizeof(buffer), 1, file) == 1)
		{
			if (buffer == -1) break; //terminator char
			if (fread(&addresses.hotkeyTable[buffer], sizeof(RadialMenuEntry), 1, file) != 1)
				break;
			if (fread(&addresses.hotkeyTexts[buffer], 1, 128, file) != 128)
				break;
		}
		fclose(file);
	}
	
}

void HotkeySystem::HotkeyExit()
{
	mesFuncs.Close(*addresses.hotkeyMes);
	//auto file = tio_fopen("hotkeys.sco", "wb");

	//tio_fclose(file);
}

void HotkeySystem::HotkeyAssignCallback(int cancelFlag)
{
	if (!cancelFlag)
	{
		int radMenuIdx = addresses.HotkeyTableSearch(*addresses.radMenuEntryToBind);
		if (radMenuIdx != -1)
			addresses.hotkeyTable[radMenuIdx].d20ActionType = D20A_UNASSIGNED;
		memcpy(&addresses.hotkeyTable[*addresses.keyIdxToBind], *addresses.radMenuEntryToBind, sizeof(RadialMenuEntry));
		strncpy(&addresses.hotkeyTexts[128 * (*addresses.keyIdxToBind)], (*addresses.radMenuEntryToBind)->text, 127);
		addresses.hotkeyTable[*addresses.keyIdxToBind].text = &addresses.hotkeyTexts[128 * (*addresses.keyIdxToBind)];

		auto file = fopen("hotkeys.sco", "wb");
		SaveHotkeys(file);
		fclose(file);

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