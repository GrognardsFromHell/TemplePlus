
#include "stdafx.h"
#include "ui_dialog.h"
#include <temple/dll.h>
#include "ui.h"
#include "tig/tig_font.h"
#include "tig/tig_msg.h"
#include "gamesystems/d20/d20_help.h"
#include "widgets/widgets.h"
#include "ui_systems.h"
#include "ui_ingame.h"
#include "../dialog.h"
#include "ui/ui_legacysystems.h"
#include "tig/tig_texture.h"
#include "ui_render.h"
#include "graphics/render_hooks.h"

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
public:
	UiDialogImpl(const UiSystemConf &config);

	void DialogScrollbarReset();
protected:
	BOOL WidgetsInit(int w, int h);
	BOOL ResponseWidgetsInit(int w, int h); // todo


	DialogSlot & mSlot = temple::GetRef<DialogSlot>(0x10BEA918);

	int & mIsActive = temple::GetRef<int>(0x10BEC348);
	uint32_t& mFlags = temple::GetRef<uint32_t>(0x10BEA5F4);
	int &lineCount = temple::GetRef<int>(0x10BEC194);
	DialogMini *& dlgLineList = temple::GetRef<DialogMini*>(0x10BEC1A4);
	

	LgcyWidgetId& mWndId = temple::GetRef<LgcyWidgetId>(0x10BEA2E4);
	LgcyWidgetId &responseWndId = temple::GetRef< LgcyWidgetId>(0x10BEC204);
	LgcyWidgetId &wnd2Id = temple::GetRef<LgcyWidgetId>(0x10BEC198);
	LgcyWidgetId &mScrollbarId = temple::GetRef<LgcyWidgetId>(0x10BEC19C);
	LgcyWidgetId &mHeadBtnId = temple::GetRef<LgcyWidgetId>(0x10BEC210);

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
	
	TigTextStyle yellowStyle = TigTextStyle::standardWhite;
	TigTextStyle darkGrayStyle = TigTextStyle::standardWhite;

	int & historyWndX = temple::GetRef<int>(0x10BEA2E0);
	int & historyWndY = temple::GetRef<int>(0x10BE9FF0);
	int & textMinY = temple::GetRef<int>(0x10BEC20C);

	TigRect &mHeadBtnTgtRect = temple::GetRef<TigRect>(0x10BEC334);
	

	// std::unique_ptr<WidgetContainer> mWnd;

	void ShowDialogWidgets();



};


//*****************************************************************************
//* Dlg-UI
//*****************************************************************************

UiDlg::UiDlg(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1014dd40);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Dlg-UI");
	}
	mImpl = std::make_unique<UiDialogImpl>(config);
	Expects(!uiDialog);
	uiDialog = this;
	dlgImpl = mImpl.get();
}
UiDlg::~UiDlg() {
	auto shutdown = temple::GetPointer<void()>(0x1014ccc0);
	shutdown();
	uiDialog = nullptr;
}
void UiDlg::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x1014de30);
	resize(&resizeArg);

	mImpl->WidgetsInit(resizeArg.rect1.width, resizeArg.rect1.height);
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
		static TigTextStyle yellowStyle;
		static TigTextStyle darkGrayStyle;
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
	
	uiManager->SetHidden(wnd2Id, false);
	uiManager->BringToFront(wnd2Id);
	// temple::GetRef<void(__cdecl)()>(0x1014C340)();
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

UiDialogImpl::UiDialogImpl(const UiSystemConf & config)
{
	yellowStyle.colors2 = yellowStyle.colors4 = yellowStyle.textColor = yellowStyle.shadowColor = &UiDialogStyles::pureYellow;
	darkGrayStyle.colors2 = darkGrayStyle.colors4 = darkGrayStyle.textColor = darkGrayStyle.shadowColor = &UiDialogStyles::darkGray;
	yellowStyle.tracking = darkGrayStyle.tracking = 3;
	yellowStyle.field2c = darkGrayStyle.field2c = -1;

	WidgetsInit(config.width, config.height);
}

/* 0x1014BDF0 */
void UiDialogImpl::DialogScrollbarReset()
{
	scrollbarYmax = lineCount - 1;
	scrollbarY = lineCount - 1;
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
	
	/*mWnd = make_unique<WidgetContainer>(611, 292);
	mWnd->SetSize({ 611, 292 });
	auto wndX = 9;
	auto wndY = h - 374 ;
	mWnd->SetPos(wndX, wndY);*/

	static LgcyWindow dlgWnd(9, h - 374, 611, 292);
	dlgWnd.flags = 1;
	dlgWnd.render = [](int widId){
		UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		const int LINE_WIDTH = 550;

		auto flags = dlgImpl->mFlags;
		auto wnd = uiManager->GetWindow(widId);
		auto responseWnd = uiManager->GetWindow(dlgImpl->responseWndId);
		
		// Render background image
		auto x = wnd->x, y = wnd->y + 18;
		auto wndBackdrop = dlgImpl->backdrop;
		if ( !( flags & UiDialogFlags::DialogHistoryShow) ){
			x = dlgImpl->historyWndX; y = dlgImpl->historyWndY;
			wndBackdrop = dlgImpl->backdropMini;
		}
		RenderHooks::RenderImgFile(wndBackdrop, x, y);

		TigRect textRect(14, 
			dlgImpl->mIsActive ? (wnd->height - 4 - responseWnd->height) : (wnd->height - 4), 
			LINE_WIDTH, 0);
		
		auto responses = dlgImpl->dlgLineList;
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
			textRect.y -= textSize.height;
			if (textRect.y < dlgImpl->textMinY)
				break;
			auto isPcLine = (responses->flags & 1) != 0;

			UiRenderer::DrawTextInWidget(widId, responses->lineText, textRect, 
				isPcLine ? dlgImpl->darkGrayStyle : dlgImpl->yellowStyle);
			if (! (flags & UiDialogFlags::DialogHistoryShow) ){
				break;
			}
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
	
	// scrollbar
	static LgcyScrollBar scrollbar;
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
	
	backdrop = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop.img");
	backdropMini1 = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop_mini_1.img");
	backdropMini2 = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop_mini_2.img");
	backdropMini3 = uiAssets->LoadImg("art\\interface\\dialog\\dialog_backdrop_mini_3.img");

	return ResponseWidgetsInit(w, h);
}

/* 0x1014D5D0 */
BOOL UiDialogImpl::ResponseWidgetsInit(int w, int h)
{
	// TODO
	return TRUE;
}


#pragma region UI Styles
ColorRect UiDialogStyles::pureYellow = ColorRect(XMCOLOR(0xFFffFF00));
ColorRect UiDialogStyles::darkGray = ColorRect(XMCOLOR(0xFF666666));
#pragma endregion styles