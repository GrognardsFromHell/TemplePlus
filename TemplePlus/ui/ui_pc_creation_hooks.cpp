
#include "stdafx.h"

#include <temple/dll.h>
#include <infrastructure/keyboard.h>

#include "util/fixes.h"
#include "tig/tig_msg.h"
#include "ui_char_editor.h"
#include "ui_pc_creation.h"
#include "config/config.h"
#include "gamesystems/d20/d20stats.h"
#include "mod_support.h"
#include "party.h"
#include "ui_systems.h"
#include "d20_race.h"
#include "ui_render.h"
#include "gamesystems/objects/objsystem.h"
#include <python/python_integration_class_spec.h>

int PcCreationFeatUiPrereqCheckUsercallWrapper();
int HookedUsercallFeatMultiselectSub_101822A0();
int(__cdecl * OrgFeatMultiselectSub_101822A0)();
int HookedFeatMultiselectSub_101822A0(feat_enums feat);


class PcCreationHooks : TempleFix
{
public:

	static CharEditorSelectionPacket &GetCharEdSelPkt();

	static void GetPartyPool(int fromIngame); //fromIngame is 0 when launching from main menu, 1 when launching from inn guestbook
	static int PartyPoolLoader();


	static BOOL StatsIncreaseBtnMsg(int widId, TigMsg* msg);
	static BOOL StatsDecreaseBtnMsg(int widId, TigMsg* msg);
	static BOOL StatsUpdateBtns();
	static void AutoAddSpellsMemorizedToGroup();

	void apply() override
	{

		replaceFunction(0x1011F290, AutoAddSpellsMemorizedToGroup);

		static BOOL(__cdecl*orgStatsWndMsg)(int, TigMsg*) = replaceFunction<BOOL(int, TigMsg*)>(0x1018BA50, [](int widId, TigMsg*msg) {
			if (!uiPcCreation.StatsWndMsg(widId, msg))
				return orgStatsWndMsg(widId, msg);
			return TRUE;
		});

		
		// Update title
		replaceFunction<void(__cdecl)()>(0x1011C470, []() { ChargenSystem::UpdateDescriptionBox(); });

		replaceFunction<void(__cdecl)(int widgetId)>(0x1011ED80, [](int id){
			uiPcCreation.MainWndRender(id);
		});
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x1011c1c0, [](int widgetId, TigMsg* msg) { 
			return uiPcCreation.MainWndMsg(widgetId, msg);
		});

		// Chargen Race System
		replaceFunction<void(__cdecl)()>(0x1018A7D0, [](){	uiSystems->GetPcCreation().GetRace().Show();	});
		replaceFunction<void(__cdecl)()>(0x1018A7B0, []() {	uiSystems->GetPcCreation().GetRace().Hide();	});
		replaceFunction<void(__cdecl)(CharEditorSelectionPacket&)>(0x1018A590, [](CharEditorSelectionPacket& pkt) {	uiSystems->GetPcCreation().GetRace().Reset(pkt);	});
		replaceFunction<void(__cdecl)()>(0x1018A5A0, []() {	uiSystems->GetPcCreation().GetRace().CheckComplete();	});
		replaceFunction<void(__cdecl)()>(0x1018A7F0, []() { RaceChargen::UpdateScrollbox(); });

		// Gender
		replaceFunction<void(__cdecl)(CharEditorSelectionPacket&, objHndl&)>(0x10189CD0, [](CharEditorSelectionPacket&selPkt, objHndl&handle) {uiPcCreation.GenderFinalize(selPkt, handle); });

		// Height
		replaceFunction<void(__cdecl)()>(0x10189530, [](){
			auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
			auto &uiPcCreationHeightSliderValue = temple::GetRef<int>(0x10C42E28);
			auto &dword_102FE17C = temple::GetRef<int>(0x102FE17C);
			auto &dword_102FE178 = temple::GetRef<int>(0x102FE178);
			
			
			auto &maxHeightInches = temple::GetRef<int>(0x10C4315C);
			auto &minHeightInches = temple::GetRef<int>(0x10C42E1C);
			
			auto &maxWeight = temple::GetRef<int>(0x10C42E08);
			auto &minWeight = temple::GetRef<int>(0x10C42E24);
			


			dword_102FE17C = 205 - uiPcCreationHeightSliderValue;
			dword_102FE178 = 204;
			auto heightDeltaInch = uiPcCreationHeightSliderValue * (maxHeightInches - minHeightInches) / 164;
			selPkt.height = heightDeltaInch + minHeightInches;
			selPkt.modelScale = selPkt.height * d20RaceSys.GetModelScale(selPkt.raceId, selPkt.genderId) / maxHeightInches;

			

			auto &textBuffer = temple::GetRef<char[16]>(0x10C42EB0);
			_snprintf(textBuffer, sizeof textBuffer, "%d'%d\"", (selPkt.height) / 12, (selPkt.height) % 12);

			UiRenderer::PushFont(PredefinedFont::ARIAL_BOLD_24);
			auto metrics = UiRenderer::MeasureTextSize(textBuffer, uiPcCreation.whiteTextGenericStyle);
			UiRenderer::PopFont();
			temple::GetRef<TigRect>(0x10C42E0C) = { dword_102FE178 +44, dword_102FE17C -metrics.height / 2 + 5,
													metrics.width, metrics.height};
			

			auto &wnd = temple::GetRef<LgcyWindow>(0x10C42EC8);
			dword_102FE178 += wnd.x;
			dword_102FE17C += wnd.y;

			selPkt.weight = minWeight + uiPcCreationHeightSliderValue * (maxWeight - minWeight) / 164;

			auto editedChar = uiPcCreation.GetEditedChar();
			if (editedChar)
				objSystem->GetObject(editedChar)->SetInt32(obj_f_model_scale, (int)(selPkt.modelScale * 100.0));

		});
		replaceFunction<void(__cdecl)()>(0x10189700, []() {uiPcCreation.HeightShow(); });

		// Hair
		// Update Hair
		replaceFunction<void(__cdecl)()>(0x10188B70, []() {uiPcCreation.HairUpdate(); });
		replaceFunction<void(__cdecl)()>(0x10188BD0, []() {uiPcCreation.HairUpdateStyleBtnTextures(); });

		replaceFunction<int(Race, int, int, int, int)>(0x100E17B0, [](Race race, int gender, int hairType, int hairColor, int helmMod) {
			return ((int)d20RaceSys.GetHairStyle(race) & 7)
				| ((gender & 1) << 3)
				| ((hairType & 7) << 4)
				| ((hairColor & 7) << 7)
				| ((helmMod & 3) << 10);
		});

		// Chargen Class system
		replaceFunction<void(__cdecl)(UiSystemConf&)>(0x10188910, [](UiSystemConf& conf) {uiPcCreation.ClassSystemInit(conf); });
		replaceFunction<void(__cdecl)()>(0x101885E0, []() {uiPcCreation.ClassWidgetsFree(); });
		replaceFunction<void(__cdecl)()>(0x101885D0, []() {uiPcCreation.ClassActivate(); });
		replaceFunction<void(__cdecl)(UiResizeArgs&)>(0x101889F0, [](UiResizeArgs& args) {uiPcCreation.ClassWidgetsResize(args); });
		replaceFunction<void(__cdecl)()>(0x101880F0, []() {uiPcCreation.ClassShow(); });
		replaceFunction<void(__cdecl)()>(0x101880D0, []() {uiPcCreation.ClassHide(); });
		replaceFunction<void(__cdecl)(CharEditorSelectionPacket&, objHndl&)>(0x10188110, [](CharEditorSelectionPacket& selPkt, objHndl& handle) {uiPcCreation.ClassFinalize(selPkt, handle); });
		// replaceFunction<void(__cdecl)()>(0x101B0620, []() {uiPcCreation.ClassCheckComplete(); }); // same function as ui_char_editor, already replaced
		replaceFunction<void(__cdecl)()>(0x10188260, []() {uiPcCreation.ClassBtnEntered(); });

		// Alignment
		replaceFunction<BOOL(__cdecl)(int, Alignment)>(0x10188170, [](int classEnum, Alignment alignment) ->BOOL {
			return pythonClassIntegration.IsAlignmentCompatible(alignment, classEnum); });

		// Skill

		// Hook for SkillIncreaseBtnMsg to raise 4 times when ctrl/alt is pressed
		static BOOL(__cdecl*orgSkillIncBtnMsg)(int, TigMsg*) = replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x101815C0, [](int widId, TigMsg* msg) {
			if (msg->type != TigMsgType::WIDGET)
				return FALSE;
			auto widMsg = (TigMsgWidget*)msg;
			if (widMsg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
				return FALSE;

			if (infrastructure::gKeyboard.IsKeyPressed(VK_CONTROL) || infrastructure::gKeyboard.IsKeyPressed(VK_LCONTROL)
				|| infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				orgSkillIncBtnMsg(widId, msg);
				int safetyCounter = 3;
				while (uiManager->GetButtonState(widId) != LgcyButtonState::Disabled && safetyCounter >= 0) {
					orgSkillIncBtnMsg(widId, msg);
					safetyCounter--;
				}
				return TRUE;
			};
			return orgSkillIncBtnMsg(widId, msg);
		});

		// Stats
		static void (__cdecl *orgStatsReset)(CharEditorSelectionPacket*) =
			replaceFunction<void (__cdecl)(CharEditorSelectionPacket*)>(0x1018ABC0,
					[](CharEditorSelectionPacket *pkt) {
						// fix strength stat prereq off-by-one during chargen
						pkt->statBeingRaised = static_cast<Stat>(-1);

						orgStatsReset(pkt);
					});

		// Feats
		replaceFunction<void(__cdecl)(UiSystemConf&)>(0x101847F0, [](UiSystemConf& conf) {uiPcCreation.FeatsSystemInit(conf); });
		replaceFunction<void(__cdecl)()>(0x10182D30, []() {uiPcCreation.FeatsFree(); });
		replaceFunction<void(__cdecl)()>(0x10182A30, []() {uiPcCreation.FeatsActivate(); });
		replaceFunction<void(__cdecl)(CharEditorSelectionPacket&)>(0x10181F40, [](CharEditorSelectionPacket& selPkt) {uiPcCreation.FeatsReset(selPkt); });
		replaceFunction<void(__cdecl)(UiResizeArgs&)>(0x10184B70, [](UiResizeArgs& args) {uiPcCreation.FeatsWidgetsResize(args); });
		replaceFunction<void(__cdecl)()>(0x10181F80, []() {uiPcCreation.FeatsShow(); });
		replaceFunction<void(__cdecl)()>(0x10181F60, []() {uiPcCreation.FeatsHide(); });
		replaceFunction<void(__cdecl)(CharEditorSelectionPacket&, objHndl&)>(0x10181FE0, [](CharEditorSelectionPacket& selPkt, objHndl& handle) {uiPcCreation.FeatsFinalize(selPkt, handle); });
		replaceFunction<BOOL(__cdecl)()>(0x10181FA0, []() {return uiPcCreation.FeatsCheckComplete(); });

		// Spell system
		replaceFunction<void(__cdecl)(UiSystemConf&)>(0x101800E0, [](UiSystemConf& conf) {uiPcCreation.SpellsSystemInit(conf); });
		replaceFunction<void(__cdecl)()>(0x1017F090, []() {uiPcCreation.SpellsFree(); });
		replaceFunction<void(__cdecl)()>(0x101804A0, []() {uiPcCreation.SpellsActivate(); });
		replaceFunction<void(__cdecl)()>(0x1017EAE0, []() {uiPcCreation.SpellsReset(); });
		replaceFunction<void(__cdecl)(UiResizeArgs&)>(0x10180390, [](UiResizeArgs& args) {uiPcCreation.SpellsWidgetsResize(args); });
		replaceFunction<void(__cdecl)()>(0x1017EB60, []() {uiPcCreation.SpellsShow(); });
		replaceFunction<void(__cdecl)()>(0x1017EB40, []() {uiPcCreation.SpellsHide(); });
		replaceFunction<void(__cdecl)(CharEditorSelectionPacket&, objHndl&)>(0x1017F0A0, [](CharEditorSelectionPacket& selPkt, objHndl& handle) {uiPcCreation.SpellsFinalize(); });
		replaceFunction<BOOL(__cdecl)()>(0x1017EB80, []() {return uiPcCreation.SpellsCheckComplete(); });

		// Deity
		replaceFunction<void(__cdecl)()>(0x10187340, []() {uiPcCreation.DeitySetPermissibles(); });

		// lax rules option for unbounded increase of stats & party member alignment
		replaceFunction(0x1018B940, StatsIncreaseBtnMsg);
		replaceFunction(0x1018B9B0, StatsDecreaseBtnMsg);
		replaceFunction(0x1018B570, StatsUpdateBtns);
		replaceFunction<BOOL(Alignment, Alignment)>(0x1011B880, [](Alignment a, Alignment b)->BOOL {
			if (config.laxRules && config.disableAlignmentRestrictions) {
				return TRUE;
			}

			return (BOOL)d20Stats.AlignmentsUnopposed(a, b);
		});

		// PC Creation UI Fixes
		replaceFunction(0x10182E80, PcCreationFeatUiPrereqCheckUsercallWrapper);
		OrgFeatMultiselectSub_101822A0 = (int(__cdecl*)()) replaceFunction(0x101822A0, HookedUsercallFeatMultiselectSub_101822A0);


		// UiPartyCreationGetPartyPool
		static void(*orgGetPartyPool)(int) = replaceFunction<void(int)>(0x10165E60, GetPartyPool);
		// static int(*orgPartyPoolLoader)() = replaceFunction<int()>(0x10165790, PartyPoolLoader);
		static void(__cdecl* orgPartyPoolExit)() = replaceFunction<void()>(0x10165A50, []() { // fixed crash due to not removing from GroupList when exiting party pool (e.g. to change the party alignment)
			int& uiPartyCreationIngame = temple::GetRef<int>(0x10BF24E0);
			
			if (!uiPartyCreationIngame) {
				// moved this part to the beginning, since UiCharHide(0) will refresh the radial menus based on discarded party members, potentially overflowing it
				static auto uiPartyPoolRemoveAllPcPortraits = temple::GetRef<void()>(0x1011B6F0); 
				uiPartyPoolRemoveAllPcPortraits();
			}
			return orgPartyPoolExit();
			});

		if (temple::Dll::GetInstance().HasCo8Hooks()) {
			writeNoops(0x1011D521); // disabling EXP draw call
		}

		// KotB alignment restrictions
		static BOOL(__cdecl*orgPartyAlignmentChoiceShow)() = replaceFunction<BOOL()>(0x1011E200, []()->BOOL
		{

			auto result = orgPartyAlignmentChoiceShow();
			if (modSupport.IsKotB()) {
				auto alignmentBtnIds = temple::GetRef<int[9]>(0x10BDA73C);
				uiManager->SetButtonState(alignmentBtnIds[0], LgcyButtonState::Disabled); // LG
				uiManager->SetButtonState(alignmentBtnIds[2], LgcyButtonState::Disabled); // CG

				uiManager->SetButtonState(alignmentBtnIds[4], LgcyButtonState::Disabled); // TN
				uiManager->SetButtonState(alignmentBtnIds[5], LgcyButtonState::Disabled); // CN

				uiManager->SetButtonState(alignmentBtnIds[6], LgcyButtonState::Disabled); // LE
				uiManager->SetButtonState(alignmentBtnIds[8], LgcyButtonState::Disabled); // CE
			}
			return result;
		});
	}
} pcCreationHooks;


CharEditorSelectionPacket & PcCreationHooks::GetCharEdSelPkt() {
	return temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
}

void PcCreationHooks::GetPartyPool(int fromIngame)
{
	int& uiPartypoolWidgetId = temple::GetRef<int>(0x10BF1764);
	int& uiPcCreationMainWndId = temple::GetRef<int>(0x10BDD690);
	int& uiPartyCreationNotFromShopmap = temple::GetRef<int>(0x10BF24E0);
	LocAndOffsets& locToCreatePcs = temple::GetRef<LocAndOffsets>(0x10BF17A8);


	uiManager->SetHidden(uiPartypoolWidgetId, false);
	uiManager->BringToFront(uiPartypoolWidgetId);
	uiManager->BringToFront(uiPcCreationMainWndId);
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
	uiManager->SetButtonState(temple::GetRef<int>(0x10BF2408), LgcyButtonState::Disabled); // Add
	uiManager->SetButtonState(temple::GetRef<int>(0x10BF2538), LgcyButtonState::Disabled); // VIEW
	uiManager->SetButtonState(temple::GetRef<int>(0x10BF2410), LgcyButtonState::Disabled); // RENAME
	uiManager->SetButtonState(temple::GetRef<int>(0x10BF239C), LgcyButtonState::Disabled); // DELETE

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
		uiManager->SetHidden(temple::GetRef<int>(0x10BDB8E0), true);
	}
	else
	{
		uiManager->SetHidden(temple::GetRef<int>(0x10BDB8E0), false);
	}
	auto UiUtilityBarHide = temple::GetRef<void()>(0x1010EEC0);
	UiUtilityBarHide();
}

int PcCreationHooks::PartyPoolLoader()
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
}
BOOL PcCreationHooks::StatsIncreaseBtnMsg(int widId, TigMsg * msg) {

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;

	auto msg_ = (TigMsgWidget*)msg;
	if (msg_->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto idx = WidgetIdIndexOf(widId, temple::GetRef<int[]>(0x10C45310), 6);
	if (idx == -1)
		return TRUE;

	auto &selPkt = GetCharEdSelPkt();
	auto abilityLvl = selPkt.abilityStats[idx];
	auto cost = 1;
	if (abilityLvl >= 16)
		cost = 3;
	else if (abilityLvl >= 14)
		cost = 2;

	auto &pbPoints = temple::GetRef<int>(0x10C453F4);
	if (pbPoints >= cost && (abilityLvl < 18 || config.laxRules)) {
		pbPoints -= cost;
		selPkt.abilityStats[idx]++;
		StatsUpdateBtns();
		temple::GetRef<void(__cdecl)(int)>(0x1011BC70)(0);
	}
	return TRUE;
}

BOOL PcCreationHooks::StatsDecreaseBtnMsg(int widId, TigMsg * msg) {

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;

	auto msg_ = (TigMsgWidget*)msg;
	if (msg_->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto idx = WidgetIdIndexOf(widId, temple::GetRef<int[]>(0x10C44DA8), 6);
	if (idx == -1)
		return TRUE;

	auto &selPkt = GetCharEdSelPkt();
	auto abilityLvl = selPkt.abilityStats[idx];
	auto cost = 1;
	if (abilityLvl >= 17)
		cost = 3;
	else if (abilityLvl >= 15)
		cost = 2;

	auto &pbPoints = temple::GetRef<int>(0x10C453F4);
	if (pbPoints < config.pointBuyPoints && (abilityLvl > 8 || config.laxRules)) {
		pbPoints += cost;
		selPkt.abilityStats[idx]--;
		StatsUpdateBtns();
		temple::GetRef<void(__cdecl)(int)>(0x1011BC70)(0);
	}
	return TRUE;
}

BOOL PcCreationHooks::StatsUpdateBtns() {

	auto textBuf = temple::GetRef<char[]>(0x10C44C34);
	auto &pbPoints = temple::GetRef<int>(0x10C453F4);
	_snprintf(textBuf, 16, "%d@1/%d", pbPoints, config.pointBuyPoints);

	auto isIronman = temple::GetRef<BOOL(__cdecl)()>(0x10003860)();
	auto isPointBuyMode = temple::GetRef<BOOL(__cdecl)()>(0x1011B730)();


	// hide/show basic/advanced toggle button
	if (isIronman) {
		if (isPointBuyMode) {
			temple::GetRef<void(__cdecl)()>(0x1018B500)(); // pointbuy toggle
		}
		uiManager->SetHidden(temple::GetRef<int>(0x10C44C48), true); // hide toggle button
	}
	else {
		uiManager->SetHidden(temple::GetRef<int>(0x10C44C48), false); // show toggle button
	}

	auto &selPkt = GetCharEdSelPkt();
	for (auto i = 0; i < 6; i++) {
		auto abLvl = selPkt.abilityStats[i];

		// increase btn
		{
			auto incBtnId = temple::GetRef<int[6]>(0x10C45310)[i];
			auto cost = 1;
			if (abLvl >= 16)
				cost = 3;
			else if (abLvl >= 14)
				cost = 2;
			if (pbPoints < cost || (abLvl == 18 && !config.laxRules))
				uiManager->SetButtonState(incBtnId, LgcyButtonState::Disabled);
			else
				uiManager->SetButtonState(incBtnId, LgcyButtonState::Normal);
			uiManager->SetHidden(incBtnId, isPointBuyMode == 0);

		}

		// dec btn
		{
			auto decBtnId = temple::GetRef<int[6]>(0x10C44DA8)[i];
			auto cost = 1;
			if (abLvl >= 17)
				cost = 3;
			else if (abLvl >= 15)
				cost = 2;

			if (pbPoints >= config.pointBuyPoints || (abLvl == 8 && !config.laxRules) || abLvl <= 5)
				uiManager->SetButtonState(decBtnId, LgcyButtonState::Disabled);
			else
				uiManager->SetButtonState(decBtnId, LgcyButtonState::Normal);
			uiManager->SetHidden(decBtnId, isPointBuyMode == 0);
		}

	}

	auto rerollBtnId = temple::GetRef<int>(0x10C45460);
	uiManager->SetHidden(rerollBtnId, isPointBuyMode != FALSE);

	return isPointBuyMode;
}

void PcCreationHooks::AutoAddSpellsMemorizedToGroup() {
	auto N = party.GroupPCsLen();
	for (auto i = 0u; i < N; i++) {
		auto dude = party.GroupPCsGetMemberN(i);
		temple::GetRef<void(__cdecl)(objHndl)>(0x1011E920)(dude);
	}
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

int __declspec(naked) HookedUsercallFeatMultiselectSub_101822A0()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}
	__asm {
		push eax;
		call HookedFeatMultiselectSub_101822A0;
		add esp, 4;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx}
	__asm retn;
}

int HookedFeatMultiselectSub_101822A0(feat_enums feat)
{
	if ((feat >= FEAT_EXOTIC_WEAPON_PROFICIENCY && feat <= FEAT_WEAPON_SPECIALIZATION) || feat == FEAT_WEAPON_FINESSE_DAGGER)
	{
		__asm mov eax, feat;
		return OrgFeatMultiselectSub_101822A0();
	}
	return 0;
}
