#include "stdafx.h"
#include "common.h"
#include "config/config.h"
#include "d20.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"
#include "ui/ui.h"
#include "util/fixes.h"
#include "tig/tig_texture.h"
#include "gamesystems/gamesystems.h"

#define MAX_PC_CREATION_PORTRAITS 8
#include "party.h"
#include "tig/tig_msg.h"
#include <location.h>
#include <tio/tio.h>
#include <mod_support.h>

struct TigMsg;
int PcCreationFeatUiPrereqCheckUsercallWrapper();
int(__cdecl * OrgFeatMultiselectSub_101822A0)();
int HookedFeatMultiselectSub_101822A0(feat_enums feat);
int HookedUsercallFeatMultiselectSub_101822A0();

struct PartyCreationPc
{
	int flags;
	int field4;
	int* field8;
	int fieldC;
	ObjectId objId;
	int field28;
	char* nameMaybe;
	char fileName[260];
	int field134;
	int field138;
	int objTypeMaybe;
	int field140;
	Alignment alignment;
	int field148;
	int field14C;
	objHndl handle;
};

const int testSizeOfCreationPc = sizeof(PartyCreationPc); // 344  0x158

struct ChargenSystem{ // incomplete
	const char* name;
	void(__cdecl *reset)(CharEditorSelectionPacket & charSpec);
	void(__cdecl *field_8)();
	BOOL(__cdecl *systemInit)(GameSystemConf *);
	int field10;
	int field14;
	void(__cdecl *reset2)();
	void(__cdecl *field_1C)();
	int(__cdecl *checkComplete)(); // checks if the char editing stage is complete (thus allowing you to move on to the next stage). This is checked at every render call.
	int field24;
	void(__cdecl *buttonExited)();
};


class PcCreationUiSystem : TempleFix
{
public:
	static WidgetType1 pcPortraitsMain;
	static int pcPortraitsMainId;
	static TigRect pcPortraitRects[MAX_PC_CREATION_PORTRAITS];
	static TigRect pcPortraitBoxRects[MAX_PC_CREATION_PORTRAITS];
	

	static BOOL PcPortraitsInit(GameSystemConf * conf);
	static BOOL PcPortraitWidgetsInit(int height);
	static BOOL PcPortraitsExit();
	static BOOL PcPortraitsResize(UiResizeArgs * resizeArgs);
	static BOOL PcPortraitsMainHideAndGet();

	static void PcPortraitsButtonActivateNext();
	static void PcPortraitsRefresh();
	static void PcPortraitsDisable();
	static int return0()
	{
		return 0;
	};
	static int PcPortraitsMsgFunc(int widgetId, TigMsg* tigMsg);
	static UiMsgFunc orgPcPortraitsMsgFunc;

	static int pcPortraitWidgIds[MAX_PC_CREATION_PORTRAITS];

	void WriteMaxPcPortraitValues();

	static void GetPartyPool(int fromIngame); //fromIngame is 0 when launching from main menu, 1 when launching from inn guestbook
	static int PartyPoolLoader();

	void apply() override
	{
		// PC Creation UI Fixes
		replaceFunction(0x10182E80, PcCreationFeatUiPrereqCheckUsercallWrapper);
		OrgFeatMultiselectSub_101822A0 = (int(__cdecl*)()) replaceFunction(0x101822A0, HookedUsercallFeatMultiselectSub_101822A0);
		//replaceFunction(0x101634D0, PcPortraitWidgetsInit);
		replaceFunction(0x10163030, PcPortraitsMainHideAndGet);
		replaceFunction(0x10163660, PcPortraitsInit);
		replaceFunction(0x101636E0, PcPortraitsResize);
		replaceFunction(0x10163410, PcPortraitsExit);

		replaceFunction(0x10163060, PcPortraitsDisable);
		replaceFunction(0x10163090, PcPortraitsButtonActivateNext);
		replaceFunction(0x10163440, PcPortraitsRefresh);
		orgPcPortraitsMsgFunc = replaceFunction(0x101633A0, PcPortraitsMsgFunc);


		WriteMaxPcPortraitValues();
		
		// UiPartyCreationGetPartyPool
		static void(*orgGetPartyPool)(int) = replaceFunction<void(int)>(0x10165E60, GetPartyPool);
		// static int(*orgPartyPoolLoader)() = replaceFunction<int()>(0x10165790, PartyPoolLoader);
		

		if (temple::Dll::GetInstance().HasCo8Hooks() ) {
			writeNoops(0x1011D521); // disabling EXP draw call
		}

		static BOOL (__cdecl*orgPartyAlignmentChoiceShow)() = replaceFunction<BOOL()>(0x1011E200, []()->BOOL
		{

			auto result = orgPartyAlignmentChoiceShow();
			if (modSupport.IsKotB()){
				auto alignmentBtnIds = temple::GetRef<int[9]>(0x10BDA73C);
				ui.ButtonSetButtonState(alignmentBtnIds[0], UBS_DISABLED); // LG
				ui.ButtonSetButtonState(alignmentBtnIds[2], UBS_DISABLED); // CG

				ui.ButtonSetButtonState(alignmentBtnIds[4], UBS_DISABLED); // TN
				ui.ButtonSetButtonState(alignmentBtnIds[5], UBS_DISABLED); // CN

				ui.ButtonSetButtonState(alignmentBtnIds[6], UBS_DISABLED); // LE
				ui.ButtonSetButtonState(alignmentBtnIds[8], UBS_DISABLED); // CE
			}
			return result;
		});
	}
} pcCreationSys;
WidgetType1 PcCreationUiSystem::pcPortraitsMain;
int PcCreationUiSystem::pcPortraitsMainId = -1;
int PcCreationUiSystem::pcPortraitWidgIds[MAX_PC_CREATION_PORTRAITS] = { -1, };
TigRect PcCreationUiSystem::pcPortraitRects[MAX_PC_CREATION_PORTRAITS];
TigRect PcCreationUiSystem::pcPortraitBoxRects[MAX_PC_CREATION_PORTRAITS];
UiMsgFunc PcCreationUiSystem::orgPcPortraitsMsgFunc;

struct PcCreationUiAddresses : temple::AddressTable
{
	int * pcCreationIdx;

	WidgetType1 * pcCreationPortraitsMainWidget;
	int * mainWindowWidgetId; // 10BF0ED4
	int * pcPortraitsWidgetIds; // 10BF0EBC  array of 5 entries


	int * uiPccPortraitTexture;
	int *uiPccPortraitHoverTexture;
	int *uiPccPortraitClickTexture;
	int * uiPccPortraitDisabledTexture;
	int * uiPcPortraitsFullMaybe;



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
	void (__cdecl*ui_render_pc_creation_portraits)(int widId);
	BOOL(__cdecl*ui_msg_pc_creation_portraits)(int widId, TigMsg*);

	PcCreationUiAddresses()
	{

		rebase(ui_render_pc_creation_portraits, 0x10163270);
		rebase(ui_msg_pc_creation_portraits   , 0x101633A0);
		rebase(pcCreationIdx,					0x10BF0BC8);
		rebase(uiPccPortraitTexture,			0x10BF0BCC);
		rebase(uiPccPortraitHoverTexture,		0x10BF0C20);
		rebase(pcCreationPortraitsMainWidget,	0x10BF0C28);
		rebase(uiPccPortraitClickTexture,		0x10BF1354);
		rebase(uiPccPortraitDisabledTexture,	0x10BF1358);
		rebase(uiPcPortraitsFullMaybe,			0x10BF0ED0);


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
}

BOOL PcCreationUiSystem::PcPortraitsInit(GameSystemConf* conf)
{
	if (textureFuncs.RegisterTexture("art\\interface\\pc_creation\\portrait.tga", addresses.uiPccPortraitTexture)
		|| textureFuncs.RegisterTexture("art\\interface\\pc_creation\\portrait_click.tga", addresses.uiPccPortraitClickTexture)
		|| textureFuncs.RegisterTexture("art\\interface\\pc_creation\\portrait_hover.tga", addresses.uiPccPortraitHoverTexture)
		|| textureFuncs.RegisterTexture("art\\interface\\pc_creation\\portrait_disabled.tga", addresses.uiPccPortraitDisabledTexture)
		)
	{
		return 0;
	}
	return PcPortraitWidgetsInit(conf->height);
}

BOOL PcCreationUiSystem::PcPortraitWidgetsInit(int height)
{
	pcPortraitsMain.WidgetType1Init(10, height - 80, 650, 63);
	pcPortraitsMain.widgetFlags = 1;
	pcPortraitsMain.render = [](int widId) {return0(); };
	pcPortraitsMain.handleMessage = [](int widId, TigMsg* msg)->BOOL {return return0(); };

	if (ui.AddWindow(&pcPortraitsMain, sizeof(WidgetType1), &pcPortraitsMainId, "pc_creation_portraits.c", 275) )
		return 0;

	for (int i = 0; i < MAX_PC_CREATION_PORTRAITS; i++)
	{
		WidgetType2 button;
		ui.ButtonInit(&button, 0, pcPortraitsMainId, pcPortraitsMain.x + 81 * i, pcPortraitsMain.y, 76, 63);

		pcPortraitBoxRects[i].x = button.x;
		pcPortraitRects[i].x = button.x + 4;

		pcPortraitBoxRects[i].y = button.y;
		pcPortraitRects[i].y = button.y + 4;


		pcPortraitBoxRects[i].width = button.width;
		pcPortraitBoxRects[i].height = button.height;

		pcPortraitRects[i].width = 51;
		pcPortraitRects[i].height = 45;

		button.render = addresses.ui_render_pc_creation_portraits;
		button.handleMessage = addresses.ui_msg_pc_creation_portraits;

		if (ui.AddButton(&button, sizeof(WidgetType2), &pcPortraitWidgIds[i], "pc_creation_portraits.c", 299)
			|| ui.BindToParent(pcPortraitsMainId, pcPortraitWidgIds[i])
			|| ui.ButtonSetButtonState(pcPortraitWidgIds[i], 4))
			return 0;

	}

	return 1;
}

BOOL PcCreationUiSystem::PcPortraitsExit()
{
	for (int i = 0; i < MAX_PC_CREATION_PORTRAITS; i++)
	{
		ui.WidgetRemoveRegardParent(pcPortraitWidgIds[i]);
	}
	return ui.WidgetAndWindowRemove(pcPortraitsMainId);
}

BOOL PcCreationUiSystem::PcPortraitsResize(UiResizeArgs* resizeArgs)
{
	for (int i = 0; i < MAX_PC_CREATION_PORTRAITS;i++)
	{
		ui.WidgetRemoveRegardParent(pcPortraitWidgIds[i]);
	}
	ui.WidgetAndWindowRemove(pcPortraitsMainId);
	return PcPortraitWidgetsInit(resizeArgs->rect1.height);
}

BOOL PcCreationUiSystem::PcPortraitsMainHideAndGet()
{
	ui.WidgetSetHidden(pcPortraitsMainId, 1);
	return ui.WidgetCopy(pcPortraitsMainId, &pcPortraitsMain);
}

void PcCreationUiSystem::PcPortraitsButtonActivateNext()
{
	for (int i = 0; i < MAX_PC_CREATION_PORTRAITS; i++)
	{
		int state;
		ui.GetButtonState(pcPortraitWidgIds[i], &state);
		if (state == 4)
		{
			ui.ButtonSetButtonState(pcPortraitWidgIds[i],0);
			if (i == MAX_PC_CREATION_PORTRAITS -1)
				*addresses.uiPcPortraitsFullMaybe = 1;
			return ;
		}
	}
	*addresses.uiPcPortraitsFullMaybe = 1;
}

void PcCreationUiSystem::PcPortraitsRefresh()
{
	ui.WidgetSetHidden(pcPortraitsMainId,0);
	ui.WidgetCopy(pcPortraitsMainId, &pcPortraitsMain);
	ui.WidgetBringToFront(pcPortraitsMainId);
	for (int i = 0; i < MAX_PC_CREATION_PORTRAITS;i++)
	{
		ui.ButtonSetButtonState(pcPortraitWidgIds[i], 4);
	}
	*addresses.uiPcPortraitsFullMaybe = 0;

	if (party.GroupPCsLen())
	{
		for (int i = 0; i < MAX_PC_CREATION_PORTRAITS;i++)
		{
			ui.ButtonSetButtonState(pcPortraitWidgIds[i], 0);
		}
	}
	*addresses.pcCreationIdx = -1;
}

void PcCreationUiSystem::PcPortraitsDisable()
{
	for (int i = 0; i < MAX_PC_CREATION_PORTRAITS;i++)
	{
		ui.ButtonSetButtonState(pcPortraitWidgIds[i], 4);
	}
	*addresses.uiPcPortraitsFullMaybe = 0;
}

int PcCreationUiSystem::PcPortraitsMsgFunc(int widgetId, TigMsg* tigMsg)
{
	int i;
	if (tigMsg->type != TigMsgType::WIDGET
		|| tigMsg->arg2 != 1
		|| (i = ui.WidgetlistIndexof(widgetId, pcPortraitWidgIds, MAX_PC_CREATION_PORTRAITS), i == -1))
		return 0;

	return orgPcPortraitsMsgFunc(widgetId, tigMsg);
}

void PcCreationUiSystem::WriteMaxPcPortraitValues()
{
	// 1016312F
	int writeVal = (int)&pcPortraitWidgIds[-1];
	write(0x1016312F + 3, &writeVal, 4);

	// ui_msg_pc_creation_portraits
	writeVal = MAX_PC_CREATION_PORTRAITS;
	write(0x101633B5 + 1, &writeVal, 1);
	writeVal = (int)&pcPortraitWidgIds;
	write(0x101633B7 + 1, &writeVal, 4);

	// ui_render_pc_creation_portraits
	writeVal = MAX_PC_CREATION_PORTRAITS;
	write(0x10163279 + 1, &writeVal, 1);
	writeVal = (int)&pcPortraitWidgIds;
	write(0x1016327B + 1, &writeVal, 4);

	writeVal = (int)&pcPortraitsMainId;
	write(0x101632E6 + 2, &writeVal, 4);
	write(0x1016337A + 2, &writeVal, 4);
	writeVal = (int)&pcPortraitBoxRects;
	write(0x101632F6 + 2, &writeVal, 4);
	writeVal = (int)&pcPortraitRects;
	write(0x10163329 + 2, &writeVal, 4);
}

void PcCreationUiSystem::GetPartyPool(int fromIngame)
{
	int& uiPartypoolWidgetId = temple::GetRef<int>(0x10BF1764);
	int& uiPcCreationMainWndId = temple::GetRef<int>(0x10BDD690);
	int& uiPartyCreationNotFromShopmap = temple::GetRef<int>(0x10BF24E0);
	LocAndOffsets& locToCreatePcs = temple::GetRef<LocAndOffsets>(0x10BF17A8);


	ui.WidgetSetHidden(uiPartypoolWidgetId, 0);
	ui.WidgetBringToFront(uiPartypoolWidgetId);
	ui.WidgetBringToFront(uiPcCreationMainWndId);
	uiPartyCreationNotFromShopmap = fromIngame;
	if (fromIngame)
	{
		auto dude = party.GroupListGetMemberN(0);
		if (dude)
		{
			locToCreatePcs = objects.GetLocationFull(dude);
		}
		if (uiPartyCreationNotFromShopmap)
		{
			int& pcCreationPartyAlignment = temple::GetRef<int>(0x11E741A8);
			pcCreationPartyAlignment = party.GetPartyAlignment();
		}
	}

	auto UiPartyCreationHidePcWidgets = temple::GetRef<void()>(0x10135080);
	UiPartyCreationHidePcWidgets();
	int& uiPartyPoolPcsIdx = temple::GetRef<int>(0x10BF1760);
	uiPartyPoolPcsIdx = -1;
	ui.ButtonSetButtonState(temple::GetRef<int>(0x10BF2408), 4); // Add
	ui.ButtonSetButtonState(temple::GetRef<int>(0x10BF2538), 4); // VIEW
	ui.ButtonSetButtonState(temple::GetRef<int>(0x10BF2410), 4); // RENAME
	ui.ButtonSetButtonState(temple::GetRef<int>(0x10BF239C), 4); // DELETE

	auto GetPcCreationPcBuffer = temple::GetRef<void()>(0x101631B0);
	GetPcCreationPcBuffer();
	auto PartyPoolLoader = temple::GetRef<int()>(0x10165790);
	PartyPoolLoader();
	auto AddPcsFromBuffer = temple::GetRef<void()>(0x10163210);
	AddPcsFromBuffer();
	auto UiPartyPoolRefreshTopButtons = temple::GetRef<void()>(0x10165150);
	UiPartyPoolRefreshTopButtons();
	auto PcPortraitsRefresh = temple::GetRef<void()>(0x10163440);
	PcPortraitsRefresh();
	auto UiPartyPoolScrollbox_10164620 = temple::GetRef<void()>(0x10164620);
	UiPartyPoolScrollbox_10164620();

	if (fromIngame || !party.GroupListGetLen())
	{
		ui.WidgetSetHidden(temple::GetRef<int>(0x10BDB8E0), 1);
	}
	else
	{
		ui.WidgetSetHidden(temple::GetRef<int>(0x10BDB8E0), 0);
	}
	auto UiUtilityBarHide = temple::GetRef<void()>(0x1010EEC0);
	UiUtilityBarHide();
}

int PcCreationUiSystem::PartyPoolLoader()
{
	int result = 1;

	/*auto PartyCreationClearPcs = temple::GetRef<void()>(0x10163E30);
	PartyCreationClearPcs();
	auto IsIronman = temple::GetRef<int()>(0x10003860);
	TioFileList filelist;
	if (IsIronman())
		tio_filelist_create(&filelist, "players\\ironman\\*.ToEEIMan");
	else
		tio_filelist_create(&filelist, "players\\*.ToEEPC");

	
	int& uiPartypoolNumPcs = temple::GetRef<int>(0x10BF237C);
	if (filelist.count)
	{
		auto uiPartyCreationPcs = temple::GetRef<PartyCreationPc*>(0x10BF253C);
		uiPartyCreationPcs = new PartyCreationPc[filelist.count];
		for (int i = 0; i < filelist.count; i++)
		{
			if (!PartyPoolGetPc(&uiPartyCreationPcs[uiPartypoolNumPcs], filelist.files[i].name))
			{
				result = 0;
				break;
			}
			logger->info("Successfully loaded PC ({}) {} ({})", 
				uiPartypoolNumPcs,
				uiPartyCreationPcs[uiPartypoolNumPcs].nameMaybe,
				uiPartyCreationPcs[uiPartypoolNumPcs].objId.ToString());
			uiPartypoolNumPcs++;
		}
	}
	tio_filelist_destroy(&filelist);

	if (result)
	{
		auto partyPoolPcIndices = temple::GetRef<int*>(0x10BF2378);
		if (partyPoolPcIndices)
			free(partyPoolPcIndices);
		partyPoolPcIndices = new int[uiPartypoolNumPcs];
		auto UiPartyPoolRefreshVisibles = temple::GetRef<void()>(0x10164B10);
		UiPartyPoolRefreshVisibles();
	}

	// not yet finished because pug found the bug by the time I got to it :P
	*/

	
	return result;
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
		return feats.FeatPrereqsCheck(*feats.charEditorObjHnd, feat, featArray, featArrayLen, (Stat)0, addresses.charEdSelPkt->statBeingRaised);

	// the vanilla multiselect range

	return feats.FeatPrereqsCheck(*feats.charEditorObjHnd, feat, featArray, featArrayLen, (Stat)0, addresses.charEdSelPkt->statBeingRaised);
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