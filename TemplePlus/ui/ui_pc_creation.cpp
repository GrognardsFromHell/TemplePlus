#include "stdafx.h"
#include "common.h"
#include <EASTL/hash_map.h>
#include <EASTL/fixed_string.h>
#include "config/config.h"
#include "d20.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"
#include "ui/ui.h"
#include "ui_render.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include <tig/tig_texture.h>
#include <tig/tig_font.h>
#include "graphics/imgfile.h"
#include "graphics/render_hooks.h"

#include "party.h"
#include "tig/tig_msg.h"
#include <location.h>
#include <tio/tio.h>
#include <mod_support.h>
#include <gamesystems/d20/d20stats.h>
#include <gamesystems/objects/objsystem.h>
#include <gamesystems/d20/d20_help.h>
#include <critter.h>
#include <infrastructure/elfhash.h>
#include <condition.h>
#include <tig/tig_mouse.h>
#include <gamesystems/deity/legacydeitysystem.h>
#include <infrastructure/keyboard.h>
#include "combat.h"
#include "ui_assets.h"
#include "ui_pc_creation.h"
#include "d20_race.h"
#include "widgets/widgets.h"
#include "ui_legacysystems.h"
#include "ui_systems.h"
#include "widgets/widget_styles.h"
#include "temple/meshes.h"
#include "animgoals/anim.h"


const Race RACE_INVALID = (Race)0xFFFFFFFF;
const int GENDER_INVALID = 2;


struct PartyCreationPc
{
	int flags;
	int dataSize;
	int* data;
	int fieldC;
	ObjectId objId;
	int nameSize;
	char* name;
	char fileName[260];
	int portraitId;
	int gender;
	int classEnum;
	int race;
	Alignment alignment;
	int hpMax;
	int field14C;
	objHndl handle;
};

const int testSizeOfCreationPc = sizeof(PartyCreationPc); // 344  0x158
temple::GlobalStruct<LgcyChargenSystem, 0x102F7938> lgcySystems;

UiPcCreation uiPcCreation;

struct PcCreationUiAddresses : temple::AddressTable
{

	int * mainWindowWidgetId; // 10BF0ED4
	int * pcPortraitsWidgetIds; // 10BF0EBC  array of 5 entries


	int * dword_10C75F30;
	int * featsMultiselectNum_10C75F34;
	feat_enums * featMultiselect_10C75F38;
	int *dword_10C76AF0;
	LgcyWidget* widg_10C77CD0;
	int * dword_10C77D50;
	int * dword_10C77D54;
	int *widIdx_10C77D80;
	feat_enums * featsMultiselectList;
	feat_enums * feat_10C79344;
	int * widgId_10C7AE14;
	char* (__cdecl*sub_10182760)(feat_enums featEnums);
	int(__cdecl*sub_101F87B0)(int widIdx, LgcyWidget* widg);
	int(__cdecl*sub_101F8E40)(int);
	
	CharEditorSelectionPacket * charEdSelPkt;
	MesHandle* pcCreationMes;

	PcCreationUiAddresses()
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
		rebase(sub_101F87B0, 0x101F87B0);
		rebase(sub_101F8E40, 0x101F8E40);

		rebase(pcCreationMes, 0x11E72EF0);
		rebase(charEdSelPkt, 0x11E72F00);
	}

} addresses;


#pragma region PC Creation System

template <typename Type, typename... Args>
std::unique_ptr<Type> UiPcCreationSys::InitializeSystem(Args&&... args) {
	logger->info("Loading PC Creation system {}", Type::Name);

	auto result(std::make_unique<Type>(std::forward<Args>(args)...));

	return std::move(result);
}


UiPcCreationSys::UiPcCreationSys(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10120420);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system pc_creation");
	}

	mRace = InitializeSystem<RaceChargen>(config);
	//mClass = InitializeSystem<ClassChargen>(config);
}
UiPcCreationSys::~UiPcCreationSys() {
	auto shutdown = temple::GetPointer<void()>(0x1011ebc0);
	shutdown();
	this;

	mRace->Reset(uiPcCreation.GetCharEditorSelPacket());
	//mClass->Reset(uiPcCreation.GetCharEditorSelPacket());
}
void UiPcCreationSys::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10120b30);
	resize(&resizeArg);
}
const std::string &UiPcCreationSys::GetName() const {
	static std::string name("pc_creation");
	return name;
}

void UiPcCreationSys::Start()
{
	static auto ui_pc_creation_start = temple::GetPointer<int()>(0x1011fdc0);
	ui_pc_creation_start();
}

bool UiPcCreationSys::IsVisible() {
	return mIsHidden == 0;
}

#pragma endregion

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

	if (feat == FEAT_WEAPON_FINESSE){
		if (addresses.charEdSelPkt->feat0 == FEAT_WEAPON_FINESSE_DAGGER
			|| addresses.charEdSelPkt->feat1 == FEAT_WEAPON_FINESSE_DAGGER
			|| addresses.charEdSelPkt->feat2 == FEAT_WEAPON_FINESSE_DAGGER
			)
			return 0;
	}

	if (feat <= FEAT_NONE || feat > FEAT_MONK_PERFECT_SELF)
		return feats.FeatPrereqsCheck(*feats.charEditorObjHnd, feat, featArray, featArrayLen, (Stat)0, addresses.charEdSelPkt->statBeingRaised);

	// the vanilla multiselect range

	return feats.FeatPrereqsCheck(*feats.charEditorObjHnd, feat, featArray, featArrayLen, (Stat)0, addresses.charEdSelPkt->statBeingRaised);
}


objHndl UiPcCreation::GetEditedChar(){
	return chargen.GetEditedChar();
}

CharEditorSelectionPacket & UiPcCreation::GetCharEditorSelPacket(){
	return chargen.GetCharEditorSelPacket();
}

LgcyWindow & UiPcCreation::GetPcCreationWnd(){
	return temple::GetRef<LgcyWindow>(0x11E73E40);
}

LgcyWindow & UiPcCreation::GetStatsWnd(){
	return temple::GetRef<LgcyWindow>(0x11E72BA0);
}

int & UiPcCreation::GetState(){
	return temple::GetRef<int>(0x102F7D68);
}


Alignment UiPcCreation::GetPartyAlignment(){
	return temple::GetRef<Alignment>(0x11E741A8);
}

void UiPcCreation::PrepareNextStages(){
}

void UiPcCreation::ResetNextStages(int systemId){
	temple::GetRef<void(__cdecl)(int)>(0x1011BC70)(systemId);
}

void UiPcCreation::ToggleClassRelatedStages(){
	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();
	auto classCode = selPkt.classCode;
	auto lvlNew = 1;
	auto &stateBtnIds = temple::GetRef<int[CG_STAGE_COUNT]>(0x10BDC434);


	uiManager->SetButtonState(stateBtnIds[CG_Stage_Abilities], LgcyButtonState::Disabled); // class features - off by default; todo expand this


	mIsSelectingBonusFeat = false;
	// feats and features
	if (classCode >= stat_level_barbarian) {

		auto classLvlNew = 1;

		if (d20ClassSys.IsSelectingFeatsOnLevelup(handle, classCode)) {
			mIsSelectingBonusFeat = true;
		}


		if (classCode == stat_level_cleric) {
			uiManager->SetButtonState(stateBtnIds[CG_Stage_Abilities], LgcyButtonState::Normal); // features
		}
		if (classCode == stat_level_ranger) {
			uiManager->SetButtonState(stateBtnIds[CG_Stage_Abilities], LgcyButtonState::Normal); // features
		}
		if (classCode == stat_level_wizard) {
			uiManager->SetButtonState(stateBtnIds[CG_Stage_Abilities], LgcyButtonState::Normal); // wizard special school
		}
	}

	// Spells
	if (d20ClassSys.IsSelectingSpellsOnLevelup(handle, classCode)) {
		uiManager->SetButtonState(stateBtnIds[CG_Stage_Spells], LgcyButtonState::Normal);
	}
	else
	{
		uiManager->SetButtonState(stateBtnIds[CG_Stage_Spells], LgcyButtonState::Disabled);
	};

}



#pragma region Class
void UiPcCreation::HeightShow() {
	static auto updateMinMaxHeights = temple::GetRef<void(__cdecl)(int, int)>(0x10189350);
	auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
	auto minHeight = d20RaceSys.GetMinHeight(selPkt.raceId, (Gender)selPkt.genderId);
	auto maxHeight = d20RaceSys.GetMaxHeight(selPkt.raceId, (Gender)selPkt.genderId);
	updateMinMaxHeights(minHeight, maxHeight);

	auto minWeight = d20RaceSys.GetMinWeight(selPkt.raceId, (Gender)selPkt.genderId);
	auto maxWeight = d20RaceSys.GetMaxWeight(selPkt.raceId, (Gender)selPkt.genderId);
	temple::GetRef<int>(0x10C42E24) = minWeight;
	temple::GetRef<int>(0x10C42E08) = maxWeight;
	static auto updateCharHeight = temple::GetRef<void(__cdecl)()>(0x10189530);
	updateCharHeight();

	auto wndId = temple::GetRef<LgcyWidgetId>(0x10C43160);
	uiManager->SetHidden(wndId, false);
	uiManager->BringToFront(wndId);
	
}

BOOL UiPcCreation::ClassSystemInit(UiSystemConf & conf){
	if (textureFuncs.RegisterTexture("art\\interface\\pc_creation\\buttonbox.tga", &buttonBox))
		return 0;

	for (auto it : d20ClassSys.baseClassEnums) {
		auto className = _strdup(d20Stats.GetStatName((Stat)it));
		classNamesUppercase[it] = className;
		for (auto &letter : classNamesUppercase[it]) {
			letter = toupper(letter);
		}
		classBtnMapping.push_back(it);
	}
	mPageCount = classBtnMapping.size() / 11;
	if (mPageCount * 11u < classBtnMapping.size())
		mPageCount++;

	return ClassWidgetsInit();
}

BOOL UiPcCreation::ClassWidgetsInit(){
	static LgcyWindow classWnd(219, 50, 431, 250);
	classWnd.x = GetPcCreationWnd().x + 219;
	classWnd.y = GetPcCreationWnd().y + 50;
	classWnd.flags = 1;
	classWnd.render = [](int widId) { uiPcCreation.StateTitleRender(widId); };
	classWndId = uiManager->AddWindow(classWnd);

	int coloff = 0, rowoff = 0;

	for (auto it : d20ClassSys.vanillaClassEnums) {
		// class buttons
		LgcyButton classBtn("Class btn", classWndId, 81 + coloff, 42 + rowoff, 130, 20);
		coloff = 139 - coloff;
		if (!coloff)
			rowoff += 29;
		if (rowoff == 5 * 29) // the bottom button
			coloff = 69;

		classBtnRects.push_back(TigRect(classBtn.x, classBtn.y, classBtn.width, classBtn.height));
		classBtn.x += classWnd.x; classBtn.y += classWnd.y;
		classBtn.render = [](int id) {uiPcCreation.ClassBtnRender(id); };
		classBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.ClassBtnMsg(id, msg); };
		classBtn.SetDefaultSounds();
		classBtnIds.push_back(uiManager->AddButton(classBtn, classWndId));

		//rects
		classBtnFrameRects.push_back(TigRect(classBtn.x - 5, classBtn.y - 5, classBtn.width + 10, classBtn.height + 10));


		UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		auto classMeasure = UiRenderer::MeasureTextSize(classNamesUppercase[it].c_str(), bigBtnTextStyle);
		TigRect rect(classBtn.x + (110 - classMeasure.width) / 2 - classWnd.x,
			classBtn.y + (20 - classMeasure.height) / 2 - classWnd.y,
			classMeasure.width, classMeasure.height);
		classTextRects.push_back(rect);
		UiRenderer::PopFont();
	}

	const int nextBtnXoffset = 329;
	const int nextBtnYoffset = 205;
	const int prevBtnXoffset = 38;
	classNextBtnTextRect = classNextBtnRect = TigRect(classWnd.x + nextBtnXoffset, classWnd.y + nextBtnYoffset, 55, 20);
	classPrevBtnTextRect = classPrevBtnRect = TigRect(classWnd.x + prevBtnXoffset, classWnd.y + nextBtnYoffset, 55, 20);
	classNextBtnFrameRect = TigRect(classWnd.x + nextBtnXoffset - 3, classWnd.y + nextBtnYoffset - 5, 55 + 6, 20 + 10);
	classPrevBtnFrameRect = TigRect(classWnd.x + prevBtnXoffset - 3, classWnd.y + nextBtnYoffset - 5, 55 + 6, 20 + 10);
	classNextBtnTextRect.x -= classWnd.x; classNextBtnTextRect.y -= classWnd.y;
	classPrevBtnTextRect.x -= classWnd.x; classPrevBtnTextRect.y -= classWnd.y;

	LgcyButton nextBtn("Class Next Button", classWndId, classWnd.x + nextBtnXoffset, classWnd.y + nextBtnYoffset-4, 55, 20);
	nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {
		if (uiPcCreation.classWndPage < uiPcCreation.mPageCount)
			uiPcCreation.classWndPage++;
		uiPcCreation.ClassSetPermissibles();
		return 1; };
	nextBtn.render = [](int id) { uiPcCreation.ClassNextBtnRender(id); };
	nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiPcCreation.ClassNextBtnMsg(widId, msg); };
	nextBtn.SetDefaultSounds();
	classNextBtn = uiManager->AddButton(nextBtn, classWndId);

	LgcyButton prevBtn("Class Prev. Button", classWndId, classWnd.x + prevBtnXoffset, classWnd.y + nextBtnYoffset - 4, 55, 20);
	prevBtn.render = [](int id) { uiPcCreation.ClassPrevBtnRender(id); };
	prevBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiPcCreation.ClassPrevBtnMsg(widId, msg); };
	prevBtn.SetDefaultSounds();
	classPrevBtn = uiManager->AddButton(prevBtn, classWndId);
	
	return TRUE;
	
}

void UiPcCreation::ClassWidgetsFree(){
	for (auto it : classBtnIds) {
		uiManager->RemoveChildWidget(it);
	}
	classBtnIds.clear();
	uiManager->RemoveChildWidget(classNextBtn);
	uiManager->RemoveChildWidget(classPrevBtn);
	uiManager->RemoveWidget(classWndId);
}

BOOL UiPcCreation::ClassShow(){
	uiManager->SetHidden(classWndId, false);
	uiManager->BringToFront(classWndId);
	return 1;
}

BOOL UiPcCreation::ClassHide(){
	uiManager->SetHidden(classWndId, true);
	return 0;
}

BOOL UiPcCreation::ClassWidgetsResize(UiResizeArgs & args){
	for (auto it : classBtnIds) {
		uiManager->RemoveChildWidget(it);
	}
	classBtnIds.clear();
	uiManager->RemoveChildWidget(classNextBtn);
	uiManager->RemoveChildWidget(classPrevBtn);
	uiManager->RemoveWidget(classWndId);
	classBtnFrameRects.clear();
	classBtnRects.clear();
	classTextRects.clear();
	return ClassWidgetsInit();
}

BOOL UiPcCreation::ClassCheckComplete(){
	auto &selPkt = GetCharEditorSelPacket();
	return (BOOL)(selPkt.classCode != 0);
}

void UiPcCreation::ClassBtnEntered(){
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.classCode){
		ClassScrollboxTextSet(selPkt.classCode);
	}
	else{
		ButtonEnteredHandler(ElfHash::Hash("TAG_CHARGEN_CLASS"));
	}
}

void UiPcCreation::ClassActivate(){
	chargen.SetIsNewChar(true);
	ClassSetPermissibles();
}

void UiPcCreation::ClassFinalize(CharEditorSelectionPacket & selPkt, objHndl & handle){
	auto obj = objSystem->GetObject(handle);
	obj->ClearArray(obj_f_critter_level_idx);
	obj->SetInt32(obj_f_critter_level_idx, 0, selPkt.classCode);
	d20StatusSys.D20StatusRefresh(handle);
	critterSys.GenerateHp(handle);
}

void UiPcCreation::ClassBtnRender(int widId) {
	auto idx = WidgetIdIndexOf(widId, &classBtnIds[0], classBtnIds.size());
	if (idx == -1)
		return;

	auto page = GetClassWndPage();
	auto classCode = GetClassCodeFromWidgetAndPage(idx, page);
	if (classCode == (Stat)-1)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classBtnFrameRects[idx], srcRect);

	auto btnState = uiManager->GetButtonState(widId);
	if (btnState != LgcyButtonState::Disabled && btnState != LgcyButtonState::Down)
	{
		auto &selPkt = GetCharEditorSelPacket();
		if (selPkt.classCode == classCode)
			btnState = LgcyButtonState::Released;
		else
			btnState = btnState == LgcyButtonState::Hovered ? LgcyButtonState::Hovered : LgcyButtonState::Normal;
	}

	auto texId = temple::GetRef<int[15]>(0x11E74140)[(int)btnState];
	static TigRect srcRect2(1, 1, 110, 20);
	auto &rect = classBtnRects[idx];
	UiRenderer::DrawTextureInWidget(classWndId, texId, rect, srcRect2);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto textt = classNamesUppercase[classCode].c_str();

	auto textMeas = UiRenderer::MeasureTextSize(textt, bigBtnTextStyle);
	TigRect classTextRect(rect.x + (rect.width - textMeas.width) / 2,
		rect.y + (rect.height - textMeas.height) / 2,
		textMeas.width, textMeas.height);

	UiRenderer::DrawTextInWidget(classWndId, textt, classTextRect, bigBtnTextStyle);
	UiRenderer::PopFont();
}

BOOL UiPcCreation::ClassBtnMsg(int widId, TigMsg * msg) {

	if (msg->type != TigMsgType::WIDGET)
		return 0;

	auto idx = WidgetIdIndexOf(widId, &classBtnIds[0], classBtnIds.size());
	if (idx == -1)
		return 0;

	auto _msg = (TigMsgWidget*)msg;
	auto classCode = GetClassCodeFromWidgetAndPage(idx, GetClassWndPage());
	if (classCode == (Stat)-1)
		return 0;

	auto handle = GetEditedChar();
	auto obj = objSystem->GetObject(handle);

	if (_msg->widgetEventType == TigMsgWidgetEvent::MouseReleased) {

		if (helpSys.IsClickForHelpActive()) {
			helpSys.PresentWikiHelp(HELP_IDX_CLASSES + classCode - stat_level_barbarian, D20HelpType::Classes);
			return TRUE;
		}
		GetCharEditorSelPacket().classCode = classCode;
		obj->ClearArray(obj_f_critter_level_idx);
		obj->SetInt32(obj_f_critter_level_idx, 0, classCode);
		d20StatusSys.D20StatusRefresh(handle);
		critterSys.GenerateHp(handle);

		ToggleClassRelatedStages();
		ResetNextStages(CG_Stage_Class);
		return TRUE;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Exited) {
		ClassBtnEntered();
		return TRUE;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Entered) {

		auto isValid = true;
		if (!IsCastingStatSufficient(classCode)) {
			isValid = false;
		}

		if (!IsAlignmentOk(classCode))
			isValid = false;

		if (!isValid) {
			temple::GetRef<void(__cdecl)(Stat)>(0x1011B990)(classCode); // sets text in the scrollbox for why you can't pick the class 
			return TRUE;
		}


		ClassScrollboxTextSet(classCode); // ChargenClassScrollboxTextSet  (class short description)
		return TRUE;
	}


	return 0;
}

BOOL UiPcCreation::ClassNextBtnMsg(int widId, TigMsg * msg) {

	if (!config.nonCoreMaterials)
		return FALSE;

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked) {
		if (classWndPage < mPageCount - 1)
			classWndPage++;
		ClassSetPermissibles();
		return TRUE;
	}

	/*if (_msg->widgetEventType == TigMsgWidgetEvent::Exited) {
	temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)("");
	return 1;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Entered) {
	auto textboxText = fmt::format("Prestige Classes");
	if (textboxText.size() >= 1024)
	textboxText[1023] = 0;
	strcpy(temple::GetRef<char[1024]>(0x10C80CC0), &textboxText[0]);
	temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C80CC0));
	return 1;
	}*/

	return FALSE;
}

BOOL UiPcCreation::ClassPrevBtnMsg(int widId, TigMsg * msg)
{

	if (!config.nonCoreMaterials)
		return FALSE;

	if (msg->type != TigMsgType::WIDGET)
		return 0;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked) {
		if (classWndPage > 0)
			classWndPage--;
		ClassSetPermissibles();
		return TRUE;
	}

	return FALSE;
}


void UiPcCreation::ClassNextBtnRender(int widId) {
	if (!config.nonCoreMaterials)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classNextBtnFrameRect, srcRect);

	auto btnState = uiManager->GetButtonState(widId);
	if (btnState != LgcyButtonState::Disabled && btnState != LgcyButtonState::Down) {
		btnState = btnState == LgcyButtonState::Hovered ? LgcyButtonState::Hovered : LgcyButtonState::Normal;
	}

	auto texId = temple::GetRef<int[15]>(0x11E74140)[(int)btnState];
	static TigRect srcRect2(1, 1, 110, 20);
	UiRenderer::DrawTexture(texId, classNextBtnRect, srcRect2);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto textt = fmt::format("NEXT");
	auto textMeas = UiRenderer::MeasureTextSize(textt, bigBtnTextStyle);
	TigRect textRect(classNextBtnTextRect.x + (classNextBtnTextRect.width - textMeas.width) / 2,
		classNextBtnTextRect.y + (classNextBtnTextRect.height - textMeas.height) / 2,
		textMeas.width, textMeas.height);
	UiRenderer::DrawTextInWidget(classWndId, textt, textRect, bigBtnTextStyle);
	UiRenderer::PopFont();

}

void UiPcCreation::ClassPrevBtnRender(int widId) {
	if (!config.nonCoreMaterials)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classPrevBtnFrameRect, srcRect);

	auto btnState = uiManager->GetButtonState(widId);
	if (btnState != LgcyButtonState::Disabled && btnState != LgcyButtonState::Down) {
		btnState = btnState == LgcyButtonState::Hovered ? LgcyButtonState::Hovered : LgcyButtonState::Normal;
	}

	auto texId = temple::GetRef<int[15]>(0x11E74140)[(int)btnState];
	static TigRect srcRect2(1, 1, 110, 20);
	UiRenderer::DrawTexture(texId, classPrevBtnRect, srcRect2);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto textt = fmt::format("PREV");
	auto textMeas = UiRenderer::MeasureTextSize(textt, bigBtnTextStyle);
	TigRect textRect(classPrevBtnTextRect.x + (classPrevBtnTextRect.width - textMeas.width) / 2,
		classPrevBtnTextRect.y + (classPrevBtnTextRect.height - textMeas.height) / 2,
		textMeas.width, textMeas.height);
	UiRenderer::DrawTextInWidget(classWndId, textt, textRect, bigBtnTextStyle);
	UiRenderer::PopFont();
}

int UiPcCreation::GetClassWndPage() {
	return classWndPage;
}

Stat UiPcCreation::GetClassCodeFromWidgetAndPage(int idx, int page) {
	if (page == 0)
		return (Stat)(stat_level_barbarian + idx);

	auto idx2 = idx + page * 11u;
	if (idx2 >= classBtnMapping.size())
		return (Stat)-1;
	return (Stat)classBtnMapping[idx2];
}

#pragma endregion

#pragma region Feats
BOOL UiPcCreation::FeatsSystemInit(UiSystemConf& conf){

	auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
	MesLine mesline;

	TigTextStyle baseStyle;
	baseStyle.flags = 0x4000;
	baseStyle.field2c = -1;
	baseStyle.shadowColor = &blackColorRect;
	baseStyle.field0 = 0;
	baseStyle.kerning = 1;
	baseStyle.leading = 0;
	baseStyle.tracking = 3;
	baseStyle.textColor = baseStyle.colors2 = baseStyle.colors4 = &whiteColorRect;

	featsCenteredStyle = featsGreyedStyle = featsNormalTextStyle = featsExistingTitleStyle = featsGoldenStyle = featsClassStyle = baseStyle;

	featsClassStyle.textColor = featsClassStyle.colors2 = featsClassStyle.colors4 = &darkGreenColorRect;

	featsCenteredStyle.flags = 0x10;

	static ColorRect goldenColor(0xFFFFD919);
	featsGoldenStyle.colors2 = featsGoldenStyle.colors4 = featsGoldenStyle.textColor = &goldenColor;

	static ColorRect greyColor(0xFF5D5D5D);
	featsGreyedStyle.colors2 = featsGreyedStyle.colors4 = featsGreyedStyle.textColor = &greyColor;

#pragma region Titles and strings
	// Feats Available title
	mesline.key = 19000;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsAvailTitleString.append(mesline.value);

	// Class Feats title
	mesline.key = 19002;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsExistingTitleString.append(mesline.value);

	// Feats title
	mesline.key = 19001;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsTitleString.append(mesline.value);

	// Class Bonus title
	mesline.key = 19003;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsClassBonusTitleString.append(mesline.value);

	static auto prefabFeatsMasterMesPairs = eastl::hash_map<int, int>({
		{ FEAT_EXOTIC_WEAPON_PROFICIENCY, 19101 },
		{ FEAT_IMPROVED_CRITICAL , 19102 },
		{ FEAT_MARTIAL_WEAPON_PROFICIENCY , 19103 },
		{ FEAT_SKILL_FOCUS , 19104 },
		{ FEAT_WEAPON_FINESSE , 19105 },
		{ FEAT_WEAPON_FOCUS, 19106 },
		{ FEAT_WEAPON_SPECIALIZATION , 19107 },
		{ FEAT_GREATER_WEAPON_FOCUS , 19108 }
	});

	for (auto it : prefabFeatsMasterMesPairs) {
		mesline.key = it.second;
		mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
		featsMasterFeatStrings[it.first].append(mesline.value);
	}
	featsMasterFeatStrings[FEAT_GREATER_WEAPON_SPECIALIZATION].append(feats.GetFeatName(FEAT_GREATER_WEAPON_SPECIALIZATION));


#pragma endregion

	featsbackdrop = new CombinedImgFile("art\\interface\\pc_creation\\meta_backdrop.img");
	if (!featsbackdrop)
		return 0;
	return FeatsWidgetsInit(conf.width, conf.height);
}

BOOL UiPcCreation::FeatsWidgetsInit(int w, int h){

	auto &pcCreationWnd = temple::GetRef<LgcyWindow>(0x11E73E40);
	featsMainWnd = LgcyWindow(pcCreationWnd.x + 219, pcCreationWnd.y + 50, 431, 250);
	featsMainWnd.flags = 1;
	featsMainWnd.render = [](int widId) {uiPcCreation.FeatsWndRender(widId); };
	featsMainWnd.handleMessage = [](int widId, TigMsg*msg) { return uiPcCreation.FeatsWndMsg(widId, msg); };
	featsMainWndId = uiManager->AddWindow(featsMainWnd);

	// multi select wnd
	featsMultiCenterX = (w - 289) / 2;
	featsMultiCenterY = (h - 355) / 2;
	featsMultiSelectWnd = LgcyWindow(0, 0, w, h);
	auto featsMultiRefX = featsMultiCenterX + featsMultiSelectWnd.x;
	auto featsMultiRefY = featsMultiCenterY + featsMultiSelectWnd.y;
	featsMultiSelectWnd.flags = 1;
	featsMultiSelectWnd.render = [](int widId) {uiPcCreation.FeatsMultiSelectWndRender(widId); };
	featsMultiSelectWnd.handleMessage = [](int widId, TigMsg*msg) { return uiPcCreation.FeatsMultiSelectWndMsg(widId, msg); };
	featsMultiSelectWndId = uiManager->AddWindow(featsMultiSelectWnd);
	//scrollbar
	featsMultiSelectScrollbar.Init(256, 71, 219);
	featsMultiSelectScrollbar.parentId = featsMultiSelectWndId;
	featsMultiSelectScrollbar.x += featsMultiRefX;
	featsMultiSelectScrollbar.y += featsMultiRefY;
	featsMultiSelectScrollbarId = uiManager->AddScrollBar(featsMultiSelectScrollbar, featsMultiSelectWndId);

	//ok btn
	{
		LgcyButton multiOkBtn("Feats Multiselect Ok Btn", featsMultiSelectWndId, 29, 307, 110, 22);
		multiOkBtn.x += featsMultiRefX; multiOkBtn.y += featsMultiRefY;
		featMultiOkRect = TigRect(multiOkBtn.x, multiOkBtn.y, multiOkBtn.width, multiOkBtn.height);
		featMultiOkTextRect = TigRect(multiOkBtn.x, multiOkBtn.y + 4, multiOkBtn.width, multiOkBtn.height - 8);
		multiOkBtn.render = [](int id) {uiPcCreation.FeatsMultiOkBtnRender(id); };
		multiOkBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.FeatsMultiOkBtnMsg(id, msg); };
		multiOkBtn.renderTooltip = nullptr;
		multiOkBtn.SetDefaultSounds();
		featsMultiOkBtnId = uiManager->AddButton(multiOkBtn, featsMultiSelectWndId);
	}

	//cancel btn
	{
		LgcyButton multiCancelBtn("Feats Multiselect Cancel Btn", featsMultiSelectWndId, 153, 307, 110, 22);
		multiCancelBtn.x += featsMultiRefX; multiCancelBtn.y += featsMultiRefY;
		featMultiCancelRect = TigRect(multiCancelBtn.x, multiCancelBtn.y, multiCancelBtn.width, multiCancelBtn.height);
		featMultiCancelTextRect = TigRect(multiCancelBtn.x, multiCancelBtn.y + 4, multiCancelBtn.width, multiCancelBtn.height - 8);
		multiCancelBtn.render = [](int id) {uiPcCreation.FeatsMultiCancelBtnRender(id); };
		multiCancelBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.FeatsMultiCancelBtnMsg(id, msg); };
		multiCancelBtn.renderTooltip = nullptr;
		multiCancelBtn.SetDefaultSounds();
		featsMultiCancelBtnId = uiManager->AddButton(multiCancelBtn, featsMultiSelectWndId);
	}

	featMultiTitleRect = TigRect(featsMultiCenterX, featsMultiCenterY + 20, 289, 12);
	featsMultiSelectBtnIds.clear();
	featsMultiBtnRects.clear();
	auto rowOff = 75;
	for (auto i = 0; i < FEATS_MULTI_BTN_COUNT; i++) {
		LgcyButton featMultiBtn("Feats Multiselect btn", featsMultiSelectWndId, 23, 75 + i*(FEATS_MULTI_BTN_HEIGHT + 2), 233, FEATS_MULTI_BTN_HEIGHT);

		featMultiBtn.x += featsMultiRefX; featMultiBtn.y += featsMultiRefY;
		featMultiBtn.render = [](int id) {uiPcCreation.FeatsMultiBtnRender(id); };
		featMultiBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.FeatsMultiBtnMsg(id, msg); };
		featMultiBtn.renderTooltip = nullptr;
		featMultiBtn.SetDefaultSounds();
		featsMultiSelectBtnIds.push_back(uiManager->AddButton(featMultiBtn, featsMultiSelectWndId));
		featsMultiBtnRects.push_back(TigRect(featMultiBtn.x, featMultiBtn.y, featMultiBtn.width, featMultiBtn.height));
	}

	featsAvailTitleRect = TigRect(17, 17, 185, 10);
	featsTitleRect = TigRect(220, 19, 185, 10);
	featsExistingTitleRect = TigRect(220, 122, 185, 10);
	featsClassBonusRect = TigRect(220, 78, 185, 10);

	// Selectable feats
	featsAvailBtnIds.clear();
	featsBtnRects.clear();
	for (auto i = 0; i < FEATS_AVAIL_BTN_COUNT; i++) {
		LgcyButton featsAvailBtn("Feats Available btn", featsMainWndId, 20, 33 + i*(FEATS_AVAIL_BTN_HEIGHT + 1), 169, FEATS_AVAIL_BTN_HEIGHT);

		featsAvailBtn.x += featsMainWnd.x; featsAvailBtn.y += featsMainWnd.y;
		featsAvailBtn.render = [](int id) {uiPcCreation.FeatsEntryBtnRender(id); };
		featsAvailBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.FeatsEntryBtnMsg(id, msg); };
		featsAvailBtn.renderTooltip = nullptr;
		featsAvailBtn.SetDefaultSounds();
		featsAvailBtnIds.push_back(uiManager->AddButton(featsAvailBtn, featsMainWndId));
		featsBtnRects.push_back(TigRect(featsAvailBtn.x - featsMainWnd.x, featsAvailBtn.y - featsMainWnd.y, featsAvailBtn.width, featsAvailBtn.height));
	}
	//scrollbar
	featsScrollbar.Init(191, 31, 201);
	featsScrollbar.parentId = featsMainWndId;
	featsScrollbar.x += featsMainWnd.x;
	featsScrollbar.y += featsMainWnd.y;
	featsScrollbarId = uiManager->AddScrollBar(featsScrollbar, featsMainWndId);


	// Existing feats
	featsExistingBtnIds.clear();
	featsExistingBtnRects.clear();
	for (auto i = 0; i < FEATS_EXISTING_BTN_COUNT; i++) {
		LgcyButton featsExistingBtn("Feats Existing btn", featsMainWndId, 225, 140 + i*(FEATS_EXISTING_BTN_HEIGHT + 1), 175, FEATS_EXISTING_BTN_HEIGHT);

		featsExistingBtn.x += featsMainWnd.x; featsExistingBtn.y += featsMainWnd.y;
		featsExistingBtn.render = [](int id) {uiPcCreation.FeatsExistingBtnRender(id); };
		featsExistingBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.FeatsExistingBtnMsg(id, msg); };
		featsExistingBtn.renderTooltip = nullptr;
		featsExistingBtn.SetDefaultSounds();
		featsExistingBtnIds.push_back(uiManager->AddButton(featsExistingBtn, featsMainWndId));
		featsExistingBtnRects.push_back(TigRect(featsExistingBtn.x - featsMainWnd.x, featsExistingBtn.y - featsMainWnd.y, featsExistingBtn.width, featsExistingBtn.height));
	}
	//scrollbar
	featsExistingScrollbar.Init(395, 137, 95);
	featsExistingScrollbar.parentId = featsMainWndId;
	featsExistingScrollbar.x += featsMainWnd.x;
	featsExistingScrollbar.y += featsMainWnd.y;
	featsExistingScrollbarId = uiManager->AddScrollBar(featsExistingScrollbar, featsMainWndId);

	featsSelectedBorderRect = TigRect(featsMainWnd.x + 220, featsMainWnd.y + 34, 185, 19);
	featsSelected2BorderRect = TigRect(featsMainWnd.x + 220, featsMainWnd.y + 34 + 20, 185, 19);
	featsClassBonusBorderRect = TigRect(featsMainWnd.x + 220, featsMainWnd.y + 97, 185, 19);
	feat0TextRect = TigRect(223, 35, 185, 12);
	feat1TextRect = TigRect(223, 35 + 21, 185, 12);
	feat2TextRect = TigRect(223, 98, 185, 12);

	return 1;
}

void UiPcCreation::FeatsFree(){
	FeatWidgetsFree();
}

void UiPcCreation::FeatWidgetsFree(){
	for (auto i = 0; i < FEATS_MULTI_BTN_COUNT; i++) {
		uiManager->RemoveChildWidget(featsMultiSelectBtnIds[i]);
	}
	featsMultiSelectBtnIds.clear();

	for (auto i = 0; i < FEATS_AVAIL_BTN_COUNT; i++) {
		uiManager->RemoveChildWidget(featsAvailBtnIds[i]);
	}
	featsAvailBtnIds.clear();

	for (auto i = 0; i < FEATS_EXISTING_BTN_COUNT; i++) {
		uiManager->RemoveChildWidget(featsExistingBtnIds[i]);
	}
	featsExistingBtnIds.clear();

	uiManager->RemoveChildWidget(featsMultiOkBtnId);
	uiManager->RemoveChildWidget(featsMultiCancelBtnId);
	uiManager->RemoveChildWidget(featsMultiSelectScrollbarId);
	uiManager->RemoveChildWidget(featsScrollbarId);
	uiManager->RemoveChildWidget(featsExistingScrollbarId);

	auto wid = uiManager->GetWindow(featsMultiSelectWndId);
	auto wid2 = uiManager->GetWindow(featsMainWndId);

	uiManager->RemoveWidget(featsMultiSelectWndId);
	uiManager->RemoveWidget(featsMainWndId);
}

BOOL UiPcCreation::FeatsWidgetsResize(UiResizeArgs& args){
	FeatWidgetsFree();
	return FeatsWidgetsInit(args.rect1.width, args.rect1.height);
}

BOOL UiPcCreation::FeatsShow(){
	featsMultiSelected = FEAT_NONE;
	uiManager->SetHidden(featsMainWndId, false);
	uiManager->BringToFront(featsMainWndId);
	return 1;
}

BOOL UiPcCreation::FeatsHide()
{
	uiManager->SetHidden(featsMainWndId, true);
	return 0;
}

void UiPcCreation::FeatsActivate()
{
	mFeatsActivated = true;

	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	mIsSelectingBonusFeat = d20ClassSys.IsSelectingFeatsOnLevelup(handle, selPkt.classCode);
	chargen.BonusFeatsClear();
	if (mIsSelectingBonusFeat)
		d20ClassSys.LevelupGetBonusFeats(handle, selPkt.classCode); // can call set_bonus_feats

	feat_enums existingFeats[122];
	auto existingCount = feats.FeatListGet(handle, existingFeats, (Stat)0, (feat_enums)0);

	mExistingFeats.clear();
	for (auto i = 0u; i<existingCount; i++) {
		auto ftEnum = existingFeats[i];
		if (selPkt.feat0 != ftEnum && selPkt.feat1 != ftEnum && selPkt.feat2 != ftEnum)
			mExistingFeats.push_back(FeatInfo(ftEnum));
	}
	static auto featSorter = [](FeatInfo &first, FeatInfo &second) {

		auto firstEnum = (feat_enums)first.featEnum;
		auto secEnum = (feat_enums)second.featEnum;

		auto firstName = uiPcCreation.GetFeatName(firstEnum);
		auto secondName = uiPcCreation.GetFeatName(secEnum);

		return _stricmp(firstName.c_str(), secondName.c_str()) < 0;
	};

	std::sort(mExistingFeats.begin(), mExistingFeats.end(), featSorter);

	featsExistingScrollbar = *uiManager->GetScrollBar(featsExistingScrollbarId);
	featsExistingScrollbar.scrollbarY = 0;
	featsExistingScrollbarY = 0;
	featsExistingScrollbar.yMax = max((int)mExistingFeats.size() - FEATS_EXISTING_BTN_COUNT, 0);
	*uiManager->GetScrollBar(featsExistingScrollbarId) = featsExistingScrollbar;

	// Available feats
	mSelectableFeats.clear();
	for (auto i = 0u; i < NUM_FEATS; i++) {
		auto feat = (feat_enums)i;
		if (!feats.IsFeatEnabled(feat) && !feats.IsFeatMultiSelectMaster(feat))
			continue;
		if (feats.IsFeatRacialOrClassAutomatic(feat))
			continue;
		if (feats.IsFeatPartOfMultiselect(feat))
			continue;
		if (feat == FEAT_NONE)
			continue;
		mSelectableFeats.push_back(FeatInfo(feat));
	}
	for (auto feat : feats.newFeats) {
		if (!feats.IsFeatEnabled(feat) && !feats.IsFeatMultiSelectMaster(feat))
			continue;
		if (!config.nonCoreMaterials && feats.IsNonCore(feat))
			continue;
		if (IsClassBonusFeat(feat)) {
			mSelectableFeats.push_back(FeatInfo(feat));
			continue;
		}
		if (feats.IsFeatRacialOrClassAutomatic(feat))
			continue;
		if (feats.IsFeatPartOfMultiselect(feat))
			continue;
		if (feat == FEAT_NONE)
			continue;

		mSelectableFeats.push_back(FeatInfo(feat));
	}
	std::sort(mSelectableFeats.begin(), mSelectableFeats.end(), featSorter);

	featsScrollbar = *uiManager->GetScrollBar(featsScrollbarId);
	featsScrollbar.scrollbarY = 0;
	featsScrollbarY = 0;
	featsScrollbar.yMax = max((int)mSelectableFeats.size() - FEATS_AVAIL_BTN_COUNT, 0);
	*uiManager->GetScrollBar(featsScrollbarId) = featsScrollbar;
}

BOOL UiPcCreation::FeatsCheckComplete()
{
	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	// is a 3rd level and no feat chosen
	if (IsSelectingNormalFeat() && selPkt.feat0 == FEAT_NONE)
		return FALSE;

	if (IsSelectingSecondFeat() && selPkt.feat1 == FEAT_NONE)
		return FALSE;

	if (IsSelectingBonusFeat() && selPkt.feat2 == FEAT_NONE) // the logic will be handled in the msg callbacks & Python API now
		return FALSE;

	return TRUE;
}

void UiPcCreation::FeatsFinalize(CharEditorSelectionPacket& selPkt, objHndl & handle){

	feats.FeatAdd(handle, selPkt.feat0);
	d20StatusSys.D20StatusRefresh(handle);
	if (selPkt.feat1 != FEAT_NONE){
		feats.FeatAdd(handle, selPkt.feat1);
		d20StatusSys.D20StatusRefresh(handle);
	}
	if (selPkt.feat2 != FEAT_NONE) {
		feats.FeatAdd(handle, selPkt.feat2);
		d20StatusSys.D20StatusRefresh(handle);
	}

}

void UiPcCreation::FeatsReset(CharEditorSelectionPacket& selPkt)
{
	mFeatsActivated = false;
	//mIsSelectingBonusFeat = false; // should not do this here, since then if a user goes back to skills and decreases/increases them, it can cause problems

	selPkt.feat0 = FEAT_NONE;
	selPkt.feat1 = FEAT_NONE;
	if (selPkt.classCode != stat_level_ranger || objects.StatLevelGet(GetEditedChar(), stat_level_ranger) != 1)
		selPkt.feat2 = FEAT_NONE;

	mExistingFeats.clear();
	mSelectableFeats.clear();
	mMultiSelectFeats.clear();
}

BOOL UiPcCreation::FeatsWndMsg(int widId, TigMsg* msg) {
	if (msg->type == TigMsgType::WIDGET) {
		auto msgW = (TigMsgWidget*)msg;
		if (msgW->widgetEventType == TigMsgWidgetEvent::Scrolled) {
			uiManager->ScrollbarGetY(featsScrollbarId, &featsScrollbarY);
			uiManager->ScrollbarGetY(featsExistingScrollbarId, &featsExistingScrollbarY);
		}
		return FALSE;
	}

	if (msg->type != TigMsgType::MOUSE)
		return FALSE;


	auto msgM = (TigMsgMouse*)msg;
	auto &selPkt = GetCharEditorSelPacket();

	if (msgM->buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED && uiManager->IsHidden(featsMultiSelectWndId)) {

		auto putFeat = false;
		feat_enums feat;

		// cycle thru widgets to find the one where the RMB happened
		for (auto i = 0; i < FEATS_AVAIL_BTN_COUNT; i++) {
			if (!featsBtnRects[i].ContainsPoint(msgM->x - featsMainWnd.x, msgM->y - featsMainWnd.y))
				continue;

			auto featIdx = i + featsScrollbarY;
			if (featIdx >= (int)mSelectableFeats.size())
				break;

			feat = (feat_enums)mSelectableFeats[featIdx].featEnum;


			if (IsSelectingNormalFeat() && selPkt.feat0 == FEAT_NONE) {
				selPkt.feat0 = feat;
				putFeat = true;
				break;
			}

			if (IsSelectingSecondFeat() && selPkt.feat1 == FEAT_NONE)
			{
				selPkt.feat1 = feat;
				putFeat = true;
				break;
			}

			if (IsSelectingBonusFeat() && IsClassBonusFeat(feat) && selPkt.feat2 == FEAT_NONE)
			{
				selPkt.feat2 = feat;
				putFeat = true;
				break;
			}
		}

		if (putFeat) {

			if (feats.IsFeatMultiSelectMaster(feat)) {
				FeatsMultiSelectActivate(feat);
			}
			FeatsSanitize();
			if (feat == FEAT_SKILL_MASTERY && selPkt.feat2 == feat) {
				auto skillMasteryActivate = temple::GetRef<void(__cdecl)(objHndl, int(__cdecl*)(int))>(0x1016C2B0);
				skillMasteryActivate(GetEditedChar(), temple::GetRef<int(__cdecl)(int)>(0x101A86D0));
			}
		}

		else if (featsSelectedBorderRect.ContainsPoint(msgM->x, msgM->y)) {
			selPkt.feat0 = FEAT_NONE;
		}
		else if (featsSelected2BorderRect.ContainsPoint(msgM->x, msgM->y)) {
			selPkt.feat1 = FEAT_NONE;
		}
		else if (featsClassBonusBorderRect.ContainsPoint(msgM->x, msgM->y) && IsSelectingBonusFeat()) {
			selPkt.feat2 = FEAT_NONE;
		}

	}

	if (!(msgM->buttonStateFlags & MouseStateFlags::MSF_SCROLLWHEEL_CHANGE))
		return TRUE;

	TigMsgMouse msgCopy = *msgM;
	msgCopy.buttonStateFlags = MouseStateFlags::MSF_SCROLLWHEEL_CHANGE;

	if ((int)msgM->x >= featsMainWnd.x + 3 && (int)msgM->x <= featsMainWnd.x + 188
		&& (int)msgM->y >= featsMainWnd.y + 36 && (int)msgM->y <= featsMainWnd.y + 263) {
		featsScrollbar = *uiManager->GetScrollBar(featsScrollbarId);
		if (featsScrollbar.handleMessage)
			return featsScrollbar.handleMessage(featsScrollbarId, (TigMsg*)&msgCopy);
	}

	if ((int)msgM->x >= featsMainWnd.x + 207 && (int)msgM->x <= featsMainWnd.x + 392
		&& (int)msgM->y >= featsMainWnd.y + 118 && (int)msgM->y <= featsMainWnd.y + 263) {
		featsExistingScrollbar = *uiManager->GetScrollBar(featsExistingScrollbarId);
		if (featsExistingScrollbar.handleMessage)
			return featsExistingScrollbar.handleMessage(featsExistingScrollbarId, (TigMsg*)&msgCopy);
	}

	return FALSE;
}

void UiPcCreation::FeatsWndRender(int widId)
{
	auto &selPkt = GetCharEditorSelPacket();

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	// Feats title
	RenderHooks::RenderRectInt(featsMainWnd.x + 17, featsMainWnd.y + 32, 185, 198, 0xFF5D5D5D);
	UiRenderer::DrawTextInWidget(widId, featsAvailTitleString, featsAvailTitleRect, whiteTextGenericStyle);

	// Feat Slot
	if (IsSelectingNormalFeat()) {
		RenderHooks::RenderRectInt(featsSelectedBorderRect.x, featsSelectedBorderRect.y, featsSelectedBorderRect.width, featsSelectedBorderRect.height, 0xFFFFffff);
		UiRenderer::DrawTextInWidget(widId, featsTitleString, featsTitleRect, featsNormalTextStyle);
		if (selPkt.feat0 != FEAT_NONE) {
			UiRenderer::DrawTextInWidget(widId, GetFeatName(selPkt.feat0), feat0TextRect, GetFeatStyle(selPkt.feat0));
		}
	}
	if (IsSelectingSecondFeat()) {
		RenderHooks::RenderRectInt(featsSelected2BorderRect.x, featsSelected2BorderRect.y, featsSelected2BorderRect.width, featsSelected2BorderRect.height, 0xFFFFffff);
		if (selPkt.feat1 != FEAT_NONE) {
			UiRenderer::DrawTextInWidget(widId, GetFeatName(selPkt.feat1), feat1TextRect, GetFeatStyle(selPkt.feat1));
		}
	}

	// Class Bonus Feat slot
	if (IsSelectingBonusFeat()) {
		// title Class Bonus Feat
		RenderHooks::RenderRectInt(featsClassBonusBorderRect.x, featsClassBonusBorderRect.y, featsClassBonusBorderRect.width, featsClassBonusBorderRect.height, 0xFFFFD919);
		UiRenderer::DrawTextInWidget(widId, featsClassBonusTitleString, featsClassBonusRect, featsGoldenStyle);

		if (selPkt.feat2 != FEAT_NONE) {
			UiRenderer::DrawTextInWidget(widId, GetFeatName(selPkt.feat2), feat2TextRect, GetFeatStyle(selPkt.feat2));
		}
	}

	// Class Feats rect+title
	RenderHooks::RenderRectInt(featsMainWnd.x + 220, featsMainWnd.y + 138, 186, 92, 0xFF5D5D5D);
	UiRenderer::DrawTextInWidget(widId, featsExistingTitleString, featsExistingTitleRect, featsExistingTitleStyle);

	StateTitleRender(widId);

	UiRenderer::PopFont();
}

BOOL UiPcCreation::FeatsEntryBtnMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = WidgetIdIndexOf(widId, &featsAvailBtnIds[0], FEATS_AVAIL_BTN_COUNT);
	auto featIdx = widIdx + featsScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mSelectableFeats.size())
		return FALSE;

	auto featInfo = mSelectableFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = uiManager->GetButton(widId);

	switch (msgW->widgetEventType) {
	case TigMsgWidgetEvent::Clicked:
		if (!FeatAlreadyPicked(feat) && FeatCanPick(feat)) {
			auto origX = msgW->x - btn->x, origY = msgW->y - btn->y;
			auto style = uiPcCreation.GetFeatStyle(feat);
			auto featCallback = [origX, origY, feat, style](int x, int y) {
				std::string text(uiPcCreation.GetFeatName(feat));
				UiRenderer::PushFont(PredefinedFont::PRIORY_12);
				TigRect rect(x - origX, y - origY, 180, uiPcCreation.FEATS_AVAIL_BTN_HEIGHT);
				tigFont.Draw(text.c_str(), rect, style);
				UiRenderer::PopFont();
			};
			mouseFuncs.SetCursorDrawCallback(featCallback, (uint32_t)&featCallback);

		}
		return TRUE;
	case TigMsgWidgetEvent::MouseReleased:
		if (helpSys.IsClickForHelpActive()) {
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
			helpSys.PresentWikiHelp(109 + feat);
			return TRUE;
		}
	case TigMsgWidgetEvent::MouseReleasedAtDifferentButton:
		if (FeatAlreadyPicked(feat) || !FeatCanPick(feat))
			return TRUE;
		mouseFuncs.SetCursorDrawCallback(nullptr, 0);

		// check if inserted into the normal slot
		if (featsSelectedBorderRect.ContainsPoint(msgW->x, msgW->y) && IsSelectingNormalFeat()) {
			selPkt.feat0 = feat;
			if (feats.IsFeatMultiSelectMaster(feat))
				FeatsMultiSelectActivate(feat);
		}
		else if (IsSelectingSecondFeat() && featsSelected2BorderRect.ContainsPoint(msgW->x, msgW->y)) {
			selPkt.feat1 = feat;
			if (feats.IsFeatMultiSelectMaster(feat))
				FeatsMultiSelectActivate(feat);
		}
		// check if inserted into the bonus slot
		else if (IsSelectingBonusFeat()
			&& featsClassBonusBorderRect.ContainsPoint(msgW->x, msgW->y) && IsClassBonusFeat(feat)) {
			selPkt.feat2 = feat;
			if (feats.IsFeatMultiSelectMaster(feat))
				FeatsMultiSelectActivate(feat);
			else if (feat == FEAT_SKILL_MASTERY) {
				auto skillMasteryActivate = temple::GetRef<void(__cdecl)(objHndl, int(__cdecl*)(int))>(0x1016C2B0);
				skillMasteryActivate(GetEditedChar(), temple::GetRef<int(__cdecl)(int)>(0x101A86D0));
			}
		}
		FeatsSanitize();
		return TRUE;
	case TigMsgWidgetEvent::Entered:
		//temple::GetRef<void(int, char*, size_t)>(0x10162A10)(FeatsMultiGetFirst(feat), temple::GetRef<char[1024]>(0x10C76B48), 1024u); // UiTooltipSetForFeat
		//temple::GetRef<void(char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C76B48)); // UiCharTextboxSet
		temple::GetRef<void(__cdecl)(feat_enums)>(0x1011BB50)(FeatsMultiGetFirst(feat));
		return TRUE;
	case TigMsgWidgetEvent::Exited:
		temple::GetRef<void(__cdecl)(char *)>(0x10162C00)(""); // UiCharTextboxSet
		return TRUE;
	default:
		return FALSE;

	}
	return TRUE;
}

void UiPcCreation::FeatsEntryBtnRender(int widId)
{
	auto widIdx = WidgetIdIndexOf(widId, &featsAvailBtnIds[0], FEATS_AVAIL_BTN_COUNT);
	auto featIdx = widIdx + featsScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mSelectableFeats.size())
		return;

	auto featInfo = mSelectableFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(featsMainWndId, GetFeatName(feat), featsBtnRects[widIdx], GetFeatStyle(feat, false));

	UiRenderer::PopFont();
}

BOOL UiPcCreation::FeatsExistingBtnMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = WidgetIdIndexOf(widId, &featsExistingBtnIds[0], FEATS_EXISTING_BTN_COUNT);
	auto featIdx = widIdx + featsExistingScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mExistingFeats.size())
		return FALSE;

	auto featInfo = mExistingFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = uiManager->GetButton(widId);

	switch (msgW->widgetEventType) {
	case TigMsgWidgetEvent::Entered:
		temple::GetRef<void(__cdecl)(feat_enums)>(0x1011BB50)(FeatsMultiGetFirst(feat));
		return TRUE;
	case TigMsgWidgetEvent::Exited:
		temple::GetRef<void(__cdecl)(char *)>(0x10162C00)(""); // UiCharTextboxSet
		return TRUE;
	default:
		return FALSE;

	}
	return TRUE;
}

void UiPcCreation::FeatsExistingBtnRender(int widId)
{
	auto widIdx = WidgetIdIndexOf(widId, &featsExistingBtnIds[0], FEATS_EXISTING_BTN_COUNT);
	auto featIdx = widIdx + featsExistingScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mExistingFeats.size())
		return;

	auto featInfo = mExistingFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(featsMainWndId, GetFeatName(feat), featsExistingBtnRects[widIdx], featsClassStyle);

	UiRenderer::PopFont();
}

void UiPcCreation::FeatsMultiSelectWndRender(int widId)
{
	featsbackdrop->SetX(featsMultiSelectWnd.x + featsMultiCenterX);
	featsbackdrop->SetY(featsMultiSelectWnd.y + featsMultiCenterY);
	featsbackdrop->Render();

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(widId, GetFeatName(mFeatsMultiMasterFeat), featMultiTitleRect, featsCenteredStyle);

	UiRenderer::PopFont();
}

BOOL UiPcCreation::FeatsMultiSelectWndMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET && msg->type != TigMsgType::KEYSTATECHANGE)
		return FALSE;

	uiManager->ScrollbarGetY(featsMultiSelectScrollbarId, &featsMultiSelectScrollbarY);

	return TRUE;
}

void UiPcCreation::FeatsMultiOkBtnRender(int widId)
{
	auto buttonState = uiManager->GetButtonState(widId);

	int texId;
	switch (buttonState) {
	case LgcyButtonState::Normal:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, texId);
		break;
	case LgcyButtonState::Hovered:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, texId);
		break;
	case LgcyButtonState::Down:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, texId);
		break;
	case LgcyButtonState::Disabled:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, texId);
		break;
	default:
		break;
	}

	static TigRect srcRect(1, 1, 110, 22);
	UiRenderer::DrawTextureInWidget(featsMultiSelectWndId, texId, featMultiOkRect, srcRect);


	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(featsMultiSelectWndId, combatSys.GetCombatMesLine(6009), featMultiOkTextRect, featsCenteredStyle);
	UiRenderer::PopFont();
}

BOOL UiPcCreation::FeatsMultiOkBtnMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return FALSE;
	auto msgW = (TigMsgWidget*)msg;
	if (msgW->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto &selPkt = GetCharEditorSelPacket();

	if (featsMultiSelected == FEAT_NONE) {
		if (selPkt.feat0 == mFeatsMultiMasterFeat) {
			selPkt.feat0 = FEAT_NONE;
		}
		if (selPkt.feat1 == mFeatsMultiMasterFeat) {
			selPkt.feat1 = FEAT_NONE;
		}
		if (selPkt.feat2 == mFeatsMultiMasterFeat) {
			selPkt.feat2 = FEAT_NONE;
		}
	}
	else
	{
		if (selPkt.feat2 == mFeatsMultiMasterFeat) {
			selPkt.feat2 = featsMultiSelected;
		}
		else if (selPkt.feat0 == mFeatsMultiMasterFeat) {
			selPkt.feat0 = featsMultiSelected;
		}
		else if (selPkt.feat1 == mFeatsMultiMasterFeat) {
			selPkt.feat1 = featsMultiSelected;
		}
	}

	mFeatsMultiMasterFeat = FEAT_NONE;
	featsMultiSelected = FEAT_NONE;
	uiManager->SetHidden(featsMultiSelectWndId, true);

	return TRUE;
}

void UiPcCreation::FeatsMultiCancelBtnRender(int widId)
{
	auto buttonState = uiManager->GetButtonState(widId);

	int texId;
	switch (buttonState) {
	case LgcyButtonState::Normal:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, texId);
		break;
	case LgcyButtonState::Hovered:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, texId);
		break;
	case LgcyButtonState::Down:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, texId);
		break;
	case LgcyButtonState::Disabled:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, texId);
		break;
	default:
		break;
	}

	static TigRect srcRect(1, 1, 110, 22);
	UiRenderer::DrawTextureInWidget(featsMultiSelectWndId, texId, featMultiCancelRect, srcRect);


	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(featsMultiSelectWndId, combatSys.GetCombatMesLine(6010), featMultiCancelTextRect, featsCenteredStyle);
	UiRenderer::PopFont();
}

BOOL UiPcCreation::FeatsMultiCancelBtnMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return FALSE;
	auto msgW = (TigMsgWidget*)msg;
	if (msgW->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto &selPkt = GetCharEditorSelPacket();

	if (selPkt.feat0 == mFeatsMultiMasterFeat) {
		selPkt.feat0 = FEAT_NONE;
	}
	if (selPkt.feat1 == mFeatsMultiMasterFeat) {
		selPkt.feat1 = FEAT_NONE;
	}
	if (selPkt.feat2 == mFeatsMultiMasterFeat) {
		selPkt.feat2 = FEAT_NONE;
	}

	mFeatsMultiMasterFeat = FEAT_NONE;
	featsMultiSelected = FEAT_NONE;
	uiManager->SetHidden(featsMultiSelectWndId, true);

	return TRUE;
}

void UiPcCreation::FeatsMultiBtnRender(int widId)
{
	auto widIdx = WidgetIdIndexOf(widId, &featsMultiSelectBtnIds[0], FEATS_MULTI_BTN_COUNT);
	auto featIdx = widIdx + featsMultiSelectScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mMultiSelectFeats.size())
		return;

	auto featInfo = mMultiSelectFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;



	auto getFeatShortName = [](feat_enums ft) {

		if (ft > NUM_FEATS)
		{
			auto dummy = 1;
		}

		if (feats.IsFeatMultiSelectMaster(ft))
			return uiPcCreation.GetFeatName(ft);


		auto mesKey = 50000 + ft;

		if (feats.IsFeatPropertySet(ft, FPF_GREAT_WEAP_SPEC_ITEM)) {
			mesKey = 50000 + (ft - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET + FEAT_WEAPON_SPECIALIZATION_GAUNTLET);
		}

		MesLine line(mesKey);
		auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
		auto text = mesFuncs.GetLineById(pcCreationMes, mesKey);
		if (text) {
			return std::string(text);
		}
		else
			return uiPcCreation.GetFeatName(ft);
	};

	auto ftName = getFeatShortName(feat);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(featsMultiSelectWndId, ftName, featsMultiBtnRects[widIdx], GetFeatStyle(feat, false));

	UiRenderer::PopFont();
}

BOOL UiPcCreation::FeatsMultiBtnMsg(int widId, TigMsg* msg)
{
	if (msg->type == TigMsgType::MOUSE)
		return TRUE;

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;


	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = WidgetIdIndexOf(widId, &featsMultiSelectBtnIds[0], FEATS_MULTI_BTN_COUNT);
	auto featIdx = widIdx + featsMultiSelectScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mMultiSelectFeats.size())
		return FALSE;

	auto featInfo = mMultiSelectFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = uiManager->GetButton(widId);

	switch (msgW->widgetEventType) {
	case TigMsgWidgetEvent::MouseReleased:
		if (helpSys.IsClickForHelpActive()) {
			helpSys.PresentWikiHelp(109 + feat);
			return TRUE;
		}
		if (FeatCanPick(feat) && !FeatAlreadyPicked(feat)) {
			featsMultiSelected = feat;
			uiManager->SetButtonState(featsMultiOkBtnId, LgcyButtonState::Normal);
		}
		else
		{
			featsMultiSelected = FEAT_NONE;
			uiManager->SetButtonState(featsMultiOkBtnId, LgcyButtonState::Disabled);
		}
		return TRUE;
	default:
		return FALSE;

	}

	return FALSE;
}

std::string UiPcCreation::GetFeatName(feat_enums feat)
{
	if (feat >= FEAT_EXOTIC_WEAPON_PROFICIENCY && feat <= FEAT_GREATER_WEAPON_FOCUS)
		return featsMasterFeatStrings[feat];

	return std::string(feats.GetFeatName(feat));
}

TigTextStyle& UiPcCreation::GetFeatStyle(feat_enums feat, bool allowMultiple)
{
	auto &selPkt = GetCharEditorSelPacket();
	auto newLvl = 1;

	if ((allowMultiple || !uiPcCreation.FeatAlreadyPicked(feat))
		&& uiPcCreation.FeatCanPick(feat))
	{
		if (uiPcCreation.featsMultiSelected == feat) {
			return uiPcCreation.blueTextStyle;
		}

		if (uiPcCreation.IsClassBonusFeat(feat)) {  // is choosing class bonus right now 
			return uiPcCreation.featsGoldenStyle;
		}
		else if (feats.IsClassFeat(feat))// class Specific feat
		{
			return uiPcCreation.featsClassStyle;
		}
		else
			return uiPcCreation.featsNormalTextStyle;
	}

	return uiPcCreation.featsGreyedStyle;
}

bool UiPcCreation::FeatAlreadyPicked(feat_enums feat) {
	if (feats.IsFeatPropertySet(feat, 0x1)  // can be gained multiple times
		|| feats.IsFeatMultiSelectMaster(feat))
		return false;
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.feat0 == feat || selPkt.feat1 == feat || selPkt.feat2 == feat)
		return true;

	auto handle = GetEditedChar();

	auto isRangerSpecial = IsSelectingRangerSpec();
	return feats.HasFeatCountByClass(handle, feat, selPkt.classCode, isRangerSpecial ? selPkt.feat2 : FEAT_ACROBATIC) != 0;
}

bool UiPcCreation::FeatCanPick(feat_enums feat)
{
	std::vector<feat_enums> featsPicked;
	auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();

	if (selPkt.feat0 != FEAT_NONE) {
		featsPicked.push_back(selPkt.feat0);
	}
	if (selPkt.feat1 != FEAT_NONE) {
		featsPicked.push_back(selPkt.feat1);
	}
	if (selPkt.feat2 != FEAT_NONE) {
		featsPicked.push_back(selPkt.feat2);
	}

	if (IsSelectingBonusFeat() && IsClassBonusFeat(feat)) {
		if (feats.IsFeatPropertySet(feat, FPF_ROGUE_BONUS))
			return true;
		if (uiPcCreation.IsBonusFeatDisregardingPrereqs(feat))
			return true;
	}


	if (!feats.IsFeatMultiSelectMaster(feat)) {
		return feats.FeatPrereqsCheck(handle, feat, featsPicked.size() > 0 ? &featsPicked[0] : nullptr, featsPicked.size(), (Stat)0, selPkt.statBeingRaised) != FALSE;
	}


	// Multiselect Master feats

	auto ftrLvl = objects.StatLevelGet(handle, stat_level_fighter);


	bool hasFocus = false;
	switch (feat) {
	case FEAT_EXOTIC_WEAPON_PROFICIENCY:
		return critterSys.GetBaseAttackBonus(handle) >= 1;
	case FEAT_IMPROVED_CRITICAL:
		return critterSys.GetBaseAttackBonus(handle) >= 8;

	case FEAT_MARTIAL_WEAPON_PROFICIENCY:
	case FEAT_SKILL_FOCUS:
		return true;

	case FEAT_WEAPON_FINESSE:
		if (critterSys.GetBaseAttackBonus(handle) < 1)
			return false;
		for (auto i = (int)FEAT_WEAPON_FINESSE_GAUNTLET; i <= FEAT_WEAPON_FINESSE_NET; i++) {
			if (feats.HasFeatCountByClass(handle, (feat_enums)i, (Stat)0, 0))
				return false;
		}
		for (auto it : featsPicked) {
			if (feats.IsFeatPropertySet(it, FPF_WEAP_FINESSE_ITEM))
				return false;
		}
		return true;

	case FEAT_WEAPON_FOCUS:
		return critterSys.GetBaseAttackBonus(handle) >= 1;

	case FEAT_WEAPON_SPECIALIZATION:

		return (ftrLvl >= 4);


	case FEAT_GREATER_WEAPON_FOCUS:
		if (ftrLvl < 8)
			return false;


		// check if has weapon focus

		for (auto i = (int)FEAT_WEAPON_FOCUS_GAUNTLET; i <= FEAT_WEAPON_FOCUS_RAY; i++) {
			if (feats.HasFeatCountByClass(handle, (feat_enums)i, (Stat)0, 0)) {
				return true;
			}
			// if not, check if it's one of the picked ones
			for (auto it : featsPicked) {
				if (it == (feat_enums)i)
					return true;
			}
		}
		return false;

	case FEAT_GREATER_WEAPON_SPECIALIZATION:
		if (ftrLvl < 12)
			return false;

		for (auto i = (int)FEAT_GREATER_WEAPON_FOCUS_GAUNTLET; i <= FEAT_GREATER_WEAPON_FOCUS_RAY; i++) {
			hasFocus = false;
			if (feats.HasFeatCountByClass(handle, (feat_enums)i, (Stat)0, 0)) {
				hasFocus = true;
			}
			// if not, check if it's one of the picked ones
			for (auto it : featsPicked) {
				if (it == (feat_enums)i)
					hasFocus = true;
				break;
			}
			// if has Greater Weapon Focus, check for Weapon Specialization
			if (hasFocus) {

				for (auto j = (int)FEAT_WEAPON_SPECIALIZATION_GAUNTLET; j <= FEAT_WEAPON_SPECIALIZATION_GRAPPLE; j++) {
					if (feats.HasFeatCountByClass(handle, (feat_enums)j, (Stat)0, 0))
						return true;
				}
			}
		}

	default:
		return feats.FeatPrereqsCheck(handle, feat, featsPicked.size() > 0 ? &featsPicked[0] : nullptr, featsPicked.size(), selPkt.classCode, selPkt.statBeingRaised) != FALSE;
	}
}

bool UiPcCreation::IsSelectingNormalFeat() {
	return true;
}

bool UiPcCreation::IsSelectingSecondFeat()
{
	auto &selPkt = GetCharEditorSelPacket();
	return d20RaceSys.BonusFirstLevelFeat(selPkt.raceId);
}

bool UiPcCreation::IsSelectingBonusFeat()
{
	return mIsSelectingBonusFeat;
}

bool UiPcCreation::IsClassBonusFeat(feat_enums feat)
{
	return chargen.IsClassBonusFeat(feat);
}

bool UiPcCreation::IsBonusFeatDisregardingPrereqs(feat_enums feat)
{
	return chargen.IsBonusFeatDisregardingPrereqs(feat);
}


void UiPcCreation::FeatsSanitize()
{
	auto &selPkt = GetCharEditorSelPacket();

	for (auto i = 0; i < 3; i++) { // check if any of the feat now lack the prereq (due to user removal). loop three times to ensure up-to-date state.
		if (selPkt.feat0 != FEAT_NONE && !FeatCanPick(selPkt.feat0))
			selPkt.feat0 = FEAT_NONE;
		if (selPkt.feat1 != FEAT_NONE && !FeatCanPick(selPkt.feat1)) {
			selPkt.feat1 = FEAT_NONE;
		}
		if (selPkt.feat2 != FEAT_NONE && !FeatCanPick(selPkt.feat2) && !IsSelectingRangerSpec())
			selPkt.feat2 = FEAT_NONE;
	}
}

void UiPcCreation::FeatsMultiSelectActivate(feat_enums feat)
{
	if (!FeatCanPick(feat))
		return;

	auto &selPkt = GetCharEditorSelPacket();
	if (feat == FEAT_WEAPON_FINESSE) {
		if (selPkt.feat0 == FEAT_WEAPON_FINESSE)
			selPkt.feat0 = FEAT_WEAPON_FINESSE_DAGGER;
		if (selPkt.feat1 == FEAT_WEAPON_FINESSE)
			selPkt.feat1 = FEAT_WEAPON_FINESSE_DAGGER;
		if (selPkt.feat2 == FEAT_WEAPON_FINESSE)
			selPkt.feat2 = FEAT_WEAPON_FINESSE_DAGGER;
		return;
	}

	mFeatsMultiMasterFeat = feat;
	featsMultiSelected = FEAT_NONE;

	// populate list
	mMultiSelectFeats.clear();

	if (feat >NUM_FEATS) {
		std::vector<feat_enums> tmp;
		feats.MultiselectGetChildren(feat, tmp);
		for (auto it : tmp) {
			mMultiSelectFeats.push_back(FeatInfo(it));
		}
	}
	else {
		auto featIt = FEAT_ACROBATIC;
		auto featProp = 0x100;
		switch (feat) {
		case FEAT_EXOTIC_WEAPON_PROFICIENCY:
			featProp = FPF_EXOTIC_WEAP_ITEM;
			break;
		case FEAT_IMPROVED_CRITICAL:
			featProp = FPF_IMPR_CRIT_ITEM;
			break;
		case FEAT_MARTIAL_WEAPON_PROFICIENCY:
			featProp = FPF_MARTIAL_WEAP_ITEM;
			break;
		case FEAT_SKILL_FOCUS:
			featProp = FPF_SKILL_FOCUS_ITEM;
			break;
		case FEAT_WEAPON_FINESSE:
			featProp = FPF_WEAP_FINESSE_ITEM;
			break;
		case FEAT_WEAPON_FOCUS:
			featProp = FPF_WEAP_FOCUS_ITEM;
			break;
		case FEAT_WEAPON_SPECIALIZATION:
			featProp = FPF_WEAP_SPEC_ITEM;
			break;
		case FEAT_GREATER_WEAPON_FOCUS:
			featProp = FPF_GREATER_WEAP_FOCUS_ITEM;
			break;
		case FEAT_GREATER_WEAPON_SPECIALIZATION:
			featProp = FPF_GREAT_WEAP_SPEC_ITEM;
			break;
		default:
			break;
		}

		for (auto ft = 0; ft < NUM_FEATS; ft++) {
			featIt = (feat_enums)ft;
			if (feats.IsFeatPropertySet(featIt, featProp) && feats.IsFeatEnabled(featIt)) {
				mMultiSelectFeats.push_back(FeatInfo(ft));
			}
		}
	}



	featsMultiSelectScrollbar = *uiManager->GetScrollBar(featsMultiSelectScrollbarId);
	featsMultiSelectScrollbar.scrollbarY = 0;
	featsMultiSelectScrollbarY = 0;
	featsMultiSelectScrollbar.yMax = max(0, (int)mMultiSelectFeats.size() - FEATS_MULTI_BTN_COUNT);
	featsMultiSelectScrollbar = *uiManager->GetScrollBar(featsMultiSelectScrollbarId);
	uiManager->SetButtonState(featsMultiOkBtnId, LgcyButtonState::Disabled);

	uiManager->SetHidden(featsMultiSelectWndId, false);
	uiManager->BringToFront(featsMultiSelectWndId);
}

feat_enums UiPcCreation::FeatsMultiGetFirst(feat_enums feat)
{
	return feats.MultiselectGetFirst(feat);
}

#pragma endregion

#pragma region Spells
/* 0x101800E0 */
BOOL UiPcCreation::SpellsSystemInit(UiSystemConf & conf)
{
	auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
	MesLine mesline;

	TigTextStyle baseStyle;
	baseStyle.flags = 0;
	baseStyle.field2c = -1;
	baseStyle.shadowColor = &blackColorRect;
	baseStyle.field0 = 0;
	baseStyle.kerning = 1;
	baseStyle.leading = 0;
	baseStyle.tracking = 3;
	baseStyle.textColor = baseStyle.colors2 = baseStyle.colors4 = &whiteColorRect;


	spellsTitleStyle = baseStyle;

	// generic spells text style
	spellsTextStyle = baseStyle;

	// Spell Level Label Style
	spellLevelLabelStyle = baseStyle;
	static ColorRect spellLevelLabelColor(0xFF43586E);
	spellLevelLabelStyle.textColor = spellLevelLabelStyle.colors2 = spellLevelLabelStyle.colors4 = &spellLevelLabelColor;

	// Spells Available Btn Style
	spellsAvailBtnStyle = baseStyle;
	static ColorRect spellsAvailColor1(0x0FF5D5D5D);
	spellsAvailBtnStyle.textColor = &spellsAvailColor1;
	spellsAvailBtnStyle.colors2 = &spellsAvailColor1;
	spellsAvailBtnStyle.colors4 = &spellsAvailColor1;


	// Spells Per Day style
	spellsPerDayStyle = baseStyle;

	spellsPerDayTitleStyle = baseStyle;

	// Spells Available title
	mesline.key = 21000;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	spellsAvailLabel.append(mesline.value);

	// Spells Chosen title
	mesline.key = 21001;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	spellsChosenLabel.append(mesline.value);

	// Spells Per Day title
	mesline.key = 21002;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	spellsPerDayLabel.append(mesline.value);

	// Spell Level label texts
	MesLine line(21100), line2(21200);
	mesFuncs.GetLine_Safe(pcCreationMes, &line);
	mesFuncs.GetLine_Safe(pcCreationMes, &line2);


	for (auto i = 0; i < NUM_SPELL_LEVELS; i++) {
		std::string text;
		text.append(line2.value);
		text[text.size() - 1] = '0' + i;
		chargen.levelLabels.push_back(text);

		text.clear();
		text.append(line.value);
		text[text.size() - 1] = '0' + i;

		chargen.spellLevelLabels.push_back(text);
	}

	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT; i++) {
		spellsPerDayTextRects.push_back(TigRect());
	}

	levelupSpellbar = new CombinedImgFile("art\\interface\\pc_creation\\levelup_spellbar.img");
	if (!levelupSpellbar)
		return 0;

	// Widgets
	return SpellsWidgetsInit();
}

void UiPcCreation::SpellsFree(){
	SpellsWidgetsFree();
}

/* 0x1017FCC0 */
BOOL UiPcCreation::SpellsWidgetsInit(){

	auto &pcWnd = GetPcCreationWnd();

	const int spellsWndX = 219, spellsWndY = 50, spellsWndW = 431, spellsWndH = 250;
	spellsWnd = LgcyWindow(pcWnd.x + spellsWndX, pcWnd.y + spellsWndY, spellsWndW, spellsWndH);
	spellsWnd.flags = 1;
	spellsWnd.render = [](int widId) {uiPcCreation.SpellsWndRender(widId); };
	spellsWnd.handleMessage = [](int widId, TigMsg*msg) { return uiPcCreation.SpellsWndMsg(widId, msg); };
	spellsWndId = uiManager->AddWindow(spellsWnd);

	// Available Spells Scrollbar
	spellsScrollbar.Init(201, 34, 152);
	spellsScrollbar.parentId = spellsWndId;
	spellsScrollbar.x += spellsWnd.x;
	spellsScrollbar.y += spellsWnd.y;
	spellsScrollbarId = uiManager->AddScrollBar(spellsScrollbar, spellsWndId);

	// Spell selection scrollbar
	spellsScrollbar2.Init(415, 34, 152);
	spellsScrollbar2.parentId = spellsWndId;
	spellsScrollbar2.x += spellsWnd.x;
	spellsScrollbar2.y += spellsWnd.y;
	spellsScrollbar2Id = uiManager->AddScrollBar(spellsScrollbar2, spellsWndId);

	int rowOff = 38;
	for (auto i = 0; i < SPELLS_BTN_COUNT; i++, rowOff += SPELLS_BTN_HEIGHT) {

		LgcyButton spellAvailBtn("Spell Available btn", spellsWndId, 4, rowOff, 193, SPELLS_BTN_HEIGHT);

		spellAvailBtn.x += spellsWnd.x; spellAvailBtn.y += spellsWnd.y;
		spellAvailBtn.render = [](int id) {uiPcCreation.SpellsAvailableEntryBtnRender(id); };
		spellAvailBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.SpellsAvailableEntryBtnMsg(id, msg); };
		spellAvailBtn.renderTooltip = nullptr;
		spellAvailBtn.SetDefaultSounds();
		spellsAvailBtnIds.push_back(uiManager->AddButton(spellAvailBtn, spellsWndId));

		LgcyButton spellChosenBtn("Spell Chosen btn", spellsWndId, 221, rowOff, 193, SPELLS_BTN_HEIGHT);

		spellChosenBtn.x += spellsWnd.x; spellChosenBtn.y += spellsWnd.y;
		spellChosenBtn.render = [](int id) {uiPcCreation.SpellsEntryBtnRender(id); };
		spellChosenBtn.handleMessage = [](int id, TigMsg* msg) { return uiPcCreation.SpellsEntryBtnMsg(id, msg); };
		spellChosenBtn.renderTooltip = nullptr;
		spellChosenBtn.SetDefaultSounds();
		spellsChosenBtnIds.push_back(uiManager->AddButton(spellChosenBtn, spellsWndId));

	}

	// titles
	spellsAvailTitleRect.x = 5;
	spellsAvailTitleRect.y = 20;
	spellsChosenTitleRect.x = 219;
	spellsChosenTitleRect.y = 20;
	UiRenderer::PushFont("priory-12", 12);

	// Spells Per Day title
	auto spellsPerDayMeasure = UiRenderer::MeasureTextSize(spellsPerDayLabel, spellsTextStyle);
	spellsPerDayTitleRect = TigRect(5, 205, 99, 12);
	spellsPerDayTitleRect.x += (spellsPerDayTitleRect.width - spellsPerDayMeasure.width) / 2;
	spellsPerDayTitleRect.width = spellsPerDayMeasure.width;
	spellsPerDayTitleRect.height = spellsPerDayMeasure.height;

	// Spell Level labels
	spellsPerDayBorderRects.clear();
	spellsLevelLabelRects.clear();
	
	for (auto lvl = 0u; lvl < NUM_SPELL_LEVELS; lvl++) {
		auto textMeas = UiRenderer::MeasureTextSize(chargen.levelLabels[lvl].c_str(), spellLevelLabelStyle);
		spellsPerDayBorderRects.push_back(TigRect(105 + lvl * 51, 201, 29, 25));
		spellsLevelLabelRects.push_back(TigRect(spellsPerDayBorderRects[lvl].x + spellsPerDayBorderRects[lvl].width / 2 - textMeas.width / 2,
			spellsPerDayBorderRects[lvl].y - textMeas.height - 2, textMeas.width, textMeas.height));
	}
	UiRenderer::PopFont();
	return 1;
}

void UiPcCreation::SpellsReset(){
	chargen.SpellsNeedResetSet(true);
	chargen.GetKnownSpellInfo().clear();
}

void UiPcCreation::SpellsWidgetsFree(){
	for (auto i = 0; i < SPELLS_BTN_COUNT; i++) {
		uiManager->RemoveChildWidget(spellsChosenBtnIds[i]);
		uiManager->RemoveChildWidget(spellsAvailBtnIds[i]);
	}
	spellsChosenBtnIds.clear();
	spellsAvailBtnIds.clear();
	uiManager->RemoveWidget(spellsWndId);
}

BOOL UiPcCreation::SpellsShow()
{
	uiManager->SetHidden(spellsWndId, false);
	uiManager->BringToFront(spellsWndId);
	return 1;
}

BOOL UiPcCreation::SpellsHide()
{
	uiManager->SetHidden(spellsWndId, true);
	return 0;
}

BOOL UiPcCreation::SpellsWidgetsResize(UiResizeArgs & args)
{
	SpellsWidgetsFree();
	SpellsWidgetsInit();
	return 0;
}

void UiPcCreation::SpellsActivate()
{
	auto handle = chargen.GetEditedChar();
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto &selPkt = chargen.GetCharEditorSelPacket();
	auto &avSpInfo = chargen.GetAvailableSpells();
	auto &knSpInfo = chargen.GetKnownSpellInfo();

	// get the new caster level for the levelled class (1 indicates a newly taken class)
	auto casterLvlNew = 1;
	auto classLeveled = selPkt.classCode;
	
	auto needsReset = chargen.SpellsNeedReset();

	static auto setScrollbars = []() {
		auto sbId = uiPcCreation.spellsScrollbarId;
		uiManager->ScrollbarSetY(sbId, 0);
		int numEntries = (int)chargen.GetAvailableSpells().size();
		uiManager->ScrollbarSetYmax(sbId, max(0, numEntries - uiPcCreation.SPELLS_BTN_COUNT));
		uiPcCreation.spellsScrollbar = *uiManager->GetScrollBar(sbId);
		uiPcCreation.spellsScrollbar.y = 0;
		uiPcCreation.spellsScrollbarY = 0;

		auto &charEdSelPkt = chargen.GetCharEditorSelPacket();
		auto sbAddedId = uiPcCreation.spellsScrollbar2Id;
		int numAdded = (int)chargen.GetKnownSpellInfo().size();
		uiManager->ScrollbarSetY(sbAddedId, 0);
		uiManager->ScrollbarSetYmax(sbAddedId, max(0, numAdded - uiPcCreation.SPELLS_BTN_COUNT));
		uiPcCreation.spellsScrollbar2 = *uiManager->GetScrollBar(sbAddedId);
		uiPcCreation.spellsScrollbar2.y = 0;
		uiPcCreation.spellsScrollbar2Y = 0;
	};

	if (!needsReset) {
		setScrollbars();
		return;
	}


	knSpInfo.clear();
	avSpInfo.clear();

	d20ClassSys.LevelupInitSpellSelection(handle, selPkt.classCode, 1);


	for (auto i = 0u; i < knSpInfo.size(); i++) {
		auto spEnum = knSpInfo[i].spEnum;
		if (spellSys.IsNewSlotDesignator(spEnum)) {
			knSpInfo[i].spEnum = 802;
			knSpInfo[i].spFlag = 3;
		}
	}

	SpellsPerDayUpdate();

	setScrollbars();
	chargen.SpellsNeedResetSet(false);
}

BOOL UiPcCreation::SpellsCheckComplete()
{
	auto selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();
	if (!d20ClassSys.IsSelectingSpellsOnLevelup(handle, selPkt.classCode))
		return true;

	if (chargen.SpellsNeedReset())
		return false;

	return d20ClassSys.LevelupSpellsCheckComplete(GetEditedChar(), selPkt.classCode);
}

void UiPcCreation::SpellsFinalize(){
	auto charEdited = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	d20ClassSys.LevelupSpellsFinalize(charEdited, selPkt.classCode, 1);
}

void UiPcCreation::SpellsReset(CharEditorSelectionPacket & selPkt)
{
	temple::GetRef<int>(0x10C4D4C4) = 1; // needsPopulateEntries
	chargen.GetKnownSpellInfo().clear();
	chargen.GetAvailableSpells().clear();
}

void UiPcCreation::SpellsWndRender(int widId)
{
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(widId, spellsAvailLabel, spellsAvailTitleRect, spellsTitleStyle);
	UiRenderer::DrawTextInWidget(widId, spellsChosenLabel, spellsChosenTitleRect, spellsTitleStyle);
	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT; i++) {
		UiRenderer::DrawTextInWidget(widId, chargen.levelLabels[i], spellsLevelLabelRects[i], spellLevelLabelStyle);
	}


	// RenderSpellsPerDay

	UiRenderer::DrawTextInWidget(widId, spellsPerDayLabel, spellsPerDayTitleRect, spellsTextStyle);
	UiRenderer::PopFont();

	UiRenderer::PushFont(PredefinedFont::ARIAL_BOLD_24);
	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT; i++) {
		RenderHooks::RenderRectInt(spellsWnd.x + spellsPerDayBorderRects[i].x, spellsWnd.y + spellsPerDayBorderRects[i].y, spellsPerDayBorderRects[i].width, spellsPerDayBorderRects[i].height, 0xFF43586E);
		UiRenderer::DrawTextInWidget(widId, spellsPerDayTexts[i], spellsPerDayTextRects[i], spellsPerDayStyle);

	}
	UiRenderer::PopFont();

	// Rects
	RenderHooks::RenderRectInt(spellsWnd.x + 5, spellsWnd.y + 35, 207, 149, 0xFF5D5D5D);
	RenderHooks::RenderRectInt(spellsWnd.x + 219, spellsWnd.y + 35, 207, 149, 0xFF5D5D5D);

	StateTitleRender(widId);
}

BOOL UiPcCreation::SpellsWndMsg(int widId, TigMsg * msg)
{
	if (msg->type == TigMsgType::WIDGET) {
		auto msgW = (TigMsgWidget*)msg;
		if (msgW->widgetEventType == TigMsgWidgetEvent::Scrolled) {
			uiManager->ScrollbarGetY(spellsScrollbarId, &spellsScrollbarY);
			uiManager->ScrollbarGetY(spellsScrollbar2Id, &spellsScrollbar2Y);
			SpellsPerDayUpdate();
			return 1;
		}
		return 0;
	}

	if (msg->type == TigMsgType::MOUSE) {
		auto msgM = (TigMsgMouse*)msg;
		if ((msgM->buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) && helpSys.IsClickForHelpActive()) {
			// LMB handler - present help for spell

			auto &knSpInfo = chargen.GetKnownSpellInfo();
			for (auto i = 0; i < SPELLS_BTN_COUNT; i++) {
				// check if mouse within button
				if (!uiManager->DoesWidgetContain(spellsChosenBtnIds[i], msgM->x, msgM->y))
					continue;

				auto spellIdx = i + spellsScrollbar2Y;
				if ((uint32_t)spellIdx >= knSpInfo.size())
					break;

				auto spEnum = knSpInfo[spellIdx].spEnum;
				// ensure is not label
				if (spellSys.IsLabel(spEnum))
					break;

				helpSys.PresentWikiHelp(HELP_IDX_SPELLS + spEnum);
				return 1;
			}
		}
		if (msgM->buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
			// RMB handler - add to known spells

			auto &knSpInfo = chargen.GetKnownSpellInfo();
			auto &avSpInfo = chargen.GetAvailableSpells();

			for (auto i = 0; i < SPELLS_BTN_COUNT; i++) {
				// get spell btn
				if (!uiManager->DoesWidgetContain(spellsAvailBtnIds[i], msgM->x, msgM->y))
					continue;
				auto spellAvailIdx = i + spellsScrollbarY;
				if ((uint32_t)spellAvailIdx >= avSpInfo.size())
					break;

				// got the avail btn, now search for suitable vacant slot
				auto spEnum = avSpInfo[spellAvailIdx].spEnum;
				auto spClass = avSpInfo[spellAvailIdx].spellClass;
				auto spLevel = avSpInfo[spellAvailIdx].spellLevel;

				if (spellSys.IsLabel(spEnum))
					break;

				if (chargen.SpellIsAlreadyKnown(spEnum, spClass) || chargen.SpellIsForbidden(spEnum, spClass))
					break;

				auto curSpellLvl = -1;
				auto foundSlot = false;
				for (auto j = 0u; j < knSpInfo.size(); j++) {
					auto spInfo = knSpInfo[j];
					if (spInfo.spellClass != spClass)
						continue;
					if (spellSys.IsLabel(spInfo.spEnum)) {
						curSpellLvl = spInfo.spellLevel;
						if (curSpellLvl > spLevel)
							break;
						continue;
					}

					if (spInfo.spEnum != SPELL_ENUM_VACANT)
						continue;

					// ensure spell slot is of correct level
					if (spInfo.spellLevel == -1 // for "wildcard" empty slots (e.g. Wizard)
						|| curSpellLvl == spLevel) {
						knSpInfo[j].spEnum = spEnum; // spell level might still be -1 so be careful when adding to spellbook later on!
						break;
					}
				}
			}
		}
		if (!(msgM->buttonStateFlags & MouseStateFlags::MSF_SCROLLWHEEL_CHANGE))
			return 1;

		TigMsgMouse msgCopy = *msgM;
		msgCopy.buttonStateFlags = MouseStateFlags::MSF_SCROLLWHEEL_CHANGE;

		if ((int)msgM->x >= spellsWnd.x + 4 && (int)msgM->x <= spellsWnd.x + 184
			&& (int)msgM->y >= spellsWnd.y && (int)msgM->y <= spellsWnd.y + 259) {
			spellsScrollbar = *uiManager->GetScrollBar(spellsScrollbarId);
			if (spellsScrollbar.handleMessage)
				return spellsScrollbar.handleMessage(spellsScrollbarId, (TigMsg*)&msgCopy);
		}

		if ((int)msgM->x >= spellsWnd.x + 206 && (int)msgM->x <= spellsWnd.x + 376
			&& (int)msgM->y >= spellsWnd.y && (int)msgM->y <= spellsWnd.y + 259) {
			spellsScrollbar2 = *uiManager->GetScrollBar(spellsScrollbar2Id);
			if (spellsScrollbar2.handleMessage)
				return spellsScrollbar2.handleMessage(spellsScrollbar2Id, (TigMsg*)&msgCopy);
		}
		return 1;

	}

	return 0;
}

void UiPcCreation::SpellsPerDayUpdate()
{
	UiRenderer::PushFont(PredefinedFont::ARIAL_BOLD_24);
	auto &selPkt = GetCharEditorSelPacket();

	spellsPerDayTexts.clear();
	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT; i++) {
		auto &handle = GetEditedChar();
		auto casterLvl = objects.StatLevelGet(handle, selPkt.classCode);
		auto numSpells = d20ClassSys.GetNumSpellsFromClass(handle, selPkt.classCode, i, casterLvl);
		if (numSpells < 0)
			numSpells = 0;
		std::string text(fmt::format("{}", numSpells));
		spellsPerDayTexts.push_back(text);

		auto textMeas = UiRenderer::MeasureTextSize(text, spellsPerDayStyle);
		spellsPerDayTextRects[i].x = spellsPerDayBorderRects[i].x +
			(spellsPerDayBorderRects[i].width - textMeas.width) / 2;
		spellsPerDayTextRects[i].y = spellsPerDayBorderRects[i].y +
			(spellsPerDayBorderRects[i].height - textMeas.height) / 2;
		spellsPerDayTextRects[i].width = textMeas.width;
		spellsPerDayTextRects[i].height = textMeas.height;
	}
	UiRenderer::PopFont();
}

BOOL UiPcCreation::SpellsEntryBtnMsg(int widId, TigMsg * msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return FALSE;
	auto widMsg = (TigMsgWidget*)msg;
	if (widMsg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto widIdx = WidgetIdIndexOf(widId, &spellsChosenBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return FALSE;

	auto &knSpInfo = chargen.GetKnownSpellInfo();

	auto spellIdx = widIdx + spellsScrollbar2Y;
	if (spellIdx >= (int)knSpInfo.size())
		return FALSE;

	auto &spInfo = knSpInfo[spellIdx];
	auto spEnum = spInfo.spEnum;

	if (spellSys.IsLabel(spEnum))
		return FALSE;

	spInfo.spEnum = SPELL_ENUM_VACANT;

	return FALSE;
}

void UiPcCreation::SpellsEntryBtnRender(int widId)
{
	auto widIdx = WidgetIdIndexOf(widId, &spellsChosenBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return;

	auto &knSpInfo = chargen.GetKnownSpellInfo();

	auto spellIdx = widIdx + spellsScrollbar2Y;
	if (spellIdx >= (int)knSpInfo.size())
		return;

	auto spInfo = knSpInfo[spellIdx];
	auto spFlag = spInfo.spFlag;
	auto spEnum = spInfo.spEnum;
	auto spLvl = spInfo.spellLevel;

	auto btn = uiManager->GetButton(widId);

	auto &selPkt = GetCharEditorSelPacket();
	if (spFlag && (!selPkt.spellEnumToRemove || spFlag != 1)) {
		RenderHooks::RenderRectInt(btn->x + 11, btn->y, btn->width - 11, btn->height, 0xFF222C37);
	}

	std::string text;
	TigRect rect(btn->x - spellsWnd.x, btn->y - spellsWnd.y, btn->width, btn->height);
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	if (spEnum == SPELL_ENUM_VACANT) {
		// don't draw text (will only draw the frame)
	}
	else if (spellSys.IsLabel(spEnum)) {
		if (spLvl >= 0 && spLvl < NUM_SPELL_LEVELS) {
			text.append(fmt::format("{}", chargen.spellLevelLabels[spLvl]));
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellLevelLabelStyle);
		}
	}
	else
	{
		text.append(fmt::format("{}", spellSys.GetSpellMesline(spEnum)));
		rect.x += 11;
		//rect.width -= 11;
		UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsTextStyle);
	}
	UiRenderer::PopFont();
}

BOOL UiPcCreation::SpellsAvailableEntryBtnMsg(int widId, TigMsg * msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = WidgetIdIndexOf(widId, &spellsAvailBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return 0;

	auto &avSpInfo = chargen.GetAvailableSpells();
	auto spellIdx = widIdx + spellsScrollbarY;
	if (spellIdx >= (int)avSpInfo.size())
		return 0;

	auto spInfo = avSpInfo[spellIdx];
	auto spFlag = spInfo.spFlag;
	auto spEnum = spInfo.spEnum;
	auto spLvl = spInfo.spellLevel;
	auto spClass = spInfo.spellClass;

	if (!spellSys.IsLabel(spEnum)) {

		auto btn = uiManager->GetButton(widId);
		auto curSpellLvl = -1;
		auto &selPkt = GetCharEditorSelPacket();
		auto &knSpInfo = chargen.GetKnownSpellInfo();

		switch (msgW->widgetEventType) {
		case TigMsgWidgetEvent::Clicked: // button down - initiate drag
			if (!chargen.SpellIsAlreadyKnown(spEnum, spClass) && !chargen.SpellIsForbidden(spEnum, spClass)) {
				auto origX = msgW->x - btn->x, origY = msgW->y - btn->y;
				auto spellCallback = [origX, origY, spEnum](int x, int y) {
					std::string text(spellSys.GetSpellMesline(spEnum));
					UiRenderer::PushFont(PredefinedFont::PRIORY_12);
					TigRect rect(x - origX, y - origY, 180, uiPcCreation.SPELLS_BTN_HEIGHT);
					tigFont.Draw(text.c_str(), rect, uiPcCreation.spellsTextStyle);
					UiRenderer::PopFont();
				};
				mouseFuncs.SetCursorDrawCallback(spellCallback, (uint32_t)&spellCallback);
			}
			return 1;
		case TigMsgWidgetEvent::MouseReleased:
			if (helpSys.IsClickForHelpActive()) {
				mouseFuncs.SetCursorDrawCallback(nullptr, 0);
				helpSys.PresentWikiHelp(spEnum + HELP_IDX_SPELLS);
				return 1;
			}
		case TigMsgWidgetEvent::MouseReleasedAtDifferentButton:
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
			if (chargen.SpellIsAlreadyKnown(spEnum, spClass)
				|| chargen.SpellIsForbidden(spEnum, spClass))
				return 1;


			for (auto i = 0u; i < knSpInfo.size(); i++) {
				auto rhsSpInfo = knSpInfo[i];

				// make sure the spell class is ok
				if (rhsSpInfo.spellClass != spClass)
					continue;

				// if encountered label - go on
				if (spellSys.IsLabel(rhsSpInfo.spEnum)) {
					curSpellLvl = rhsSpInfo.spellLevel;
					continue;
				}

				// else - make sure is visible slot
				if ((int)i < spellsScrollbar2Y)
					continue;
				if ((int)i >= spellsScrollbar2Y + SPELLS_BTN_COUNT)
					break;

				auto chosenWidIdx = (int)i - spellsScrollbar2Y;
				if (!uiManager->DoesWidgetContain(spellsChosenBtnIds[chosenWidIdx], msgW->x, msgW->y))
					continue;

				if (rhsSpInfo.spellLevel == -1 // wildcard slot
					|| rhsSpInfo.spellLevel == spLvl
					&& rhsSpInfo.spFlag != 0
					&& (rhsSpInfo.spFlag != 1 || !selPkt.spellEnumToRemove)
					) {

					if (rhsSpInfo.spFlag == 1) { // replaceable spell
						knSpInfo[i].spFlag = 2;
						selPkt.spellEnumToRemove = rhsSpInfo.spEnum;
					}
					else if (rhsSpInfo.spFlag == 2 && selPkt.spellEnumToRemove == spEnum) { // was already replaced, and now restoring
						knSpInfo[i].spFlag = 1;
						selPkt.spellEnumToRemove = 0;
					}
					knSpInfo[i].spEnum = spEnum;
					return 1;
				}

			}

			return 1;
		case TigMsgWidgetEvent::Entered:
			temple::GetRef<void(int)>(0x1011BA70)(spEnum); // UiPcCreationScrollboxSetSpell
			return 1;
		case TigMsgWidgetEvent::Exited:
			ButtonEnteredHandler("TAG_CHARGEN_SPELLS");
			return 1;
		default:
			return 0;
		}
	}

	/*if (msgW->widgetEventType == TigMsgWidgetEvent::Entered){
	std::string text;
	text.append(fmt::format(""));
	auto helpTopicId = ElfHash::Hash(text);
	temple::GetRef<void(__cdecl)(uint32_t)>(0x)(helpTopicId);
	return 1;
	}*/

	if (msgW->widgetEventType == TigMsgWidgetEvent::Exited) {
		ButtonEnteredHandler("TAG_CHARGEN_SPELLS");
		return 1;
	}

	return 0;

}

void UiPcCreation::SpellsAvailableEntryBtnRender(int widId)
{
	auto widIdx = WidgetIdIndexOf(widId, &spellsAvailBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return;

	auto &avSpInfo = chargen.GetAvailableSpells();

	auto spellIdx = widIdx + spellsScrollbarY;
	if (spellIdx >= (int)avSpInfo.size())
		return;

	auto btn = uiManager->GetButton(widId);
	auto spEnum = avSpInfo[spellIdx].spEnum;

	std::string text;
	TigRect rect(btn->x - spellsWnd.x, btn->y - spellsWnd.y, btn->width, btn->height);
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	if (spellSys.IsLabel(spEnum)) {
		rect.x += 2;
		auto spLvl = avSpInfo[spellIdx].spellLevel;
		if (spLvl >= 0 && spLvl < NUM_SPELL_LEVELS)
		{
			text.append(fmt::format("{}", chargen.spellLevelLabels[spLvl]));
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellLevelLabelStyle);
		}

	}
	else
	{
		text.append(fmt::format("{}", spellSys.GetSpellMesline(spEnum)));
		rect.x += 12;
		//rect.width -= 11;
		if (chargen.SpellIsAlreadyKnown(spEnum, avSpInfo[spellIdx].spellClass)
			|| chargen.SpellIsForbidden(spEnum, avSpInfo[spellIdx].spellClass))
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsAvailBtnStyle);
		else
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsTextStyle);
	}
	UiRenderer::PopFont();
}


#pragma endregion

void UiPcCreation::StateTitleRender(int widId){
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	int state = GetState();
	auto &selPkt = GetCharEditorSelPacket();
	auto &stateTitles = temple::GetRef<const char*[14]>(0x10BDAE28);
	auto &altStateTitles = temple::GetRef<const char*[14]>(0x10BDAE60);


	auto &rect = temple::GetRef<TigRect>(0x10BDB8E8);
	auto &style = temple::GetRef<TigTextStyle>(0x10BDD638);
	if (!selPkt.isPointbuy)
		UiRenderer::DrawTextInWidget(widId, stateTitles[state], rect, style);
	else
		UiRenderer::DrawTextInWidget(widId, altStateTitles[state], rect, style);
	UiRenderer::PopFont();
}

int UiPcCreation::GetStatIdx(const int x0, const int xMax, int x, int y, int * xyOut)
{
	auto &statsWnd = GetStatsWnd();

	auto relX = x - statsWnd.x;
	auto relY = y - statsWnd.y;

	auto idx = 0;
	const int y0 = 71;
	const int yBtnSize = 31;

	if (relX < x0 || relX > xMax)
		return -1;

	auto btnY = y0;
	auto foundIt = false;
	for (auto i=0; i < 6; i++){

		if ( relY <= btnY && relY >= btnY - 27){
			foundIt = true;
			idx = i;
			break;
		}

		btnY += 31;
	}

	if (!foundIt)
		return -1;

	if (xyOut){
		xyOut[0] = relX - x0;
		xyOut[1] = relY - yBtnSize*idx - 44;
	}
	return idx;
}

int UiPcCreation::GetRolledStatIdx(int x, int y, int * xyOut)
{
	return GetStatIdx(203, 241, x, y, xyOut);
}

int UiPcCreation::GetAssignedStatIdx(int x, int y, int * xyOut)
{
	return GetStatIdx(87, 125, x, y, xyOut);
}

BOOL UiPcCreation::StatsWndMsg(int widId, TigMsg * msg){
	// returning FALSE indicates it should pass the handling on to the original handler

	if (msg->type != TigMsgType::MOUSE)
		return FALSE;

	auto msgMouse = (TigMsgMouse*)msg;
	auto mouseFlags = msgMouse->buttonStateFlags;

	if (mouseFlags & MouseStateFlags::MSF_RMB_RELEASED){
		auto rolledStatIdx = GetRolledStatIdx(msgMouse->x, msgMouse->y);
		if (rolledStatIdx == -1)
			return FALSE;

		auto rolledStats = chargen.GetRolledStats();
		if (rolledStats[rolledStatIdx] <= 0)
			return FALSE;

		auto &selPkt = chargen.GetCharEditorSelPacket();
		for (auto i = 0; i < 6; i++) {
			if (selPkt.abilityStats[i] <= 0){
				selPkt.abilityStats[i] = rolledStats[rolledStatIdx];
				rolledStats[rolledStatIdx] = -1;
				return TRUE;
			}
		}
	}

	return FALSE;
}

void UiPcCreation::RaceWndRender(int widId){
	StateTitleRender(widId);
}

void UiPcCreation::GenderFinalize(CharEditorSelectionPacket& selPkt, objHndl& handle){
	int protoId = uiPcCreation.GetProtoIdByRaceGender(selPkt.raceId, selPkt.genderId);
	auto protoHandle = objSystem->GetProtoHandle(protoId);

	auto loc = locXY{ 480, 480 };
	handle = objSystem->CreateObject(protoHandle, loc);
	if (!handle){
		logger->error("pc_creation: FATAL ERROR, could not create player!");
		exit(0);
	}
	
	for (auto i=0; i <= Stat::stat_charisma; i++){
		objects.StatLevelSetBase(handle, (Stat)i, selPkt.abilityStats[i]);
	}

	auto animHandle = objects.GetAnimHandle(handle);
	auto animParams = objects.GetAnimParams(handle);
	animHandle->Advance(1.0, 0, 0, animParams);

	if (selPkt.isPointbuy){
		objects.setInt32(handle, obj_f_pc_roll_count, -25);
	}
	else{
		objects.setInt32(handle, obj_f_pc_roll_count, selPkt.numRerolls);
	}

}

void UiPcCreation::HairUpdate(){
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.raceId != RACE_INVALID && selPkt.genderId != GENDER_INVALID){
		HairStyle hairStyle;
		hairStyle.size = HairStyleSize::Big;
		hairStyle.race = d20RaceSys.GetHairStyle(selPkt.raceId);
		hairStyle.gender = (Gender)selPkt.genderId;
		hairStyle.color = selPkt.hairColor;
		hairStyle.style = selPkt.hairStyle;
		
		auto critter = GetEditedChar();
		auto hairPacked = hairStyle.Pack();
		objSystem->GetObject(critter)->SetInt32(obj_f_critter_hair_style, hairPacked);
		critterSys.UpdateModelEquipment(critter);
	}
}

void UiPcCreation::HairUpdateStyleBtnTextures(){
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.raceId != RACE_INVALID && selPkt.genderId != GENDER_INVALID) {

		HairStyle hairStyle;
		hairStyle.size = HairStyleSize::Small;
		hairStyle.race = d20RaceSys.GetHairStyle(selPkt.raceId);
		hairStyle.gender = (Gender)selPkt.genderId;
		hairStyle.color = selPkt.hairColor;
		hairStyle.style = selPkt.hairStyle;

		auto &hairStyleBtnTextures = temple::GetRef<int[]>(0x10C42788);

		for (auto i=0; i < 8; i++){
			hairStyle.style = i;
			auto s = critterSys.GetHairStylePreviewTexture(hairStyle);
			textureFuncs.RegisterTexture(s.c_str(), &hairStyleBtnTextures[i]);
		}

	}
}


BOOL UiPcCreation::FinishBtnMsg(int widId, TigMsg * msg)
{
	if (msg->type == TigMsgType::MOUSE)
		return TRUE;

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return TRUE;

	auto stComplete = GetStatesComplete();
	if (stComplete != CG_STAGE_COUNT)
		return TRUE;

	auto &selPkt = GetCharEditorSelPacket();
	auto charEdited = GetEditedChar();

	// add spell casting condition
	if (d20ClassSys.IsCastingClass(selPkt.classCode)) {
		auto spellcastCond = (std::string)d20ClassSys.GetSpellCastingCondition(selPkt.classCode);
		if (spellcastCond.size()) {
			conds.AddTo(charEdited, spellcastCond, { 0,0,0,0, 0,0,0,0 });
		}
	}
	return TRUE;
}

void UiPcCreation::MainWndRender(int id) {
	if (!uiSystems->GetPcCreation().IsVisible()) return;

	auto stage = 0;
	auto &mStagesComplete = GetStatesComplete();
	auto &mActiveStage = GetState();
	auto &selPkt = GetCharEditorSelPacket();

	for (stage = 0; stage < std::min(mStagesComplete + 1, (int)ChargenStages::CG_STAGE_COUNT); stage++){
		auto &sys = lgcySystems[stage];
		if (!sys.CheckComplete)
			break;
		if (!sys.CheckComplete())
			break;
	}

	if (stage != mStagesComplete){
		mStagesComplete = stage;
		if (mActiveStage > stage)
			mActiveStage = stage;
		
		// reset the next stages
		for (auto nextStage = stage + 1; nextStage < ChargenStages::CG_STAGE_COUNT; nextStage++){
			if (lgcySystems[nextStage].Reset){
				lgcySystems[nextStage].Reset(selPkt);
			}
		}
		ChargenSystem::UpdateDescriptionBox();
	}

	auto wnd = uiManager->GetWindow(id);
	RenderHooks::RenderImgFile(temple::GetRef<ImgFile*>(0x10BDAFE0), wnd->x, wnd->y);
	RenderCharInfos(id);
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(id, 
		temple::GetRef<char[] >(0x10BDB100), 
		temple::GetRef<TigRect>(0x10BDB004), 
		temple::GetRef<TigTextStyle>(0x10BDDCC8));
	UiRenderer::PopFont();

	auto renderCharModel = temple::GetRef<void(__cdecl)(int)>(0x1011C320);
	renderCharModel(id);
}

BOOL UiPcCreation::MainWndMsg(int widgetId, TigMsg* msg) {
	if (*feats.charEditorObjHnd)
	{
		static auto lastAnimTime = timeGetTime();
		auto now = timeGetTime();
		// Do not allow more than 1 second of animation at once
		auto elapsedTime = std::min(1.0f, (now - lastAnimTime) / 1000.0f);
		lastAnimTime = now;

		auto anim = objects.GetAnimHandle(*feats.charEditorObjHnd);
		if (anim) {
			// Vanilla did not properly initialize the AAS params here
			auto animParams = objects.GetAnimParams(*feats.charEditorObjHnd);
			anim->Advance(elapsedTime, 0, 0, animParams);
		}
	}

	// The following code handles rotating the character model preview
	static auto &uiPcCreationWnd = temple::GetRef<LgcyWindow>(0x11e73e40);
	static int rotateState = 0;
	static int rotatePivot = 0;

	switch (msg->type) {
	case TigMsgType::MOUSE:
	{
		int x = msg->arg1;
		int y = msg->arg2;
		if (x < uiPcCreationWnd.x + 26
			|| x > uiPcCreationWnd.x + 214
			|| y < uiPcCreationWnd.y + 52
			|| y > uiPcCreationWnd.y + 233)
		{
			// Mouse has left character portrait
			rotateState = 0;
		}
		else if (rotateState == 1)
		{
			rotatePivot = x;
			rotateState = 2;
		}
		else if (rotateState == 2)
		{
			// One full rotation for every 200 pixel mouse movement
			auto rotationDelta = ((int) x - rotatePivot) / 200.0f * XM_2PI;
			rotatePivot = x;
			static auto& modelRotation = temple::GetRef<float>(0x10bddd28);
			modelRotation = modelRotation - rotationDelta;
			// Normalize rotation to [0,2pi], note that this does not need to be fully accurate
			modelRotation = modelRotation - XM_2PI * std::floor((modelRotation + XM_PI) * XM_1DIV2PI);
		}
		return TRUE;
	}

	case TigMsgType::WIDGET:
	{
		auto widgetMsg = (TigMsgWidget*) msg;
		if (widgetMsg->widgetEventType == TigMsgWidgetEvent::Clicked) {
			rotateState = 1;
		} else if (widgetMsg->widgetEventType == TigMsgWidgetEvent::MouseReleased
			|| widgetMsg->widgetEventType == TigMsgWidgetEvent::Exited) {
			rotateState = 0;
		}
		return TRUE;
	}

	default:
		return FALSE;
	}
}

void UiPcCreation::RenderCharInfos(int widId){
	
	for (auto stat = 0; stat <= Stat::stat_charisma; stat++){
		RenderCharStatTexts(stat, widId);
		RenderCharStats(stat, widId);
	}
	RenderCharDimensions(widId);
	RenderCharExpLvl(widId);
	RenderCharSavingThrows(widId);
	RenderCharHpAc(widId);
	RenderCharMovementInit(widId);
	RenderCharToHitBonus(widId);
}

void UiPcCreation::RenderCharStatTexts(int stat, int widId) {
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.abilityStats[stat] == -1){
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDD688),
			TigRect(26, 289 + 15*stat, 99, 14), TigRect(1, 1, 99, 14));
	}
	else{
		if (!modSupport.IsCo8()){ // Disable text draw (because Co8 has repalced it with a background)
			DrawTextInWidgetCentered(widId,
				d20Stats.GetStatShortName((Stat)stat),
				TigRect(25, 288 + 15 * stat, 37, 14), blackTextGenericStyle);
		}
	}
}

void UiPcCreation::RenderCharStats(int stat, int widId)
{
	auto &selPkt = GetCharEditorSelPacket();
	auto abilityStat = selPkt.abilityStats[stat];
	if (abilityStat == -1)
		return;
	
	if (selPkt.raceId != RACE_INVALID){
		abilityStat += d20RaceSys.GetStatModifier(selPkt.raceId, stat);
	}
	if (abilityStat < 3)
		abilityStat = 3;

	DrawTextInWidgetCentered(widId, fmt::format("{}", abilityStat),
		TigRect(67, 15 * stat + 288, 26, 14), whiteTextGenericStyle);

	auto modFromStat = objects.GetModFromStatLevel(abilityStat);
	std::string modString;
	if (modFromStat >= 0)
		modString.append("+");
	modString.append(fmt::format("{}", modFromStat));
	DrawTextInWidgetCentered(widId, modString,
		TigRect(98, 15 * stat + 288, 26, 14), whiteTextGenericStyle);

}

void UiPcCreation::RenderCharDimensions(int widId){
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.modelScale == 0.0){
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDA760),
			TigRect(26, 419 , 87, 14), TigRect(1, 1, 87, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDA760),
			TigRect(117, 419, 87, 14), TigRect(1, 1, 87, 14));
	}
	else{
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_height),
			TigRect(25, 418, 47, 14), blackTextGenericStyle);
		DrawTextInWidgetCentered(widId,
			fmt::format("{}'{}\"", selPkt.height / 12, selPkt.height % 12),
			TigRect(78, 419, 33, 12), whiteTextGenericStyle);

		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_weight),
			TigRect(116, 418, 47, 14), blackTextGenericStyle);
		DrawTextInWidgetCentered(widId,
			fmt::format("{}",selPkt.weight),
			TigRect(169, 419, 32, 12), whiteTextGenericStyle);
	}
}

void UiPcCreation::RenderCharExpLvl(int widId) {
	auto &selPkt = GetCharEditorSelPacket();

	auto editedChar = GetEditedChar();

	if (editedChar && GetStatesComplete() >= ChargenStages::CG_Stage_Class && selPkt.classCode){
		if (!modSupport.IsCo8()){ // Co8 replaced the text with the background image
			DrawTextInWidgetCentered(widId,
				d20Stats.GetStatShortName(Stat::stat_experience),
				TigRect(25, 268, 37, 14), blackTextGenericStyle);
		}
		DrawTextInWidgetCentered(widId,
			"0", TigRect(68, 269, 57, 12), whiteTextGenericStyle);

		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_level),
			TigRect(134, 268, 37, 14), blackTextGenericStyle);
		auto lvl = critterSys.GetEffectiveLevel(editedChar);
		DrawTextInWidgetCentered(widId,
			fmt::format("{}", lvl), TigRect(177, 269, 24, 12), whiteTextGenericStyle);
	}
	else {
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDD41C),
			TigRect(26, 269, 99, 14), TigRect(1, 1, 99, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDAE0C),
			TigRect(135, 269, 68, 14), TigRect(1, 1, 68, 14));
	}
}

void UiPcCreation::RenderCharSavingThrows(int widId){
	auto &selPkt = GetCharEditorSelPacket();
	auto editedChar = GetEditedChar();
	if (editedChar && GetStatesComplete() >= ChargenStages::CG_Stage_Class && selPkt.classCode) {
		auto fortThrow = objects.StatLevelGet(editedChar, Stat::stat_save_fortitude);
		auto refThrow = objects.StatLevelGet(editedChar, Stat::stat_save_reflexes);
		auto willThrow = objects.StatLevelGet(editedChar, Stat::stat_save_willpower);
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_save_fortitude),
			TigRect(134, 330, 37, 14), blackTextGenericStyle);
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_save_reflexes),
			TigRect(134, 345, 37, 14), blackTextGenericStyle);
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_save_willpower),
			TigRect(134, 360, 37, 14), blackTextGenericStyle);

		DrawTextInWidgetCentered(widId, fmt::format("{}", fortThrow), 
			TigRect(177, 331, 24, 12), whiteTextGenericStyle);
		DrawTextInWidgetCentered(widId, fmt::format("{}", refThrow),
			TigRect(177, 346, 24, 12), whiteTextGenericStyle);
		DrawTextInWidgetCentered(widId, fmt::format("{}", willThrow),
			TigRect(177, 361, 24, 12), whiteTextGenericStyle);
	}
	else{
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDB928),
			TigRect(135, 331, 68, 14), TigRect(1, 1, 68, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDB928),
			TigRect(135, 361, 68, 14), TigRect(1, 1, 68, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDB928),
			TigRect(135, 346, 68, 14), TigRect(1, 1, 68, 14));
	}
}

void UiPcCreation::RenderCharHpAc(int widId){
	auto &selPkt = GetCharEditorSelPacket();
	auto editedChar = GetEditedChar();
	if (editedChar && GetStatesComplete() > ChargenStages::CG_Stage_Class) {
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_hp_current),
			TigRect(134, 291, 23, 14), blackTextGenericStyle);
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_ac),
			TigRect(134, 306, 23, 14), blackTextGenericStyle);
		
		DrawTextInWidgetCentered(widId, fmt::format("{}", objects.StatLevelGet(editedChar, Stat::stat_hp_max)),
			TigRect(165, 292, 37, 12), whiteTextGenericStyle);
		DrawTextInWidgetCentered(widId, fmt::format("{}", critterSys.GetArmorClass(editedChar)),
			TigRect(165, 307, 37, 12), whiteTextGenericStyle);
	}
	else {
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDA730),
			TigRect(135, 292, 68, 14), TigRect(1, 1, 68, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDA730),
			TigRect(135, 307, 68, 14), TigRect(1, 1, 68, 14));
	}
}

void UiPcCreation::RenderCharMovementInit(int widId) {
	auto &selPkt = GetCharEditorSelPacket();
	auto editedChar = GetEditedChar();
	if (editedChar && GetStatesComplete() > ChargenStages::CG_Stage_Class) {
		if (!modSupport.IsCo8()){
			DrawTextInWidgetCentered(widId,
				d20Stats.GetStatShortName(Stat::stat_movement_speed),
				TigRect(25, 398, 56, 14), blackTextGenericStyle);
			DrawTextInWidgetCentered(widId,
				d20Stats.GetStatShortName(Stat::stat_initiative_bonus),
				TigRect(25, 383, 56, 14), blackTextGenericStyle);
		}
		auto moveSpeed = objects.StatLevelGet(editedChar, Stat::stat_movement_speed);
		DrawTextInWidgetCentered(widId, fmt::format("{}", moveSpeed),
			TigRect(87, 399, 24, 12), whiteTextGenericStyle);
		DrawTextInWidgetCentered(widId, fmt::format("{}", objects.StatLevelGet(editedChar, Stat::stat_initiative_bonus)),
			TigRect(87, 384, 24, 12), whiteTextGenericStyle);
	}
	else {
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDAEEC),
			TigRect(26, 399, 87, 14), TigRect(1, 1, 87, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDAEEC),
			TigRect(26, 384, 87, 14), TigRect(1, 1, 87, 14));
	}
}

void UiPcCreation::RenderCharToHitBonus(int widId){
	auto &selPkt = GetCharEditorSelPacket();
	auto editedChar = GetEditedChar();


	if (!modSupport.IsCo8()) {
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_melee_attack_bonus),
			TigRect(116, 383, 56, 14), blackTextGenericStyle);
		DrawTextInWidgetCentered(widId,
			d20Stats.GetStatShortName(Stat::stat_ranged_attack_bonus),
			TigRect(116, 398, 56, 14), blackTextGenericStyle);
	}

	if (editedChar && GetStatesComplete() > ChargenStages::CG_Stage_Class) {
		
		auto meleeBonus = objects.StatLevelGet(editedChar, Stat::stat_melee_attack_bonus);
		auto rangedBonus = objects.StatLevelGet(editedChar, Stat::stat_ranged_attack_bonus);

		std::string s;
		if (meleeBonus >= 0)
			s.append("+");
		s.append(fmt::format("{}", meleeBonus));
		DrawTextInWidgetCentered(widId, s,
			TigRect(178, 384, 24, 12), whiteTextGenericStyle);

		s.clear();
		if (rangedBonus >= 0)
			s.append("+");
		s.append(fmt::format("{}", rangedBonus));
		DrawTextInWidgetCentered(widId, s,
			TigRect(178, 399, 24, 12), whiteTextGenericStyle);
	}
	else {
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDAEEC),
			TigRect(117, 399, 87, 14), TigRect(1, 1, 87, 14));
		UiRenderer::DrawTextureInWidget(widId, temple::GetRef<int>(0x10BDAEEC),
			TigRect(117, 384, 87, 14), TigRect(1, 1, 87, 14));
	}
}

void UiPcCreation::DrawTextInWidgetCentered(int widgetId, const string & text, const TigRect & rect, const TigTextStyle & style)
{
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidgetCentered(widgetId, text, rect, style);
	UiRenderer::PopFont();
}


int UiPcCreation::GetRaceWndPage()
{
	return 0;
}


int& UiPcCreation::GetStatesComplete(){
	return temple::GetRef<int>(0x10BDD5D4);
}

void UiPcCreation::ClassSetPermissibles(){

	auto page = GetClassWndPage();
	auto idx = 0;
	auto handle = GetEditedChar();

	for (auto it : classBtnIds) {
		auto classCode = GetClassCodeFromWidgetAndPage(idx++, page);
		if (classCode == (Stat)-1)
			uiManager->SetButtonState(it, LgcyButtonState::Disabled);

		auto isValid = true;

		if (!d20ClassSys.ReqsMet(handle, classCode)){
			isValid = false;
		}
		if (!IsCastingStatSufficient(classCode) || !IsAlignmentOk(classCode))
			isValid = false;
		
		if (isValid){
			uiManager->SetButtonState(it, LgcyButtonState::Normal);
		}
		else {
			uiManager->SetButtonState(it, LgcyButtonState::Disabled);
		}

	}

	if (!config.newClasses) {
		uiManager->SetButtonState(classNextBtn, LgcyButtonState::Disabled);
		uiManager->SetButtonState(classPrevBtn, LgcyButtonState::Disabled);
		return;
	}

	if (page > 0)
		uiManager->SetButtonState(classPrevBtn, LgcyButtonState::Normal);
	else
		uiManager->SetButtonState(classPrevBtn, LgcyButtonState::Disabled);

	if (page < mPageCount - 1)
		uiManager->SetButtonState(classNextBtn, LgcyButtonState::Normal);
	else
		uiManager->SetButtonState(classNextBtn, LgcyButtonState::Disabled);
}


void UiPcCreation::DeitySetPermissibles(){
	for (auto i = 0; i < DEITY_BTN_COUNT; i++){
		if (deitySys.CanPickDeity(GetEditedChar(), i)){
			uiManager->SetButtonState(GetDeityBtnId(i), LgcyButtonState::Normal);
		} else
			uiManager->SetButtonState(GetDeityBtnId(i), LgcyButtonState::Disabled);	
	}
}

bool UiPcCreation::IsCastingStatSufficient(Stat classEnum){

	auto handle = GetEditedChar();

	if (d20ClassSys.IsCastingClass(classEnum) 
		&& !d20ClassSys.IsLateCastingClass(classEnum)){
		return objects.StatLevelGet(handle, d20ClassSys.GetSpellStat(classEnum)) > 10;
	}
	return true;
}

bool UiPcCreation::IsAlignmentOk(Stat classCode){

	if (!(config.laxRules && config.disableAlignmentRestrictions)) {
		auto hasOkAlignment = false;;
		static std::vector<Alignment> alignments{
			ALIGNMENT_LAWFUL_GOOD, ALIGNMENT_LAWFUL, ALIGNMENT_LAWFUL_EVIL,
			ALIGNMENT_GOOD, ALIGNMENT_NEUTRAL, ALIGNMENT_EVIL,
			ALIGNMENT_CHAOTIC_GOOD, ALIGNMENT_CHAOTIC, ALIGNMENT_CHAOTIC_EVIL };

		auto partyAlignment = GetPartyAlignment();

		// make sure class is compatible with at least one alignment that is compatible with the party alignment
		for (auto al : alignments) {

			if (d20Stats.AlignmentsUnopposed(al, partyAlignment)) {
				if (d20ClassSys.IsCompatibleWithAlignment(classCode, al)) {
					hasOkAlignment = true;
					break;
				}
			}
		}
		if (!hasOkAlignment)
			return false;
	}
	return true;
}

void UiPcCreation::ClassScrollboxTextSet(Stat classEnum){
	temple::GetRef<void(__cdecl)(Stat)>(0x1011B920)(classEnum);
}

void UiPcCreation::ButtonEnteredHandler(int helpId){
	temple::GetRef<void(__cdecl)(int)>(0x1011B890)(helpId);
}

void UiPcCreation::ButtonEnteredHandler(const std::string &s)
{
	ButtonEnteredHandler(ElfHash::Hash(s));
}

int UiPcCreation::GetNewLvl(Stat classEnum) {
	return 1;
}

int UiPcCreation::GetProtoIdByRaceGender(Race race, int genderId)
{
	auto raceProto = d20RaceSys.GetProtoId(race);
	return raceProto + (1 - genderId);
}


bool UiPcCreation::IsSelectingRangerSpec()
{
	return false;
	/*auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();
	auto isRangerSpecial = selPkt.classCode == stat_level_ranger && (objects.StatLevelGet(handle, stat_level_ranger) + 1) == 2;
	return isRangerSpecial;
	*/
}


int &UiPcCreation::GetDeityBtnId(int deityId){
	return temple::GetRef<int[20]>(0x10C3EE80)[deityId];
}

#pragma region Race

RaceChargen::RaceChargen(const UiSystemConf & conf) : ChargenSystem(){
	mStage = ChargenStages::CG_Stage_Race;
	SystemInit(&conf);
}

BOOL RaceChargen::SystemInit(const UiSystemConf *conf){
	ChargenSystem::SystemInit(conf);
	WidgetsInit(conf->width, conf->height);
	return TRUE;
}

bool RaceChargen::WidgetsInit(int w, int h){

	mWnd->SetSize({ 431, 250 });
	auto wndX = 219 + ((w - 788) / 2);
	auto wndY = 50  + ((h <= 600) ? 12 : ((h - 497) / 2));
	mWnd->SetPos(wndX, wndY);

	AddPageButtonsToWnd(mWnd);
	auto numSelectables = d20RaceSys.selectableBaseRaces.size();
	auto numPages = (numSelectables / 7) + ((numSelectables % 7)?1:0);
	SetPageCount(numPages);
	
	// Race Buttons
	auto x = 156, y = 24;
	for (auto it: d20RaceSys.vanillaRaceEnums){
		auto newBtn = make_unique<ChargenPagedButton>();
		newBtn->SetPos(x, y);
		
		for (auto i= it; i < d20RaceSys.selectableBaseRaces.size(); i+= 7){
			auto race = (RaceBase)d20RaceSys.selectableBaseRaces[i];
			auto raceName = toupper(d20Stats.GetRaceName((Race)race));
			if (i == it)
				newBtn->SetText(raceName);
			newBtn->SetPageText(i/7, raceName);
			newBtn->SetPageDatum(i/7, d20RaceSys.selectableBaseRaces[i]);
		}
		
		auto &pbtn = *newBtn;
		newBtn->SetClickHandler([&](){
			auto raceBase = (RaceBase)pbtn.GetDatum();
			auto race = (Race)raceBase;
			if (helpSys.IsClickForHelpActive()){
				helpSys.PresentWikiHelp(101 + race, D20HelpType::Races);
				return;
			}
			auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
			selPkt.raceId = (Race)race;
			UpdateDescriptionBox();
			uiPcCreation.ResetNextStages(ChargenStages::CG_Stage_Race);
			UpdateActiveRace();

		});
		newBtn->SetMouseMsgHandler([=](const TigMouseMsg&msg){
			return true;
		});
		newBtn->SetWidgetMsgHandler([&](const TigMsgWidget &msg){
			auto race = (Race)pbtn.GetDatum();
			if (msg.widgetEventType == TigMsgWidgetEvent::Entered){
				SetScrollboxText(race);
				return true;
			}
			if (msg.widgetEventType == TigMsgWidgetEvent::Exited) {
				UpdateScrollbox();
				return true;
			}
			return true;
		});
		newBtn->SetActivationState(ChargenBigButton::ChargenButtonActivationState::Active);
		mBigButtons.push_back(newBtn->GetWidgetId());
		mWnd->Add(std::move(newBtn));
		y += 29;
	}
	
	// Subrace Buttons
	x = 306; y = 24;
	for (auto it : {0,1,2,3,4,5,6}) {
		auto newBtn = make_unique<ChargenPagedButton>();
		auto race = (Race)it;
		auto raceName = toupper(d20Stats.GetRaceName(race));
		newBtn->SetPos(x, y);

		if (it == 0){
			newBtn->SetText("COMMON");
		} else
			newBtn->SetText("SUBRACE");
		
		newBtn->SetPageText(0, raceName);
		newBtn->SetPageDatum(0, it);

		auto &pbtn = *newBtn;
		newBtn->SetClickHandler([&]() {
			auto race = (Race)pbtn.GetDatum();
			if (helpSys.IsClickForHelpActive()) {
				helpSys.PresentWikiHelp(101 + race, D20HelpType::Races);
				return;
			}
			auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
			selPkt.raceId = race;
			UpdateDescriptionBox();
			uiPcCreation.ResetNextStages(ChargenStages::CG_Stage_Race);
			UpdateActiveRace();
		});
		newBtn->SetMouseMsgHandler([=](const TigMouseMsg&msg) {
			return true;
		});
		newBtn->SetWidgetMsgHandler([&](const TigMsgWidget &msg) {
			auto race = (Race)pbtn.GetDatum();
			if (msg.widgetEventType == TigMsgWidgetEvent::Entered) {
				SetScrollboxText(race);
				return true;
			}
			if (msg.widgetEventType == TigMsgWidgetEvent::Exited) {
				UpdateScrollbox();
				return true;
			}
			return true;
		});
		newBtn->SetActivationState(ChargenBigButton::ChargenButtonActivationState::Active);
		newBtn->Hide();
		mSubraceBtns.push_back(newBtn->GetWidgetId());

		mWnd->Add(std::move(newBtn));
		y += 29;
	}

	SetPageUpdateHandler([&](){
		for (auto it:mBigButtons){
			auto btn = (ChargenPagedButton*)uiManager->GetAdvancedWidget(it);
			if (!btn) continue;
			auto curPage = GetPage();
			if (curPage >= btn->GetPageCount() ){
				btn->Hide();
				continue;
			}
			btn->Show();
			btn->SetPage(curPage);
			btn->SetText(btn->GetText());
		}
	});
	
	mWnd->Hide();

	return true;
}

void RaceChargen::Reset(CharEditorSelectionPacket & charSpec) {
	charSpec.raceId = RACE_INVALID;
	SetPage(0);
	UpdateActiveRace();
	UpdateSubraceButtons((RaceBase)RACE_INVALID);
}

BOOL RaceChargen::CheckComplete(){
	return (uiPcCreation.GetCharEditorSelPacket().raceId != RACE_INVALID)?TRUE:FALSE;
}


void RaceChargen::SetScrollboxText(Race race)
{
	temple::GetRef<void(__cdecl)(Race)>(0x1011BAE0)(race);
}

void RaceChargen::UpdateScrollbox()
{
	auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
	if (selPkt.raceId == RACE_INVALID){
		uiPcCreation.ButtonEnteredHandler("TAG_CHARGEN_RACE");
	}
	else{
		SetScrollboxText(selPkt.raceId);
	}
}

void RaceChargen::UpdateActiveRace()
{
	auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
	auto raceBase = d20RaceSys.GetBaseRace(selPkt.raceId);
	if (selPkt.raceId == RACE_INVALID){
		raceBase = (RaceBase)RACE_INVALID;
	}
	 
	UpdateSubraceButtons(raceBase);

	for (auto it: mBigButtons){
		auto btn = dynamic_cast<ChargenPagedButton*>(uiManager->GetAdvancedWidget(it));
		if (!btn) continue;
		auto isActive = raceBase == btn->GetDatum();
		btn->SetActive(isActive);
	}
	for (auto it: mSubraceBtns){
		auto btn = dynamic_cast<ChargenPagedButton*>(uiManager->GetAdvancedWidget(it));
		if (!btn) continue;
		auto isActive = selPkt.raceId == btn->GetDatum();
		btn->SetActive(isActive);
	}
}

void RaceChargen::UpdateSubraceButtons(RaceBase raceBase)
{
	auto race = (Race)raceBase;
	for (auto it: mSubraceBtns){
		auto subraceBtn = (ChargenPagedButton*)uiManager->GetAdvancedWidget(it);
		subraceBtn->Hide();
	}

	if (raceBase == RACE_INVALID)
		return;

	if (!d20RaceSys.HasSubrace(race))
		return;
	
	auto subraceBtn = (ChargenPagedButton*)uiManager->GetAdvancedWidget(mSubraceBtns[0]);
	subraceBtn->SetPageDatum(0, raceBase);
	subraceBtn->Show();


	auto &subraces = d20RaceSys.GetSubraces(raceBase);
	auto i = 1;
	for (auto it : subraces) {
		auto subrace = it;
		auto subraceBtn = (ChargenPagedButton*)uiManager->GetAdvancedWidget(mSubraceBtns[i]);
		subraceBtn->SetPageDatum(0, it);
		auto raceName = toupper(d20Stats.GetRaceName(subrace));
		subraceBtn->SetText(raceName);
		subraceBtn->Show();
		i++;
	}

	
}

#pragma endregion

ClassChargen::ClassChargen(const UiSystemConf & conf){
}


BOOL ChargenSystem::SystemInit(const UiSystemConf *) {

	mWnd = make_unique<WidgetContainer>(431, 250);

	MakeStateTitle();
	return TRUE;
}

void ChargenSystem::UpdateDescriptionBox() {
	//temple::GetRef<void(__cdecl)()>(0x1011C470)();

	auto &textBuffer = temple::GetRef<char[2000]>(0x10BDB100);
	auto &selPkt = uiPcCreation.GetCharEditorSelPacket();
	
	std::string desc;
	
	// Alignment
	if (selPkt.alignment != -1){
		auto s = d20Stats.GetAlignmentName(selPkt.alignment);
		if (s)
			desc.append(s);
		desc.append(" ");
	}
	// Gender
	if (selPkt.genderId != 2){
		auto s = d20Stats.GetGenderName(selPkt.genderId);
		if (s)
			desc.append(s);
		desc.append(" ");
	}
	// Race
	if (selPkt.raceId != RACE_INVALID){
		auto s = d20Stats.GetRaceName(selPkt.raceId);
		if (s)
			desc.append(s);
		desc.append(" ");
	}
	// Deity
	if (selPkt.deityId != DEITY_COUNT_SELECTABLE_VANILLA){
		desc.append("@1");
		desc.append(temple::GetRef<const char*>(0x10BDA76C)); // "Worships"
		desc.append("@0");
		desc.append(deitySys.GetName(selPkt.deityId));
	}

	// copy string to dislpay buffer
	memcpy(textBuffer, desc.c_str(), desc.size());
	textBuffer[desc.size()] = '\0';

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	auto met = UiRenderer::MeasureTextSize(desc, temple::GetRef<TigTextStyle>(0x10BDDCC8));

	UiRenderer::PopFont();

	auto &rect = temple::GetRef<TigRect>(0x10BDB004);
	rect.x = (439 - met.width) / 2 + 215;
	rect.y = (16 - met.height) / 2 + 28;
	rect.height = met.height;
	rect.width = met.width;
}

void ChargenSystem::MakeStateTitle()
{
	auto stateTitle = make_unique<WidgetText>();
	stateTitle->SetX(4); stateTitle->SetY(4);
	stateTitle->SetFixedHeight(14);
	stateTitle->SetCenterVertically(true);
	if (modSupport.IsCo8()) {
		stateTitle->SetStyle(widgetTextStyles->GetTextStyle("priory-title"));
	}
	else {
		stateTitle->SetStyle(widgetTextStyles->GetTextStyle("arial-10-title-text"));
		stateTitle->SetFixedHeight(14);
	}
	auto &stateTitles = temple::GetRef<const char*[14]>(0x10BDAE28);
	stateTitle->SetText(stateTitles[mStage]);
	mWnd->AddContent(std::move(stateTitle));
}
