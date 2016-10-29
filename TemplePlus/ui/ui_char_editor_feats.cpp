#include "stdafx.h"
#include "common.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"
#include "ui/ui.h"
#include "util/fixes.h"
#include "EASTL/hash_map.h"


int FeatUiPrereqCheckUsercallWrapper();

struct UiCharEditorFeatsAddresses : temple::AddressTable
{
	int * dword_10C75F30;
	int * featsMultiselectNum_10C75F34;
	feat_enums * featMultiselect_10C75F38;
	int *dword_10C76AF0;
	WidgetType3* featsScrollbar;
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

	UiCharEditorFeatsAddresses()
	{
		rebase(dword_10C75F30, 0x10C75F30);
		rebase(featsMultiselectNum_10C75F34, 0x10C75F34);
		rebase(featMultiselect_10C75F38, 0x10C75F38);
		rebase(dword_10C76AF0, 0x10C76AF0);
		rebase(featsScrollbar, 0x10C77CD0);
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


int(__cdecl * OrgFeatMultiselectSub_101A8080)();
int __declspec(naked) HookedUsercallFeatMultiselectSub_101A8080()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}
	__asm{
		push eax;
		call HookedFeatMultiselectSub_101A8080;
		add esp, 4;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx} 
	__asm retn;
};


int HookedFeatMultiselectSub_101A8080(feat_enums feat) // redundant now
{
	if ( (feat >= FEAT_EXOTIC_WEAPON_PROFICIENCY && feat <= FEAT_WEAPON_SPECIALIZATION) || feat == FEAT_WEAPON_FINESSE_DAGGER )
	{
		__asm mov eax, feat;
		return OrgFeatMultiselectSub_101A8080();
	}
	if (feat != FEAT_GREATER_WEAPON_SPECIALIZATION)
		return 0;

	*addresses.featMultiselect_10C75F38 = feat;
	*addresses.feat_10C79344 = FEAT_NONE;
	*addresses.featsMultiselectNum_10C75F34 = 0;

	for (int feat1 = FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET; feat1 < NUM_FEATS; feat1++)
	{
		if (feats.IsFeatPropertySet((feat_enums)feat1, 0x100110) && feats.IsFeatEnabled((feat_enums)feat1))
		{
			addresses.featsMultiselectList[(*addresses.featsMultiselectNum_10C75F34)++] = (feat_enums)feat1;
		}
	}


	addresses.j_CopyWidget_101F87A0(*addresses.widIdx_10C77D80, addresses.featsScrollbar);
	*addresses.dword_10C77D54 = 0;
	*addresses.dword_10C75F30 = 0;
	*addresses.dword_10C77D50 = ( (*addresses.featsMultiselectNum_10C75F34) - 15) & ((*addresses.featsMultiselectNum_10C75F34 - 15 < 0) - 1);
	addresses.sub_101F87B0(*addresses.widIdx_10C77D80, addresses.featsScrollbar);
	addresses.sub_101F9510(*addresses.dword_10C76AF0, 4);
	addresses.sub_101F9100(*addresses.widgId_10C7AE14, 0);
	return addresses.sub_101F8E40(*addresses.widgId_10C7AE14);
		
}


const char * HookedGetMultiselectShortName(feat_enums feat);
int __declspec(naked) GetMultiselectShortNameUsercall()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}
	__asm{
		push esi;
		call HookedGetMultiselectShortName;
		add esp, 4;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx}
	__asm retn;
};


const char * HookedGetMultiselectShortName(feat_enums feat)
{
	MesLine mesLine;
	if (feat >= FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET && feat <= FEAT_GREATER_WEAPON_SPECIALIZATION_GRAPPLE)
	{
		mesLine.key = (feat - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET + FEAT_WEAPON_SPECIALIZATION_GAUNTLET) + 50000;
	} else if (feat > FEAT_NONE)
	{
		return addresses.sub_10182760(feat);
	} else
		mesLine.key = feat + 50000;
	if (!mesFuncs.GetLine(*addresses.pcCreationMes, &mesLine) || !*mesLine.value)
	{
		return addresses.sub_10182760(feat);
	}
	return mesLine.value;
}

class UiCharEditorFeatsReplacements : public TempleFix
{
public: 
	static void ExistingFeatRender(int widgetId)
	{
		orgExistingFeatRender(widgetId);
	};
	static void(*orgExistingFeatRender)(int);

	void apply() override 
	{
		replaceFunction(0x101A8D70, FeatUiPrereqCheckUsercallWrapper);
		OrgFeatMultiselectSub_101A8080 = (int(__cdecl*)()) replaceFunction(0x101A8080, HookedUsercallFeatMultiselectSub_101A8080);

		writeHex(0x101A940A, "90 90"); // affects right click

		writeHex(0x101A9F81, "90 90");
		writeHex(0x101AA053, "90 90");
		writeHex(0x101A9E57, "90 90 90  90 90 90"); // affects drag n' drop

		replaceFunction(0x101A8D20, GetMultiselectShortNameUsercall);

		//orgFeatRender = replaceFunction(0x101A8850, FeatRender);
	}
} uiCharEdFeatsReplacements;
void(*UiCharEditorFeatsReplacements::orgExistingFeatRender)(int);


int __cdecl FeatUiPrereqCheck(feat_enums feat)
{
	int featArrayLen = 0;
	feat_enums featArray[10] ;
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


int __declspec(naked) FeatUiPrereqCheckUsercallWrapper()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}
	
	__asm
	{
		push edi;
		call FeatUiPrereqCheck;
		pop edi;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx }
	__asm retn;
}