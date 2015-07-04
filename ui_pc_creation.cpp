#include "stdafx.h"
#include "common.h"
#include "util/config.h"
#include "d20.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"
#include "ui/ui.h"


int PcCreationFeatUiPrereqCheckUsercallWrapper();
int(__cdecl * OrgFeatMultiselectSub_101822A0)();
int HookedFeatMultiselectSub_101822A0(feat_enums feat);
int HookedUsercallFeatMultiselectSub_101822A0();


class PcCreationUiSystem : TempleFix
{
public:
	const char* name() override { return "PC Creation UI Fixes"; }
	void apply() override
	{
		replaceFunction(0x10182E80, PcCreationFeatUiPrereqCheckUsercallWrapper);
		OrgFeatMultiselectSub_101822A0 = (int(__cdecl*)()) replaceFunction(0x101822A0, HookedUsercallFeatMultiselectSub_101822A0);
	}
} pcCreationSys;


struct PcCreationUiFeatsAddresses : AddressTable
{
	int * dword_10C75F30;
	int * featsMultiselectNum_10C75F34;
	feat_enums * featMultiselect_10C75F38;
	int *dword_10C76AF0;
	Widget* widg_10C77CD0;
	int * dword_10C77D50;
	int * dword_10C77D54;
	int *widIdx_10C77D80;
	feat_enums * featsMultiselectList;
	feat_enums * feat_10C79344;
	int * widgId_10C7AE14;
	char* (__cdecl*sub_10182760)(feat_enums featEnums);
	int(__cdecl* j_CopyWidget_101F87A0)(int widIdx, Widget* widg);
	int(__cdecl*sub_101F87B0)(int widIdx, Widget* widg);
	int(__cdecl*sub_101F8E40)(int);
	int(__cdecl*sub_101F9100)(int widId, int);
	int(__cdecl*sub_101F9510)(int, int);

	CharEditorSelectionPacket * charEdSelPkt;
	MesHandle* pcCreationMes;

	PcCreationUiFeatsAddresses()
	{
		rebase(dword_10C75F30, 0x10C75F30);
		rebase(featsMultiselectNum_10C75F34, 0x10C75F34);
		rebase(featMultiselect_10C75F38, 0x10C75F38);
		rebase(dword_10C76AF0, 0x10C76AF0);
		rebase(widg_10C77CD0, 0x10C77CD0);
		rebase(dword_10C77D50, 0x10C77D50);
		rebase(dword_10C77D54, 0x10C77D54);
		rebase(widIdx_10C77D80, 0x10C77D80);
		rebase(featsMultiselectList, 0x10C78920);
		rebase(feat_10C79344, 0x10C79344);
		rebase(widgId_10C7AE14, 0x10C7AE14);

		rebase(sub_10182760, 0x10182760);
		rebase(j_CopyWidget_101F87A0, 0x101F87A0);
		rebase(sub_101F87B0, 0x101F87B0);
		rebase(sub_101F8E40, 0x101F8E40);
		rebase(sub_101F9100, 0x101F9100);
		rebase(sub_101F9510, 0x101F9510);

		rebase(pcCreationMes, 0x11E72EF0);
		rebase(charEdSelPkt, 0x11E72F00);

	}


} addresses;




int __declspec(naked) HookedUsercallFeatMultiselectSub_101822A0()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}
	__asm{
		push eax;
		call HookedFeatMultiselectSub_101822A0;
		add esp, 4;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx}
	__asm retn;
};


int HookedFeatMultiselectSub_101822A0(feat_enums feat)
{
	if ((feat >= FEAT_EXOTIC_WEAPON_PROFICIENCY && feat <= FEAT_WEAPON_SPECIALIZATION) || feat == FEAT_WEAPON_FINESSE_DAGGER)
	{
		__asm mov eax, feat;
		return OrgFeatMultiselectSub_101822A0();
	}
	return 0;
}



int __cdecl PcCreationFeatUiPrereqCheck(feat_enums feat)
{
	int featArrayLen = 0;
	feat_enums featArray[10];
	if (addresses.charEdSelPkt->feat0 != FEAT_NONE)
		featArray[featArrayLen++] = addresses.charEdSelPkt->feat0;
	if (addresses.charEdSelPkt->feat1 != FEAT_NONE)
		featArray[featArrayLen++] = addresses.charEdSelPkt->feat1;
	if (addresses.charEdSelPkt->feat2 != FEAT_NONE)
		featArray[featArrayLen++] = addresses.charEdSelPkt->feat2;
	if (feat == FEAT_IMPROVED_TRIP)
	{
		if (objects.StatLevelGet(*feats.charEditorObjHnd, stat_level_monk) == 5
			&& addresses.charEdSelPkt->classCode == stat_level_monk)
			return 1;
	}
	if (feat <= FEAT_NONE || feat > FEAT_MONK_PERFECT_SELF)
		return feats.FeatPrereqsCheck(*feats.charEditorObjHnd, feat, featArray, featArrayLen, addresses.charEdSelPkt->classCode, addresses.charEdSelPkt->statBeingRaised);

	// the vanilla multiselect range

	return feats.FeatPrereqsCheck(*feats.charEditorObjHnd, feat, featArray, featArrayLen, addresses.charEdSelPkt->classCode, addresses.charEdSelPkt->statBeingRaised);
}


int __declspec(naked) PcCreationFeatUiPrereqCheckUsercallWrapper()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}

	__asm
	{
		push ecx;
		call PcCreationFeatUiPrereqCheck;
		pop edi;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx }
	__asm retn;
}