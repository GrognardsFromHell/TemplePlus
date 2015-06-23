#include "stdafx.h"
#include "common.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"



int FeatUiPrereqCheckUsercallWrapper();

struct UiCharEditorFeatsAddresses : AddressTable
{
	CharEditorSelectionPacket * charEdSelPkt;
	UiCharEditorFeatsAddresses()
	{
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


int HookedFeatMultiselectSub_101A8080(feat_enums feat)
{
	if (feat >= FEAT_EXOTIC_WEAPON_PROFICIENCY && feat <= FEAT_WEAPON_SPECIALIZATION)
	{
		__asm mov eax, feat;
		return OrgFeatMultiselectSub_101A8080();
	}
	return 0;
		
}

class UiCharEditorFeatsReplacements : public TempleFix
{
public: 
	const char* name() override 
	{ 
		return "UiCharEditor Function Replacements";
	} 

	void apply() override 
	{
		replaceFunction(0x101A8D70, FeatUiPrereqCheckUsercallWrapper);
		OrgFeatMultiselectSub_101A8080 = (int(__cdecl*)()) replaceFunction(0x101A8080, HookedUsercallFeatMultiselectSub_101A8080);

	}
} uiCharEdFeatsReplacements;



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