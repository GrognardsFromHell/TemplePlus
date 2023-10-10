
#include "stdafx.h"
#include "ui_dialog.h"
#include <temple/dll.h>
#include "ui.h"
#include "tig/tig_font.h"
#include "tig/tig_msg.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/d20/d20_help.h"
#include "widgets/widgets.h"
#include "ui_systems.h"
#include "ui_ingame.h"
#include "../dialog.h"
#include "ui/ui_legacysystems.h"
#include "tig/tig_texture.h"
#include "ui_render.h"
#include "graphics/render_hooks.h"
#include <map>
#include "gamesystems/legacysystems.h"
#include "mod_support.h"
#include "gamesystems\objects\objsystem.h"

UiDlg* uiDialog = nullptr;
UiDialogImpl * dlgImpl = nullptr;

enum UiDialogFlags : uint32_t
{
	UNK1 = 1, 
	DialogHistoryShow = 2
};

struct DialogSlot
{
	int dialogHandle;
	int unk;
	DialogState state;
	int scriptId;
	int unk_186C;
	int dialogEngaged;
	int unk1874;
};

static struct UiDialogAddresses : temple::AddressTable {

	void(__cdecl *Hide)();
	bool(__cdecl *IsDialogEngaged)();
	DialogState* (__cdecl *GetCurrentDlg)();
	void(__cdecl *ReShowDialog)(DialogState *info, int line);
	void(__cdecl *Unk)(); // This is speech related
	int(__cdecl *ShowTextBubble)(objHndl pc, objHndl speakingTo, const char *text, int speechId);

	UiDialogAddresses() {
		rebase(Hide, 0x1014CA20);
		rebase(IsDialogEngaged, 0x1014BFE0);
		rebase(GetCurrentDlg, 0x1014BA30);
		rebase(ReShowDialog, 0x100388D0);
		rebase(Unk, 0x1014C8F0);
		rebase(ShowTextBubble, 0x1014CDE0);
	}
} addresses;



class UiDialogImpl {
	friend class UiDlg;
	friend class UiDialogHooks;
public:
	UiDialogImpl(const UiSystemConf &config);

	void DialogScrollbarReset();
	
protected:
	BOOL WidgetsInit(int w, int h);
	void WidgetsFree();

	BOOL ResponseWidgetsInit(int w, int h);
	void ResponseWidgetsFree();

	BOOL DummyWndInit(int w, int h);


	DialogSlot & mSlot = temple::GetRef<DialogSlot>(0x10BEA918);

	int & mIsActive = temple::GetRef<int>(0x10BEC348);
	uint32_t& mFlags = temple::GetRef<uint32_t>(0x10BEA5F4);
	int &mLineCount = temple::GetRef<int>(0x10BEC194);
	DialogMini *& mDlgLineList = temple::GetRef<DialogMini*>(0x10BEC1A4);
	DialogMini *& mDlgLineList2 = temple::GetRef<DialogMini*>(0x10BEC1A0);
	char **mResponseTexts = temple::GetPointer<char*>(0x10BE9B38); // size 5 array
	std::map<int, std::string> mResponseNumbers; // = temple::GetPointer<char*>(0x10BEA5F8); // size 5 array
	int *mSkillsUsedInReply = temple::GetPointer<int>(0x10BEC214); // size 5 array

	LgcyWidgetId& mWndId = temple::GetRef<LgcyWidgetId>(0x10BEA2E4);
	LgcyWidgetId &mResponseWndId = temple::GetRef< LgcyWidgetId>(0x10BEC204);
	LgcyWidgetId &mDummyWndId = temple::GetRef<LgcyWidgetId>(0x10BEC198);
	LgcyWidgetId &mScrollbarId = temple::GetRef<LgcyWidgetId>(0x10BEC19C);
	LgcyWidgetId &mHeadBtnId = temple::GetRef<LgcyWidgetId>(0x10BEC210);
	LgcyWidgetId *mReplyBtnIds = temple::GetPointer<LgcyWidgetId>(0x10BEA8A8); // size 5 array

	int & scrollbarYmax = temple::GetRef<int>(0x10BEC2D4);
	int & scrollbarY = temple::GetRef<int>(0x10BEA5F0);

	int &mHeadBtnTexNormal = temple::GetRef<int>(0x10BEA910), 
	&mHeadBtnTexHovered = temple::GetRef<int>(0x10BEC190),
	&mHeadBtnTexPressed = temple::GetRef<int>(0x10BEA5EC),
	&mHeadBtnTexDisabled = temple::GetRef<int>(0x10BEA5D0);

	ImgFile *& backdrop= temple::GetRef<ImgFile*>(0x10BEC330);
	ImgFile *& backdropMini1 = temple::GetRef<ImgFile*>(0x10BEC1A8);
	ImgFile *& backdropMini2 = temple::GetRef<ImgFile*>(0x10BEC208);
	ImgFile *& backdropMini3 = temple::GetRef<ImgFile*>(0x10BEC2D0);
	ImgFile *& backdropMini  = temple::GetRef<ImgFile*>(0x10BE9B4C); // chooses from one of the above 3
	ImgFile* &mResponseBarImg = temple::GetRef<ImgFile*>(0x10BEC200);
	int mSkillTextures[6] = {0,}; //= temple::GetPointer<int>(0x10BEA5D4); //size 6 array;

	TigTextStyle yellowStyle, darkGrayStyle 
	, replyStyleNormal 
	, replyStyleHovered 
	, replyStylePressed 
	, skilledReplyNormal 
	, skilledReplyHovered 
	, skilledReplyPressed ;

	int & historyWndX = temple::GetRef<int>(0x10BEA2E0);
	int & historyWndY = temple::GetRef<int>(0x10BE9FF0);
	int & textMinY = temple::GetRef<int>(0x10BEC20C);

	TigRect &mHeadBtnTgtRect = temple::GetRef<TigRect>(0x10BEC334);
	TigRect mResponseRects[5]; //= temple::GetPointer<TigRect>(0x10BE9A38); //size 5 array
	TigRect mResponseNumberRects[5]; // = temple::GetPointer<TigRect>(0x10BEA8C0); //size 5 array;
	TigRect mSkillIconRects[5]; //= temple::GetPointer<TigRect>(0x10BE9F50); //size 5 array;
	int &mMusicVolume = temple::GetRef<int>(0x10BEA8A4);
	


	// std::unique_ptr<WidgetContainer> mWnd;

	void ShowDialogWidgets();
	void UpdateWidgets();



};


class UiDialogHooks : public TempleFix
{
	void apply() override{
		replaceFunction<void(__cdecl)()>(0x1014C030, [](){
			if (dlgImpl){
				dlgImpl->UpdateWidgets();
			}
		});
		static void(__cdecl * UiDialogBegin)() = replaceFunction<void(__cdecl)()>(0x1014C8F0, []() {
			/* 
				Global variable ObjHnd(0x10BEA958) is not re-validated after map change, or handle was actually destroyed.
				Should be nulled if invalid. Otherwise crash.
			*/
			if (!objSystem->IsValidHandle(temple::GetRef<objHndl>(0x10BEA958)))
			{
				temple::GetRef<objHndl>(0x10BEA958) = objHndl::null;
			}
			UiDialogBegin();
		});

		static void(__cdecl * orgDialogChoiceParse)(DialogState*, int) =
			replaceFunction<void(__cdecl)(DialogState*, int)>(0x10038A50, [](DialogState *state, int responseIdx) 
				{
					if (!state) {
						return;
					}
					if (responseIdx >= 0 && responseIdx < DIALOG_REPLIES_MAX) {
						logger->info("DialogChoiceParse: NPC text \"{}\" ", state->npcLineText );
						logger->info("DialogChoiceParse: Dialog response chosen: {} response ID: {}", responseIdx, state->npcReplyIds[responseIdx]);
					}
					return orgDialogChoiceParse(state, responseIdx);
				});
	}
} uiDialogHooks;

//*****************************************************************************
//* Dlg-UI
//*****************************************************************************

/* 0x1014dd40 */
UiDlg::UiDlg(const UiSystemConf &config) {
	mImpl = std::make_unique<UiDialogImpl>(config);
	Expects(!uiDialog);
	uiDialog = this;
	dlgImpl = mImpl.get();
}

/* 0x1014ccc0 */
UiDlg::~UiDlg() {
	
	uiManager->RemoveWindow(mImpl->mDummyWndId);
	uiManager->RemoveWidget(mImpl->mDummyWndId);
	mImpl->WidgetsFree();

	for (auto it= mImpl->mDlgLineList; it != nullptr; it = mImpl->mDlgLineList){
		auto curLine = it;
		mImpl->mDlgLineList = it->prev;
		if (it->lineText){
			free((char*)it->lineText);
		}
		free(curLine);
	}

	temple::GetRef<void(__cdecl)()>(0x10034AB0)(); // frees speech stream

	uiDialog = nullptr;
}

/* 0x1014de30 */
void UiDlg::ResizeViewport(const UiResizeArgs& resizeArg) {
	
	uiManager->RemoveWindow(mImpl->mDummyWndId);
	uiManager->RemoveWidget(mImpl->mDummyWndId);

	mImpl->WidgetsFree();
	mImpl->WidgetsInit(resizeArg.rect1.width, resizeArg.rect1.height);
	mImpl->DummyWndInit(resizeArg.rect1.width, resizeArg.rect1.height);
}
void UiDlg::Reset() {
	auto reset = temple::GetPointer<void()>(0x1014ccf0);
	reset();
}
bool UiDlg::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1014c830);
	return save(file) == 1;
}
bool UiDlg::LoadGame(const UiSaveFile &save) {
	auto load = temple::GetPointer<int(const UiSaveFile*)>(0x1014cd50);
	return load(&save) == 1;
}
const std::string &UiDlg::GetName() const {
	static std::string name("Dlg-UI");
	return name;
}

bool UiDlg::IsActive() const
{
	return !(mImpl->mFlags & 1) || !uiManager->IsHidden(mImpl->mWndId);
}

bool UiDlg::IsDialogEngaged() const
{
	return mImpl->mSlot.dialogEngaged != FALSE;
}



class UiDialogStyles {
	public:
		static ColorRect pureYellow;
		static ColorRect darkGray;

		static ColorRect brightYellow;
		static ColorRect brightYellow2;
		static ColorRect brightYellow3;

		static ColorRect pureGreen;
		static ColorRect darkGreen;
		static ColorRect pureWhite;

		UiDialogStyles();
};

void UiDlg::Hide() {
	addresses.Hide();
}


DialogState* UiDlg::GetCurrentDialog() {
	return addresses.GetCurrentDlg();
}

/* 0x1014C340 */
void UiDialogImpl::ShowDialogWidgets()
{
	uiSystems->GetInGame().ResetInput();
	
	uiManager->SetHidden(mDummyWndId, false);
	uiManager->BringToFront(mDummyWndId);
	// temple::GetRef<void(__cdecl)()>(0x1014C340)();
}

/* 0x1014C030 */
void UiDialogImpl::UpdateWidgets(){

	if (!mIsActive){
		if (mFlags & UiDialogFlags::DialogHistoryShow) {
			textMinY = 26;
			auto scrollbar = uiManager->GetScrollBar(mScrollbarId);
			scrollbar->height = 261;
			uiManager->SetHidden(mScrollbarId, false);
			uiManager->SetHidden(mResponseWndId, true);
			
			auto headBtn = uiManager->GetButton(mHeadBtnId);
			auto wnd = uiManager->GetWindow(mWndId);
			headBtn->y = wnd->y + 1;
			mHeadBtnTgtRect.y = wnd->y + 1;

			uiSystems->GetUtilityBar().DialogBtnHide();
			return;
		}
		else {
			uiSystems->GetUtilityBar().DialogHistoryBtnToggle();
		}
		return;
	}

	
	auto headBtn = uiManager->GetButton(mHeadBtnId);
	auto wnd = uiManager->GetWindow(mWndId);
	if (mFlags & UiDialogFlags::DialogHistoryShow){
		textMinY = 26;
		auto scrollbar = uiManager->GetScrollBar(mScrollbarId);
		scrollbar->height = 126;
		uiManager->SetHidden(mScrollbarId, false);
		uiManager->SetHidden(mResponseWndId, false);
		uiManager->BringToFront(mResponseWndId);
		uiSystems->GetUtilityBar().DialogBtnHide();

		headBtn->y = wnd->y + 1;
		mHeadBtnTgtRect.y = wnd->y + 1;
		return;
	}
	
	uiManager->SetHidden(mScrollbarId, true);
	uiManager->SetHidden(mResponseWndId, false);
	uiManager->BringToFront(mResponseWndId);
	uiSystems->GetUtilityBar().DialogBtnHide();
	if (!mDlgLineList){
		textMinY = -90;
		backdropMini = backdropMini1;
		mHeadBtnTgtRect.y = headBtn->y = wnd->y + 109;
		historyWndY = wnd->y + 126;
	}
	else{
		if (config.enlargeDialogFonts)
			UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		else
			UiRenderer::PushFont(PredefinedFont::ARIAL_10);
		auto meas = UiRenderer::MeasureTextSize(mDlgLineList->lineText, yellowStyle, 550, 0);
		UiRenderer::PopFont();
		if (modSupport.IsCo8()){
			textMinY = -90;
			backdropMini = backdropMini1;
			mHeadBtnTgtRect.y = headBtn->y = wnd->y + 59;
			historyWndY = wnd->y + 77;
		}
		else if (meas.height > 26){
			textMinY = -58;
			backdropMini = backdropMini3;
			mHeadBtnTgtRect.y = headBtn->y = wnd->y + 77;
			historyWndY = wnd->y + 94;
		}
		else if (meas.height > 13){
			textMinY = -77;
			backdropMini = backdropMini2;
			mHeadBtnTgtRect.y = headBtn->y = wnd->y + 96;
			historyWndY = wnd->y + 113;
		}
		else{
			textMinY = -90;
			backdropMini = backdropMini1;
			mHeadBtnTgtRect.y = headBtn->y = wnd->y + 109;
			historyWndY = wnd->y + 126;
		}
	}
	historyWndX = wnd->x;
	return;
}

void UiDlg::ReShowDialog(DialogState* info, int line) {
	addresses.ReShowDialog(info, line);
}

void UiDlg::Unk() {
	addresses.Unk();
}

void UiDlg::ShowTextBubble(objHndl speaker, objHndl speakingTo, const string &text, int speechId) {
	addresses.ShowTextBubble(speaker, speakingTo, text.c_str(), speechId);
}

UiDialogImpl::UiDialogImpl(const UiSystemConf & conf)
{
	if (modSupport.IsCo8()){
		::config.enlargeDialogFonts = true;
	}

	TigTextStyle templateStyle = TigTextStyle::standardWhite;
	templateStyle.tracking = 3;
	templateStyle.field2c = -1;

	yellowStyle = darkGrayStyle = replyStylePressed = replyStyleHovered = replyStyleNormal
		= skilledReplyNormal = skilledReplyHovered = skilledReplyPressed = templateStyle;

	yellowStyle.SetColors(&UiDialogStyles::pureYellow);
	darkGrayStyle.SetColors(&UiDialogStyles::darkGray);

	replyStyleNormal.SetColors(&UiDialogStyles::pureGreen);
	replyStyleHovered.SetColors(&UiDialogStyles::darkGreen);
	replyStylePressed.SetColors(&UiDialogStyles::pureWhite);

	skilledReplyNormal.SetColors(&UiDialogStyles::brightYellow);
	skilledReplyHovered.SetColors(&UiDialogStyles::brightYellow2);
	skilledReplyPressed.SetColors(&UiDialogStyles::brightYellow3);
	

	mMusicVolume = temple::GetRef<int(__cdecl)()>(0x1003C9F0)();

	MesFile::Content dlgUiMes = MesFile::ParseFile("mes\\dlg_ui.mes");
	for (auto i=0; i < 5; ++i){
		if (dlgUiMes[i].find(i))
			mResponseNumbers[i] = dlgUiMes[i].c_str();
	}

	mSlot.dialogEngaged = 0;
	mSlot.unk_186C = 0;
	auto scriptDialogCb = temple::GetRef<void(__cdecl)(objHndl, objHndl, int, int, int)>(0x1014CED0);
	auto dialogBubbleCb = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int)>(0x1014CDE0);
	auto dialogEndCb = temple::GetRef<void(__cdecl)(objHndl, int)>(0x1014BA40);
	gameSystems->GetScript().SetDialogFuncs(scriptDialogCb, dialogBubbleCb);
	gameSystems->GetAI().SetDialogFuncs( dialogEndCb ,dialogBubbleCb);

	mFlags = 1;
	mLineCount = 0;

	// Write-only values?
	temple::GetRef<int>(0x10BEC34C) = 1;
	temple::GetRef<int>(0x10BEC350) = 0;
	mDlgLineList = nullptr;
	mDlgLineList2 = nullptr;
	

	WidgetsInit(conf.width, conf.height);
	DummyWndInit(conf.width, conf.height);
}

/* 0x1014BDF0 */
void UiDialogImpl::DialogScrollbarReset()
{
	scrollbarYmax = mLineCount - 1;
	scrollbarY = mLineCount - 1;
	uiManager->ScrollbarSetYmax(mScrollbarId, scrollbarYmax);
	uiManager->ScrollbarSetY(mScrollbarId, scrollbarY);
}

/* 0x1014CAC0 */
void UiDlg::ShowHistory()
{
	return temple::GetRef<void(__cdecl)()>(0x1014CAC0)();
}

/* 0x1014D9B0 */
BOOL UiDialogImpl::WidgetsInit(int w, int h)
{
	
	LgcyWindow dlgWnd(9, h - 374, 611, 292);
	dlgWnd.flags = 1;
	dlgWnd.render = [](int widId){
		if (config.enlargeDialogFonts)
			UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		else
			UiRenderer::PushFont(PredefinedFont::ARIAL_10);
		const int LINE_WIDTH = 550;

		auto flags = dlgImpl->mFlags;
		auto wnd = uiManager->GetWindow(widId);
		auto responseWnd = uiManager->GetWindow(dlgImpl->mResponseWndId);
		
		// Render background image
		auto x = wnd->x, y = wnd->y + 18;
		auto wndBackdrop = dlgImpl->backdrop;
		if ( !( flags & UiDialogFlags::DialogHistoryShow) ){
			x = dlgImpl->historyWndX; y = dlgImpl->historyWndY;
			wndBackdrop = dlgImpl->backdropMini;
		}
		RenderHooks::RenderImgFile(wndBackdrop, x, y);

		TigRect textRect(14, 
			dlgImpl->mIsActive ? (wnd->height - 5 - responseWnd->height) : (wnd->height - 6), 
			LINE_WIDTH, 0);
		
		auto responses = dlgImpl->mDlgLineList;
		if (flags & UiDialogFlags::DialogHistoryShow){
			auto ymax = dlgImpl->scrollbarYmax;
			auto scrollbarY = dlgImpl->scrollbarY;
			while (responses && scrollbarY < ymax ){
				responses = responses->prev;
				ymax--;
			}
		}

		while (responses){
			auto lineText = responses->lineText;
			auto textSize = UiRenderer::MeasureTextSize(lineText, dlgImpl->yellowStyle, LINE_WIDTH, 0);
			textRect.height = textSize.height;
			textRect.y -= textSize.height + 1;
			if (textRect.y < dlgImpl->textMinY)
				break;
			auto isPcLine = (responses->flags & 1) != 0;

			UiRenderer::DrawTextInWidget(widId, responses->lineText, textRect, 
				isPcLine ? dlgImpl->darkGrayStyle : dlgImpl->yellowStyle);
			if (! (flags & UiDialogFlags::DialogHistoryShow) ){
				break;
			}
			textRect.y--; // create additional spacing between history lines..
			responses = responses->prev;
		}

		UiRenderer::PopFont();
	};
	dlgWnd.handleMessage = [](int widId, TigMsg* msg){

		if (msg->type == TigMsgType::WIDGET){
			auto widMsg = (TigMsgWidget*)(msg);
			if (widMsg->widgetEventType == TigMsgWidgetEvent::Scrolled){
				uiManager->ScrollbarGetY(dlgImpl->mScrollbarId, &dlgImpl->scrollbarY);
				return TRUE;
			}
			if (widMsg->widgetEventType == TigMsgWidgetEvent::MouseReleased) {
				helpSys.PresentWikiHelpIfActive(19);
			}
			return TRUE;
		}
		if (msg->type == TigMsgType::MOUSE){
			TigMsgMouse* mouseMsg = (TigMsgMouse*)msg;
			if (!(mouseMsg->buttonStateFlags & MouseStateFlags::MSF_SCROLLWHEEL_CHANGE))
				return TRUE;
			auto scrollbar = uiManager->GetScrollBar(dlgImpl->mScrollbarId);
			if (!scrollbar->handleMessage)
				return TRUE;
			return scrollbar->handleMessage(scrollbar->widgetId, msg);
		}
		return FALSE;
	};
	mWndId = uiManager->AddWindow(dlgWnd);
	temple::GetRef<LgcyWindow>(0x10BEA2E8) = dlgWnd; // they take some coords from this place... bah. Todo: replace this shit

	// scrollbar
	LgcyScrollBar scrollbar;
	scrollbar.Init(592, 28, 126, mWndId);
	mScrollbarId = uiManager->AddScrollBar(scrollbar, mWndId);
	DialogScrollbarReset();

	// Dialog history button
	textureFuncs.RegisterTexture("art\\interface\\dialog\\head_normal.tga", &mHeadBtnTexNormal);
	textureFuncs.RegisterTexture("art\\interface\\dialog\\head_hovered.tga", &mHeadBtnTexHovered);
	textureFuncs.RegisterTexture("art\\interface\\dialog\\head_pressed.tga", &mHeadBtnTexPressed);
	textureFuncs.RegisterTexture("art\\interface\\dialog\\head_disabled.tga", &mHeadBtnTexDisabled);
	
	mHeadBtnId = dlgWnd.AddChildButton("Head btn", 581, 1, 23, 18,
		[](int id) {
		auto state = uiManager->GetButtonState(id);
		auto texId = dlgImpl->mHeadBtnTexNormal;
		switch (state){
		case LgcyButtonState::Disabled:
			texId = dlgImpl->mHeadBtnTexDisabled;
			break;
		case LgcyButtonState::Down:
			texId = dlgImpl->mHeadBtnTexPressed;
			break;
		case LgcyButtonState::Hovered:
			texId = dlgImpl->mHeadBtnTexHovered;
			break;
		default:
			break;
		}
		static TigRect srcRect(1,1,23,18);
		auto destRect = dlgImpl->mHeadBtnTgtRect;
		UiRenderer:: /*DrawTextureInWidget*/ DrawTexture( /*dlgImpl->mWndId, */texId, destRect, srcRect);
		},
		[](int id, TigMsg*msg) {
			if (!msg->IsWidgetEvent(TigMsgWidgetEvent::MouseReleased)){
				return FALSE;
			}
		if (helpSys.PresentWikiHelpIfActive(19)){
			return TRUE;
		}
		uiDialog->ShowHistory();
		return TRUE;
		});
	mHeadBtnTgtRect = TigRect(dlgWnd.x + 581, dlgWnd.y+1, 23, 18);
	backdrop = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop.img");
	backdropMini1 = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop_mini_1.img");
	backdropMini2 = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop_mini_2.img");
	backdropMini3 = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop_mini_3.img");

	return ResponseWidgetsInit(w, h);
}

void UiDialogImpl::WidgetsFree(){
	ResponseWidgetsFree();
	uiManager->RemoveChildWidget(mHeadBtnId);
	uiManager->RemoveWidget(mScrollbarId);
	uiManager->RemoveWindow(mWndId);
	uiManager->RemoveWidget(mWndId);
	free(backdrop);
	free(backdropMini1);
	free(backdropMini2);
	free(backdropMini3);
}

/* 0x1014D5D0 */
BOOL UiDialogImpl::ResponseWidgetsInit(int w, int h)
{
	
	static LgcyWindow responseWnd(16, h - 221, 611, 139);
	responseWnd.flags = 1;
	responseWnd.render = [](int id){
		auto flags = dlgImpl->mFlags;
		if (flags & UiDialogFlags::DialogHistoryShow){
			auto wnd = uiManager->GetWindow(id);
			if (!wnd) return;
			RenderHooks::RenderImgFile(dlgImpl->mResponseBarImg, wnd->x, wnd->y);
		}
	};
	responseWnd.handleMessage = [](int id, TigMsg*msg) {
		if (msg->type == TigMsgType::MOUSE || msg->type == TigMsgType::WIDGET)
			return TRUE;
		if (msg->type == TigMsgType::CHAR){
			auto chr = msg->arg1 & 0xFF;
			if (chr >= '1' && chr <= '5'){
				int responseIdx = chr - '1';
				if (responseIdx < dlgImpl->mSlot.state.pcLines){
					//uiDialog->PcReplyLineExecute(dlgImpl->mSlot, idx);
					auto result = temple::GetRef<BOOL(__cdecl)(LgcyWidgetId, TigMsg*)>(0x1014D510)(id, msg);
					return result;
				}
				return TRUE;
			}
		}
		return FALSE;
	}; 
	mResponseWndId = uiManager->AddWindow(responseWnd);
	
	const int REPLY_COUNT = 5;
	const int REPLY_TEXT_W = 559;
	const int REPLY_HEIGHT = 23;
	for (auto i = 0; i < REPLY_COUNT; i++) {
		auto y = 9 + 25 * i;
		auto x = 1;
		mSkillIconRects[i] = TigRect(x + responseWnd.x + 2, y + responseWnd.y + 1, 15, 15);
		mResponseRects[i] = TigRect(x + 36, y, REPLY_TEXT_W, REPLY_HEIGHT);
		mResponseNumberRects[i] = TigRect(x + 17, y, 18, REPLY_HEIGHT);
		mSkillsUsedInReply[i] = 0;
		mResponseTexts[i] = nullptr;
		mReplyBtnIds[i] = responseWnd.AddChildButton("Dialog reply btn", 1, y, 594, REPLY_HEIGHT,
			[](int id) {
			auto idx = WidgetIdIndexOf(id, dlgImpl->mReplyBtnIds, 5);
			if (idx <= -1 || idx >= /*REPLY_COUNT*/ 5) return;
			if (!dlgImpl->mResponseTexts[idx]){
				return;
			}
			if (config.enlargeDialogFonts)
				UiRenderer::PushFont(PredefinedFont::PRIORY_12);
			else
				UiRenderer::PushFont(PredefinedFont::ARIAL_10);
			

			auto state = uiManager->GetButtonState(id);
			auto style = &dlgImpl->replyStyleNormal;
			auto skillUsed = dlgImpl->mSkillsUsedInReply[idx];
			if (skillUsed){
				switch (state) {
				case LgcyButtonState::Normal:
				case LgcyButtonState::Disabled:
				case LgcyButtonState::Released:
					style = &dlgImpl->skilledReplyNormal;
					break;
				case LgcyButtonState::Hovered:
					style = &dlgImpl->skilledReplyHovered;
					break;
				case LgcyButtonState::Down:
					style = &dlgImpl->skilledReplyPressed;
					break;
				default:
					style = &dlgImpl->skilledReplyNormal;
					break;
				}
			}
			else{
				switch (state) {
				case LgcyButtonState::Normal:
				case LgcyButtonState::Disabled:
				case LgcyButtonState::Released:
					style = &dlgImpl->replyStyleNormal;
					break;
				case LgcyButtonState::Hovered:
					style = &dlgImpl->replyStyleHovered;
					break;
				case LgcyButtonState::Down:
					style = &dlgImpl->replyStylePressed;
					break;
				default:
					style = &dlgImpl->replyStyleNormal;
					break;
				}
			}

			UiRenderer::DrawTextInWidget(dlgImpl->mResponseWndId, 
				dlgImpl->mResponseTexts[idx],
				dlgImpl->mResponseRects[idx], *style);
			UiRenderer::DrawTextInWidget(dlgImpl->mResponseWndId,
				dlgImpl->mResponseNumbers[idx],
				dlgImpl->mResponseNumberRects[idx], *style);
			UiRenderer::PopFont();

			if (skillUsed){
				auto texId = dlgImpl->mSkillTextures[skillUsed];
				static TigRect srcRect(1, 1, 15, 15);
				auto destRect = dlgImpl->mSkillIconRects[idx];
				UiRenderer::DrawTexture(texId, destRect, srcRect);
			}

		},
			[](int id, TigMsg*msg) {
			if (!msg->IsWidgetEvent(TigMsgWidgetEvent::MouseReleased)) {
				return FALSE;
			}
			if (helpSys.PresentWikiHelpIfActive(19)) {
				return TRUE;
			}

			auto idx = WidgetIdIndexOf(id, dlgImpl->mReplyBtnIds, /*REPLY_COUNT*/5);
			if (idx <= -1 || idx >= 5 /*REPLY_COUNT*/ || idx >= dlgImpl->mSlot.state.pcLines)
				return TRUE;

			auto result = temple::GetRef<BOOL(__cdecl)(int, TigMsg*)>(0x1014D560)(id, msg);
			//uiDialog->PcReplyLineExecute(dlgImpl->mSlot, idx);
			return result;
		});
	}

	mResponseBarImg = uiAssets->LoadImg("art\\interface\\dialog\\response_split_bar.img");
	for (auto i=0; i<6; ++i){
		const char ** iconFilename = temple::GetPointer<const char*>(0x102FA978);
		if (iconFilename[i]){
			textureFuncs.RegisterTexture(iconFilename[i], &mSkillTextures[i]);
		}
	}


	return TRUE;
}

void UiDialogImpl::ResponseWidgetsFree(){
	for (auto i=0; i < 5; i++){
		uiManager->RemoveChildWidget(mReplyBtnIds[i]);
	}
	uiManager->RemoveWindow(mResponseWndId);
	uiManager->RemoveWidget(mResponseWndId);
	free(mResponseBarImg);
}

BOOL UiDialogImpl::DummyWndInit(int w, int h)
{
	// This is a dummy window. Its purpose is to catch mouse clicks :)

	static LgcyWindow dummyWnd(0, 0, w, h);
	dummyWnd.flags = 1;
	dummyWnd.render = [](int widId){
		// this is a dummy window
	};
	dummyWnd.handleMessage = [](int widId, TigMsg* msg) {

		if (msg->type == TigMsgType::KEYSTATECHANGE) {
			return msg->arg1 == 1? TRUE: FALSE;
		}
		if (msg->type == TigMsgType::CHAR) {
			return FALSE;
		}
		return TRUE;
	};
	mDummyWndId = uiManager->AddWindow(dummyWnd);

	return TRUE;
}


#pragma region UI Styles
ColorRect UiDialogStyles::pureYellow = ColorRect(XMCOLOR(0xFFffFF00));
ColorRect UiDialogStyles::darkGray = ColorRect(XMCOLOR(0xFF666666));

ColorRect UiDialogStyles::pureGreen = ColorRect(XMCOLOR(0xFF00FF00));
ColorRect UiDialogStyles::darkGreen = ColorRect(XMCOLOR(0xFF99FF99));
ColorRect UiDialogStyles::pureWhite = ColorRect(XMCOLOR(0xFFffFFff));

ColorRect UiDialogStyles::brightYellow  = ColorRect(XMCOLOR(0xFFE4CE3E));
ColorRect UiDialogStyles::brightYellow2 = ColorRect(XMCOLOR(0xFFFEFE19));
ColorRect UiDialogStyles::brightYellow3 = ColorRect(XMCOLOR(0xFFFEFF00));
#pragma endregion styles